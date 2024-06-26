qs2
================

[![R-CMD-check](https://github.com/traversc/qs2/workflows/R-CMD-check/badge.svg)](https://github.com/traversc/qs2/actions)
[![CRAN-Status-Badge](http://www.r-pkg.org/badges/version/qs2)](https://cran.r-project.org/package=qs2)
[![CRAN-Downloads-Badge](https://cranlogs.r-pkg.org/badges/qs2)](https://cran.r-project.org/package=qs2)
[![CRAN-Downloads-Total-Badge](https://cranlogs.r-pkg.org/badges/grand-total/qs2)](https://cran.r-project.org/package=qs2)

*qs2: a framework for efficient serialization*

`qs2` is the successor to the `qs` package. The goal is to have cutting
edge performance for saving and loading objects in R.

The package introduces a new format for saving objects to disk.

``` r
qs_save(data, "myfile.qs2")
data <- qs_read("myfile.qs2")
```

The `qs2` format directly uses R serialization and is a drop in
replacement for `saveRDS`. If you are familiar with the `qs` package,
the benefits of `qs2` are the same. Compared to `saveRDS` it can be an
order of magnitude faster while having similar levels of compression.

Use the file extension `qs2` to distinguish it from the original `qs`
package. It is not compatible with the original `qs` format.

# Installation

``` r
install.packages("qs2")
```

To enable multithreading on Mac or Linux, compile from source. It is
enabled by default on Windows.

``` r
remotes::install_cran("qs2", type = "source", configure.args = " --with-TBB --with-simd=AVX2")
```

Multithreading uses `Intel Thread Building Blocks` via the
`RcppParallel` package.

# The qdata format

This package also introduces the `qdata` format which has its own layout
and works with only R data types (vectors, lists, data frames,
matrices). It will replace internal types (functions, promises, external
pointers, environments, objects) with NULL.

This has slightly better performance but is not general. If you have
clean data and understand the caveats, you may want to use it. The
eventual goal of `qdata` is to also have interoperability with other
languages, particularly `Python`.

``` r
qd_save(data, "myfile.qs2")
data <- qd_read("myfile.qs2")
```

# Converting qs2 to RDS

Because the `qs2` format directly uses R serialization you can convert
it to RDS and vice versa.

``` r
file_qs2 <- tempfile(fileext = ".qs2")
file_rds <- tempfile(fileext = ".RDS")
x <- runif(1e6)

# save `x` with qs_save
qs_save(x, file_qs2)

# convert the file to RDS
qs_to_rds(input_file = file_qs2, output_file = file_rds)

# read `x` back in with `readRDS`
xrds <- readRDS(file_rds)
stopifnot(identical(x, xrds))
```

## Benchmarks

A summary table across 3 datasets is presented below. You may draw your
own conclusions or run your own benchmarks.

| Algorithm       | nthreads | Save Time | Read Time | Compression |
| --------------- | -------- | --------- | --------- | ----------- |
| qs2             | 1        | 4.41      | 3.32      | 2.56        |
| qdata           | 1        | 4.08      | 3.11      | 2.58        |
| qs-legacy       | 1        | 4.24      | 3.17      | 2.51        |
| saveRDS         | 1        | 27.21     | 6.84      | 2.32        |
| base::serialize | 1        | 10.17     | 11.51     | 1.10        |
| fst             | 1        | 2.76      | 3.80      | 1.59        |
| parquet         | 1        | 4.99      | 3.47      | 2.33        |
| qs2             | 4        | 1.36      | 2.73      | 2.56        |
| qdata           | 4        | 1.28      | 2.51      | 2.58        |
| qs              | 4        | 1.84      | 2.64      | 2.51        |
| fst             | 4        | 2.66      | 3.76      | 1.59        |
| parquet         | 4        | 4.93      | 3.08      | 2.33        |

**Notes on running each algorithm**

  - `qs2`, `qdata` and `qs` used `compress_level = 5`
  - `parquet` (from the `arrow` package) were run with `compression =
    zstd` and `compress_level = 5`
  - `fst` used `compress_level = 55`
  - `base::serialize` was run with `ascii = FALSE` and `xdr = FALSE`

**Datasets**

  - `enwik8` the first 1E8 lines of English Wikipedia
  - `gaia` galactic coordinates and color of 7.2 million stars
  - `tcell` the genomic sequencing of a COVID patient’s immune system

These datasets are openly licensed and represent a combination of
numeric and textual data across multiple domains.

See `inst/benchmarks` on Github for comprehensive comparisons,
reproducibility and data sources.
