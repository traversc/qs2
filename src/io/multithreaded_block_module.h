#ifndef _QS2_MULTITHREADED_BLOCK_MODULE_H
#define _QS2_MULTITHREADED_BLOCK_MODULE_H

#include "io/io_common.h"

#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_vector.h>
#include <tbb/flow_graph.h>
#include <tbb/enumerable_thread_specific.h>

struct OrderedBlock {
    std::shared_ptr<char[]> block;
    uint64_t blocksize;
    uint64_t blocknumber;
    OrderedBlock(std::shared_ptr<char[]> block, uint64_t blocksize, uint64_t blocknumber) : 
    block(block), blocksize(blocksize), blocknumber(blocknumber) {}
    OrderedBlock() : block(), blocksize(0), blocknumber(0) {}
    bool is_already_copied() const {
        return block.get() == nullptr;
    }
};

struct OrderedPtr {
    const char * block;
    uint64_t blocknumber;
    OrderedPtr(const char * block, uint64_t blocknumber) : 
    block(block), blocknumber(blocknumber) {}
    OrderedPtr() : block(), blocknumber(0) {}
};

// sequencer requires copy constructor for message, which means using shared_ptr
// would be better if we could use unique_ptr
// nthreads must be >= 2
template <class stream_writer, class compressor, class hasher, ErrorType E>
struct BlockCompressWriterMT {
    // template class objects
    stream_writer & myFile;
    tbb::enumerable_thread_specific<compressor> cp;
    hasher hp; // must be serial
    int compress_level;
    
    // intermediate data objects
    tbb::concurrent_queue<std::shared_ptr<char[]>> available_blocks;
    tbb::concurrent_queue<std::shared_ptr<char[]>> available_zblocks;

    // current data block
    std::shared_ptr<char[]> current_block;
    uint64_t current_blocksize;
    uint64_t current_blocknumber;

    // flow graph
    tbb::task_group_context tgc;
    tbb::flow::graph myGraph;
    tbb::flow::function_node<OrderedBlock, OrderedBlock> compressor_node;
    tbb::flow::function_node<OrderedPtr, OrderedBlock> compressor_node_direct;
    tbb::flow::sequencer_node<OrderedBlock> sequencer_node;
    tbb::flow::function_node<OrderedBlock, int, tbb::flow::rejecting> writer_node;
    BlockCompressWriterMT(stream_writer & f, int cl) :
    // template class objects
    myFile(f),
    cp(), // each thread specific compressor should be default constructed
    hp(),
    compress_level(cl),

    // intermediate data objects
    available_blocks(),
    available_zblocks(),

    // current data block
    current_block(MAKE_SHARED_BLOCK(MAX_BLOCKSIZE)),
    current_blocksize(0),
    current_blocknumber(0),

