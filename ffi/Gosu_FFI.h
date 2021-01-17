#pragma once

// https://stackoverflow.com/a/2164853

// Microsoft
#if defined(_MSC_VER)
#define GOSU_FFI_API_EXPORTS __declspec(dllexport)
#define GOSU_FFI_API_IMPORTS __declspec(dllimport)
// GCC
#elif defined(__GNUC__)
#define GOSU_FFI_API_EXPORTS __attribute__((visibility("default")))
#define GOSU_FFI_API_IMPORTS
// Do nothing and hope for the best
#else
#define GOSU_FFI_API_EXPORTS
#define GOSU_FFI_API_IMPORTS
#pragma warning Unknown dynamic link import/export semantics.
#endif

#if GOSU_FFI_EXPORTS
// When building Gosu, also add [[maybe_unused]] to the definition (which is fine because we only
// build Gosu using a C++17 compiler) to silence warnings about unused functions.
#define GOSU_FFI_API extern "C" [[maybe_unused]] GOSU_FFI_API_EXPORTS
#elif defined(__cplusplus)
#define GOSU_FFI_API extern "C" GOSU_FFI_API_IMPORTS
#else
#define GOSU_FFI_API GOSU_FFI_API_IMPORTS
#endif
