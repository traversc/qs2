// Generated by using Rcpp::compileAttributes() -> do not edit by hand
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include <Rcpp.h>

using namespace Rcpp;

#ifdef RCPP_USE_GLOBAL_ROSTREAM
Rcpp::Rostream<true>&  Rcpp::Rcout = Rcpp::Rcpp_cout_get();
Rcpp::Rostream<false>& Rcpp::Rcerr = Rcpp::Rcpp_cerr_get();
#endif

// qs_save
SEXP qs_save(SEXP object, const std::string& file, const int compress_level, const bool shuffle, const bool store_checksum, const int nthreads);
RcppExport SEXP _qs2_qs_save(SEXP objectSEXP, SEXP fileSEXP, SEXP compress_levelSEXP, SEXP shuffleSEXP, SEXP store_checksumSEXP, SEXP nthreadsSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::traits::input_parameter< SEXP >::type object(objectSEXP);
    Rcpp::traits::input_parameter< const std::string& >::type file(fileSEXP);
    Rcpp::traits::input_parameter< const int >::type compress_level(compress_levelSEXP);
    Rcpp::traits::input_parameter< const bool >::type shuffle(shuffleSEXP);
    Rcpp::traits::input_parameter< const bool >::type store_checksum(store_checksumSEXP);
    Rcpp::traits::input_parameter< const int >::type nthreads(nthreadsSEXP);
    rcpp_result_gen = Rcpp::wrap(qs_save(object, file, compress_level, shuffle, store_checksum, nthreads));
    return rcpp_result_gen;
END_RCPP
}
// qs_read
SEXP qs_read(const std::string& file, const bool validate_checksum, const int nthreads);
RcppExport SEXP _qs2_qs_read(SEXP fileSEXP, SEXP validate_checksumSEXP, SEXP nthreadsSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::traits::input_parameter< const std::string& >::type file(fileSEXP);
    Rcpp::traits::input_parameter< const bool >::type validate_checksum(validate_checksumSEXP);
    Rcpp::traits::input_parameter< const int >::type nthreads(nthreadsSEXP);
    rcpp_result_gen = Rcpp::wrap(qs_read(file, validate_checksum, nthreads));
    return rcpp_result_gen;
END_RCPP
}
// qd_save
SEXP qd_save(SEXP object, const std::string& file, const int compress_level, const bool shuffle, const bool store_checksum, const bool warn_unsupported_types, const int nthreads);
RcppExport SEXP _qs2_qd_save(SEXP objectSEXP, SEXP fileSEXP, SEXP compress_levelSEXP, SEXP shuffleSEXP, SEXP store_checksumSEXP, SEXP warn_unsupported_typesSEXP, SEXP nthreadsSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::traits::input_parameter< SEXP >::type object(objectSEXP);
    Rcpp::traits::input_parameter< const std::string& >::type file(fileSEXP);
    Rcpp::traits::input_parameter< const int >::type compress_level(compress_levelSEXP);
    Rcpp::traits::input_parameter< const bool >::type shuffle(shuffleSEXP);
    Rcpp::traits::input_parameter< const bool >::type store_checksum(store_checksumSEXP);
    Rcpp::traits::input_parameter< const bool >::type warn_unsupported_types(warn_unsupported_typesSEXP);
    Rcpp::traits::input_parameter< const int >::type nthreads(nthreadsSEXP);
    rcpp_result_gen = Rcpp::wrap(qd_save(object, file, compress_level, shuffle, store_checksum, warn_unsupported_types, nthreads));
    return rcpp_result_gen;
END_RCPP
}
// qd_read
SEXP qd_read(const std::string& file, const bool use_alt_rep, const bool validate_checksum, const int nthreads);
RcppExport SEXP _qs2_qd_read(SEXP fileSEXP, SEXP use_alt_repSEXP, SEXP validate_checksumSEXP, SEXP nthreadsSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::traits::input_parameter< const std::string& >::type file(fileSEXP);
    Rcpp::traits::input_parameter< const bool >::type use_alt_rep(use_alt_repSEXP);
    Rcpp::traits::input_parameter< const bool >::type validate_checksum(validate_checksumSEXP);
    Rcpp::traits::input_parameter< const int >::type nthreads(nthreadsSEXP);
    rcpp_result_gen = Rcpp::wrap(qd_read(file, use_alt_rep, validate_checksum, nthreads));
    return rcpp_result_gen;
END_RCPP
}
// qx_dump
List qx_dump(const std::string& file);
RcppExport SEXP _qs2_qx_dump(SEXP fileSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::traits::input_parameter< const std::string& >::type file(fileSEXP);
    rcpp_result_gen = Rcpp::wrap(qx_dump(file));
    return rcpp_result_gen;
END_RCPP
}
// check_SIMD
std::string check_SIMD();
RcppExport SEXP _qs2_check_SIMD() {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    rcpp_result_gen = Rcpp::wrap(check_SIMD());
    return rcpp_result_gen;
END_RCPP
}
// check_TBB
bool check_TBB();
RcppExport SEXP _qs2_check_TBB() {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    rcpp_result_gen = Rcpp::wrap(check_TBB());
    return rcpp_result_gen;
END_RCPP
}
// zstd_compress_raw
std::vector<unsigned char> zstd_compress_raw(SEXP const data, const int compress_level);
RcppExport SEXP _qs2_zstd_compress_raw(SEXP dataSEXP, SEXP compress_levelSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::traits::input_parameter< SEXP const >::type data(dataSEXP);
    Rcpp::traits::input_parameter< const int >::type compress_level(compress_levelSEXP);
    rcpp_result_gen = Rcpp::wrap(zstd_compress_raw(data, compress_level));
    return rcpp_result_gen;
END_RCPP
}
// zstd_decompress_raw
RawVector zstd_decompress_raw(SEXP const data);
RcppExport SEXP _qs2_zstd_decompress_raw(SEXP dataSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::traits::input_parameter< SEXP const >::type data(dataSEXP);
    rcpp_result_gen = Rcpp::wrap(zstd_decompress_raw(data));
    return rcpp_result_gen;
END_RCPP
}
// blosc_shuffle_raw
std::vector<unsigned char> blosc_shuffle_raw(SEXP const data, int bytesofsize);
RcppExport SEXP _qs2_blosc_shuffle_raw(SEXP dataSEXP, SEXP bytesofsizeSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::traits::input_parameter< SEXP const >::type data(dataSEXP);
    Rcpp::traits::input_parameter< int >::type bytesofsize(bytesofsizeSEXP);
    rcpp_result_gen = Rcpp::wrap(blosc_shuffle_raw(data, bytesofsize));
    return rcpp_result_gen;
END_RCPP
}
// blosc_unshuffle_raw
std::vector<unsigned char> blosc_unshuffle_raw(SEXP const data, int bytesofsize);
RcppExport SEXP _qs2_blosc_unshuffle_raw(SEXP dataSEXP, SEXP bytesofsizeSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::traits::input_parameter< SEXP const >::type data(dataSEXP);
    Rcpp::traits::input_parameter< int >::type bytesofsize(bytesofsizeSEXP);
    rcpp_result_gen = Rcpp::wrap(blosc_unshuffle_raw(data, bytesofsize));
    return rcpp_result_gen;
END_RCPP
}