    // flow graph
    tgc(),
    myGraph(this->tgc),
    compressor_node(this->myGraph, tbb::flow::unlimited, 
        [this](OrderedBlock block) {
                        // get zblock from available_zblocks
            OrderedBlock zblock;
            if(!available_zblocks.try_pop(zblock.block)) {
                zblock.block = MAKE_SHARED_BLOCK_ASSIGNMENT(MAX_ZBLOCKSIZE);
            }
            // do thread local compression
            typename tbb::enumerable_thread_specific<compressor>::reference cp_local = cp.local();
            zblock.blocksize = cp_local.compress(zblock.block.get(), MAX_ZBLOCKSIZE, 
                                                 block.block.get(), block.blocksize,
                                                 compress_level);
            zblock.blocknumber = block.blocknumber;
            
            // return input block to available_blocks
            available_blocks.push(block.block);
            return zblock;
        }),
    compressor_node_direct(this->myGraph, tbb::flow::unlimited, 
        [this](OrderedPtr ptr) {
                        // get zblock from available_zblocks
            OrderedBlock zblock;
            if(!available_zblocks.try_pop(zblock.block)) {
                zblock.block = MAKE_SHARED_BLOCK_ASSIGNMENT(MAX_ZBLOCKSIZE);
            }
            // do thread local compression
            typename tbb::enumerable_thread_specific<compressor>::reference cp_local = cp.local();
            zblock.blocksize = cp_local.compress(zblock.block.get(), MAX_ZBLOCKSIZE, 
                                                 ptr.block, MAX_BLOCKSIZE,
                                                 compress_level);
            zblock.blocknumber = ptr.blocknumber;
            return zblock;
        }),
    sequencer_node(this->myGraph,
        [](OrderedBlock zblock) {
            return zblock.blocknumber;
        }),
    writer_node(this->myGraph, tbb::flow::serial,
        [this](OrderedBlock zblock) {
            write_and_update(static_cast<uint32_t>(zblock.blocksize));
            write_and_update(zblock.block.get(), zblock.blocksize & (~BLOCK_METADATA));

            // return input zblock to available_zblocks
            available_zblocks.push(zblock.block);

            // return 0 to indicate success
            return 0;
        }) {

        // pre-populate available blocks and zblocks queues
        // for(int i = 0; i < nthreads-1; ++i) {
        //     available_blocks.push(MAKE_SHARED_BLOCK_ASSIGNMENT(MAX_BLOCKSIZE));
        //     available_zblocks.push(MAKE_SHARED_BLOCK_ASSIGNMENT(MAX_ZBLOCKSIZE));
        // }
        
        // connect computation graph
        tbb::flow::make_edge(compressor_node, sequencer_node);
        tbb::flow::make_edge(compressor_node_direct, sequencer_node);
        tbb::flow::make_edge(sequencer_node, writer_node);
    }
    private:
    void write_and_update(const char * const inbuffer, const uint64_t len) {
        myFile.write(inbuffer, len);
        hp.update(inbuffer, len);
    }
    template <typename POD>
    void write_and_update(const POD value) {
        myFile.writeInteger(value);
        hp.update(value);
    }
    void flush() {
        if(current_blocksize > 0) {
            compressor_node.try_put(OrderedBlock(current_block, current_blocksize, current_blocknumber));
            current_blocknumber++;
            current_blocksize = 0;
            // replace current_block with available block
            if(!available_blocks.try_pop(current_block)) {
                current_block = MAKE_SHARED_BLOCK_ASSIGNMENT(MAX_BLOCKSIZE);
            }
        }
    }
    
    public:
    uint64_t finish() {
        flush();
        myGraph.wait_for_all();
        return hp.digest();
    }
    void cleanup() {
        if(! tgc.is_group_execution_cancelled()) {
            tgc.cancel_group_execution();
        }
        myGraph.wait_for_all();
    }
    void cleanup_and_throw(std::string msg) {
        cleanup();
        throw_error<E>(msg.c_str());
    }

    void push_data(const char * const inbuffer, const uint64_t len) {
        uint64_t current_pointer_consumed = 0;
        while(current_pointer_consumed < len) {
            if(current_blocksize >= MAX_BLOCKSIZE) { flush(); }
            
            if(current_blocksize == 0 && len - current_pointer_consumed >= MAX_BLOCKSIZE) {
                compressor_node_direct.try_put(OrderedPtr(inbuffer + current_pointer_consumed, current_blocknumber));
                current_blocknumber++;
                current_pointer_consumed += MAX_BLOCKSIZE;
                current_blocksize = 0;
            } else {
                uint64_t remaining_pointer_available = len - current_pointer_consumed;
                uint64_t add_length = remaining_pointer_available < (MAX_BLOCKSIZE - current_blocksize) ? remaining_pointer_available : MAX_BLOCKSIZE-current_blocksize;
                std::memcpy(current_block.get() + current_blocksize, inbuffer + current_pointer_consumed, add_length);
                current_blocksize += add_length;
                current_pointer_consumed += add_length;
            }
        }
    }

    // same as ST
    template<typename POD> void push_pod(const POD pod) {
        if(current_blocksize > MIN_BLOCKSIZE) { flush(); }
        const char * ptr = reinterpret_cast<const char*>(&pod);
        std::memcpy(current_block.get() + current_blocksize, ptr, sizeof(POD));
        current_blocksize += sizeof(POD);
    }

    // same as ST
    template<typename POD> void push_pod_contiguous(const POD pod) {
        const char * ptr = reinterpret_cast<const char*>(&pod);
        memcpy(current_block.get() + current_blocksize, ptr, sizeof(POD));
        current_blocksize += sizeof(POD);
    }

    // runtime determination of contiguous
    template<typename POD> void push_pod(const POD pod, const bool contiguous) {
        if(contiguous) { push_pod_contiguous(pod); } else { push_pod(pod); }
    }
};

