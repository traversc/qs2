# helper script for save file benchmarks
suppressPackageStartupMessages({
  library(dplyr, quietly=TRUE)
  library(data.table, quietly=TRUE)
  library(stringplus, quietly=TRUE)
})

set_string_ops("&", "|")

args <- commandArgs(trailingOnly = TRUE)
algo <- args[1]
nthreads <- args[2] %>% as.numeric
cl <- args[3] %>% as.numeric
input_path <- args[4] # must be full path not relative; must be csv.gz read in by data.table
output_path <- args[5] # save data to here
results_path <- args[6] # write results to here

now <- function() assign(".time", Sys.time(), envir = globalenv())
later <- function() { as.numeric(Sys.time() - get(".time", envir = globalenv()), units = "secs") }

DATA <- data.table::fread(input_path, sep = ",", data.table=FALSE)

if(algo == "qs-legacy") {
  suppressPackageStartupMessages( library(qs, quietly=TRUE) )
  now()
  qs::qsave(DATA, file = output_path, preset = "custom", algorithm = "zstd", compress_level = cl, nthreads = nthreads, check_hash = FALSE)
  save_time <- later()
} else if(algo == "qs2") {
  suppressPackageStartupMessages( library(qs2, quietly=TRUE) )
  now()
  qs2::qs_save(DATA, file = output_path, compress_level = cl, nthreads = nthreads)
  save_time <- later()
} else if(algo == "qdata") {
  suppressPackageStartupMessages( library(qs2, quietly=TRUE) )
  now()
  qs2::qd_save(DATA, file = output_path, compress_level = cl, nthreads = nthreads)
  save_time <- later()
} else if(algo == "fst") {
  suppressPackageStartupMessages( library(fst, quietly=TRUE) )
  fst::threads_fst(nr_of_threads = nthreads)
  now()
  fst::write_fst(DATA, path = output_path, compress = cl)
  save_time <- later()
} else if(algo == "parquet") {
  suppressPackageStartupMessages( library(arrow, quietly=TRUE) )
  arrow::set_cpu_count(num_threads = nthreads)
  now()
  arrow::write_parquet(DATA, sink = output_path, compression = "zstd", compression_level = cl)
  save_time <- later()
}

writeLines(text = c(save_time) %>% as.character, con = results_path)
