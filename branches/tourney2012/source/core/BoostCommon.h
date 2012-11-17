//---------------------------------------------------
// Name: OpenNero : BoostCommon
// Desc: Common boost:: things
// Author: John Sheblak
// Contact: jbsheblak@mail.utexas.edu
//---------------------------------------------------

#ifndef _CORE_BOOST_H_
#define _CORE_BOOST_H_

// this prevents a compilation problem on Mac OS X
#define BOOST_DATE_TIME_NO_LOCALE 1

//common includes
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/enable_shared_from_this.hpp>

using namespace boost;

// macros

// to take away time from forward decl everything...
// TODO: doxygen document
#define BOOST_SHARED_DECL( classname )    class classname; typedef boost::shared_ptr<classname> classname##Ptr
#define BOOST_WEAK_DECL( classname )      class classname; typedef boost::weak_ptr<classname> classname##WPtr
#define BOOST_INTRUSIVE_DECL( classname ) class classname; typedef boost::intrusive_ptr<classname>  classname##IPtr
#define BOOST_SMART_PTR_DECL( classname ) BOOST_SHARED_DECL(classname); BOOST_WEAK_DECL(classname); BOOST_INTRUSIVE_DECL(classname)
#define BOOST_PTR_DECL(classname) BOOST_SMART_PTR_DECL(classname)

#define BOOST_SHARED_STRUCT( structname )    struct structname; typedef boost::shared_ptr<structname> structname##Ptr
#define BOOST_WEAK_STRUCT( structname )      struct structname; typedef boost::weak_ptr<structname> structname##WPtr
#define BOOST_INTRUSIVE_STRUCT( structname ) struct structname; typedef boost::intrusive_ptr<structname>  structname##IPtr
#define BOOST_SMART_PTR_STRUCT( structname ) BOOST_SHARED_STRUCT(structname); BOOST_WEAK_STRUCT(structname); BOOST_INTRUSIVE_STRUCT(structname)
#define BOOST_PTR_STRUCT(structname) BOOST_SMART_PTR_STRUCT(structname)

#define BOOST_SHARED_THIS( classname ) boost::enable_shared_from_this<classname>

#endif //end _CORE_BOOST_H_
