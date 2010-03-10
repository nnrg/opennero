//---------------------------------------------------
// Name: OpenNero : Types
// Desc:  primitive data types
//---------------------------------------------------

#ifndef _CORE_COMMON_TYPES_H_
#define _CORE_COMMON_TYPES_H_

//primitive data types (evolving flavors)
typedef signed char        S8;      
typedef unsigned char      U8;
typedef signed short       S16;     
typedef unsigned short     U16;     
typedef signed int         S32;
typedef signed long		   S64;
typedef unsigned int       U32;   
typedef unsigned long	   U64;
typedef float              F32;     
typedef double             F64; 

//primitive data types(second round)
typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed int         int32_t;
//typedef signed long		   int64_t; // gcc complains, remove until needed

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;   
typedef unsigned int       uint32_t;
//typedef unsigned long	   uint64_t; // gcc complains, remove until needed

typedef float              float32_t;
typedef double             float64_t;

typedef char        char8_t;
typedef wchar_t     char16_t;

#endif //end _CORE_COMMON_TYPES_H_