static const R_CallMethodDef CallEntries[] = {
    {"_qs2_qs_save", (DL_FUNC) &_qs2_qs_save, 6},
    {"_qs2_qs_read", (DL_FUNC) &_qs2_qs_read, 3},
    {"_qs2_qd_save", (DL_FUNC) &_qs2_qd_save, 7},
    {"_qs2_qd_read", (DL_FUNC) &_qs2_qd_read, 4},
    {"_qs2_qx_dump", (DL_FUNC) &_qs2_qx_dump, 1},
    {"_qs2_check_SIMD", (DL_FUNC) &_qs2_check_SIMD, 0},
    {"_qs2_check_TBB", (DL_FUNC) &_qs2_check_TBB, 0},
    {"_qs2_zstd_compress_raw", (DL_FUNC) &_qs2_zstd_compress_raw, 2},
    {"_qs2_zstd_decompress_raw", (DL_FUNC) &_qs2_zstd_decompress_raw, 1},
    {"_qs2_blosc_shuffle_raw", (DL_FUNC) &_qs2_blosc_shuffle_raw, 2},
    {"_qs2_blosc_unshuffle_raw", (DL_FUNC) &_qs2_blosc_unshuffle_raw, 2},
    {NULL, NULL, 0}
};

RcppExport void R_init_qs2(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
