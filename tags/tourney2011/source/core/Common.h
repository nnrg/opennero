//---------------------------------------------------
// Name: OpenNero : Common
// Desc:  common includes
//---------------------------------------------------

#ifndef _CORE_COMMON_H_
#define _CORE_COMMON_H_

#ifndef _CORE_PREPROCESSOR_H_
#include "Preprocessor.h"
#endif

#ifndef _OPENNERO_CORE_ERROR_H_
#include "Error.h"
#endif

#include <vector>
#include <list>
#include <map>
#include <queue>
#include <string>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <ctime>
#include <irrlicht.h>

#include "core/ONTypes.h"
#include "core/Log.h"
#include "core/ONTime.h"
#include "core/BoostCommon.h"

/**
@brief OpenNero (Neuro-Evolving Robotic Operatives) is a development and benchmark platform for AI research
&copy; 2007 The University of Texas at Austin
<a href="http://www.opennero.org">OpenNero</a> is an open source development and benchmarking game platform for 
artificial intelligence research. It is developed by students in the 
<a href="http://www.cs.utexas.edu/">Department of Computer Sciences</a> at the 
<a href="http://www.utexas.edu/">University of Texas at Austin</a>.
@author Igor Karpov (ikarpov@cs.utexas.edu)
@author John Sheblak (jbsheblak@cs.utexas.edu)
*/
namespace OpenNero
{
	/// delete if not null
	template <typename T> inline void SAFE_DELETE( T* x ) { if(x) { delete x; x = 0; } }
	/// delete array if not null
	template <typename T> inline void SAFE_DELETE_ARRAY( T* x ) { if(x) { delete [] x; x = 0; } }
	/// delete array if not null
	template <typename T> inline void SAFE_ARRAY_DELETE( T* x ) { SAFE_DELETE_ARRAY(x); }

    /// minimum of two values
	template <typename T> inline T MIN( T x, T y ) { return (x) < (y) ? (x) : (y); }

    /// maximum of two values
	template <typename T> inline T MAX( T x, T y ) { return (x) > (y) ? (x) : (y); }

    /// clamp a value between low and high
	template <typename T> inline T CLAMP( T val, T low, T high ) { return MAX( low, MIN( high, val ) ); }

    /// soft equality between floating point numbers
	template <typename T> inline bool fequal( T x, T y ) { return MAX(x,y) - MIN(x,y) < (T)0.000001f; }

	static const F32 PI = 3.141592653589793238462643383f;
	static const F32 TWOPI = PI * 2.0f;
	static const F32 HALFPI = PI * 0.5f;
	static const F32 ONEANDHALFPI = PI * 1.5f;

	const F32 RAD_2_DEG = irr::core::RADTODEG;
	const F32 DEG_2_RAD = irr::core::DEGTORAD;

    /// Simulation ID
    typedef uint32_t SimId;

} // end OpenNero

namespace std {
    /// output for vector
    template <typename T>
    ostream& operator<<(ostream& out, const vector<T>& v)
    {
        out << "[ ";
        copy(v.begin(), v.end(), ostream_iterator<T>(out, " "));
        out << "]";
        return out;
    }
}

#endif //end _CORE_COMMON_H_