template <class stream_reader, class decompressor, ErrorType E> 
struct BlockCompressReaderMT {
    // template class objects
    stream_reader & myFile;
    tbb::enumerable_thread_specific<decompressor> dp;
    decompressor dp_main; // decompressor for main thread to moonlight as compressor_node

    // intermediate data objects
    tbb::concurrent_queue<std::shared_ptr<char[]>> available_zblocks;
    tbb::concurrent_queue<std::shared_ptr<char[]>> available_blocks;
    tbb::concurrent_queue<OrderedBlock> ordered_zblocks;

    // current data block
    std::shared_ptr<char[]> current_block;
    uint64_t current_blocksize;
    uint64_t data_offset;

    // global control objects
    std::atomic<bool> end_of_file; // set after blocks_to_process and there are insufficient bytes from reader stream for another block
    std::atomic<uint64_t> blocks_to_process; // how many blocks have been read in, incremented by reader_node
    tbb::concurrent_unordered_map<uint64_t, char *> block_assignments; // used to decompress directly into R buffer
    uint64_t blocks_processed; // only written and read by main thread

    // flow graph
    tbb::task_group_context tgc;
    tbb::flow::graph myGraph;
    tbb::flow::source_node<tbb::flow::continue_msg> reader_node;
    tbb::flow::function_node<tbb::flow::continue_msg, int> decompressor_node;
    tbb::flow::sequencer_node<OrderedBlock> sequencer_node;

    BlockCompressReaderMT(stream_reader & f) :
    // template class objects
    myFile(f),
    dp(),
    dp_main(),

    // intermediate data objects
    available_zblocks(),
    available_blocks(),
    ordered_zblocks(),

    // current data block
    current_block(MAKE_SHARED_BLOCK(MAX_BLOCKSIZE)),
    current_blocksize(0), 
    data_offset(0),

    // global control objects
    end_of_file(false),
    blocks_to_process(0),
    blocks_processed(0),
    
