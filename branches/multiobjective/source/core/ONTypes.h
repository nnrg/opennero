//---------------------------------------------------
// Name: OpenNero : Types
// Desc:  primitive data types
//---------------------------------------------------

#ifndef _CORE_COMMON_TYPES_H_
#define _CORE_COMMON_TYPES_H_

namespace OpenNero
{
    //primitive data types (evolving flavors)
    /// signed 8 bit type
    typedef signed char        S8;      
    /// unsigned 8 bit type
    typedef unsigned char      U8;
    /// signed 16 bit type
    typedef signed short       S16;     
    /// unsigned 16 bit type
    typedef unsigned short     U16;     
    /// signed 32 bit type
    typedef signed int         S32;
    /// signed 64 bit type
    typedef signed long        S64;
    /// unsigned 32 bit type
    typedef unsigned int       U32;   
    /// unsigned 64 bit type
    typedef unsigned long      U64;
    /// 32 bit floating point typ
    typedef float              F32;
    /// 64 bit floating point type     
    typedef double             F64; 

    //primitive data types(second round)
    /// 8 bit integer type
    typedef signed char        int8_t;
    /// 16 bit integer type
    typedef signed short       int16_t;
    /// 32 bit integer type
    typedef signed int         int32_t;
    /// 64 bit integer type
    typedef signed long        int64_t;

    /// 8 bit unsigned integer type
    typedef unsigned char      uint8_t;
    /// 16 bit unsigned integer type
    typedef unsigned short     uint16_t;   
    /// 32 bit unsigned integer type
    typedef unsigned int       uint32_t;
    /// 64 bit unsigned integer type
    typedef unsigned long      uint64_t;

    /// 32 bit floating point type
    typedef float              float32_t;
    /// 64 bit floating point type
    typedef double             float64_t;

    /// 8 bit char type
    typedef char        char8_t;
    /// 16 bit char type
    typedef wchar_t     char16_t;

} // end OpenNero

#endif //end _CORE_COMMON_TYPES_H_
