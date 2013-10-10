/*
 *  XMLSerializable.cpp
 *  OpenNERO
 *
 *  Created by Igor Karpov on 7/2/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "XMLSerializable.h"

#include "neat.h"
#include "gene.h"
#include "link.h"
#include "network.h"
#include "innovation.h"
#include "nnode.h"
#include "organism.h"
#include "pool.h"
#include "population.h"
#include "species.h"
#include "trait.h"
#include "factor.h"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/weak_ptr.hpp>

#include <iostream>

namespace NEAT
{
    //typedef boost::archive::xml_oarchive ARCHIVE;
    typedef boost::archive::text_oarchive ARCHIVE;

    std::ostream& operator<<(std::ostream& out, const TraitPtr& x) 
    {
        ARCHIVE out_archive(out);
        out_archive << BOOST_SERIALIZATION_NVP(x);
        return out;
    }
    
    std::ostream& operator<<(std::ostream& out, const NetworkPtr& x) 
    {
        ARCHIVE out_archive(out);
        out_archive << BOOST_SERIALIZATION_NVP(x);
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const SpeciesPtr& x) 
    {
        ARCHIVE out_archive(out);
        out_archive << BOOST_SERIALIZATION_NVP(x);
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const PopulationPtr& x) {
        ARCHIVE out_archive(out);
        out_archive << BOOST_SERIALIZATION_NVP(x);
        return out;
    }    

    std::istream& operator>>(std::istream& in, PopulationPtr& x)
    {
        boost::archive::xml_iarchive in_archive(in);
        in_archive >> BOOST_SERIALIZATION_NVP(x);
        return in;
    }

    std::ostream& operator<<(std::ostream& out, const OrganismPtr& x) {
        ARCHIVE out_archive(out);
        out_archive << BOOST_SERIALIZATION_NVP(x);
        return out;
    }

    std::istream& operator>>(std::istream& in, OrganismPtr& x)
    {
        boost::archive::xml_iarchive in_archive(in);
        in_archive >> BOOST_SERIALIZATION_NVP(x);
        return in;
    }

    std::ostream& operator<<(std::ostream& out, const NNodePtr& x) {
        ARCHIVE out_archive(out);
        out_archive << BOOST_SERIALIZATION_NVP(x);
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const LinkPtr& x) {
        ARCHIVE out_archive(out);
        out_archive << BOOST_SERIALIZATION_NVP(x);
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const GenomePtr& x) {
        ARCHIVE out_archive(out);
        out_archive << BOOST_SERIALIZATION_NVP(x);
        return out;
    }
    
    std::istream& operator>>(std::istream& in, GenomePtr& x)
    {
        boost::archive::xml_iarchive in_archive(in);
        in_archive >> BOOST_SERIALIZATION_NVP(x);
        return in;
    }

    std::ostream& operator<<(std::ostream& out, const GenePtr& x) {
        ARCHIVE out_archive(out);
        out_archive << BOOST_SERIALIZATION_NVP(x);
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const FactorPtr& x) {
        ARCHIVE out_archive(out);
        out_archive << BOOST_SERIALIZATION_NVP(x);
        return out;
    }
    
}