    // flow graph
    tgc(),
    myGraph(this->tgc),
    reader_node(this->myGraph,
    [this](tbb::flow::continue_msg & cont) {
        // read size of next zblock. if insufficient bytes read into uint32_t, end of file
        uint32_t zsize;
        bool ok = this->myFile.readInteger(zsize);
        if(!ok) {
            end_of_file.store(true);
            return false;
        }

        // get zblock from available_zblocks or make new
        OrderedBlock zblock;
        if(!available_zblocks.try_pop(zblock.block)) {
            zblock.block = MAKE_SHARED_BLOCK_ASSIGNMENT(MAX_ZBLOCKSIZE);
        }

        // read zblock. if bytes_read is less than zsize, end of file
        size_t bytes_read = this->myFile.read(zblock.block.get(), zsize & (~BLOCK_METADATA));
        if(bytes_read != (zsize & (~BLOCK_METADATA))) {
            end_of_file.store(true);
            return false;
        }

        // set zblock size and block number and push to ordered_zblocks queue
        // Also increment blocks_to_process BEFORE zblock is added to ordered_zblocks
        zblock.blocksize = zsize;
        zblock.blocknumber = blocks_to_process.fetch_add(1);
        this->ordered_zblocks.push(zblock);

        // return continue token to decompressor node
        cont = tbb::flow::continue_msg{};
        return true;
    }, false),
    decompressor_node(this->myGraph, tbb::flow::unlimited,
    [this](tbb::flow::continue_msg cont) {
        typename tbb::enumerable_thread_specific<decompressor>::reference dp_local = dp.local();
        // decompress routine manually try_put's a block to sequencer_node, return value is just a dummy 0 integer and does nothing
        return this->decompressor_node_routine(dp_local);
    }),
    sequencer_node(this->myGraph, 
    [this](OrderedBlock block) {
        TOUT("sequencer ", block.blocknumber, " with size ", block.blocksize);
        // check if block has block assignment and is not already copy, and if so copy here
        auto it = block_assignments.find(block.blocknumber);
        if(it != block_assignments.end() && !block.is_already_copied()) {
            TOUT("direct copy ", block.blocknumber, " with size ", block.blocksize);
            // blocksize should always be MAX_BLOCKSIZE here if we are copying directly
            std::memcpy(it->second, block.block.get(), block.blocksize);
            // replace block with null pointer to indicate it has been copied and return block to available blocks
            std::shared_ptr<char[]> swap_block;
            std::swap(swap_block, block.block);
            available_blocks.push(std::move(swap_block));
        }
        return block.blocknumber;
    }) {
        // pre-populate available blocks and zblocks queues
        // for(int i = 0; i < nthreads-1; ++i) {
        //     available_blocks.push(MAKE_SHARED_BLOCK_ASSIGNMENT(MAX_BLOCKSIZE));
        //     available_zblocks.push(MAKE_SHARED_BLOCK_ASSIGNMENT(MAX_ZBLOCKSIZE));
        // }
        
        // connect computation graph
        tbb::flow::make_edge(reader_node, decompressor_node);
        reader_node.activate();
    }
    private:
    int decompressor_node_routine(decompressor & dp_local) {
        // try_pop from ordered_zblocks queue
        OrderedBlock zblock;
        if(!ordered_zblocks.try_pop(zblock)) {
            return 0; // no available zblock, could be stolen by main thread
        }
        TOUT("decompressor ", zblock.blocknumber, " with size ", zblock.blocksize);
        
        OrderedBlock block;
        // check for block assignment
        auto it = block_assignments.find(zblock.blocknumber);
        if(it != block_assignments.end()) {
            // blocksize should always be MAX_BLOCKSIZE here if we are copying directly
            block.blocksize = dp_local.decompress(it->second, MAX_BLOCKSIZE, zblock.block.get(), zblock.blocksize);
            // auto blockhash = XXH64(it->second, block.blocksize, 0);
            // TOUT("direct decompress ", zblock.blocknumber, " to address ", (void*)it->second, " from address ", (void*)zblock.block.get(), " with hash ", blockhash);
        } else {
            // get available block and decompress to block
            if(!available_blocks.try_pop(block.block)) {
                block.block = MAKE_SHARED_BLOCK_ASSIGNMENT(MAX_BLOCKSIZE);
            }
            block.blocksize = dp_local.decompress(block.block.get(), MAX_BLOCKSIZE, zblock.block.get(), zblock.blocksize);
            // auto blockhash = XXH64(block.block.get(), block.blocksize, 0);
            // TOUT("decompress ", zblock.blocknumber, " to new block with address ", (void*)block.block.get(), " from address ", (void*)zblock.block.get(), " with hash ", blockhash);
        }
        // check for decompression error
        if(decompressor::is_error(block.blocksize)) {
            // don't throw error within graph
            // main thread will check for cancellelation and throw
            tgc.cancel_group_execution();
            return -1;
        }

        block.blocknumber = zblock.blocknumber;

        // return zblock to available_zblocks queue
        available_zblocks.push(zblock.block);

        // push to sequencer queue
        this->sequencer_node.try_put(block);
        return 0;
    }
    // returns is_already_copied
    bool get_new_block() {
        OrderedBlock block;
        while( true ) {
            // try_get from sequencer queue until a new block is available
            if( sequencer_node.try_get(block) ) {
                TOUT("get_new_block ", block.blocknumber, " with size ", block.blocksize, " address ", (void*)block.block.get());
                // check if already copied, if so block is nullptr
                // if its already copied then we do not need to set data_offset to 0 since it was already used
                bool already_copied = block.is_already_copied();
                if(already_copied) {
                    // put old block back in available_blocks
                    std::swap(current_block, block.block);
                    available_blocks.push(block.block);
                    // replace previous block with new block and increment blocks_processed
                }
                // auto blockhash = XXH64(current_block.get(), current_blocksize, 0);
                // TOUT("get_new_block current block ", blocks_processed, " with size ", current_blocksize, " address ", (void*)current_block.get(), " already copied " , already_copied, " with hash ", blockhash);
                current_blocksize = block.blocksize;
                data_offset = 0;
                blocks_processed++;
                return already_copied;
            } else {
                // moonlight by trying to run decompression node routine
                decompressor_node_routine(dp_main);
            }

            // check if end_of_file and blocks_used >= blocks_to_process
            // which means the file unexpectedly ended, because we are still expecting blocks
            if(end_of_file && blocks_processed >= blocks_to_process) {
                cleanup_and_throw("Unexpected end of file");
            }
            if(tgc.is_group_execution_cancelled()) {
                cleanup_and_throw("File read / decompression error");
            }
        }
    }
    public:

