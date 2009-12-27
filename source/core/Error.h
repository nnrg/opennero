//---------------------------------------------------
// Name: OpenNero : Error
// Desc:  different error handling utilities
//---------------------------------------------------

#ifndef _OPENNERO_CORE_ERROR_H_
#define _OPENNERO_CORE_ERROR_H_

#include <assert.h>
#include <sstream>
#include "core/Preprocessor.h"
#include "core/AssertExt.h"

namespace OpenNero 
{
	namespace Log 
	{
		extern void LogWarning( const char* type,  const char* connectionName, const char* msg );
        extern void LogError( const char* type,  const char* connectionName, const char* msg );
	}
}

// Assertions
// Desired Output:
// AssertionFailure! [ File : Line ] -> Exp - Msg

#if NERO_ENABLE_ASSERTS    
    // TODO : It would be nice if we could put the breakpoint directly in this line so the callstack would lead 
    // directly to the assert, not our assert extension code. As of now, it seems visual studio has trouble parsing
    // this line with NERO_BREAK inside of it.
    #define AssertDie( exp, msg ) do { static bool __enable_assert = true; if( !(exp) && __enable_assert ) { std::stringstream str; str << "Assertion Failure! [ " << __FILE__ << ":" << __LINE__ << " ] -> " << #exp << " - " << msg; OpenNero::Log::LogError(NULL,NULL,str.str().c_str()); assert( (exp) || OpenNero::AssertExt::ShowAssert( str.str(), __enable_assert ) );  } } while(0)        
#else
    #define AssertDie( exp, msg )
#endif

#define AssertDieMsg( exp, msg ) AssertDie( (exp), (msg) )
#define Assert( exp ) AssertDie( exp, "" )
#define AssertMsg( exp, msg ) AssertDie( exp, msg )

// Warnings : Test the Condition but only print a message if it fails
#if NERO_ENABLE_ASSERT_WARNINGS
    #define AssertWarnMsg( exp, msg ) do { if( !(exp) ) { std::stringstream str; str << "Assertion Warning! [ " << __FILE__ << ":" << __LINE__ << " ] -> " << #exp << " - " << msg; OpenNero::Log::LogWarning(NULL,NULL, str.str().c_str()); } } while(0)
#else
    #define AssertWarnMsg( exp, msg )
#endif

#define AssertWarn( exp ) AssertWarnMsg( exp, "" )

#endif //end _OPENNERO_CORE_ERROR_H_
