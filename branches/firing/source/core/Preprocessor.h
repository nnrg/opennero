//---------------------------------------------------
// Name: OpenNero : Preprocessor
// Desc:  All preprecessor defines for various platforms
//---------------------------------------------------

#ifndef _CORE_PREPROCESSOR_H_
#define _CORE_PREPROCESSOR_H_

#include <cstdarg>

// platform abstraction

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) // Windows detected
    #define NERO_PLATFORM_WINDOWS 1
    #define NERO_PLATFORM_LINUX   0
    #define NERO_PLATFORM_MAC     0
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__) // Apple detected
    #define NERO_PLATFORM_WINDOWS 0
    #define NERO_PLATFORM_LINUX   0
    #define NERO_PLATFORM_MAC     1
#elif defined(linux) || defined(__linux) || defined(__linux__) // Linux detected
    #define NERO_PLATFORM_WINDOWS 0
    #define NERO_PLATFORM_LINUX   1
    #define NERO_PLATFORM_MAC     0
#else
    #error "Unrecognized platform!"
#endif

// target abstraction

#if NERO_PLATFORM_WINDOWS
    #if defined(_DEBUG)
        #define NERO_DEBUG 1
    #else
        #define NERO_DEBUG 0
    #endif
#elif defined(NDEBUG)
    #define NERO_DEBUG 0
#else
    #define NERO_DEBUG 1
#endif

// release
#define NERO_RELEASE !NERO_DEBUG

// !!! Important !!!
// set this flag to 1 before shipping!
#define NERO_FINAL_RELEASE 0

// test
#ifdef OPEN_NERO_TEST
    #define NERO_TEST 1
#else
    #define NERO_TEST 0
#endif

// breakpoint insertion
#if NERO_PLATFORM_WINDOWS
#if !defined(_WIN64)	
    #define NERO_BREAK __asm int 3
#else
    #define NERO_BREAK
#endif
#elif NERO_PLATFORM_LINUX
    #define NERO_BREAK asm("int $0x3")
#else
    #define NERO_BREAK
#endif

#define NERO_BUILD_PHYSICS 0

// assertions
#define NERO_ENABLE_ASSERTS (NERO_DEBUG && !NERO_TEST)
#define NERO_ENABLE_ASSERT_WARNINGS NERO_ENABLE_ASSERTS

// unit tests
#define NERO_ENABLE_UNIT_TESTS NERO_TEST


#endif //end _CORE_PREPROCESSOR_H_