    void finish() {
        myGraph.wait_for_all();
    }
    void cleanup() {
        if(! tgc.is_group_execution_cancelled()) {
            tgc.cancel_group_execution();
        }
        myGraph.wait_for_all();
    }
    void cleanup_and_throw(std::string msg) {
        cleanup();
        throw_error<E>(msg.c_str());
    }
    void do_block_assignments(uint64_t len, uint64_t bytes_accounted, uint64_t blocks_processed, char * outbuffer) {
        while(bytes_accounted < len) {
            if(len - bytes_accounted >= MAX_BLOCKSIZE) {
                // IMPORTANT: do not use operator[]
                // it returns a reference to a new value with default value (nullptr)
                // meaning assigning the assignment (outbuffer + bytes_accounted) is NOT atomic
                // block_assignments[blocks_processed] = outbuffer + bytes_accounted;
                TOUT("do_block_assignments ", blocks_processed, " to address ", (void*)(outbuffer + bytes_accounted));
                block_assignments.insert(std::pair<uint64_t, char *>(blocks_processed, outbuffer + bytes_accounted));
                bytes_accounted += MAX_BLOCKSIZE;
                blocks_processed++;
            } else {
                break; // no more full blocks can be memcpy'd
            }
        }
    }
    void get_data(char * outbuffer, const uint64_t len) {
        TOUT("get_data ", len, " with current_blocksize ", current_blocksize, " data_offset ", data_offset, " address ", (void*)current_block.get());
        if(current_blocksize - data_offset >= len) {
            std::memcpy(outbuffer, current_block.get()+data_offset, len);
            data_offset += len;
        } else {
            uint64_t bytes_accounted = current_blocksize - data_offset;
            std::memcpy(outbuffer, current_block.get()+data_offset, bytes_accounted);
            // do block assignments first so worker threads can decompress directly into R buffer
            do_block_assignments(len, bytes_accounted, blocks_processed, outbuffer);
            while(bytes_accounted < len) {
                if(len - bytes_accounted >= MAX_BLOCKSIZE) {
                    bool already_copied = get_new_block();
                    // check if block_already_copied (nullptr) and memcpy if it has not been
                    if(!already_copied) {
                        TOUT("memcpy full block blocknumber ", blocks_processed-1, " len ", len, " bytes_accounted ", bytes_accounted, " address ", (void*)(outbuffer + bytes_accounted));
                        std::memcpy(outbuffer + bytes_accounted, current_block.get(), MAX_BLOCKSIZE);
                    }
                    bytes_accounted += MAX_BLOCKSIZE;
                    data_offset = MAX_BLOCKSIZE;
                } else {
                    get_new_block();
                    // if not enough bytes in block then something went wrong, throw error
                    if(current_blocksize < len - bytes_accounted) {
                        cleanup_and_throw("Corrupted block data");
                    }
                    std::memcpy(outbuffer + bytes_accounted, current_block.get(), len - bytes_accounted);
                    TOUT("memcpy partial block len ", len, " bytes_accounted ", bytes_accounted, " address ", (void*)current_block.get());
                    data_offset = len - bytes_accounted;
                    bytes_accounted += data_offset;
                }
            }
        }
    }

    // Same as ST
    template<typename POD> POD get_pod() {
        TOUT("get_pod ", sizeof(POD), " with current_blocksize ", current_blocksize, " data_offset ", data_offset, " address ", (void*)current_block.get());
        if(current_blocksize == data_offset) {
            get_new_block();
        }
        if(current_blocksize - data_offset < sizeof(POD)) {
            cleanup_and_throw("Corrupted block data");
        }
        POD pod;
        memcpy(&pod, current_block.get()+data_offset, sizeof(POD));
        data_offset += sizeof(POD);
        TOUT("pod value ", (int64_t)pod);
        return pod;
    }

    // same as ST
    // unconditionally read from block without checking remaining length
    template<typename POD> POD get_pod_contiguous() {
        TOUT("get_pod ", sizeof(POD), " with current_blocksize ", current_blocksize, " data_offset ", data_offset, " address ", (void*)current_block.get());
        if(current_blocksize - data_offset < sizeof(POD)) {
            cleanup_and_throw("Corrupted block data");
        }
        POD pod;
        memcpy(&pod, current_block.get()+data_offset, sizeof(POD));
        data_offset += sizeof(POD);
        TOUT("pod value ", (int64_t)pod);
        return pod;
    }
};

#endif
