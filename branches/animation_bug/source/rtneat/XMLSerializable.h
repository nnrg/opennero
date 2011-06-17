#ifndef _XMLSERIALIZABLE_H
#define _XMLSERIALIZABLE_H

#include "core/Common.h"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/serialization.hpp>

#include <iostream>

namespace NEAT
{
    /// an interface for objects that can be written as XML
    struct XMLSerializable {
    };
}

#endif // #ifndef _XMLSERIALIZABLE_H
