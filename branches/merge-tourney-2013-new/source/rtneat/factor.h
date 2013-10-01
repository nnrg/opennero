#ifndef _FACTOR_H_
#define _FACTOR_H_

#include <vector>
#include <ostream>
#include <string>
#include <boost/enable_shared_from_this.hpp>
#include "neat.h"
#include "XMLSerializable.h"

namespace NEAT
{
    /// A Factor is a record of an event in the genome's 
    /// history that has influenced its evolution.
    class Factor 
        : public boost::enable_shared_from_this<Factor>
        , public XMLSerializable
    {
        friend class boost::serialization::access;
        int _id; ///< id of this factor
        std::string _record; ///< content of this factor
        Factor() {}
    public:
        /// Create a factor
        /// @param record the contents of the factor
        /// @param id the id of the factor
        Factor(const std::string& record, int id);

        /// Destructor
        ~Factor() {}

        /// Print this factor to a population file
        void print_to_file(std::ofstream &outFile);

        /// serialize this factor to/from a Boost ser. archive
        template<class Archive> void serialize
            (Archive & ar, const unsigned int version) {
            ar & BOOST_SERIALIZATION_NVP(_id);
            ar & BOOST_SERIALIZATION_NVP(_record);
        }
    }; // class Factor

    std::ostream& operator<<(std::ostream& out, const FactorPtr& x);
} // name space NEAT

#endif //  header wrapper
