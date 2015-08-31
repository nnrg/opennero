//---------------------------------------------------
// Name: OpenNero : HashMap
// Desc:  abstraction for the hash map
//---------------------------------------------------

#ifndef _CORE_HASH_MAP_H_
#define _CORE_HASH_MAP_H_

#ifndef _CORE_PREPROCESSOR_H_
    #include "core/Preprocessor.h"
#endif

/** This file puts the appropriate namespaces into the environment for hash_map based on system */
#if NERO_PLATFORM_WINDOWS
#include <hash_map>
using namespace stdext;
#else
#if NERO_PLATFORM_MAC
#include <unordered_map>
#define hash_map std::unordered_map
#else
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 2
// gcc 4.2 and greater uses tr1::unordered_set
// see http://gcc.gnu.org/gcc-4.3/changes.html
#include <tr1/unordered_map>
#define hash_map std::tr1::unordered_map
#else
#include <ext/hash_map>
/// @cond
namespace __gnu_cxx
{
        template<> struct hash< std::string >
        {
                size_t operator()( const std::string& x ) const
                {
                        return hash< const char* >()( x.c_str() );
                }
        };
}
/// @endcond
#define hash_map __gnu_cxx::hash_map
#endif
#endif
#endif

#endif //end _CORE_HASH_MAP_H_
