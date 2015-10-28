#ifndef _FACTOR_H_
#define _FACTOR_H_

#include <vector>
#include <ostream>
#include <string>
#include "neat.h"

namespace NEAT
{
    /// history that has influenced its evolution.
    class Factor
    {
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
        
        friend std::ostream& operator<<(std::ostream& out, const FactorPtr& factor);    /// A Factor is a record of an event in the genome's 
    }; // class Factor

    std::ostream& operator<<(std::ostream& out, const FactorPtr& factor);
    
} // name space NEAT

#endif //  header wrapper
