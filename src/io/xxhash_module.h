#ifndef _QS2_XXHASH_MODULE_H
#define _QS2_XXHASH_MODULE_H

#include "io/io_common.h"

struct xxHashEnv {
    XXH3_state_t* state;
    xxHashEnv() : state(XXH3_createState()) {
        XXH3_64bits_reset(state);
    }
    ~xxHashEnv() {
        XXH3_freeState(state);
    }
    void reset() {
        XXH3_64bits_reset(state);
    }
    void update(const void * const buf, const uint64_t length) {
        XXH3_64bits_update(state, buf, length);
    }
    template <typename POD>
    void update(const POD & value) {
        XXH3_64bits_update(state, &value, sizeof(POD));
    }
    uint64_t digest() {
        uint64_t hash = XXH3_64bits_digest(state);
        // there is a 1/2^64 chance that the hash is zero
        // but we use it as a sentinel value so we need to make sure it's not zero
        if(hash == 0) {
            hash = 1;
        }
        return hash;
    }
};

// do nothing and return zero
struct noHashEnv {
    noHashEnv() {}
    void reset() {}
    void update(const void * const buf, const uint64_t length) {
        // do nothing
    }
    template <typename POD>
    void update(const POD & value) {
        // do nothing
    }
    uint64_t digest() {
        return 0;
    }
};

#endif
