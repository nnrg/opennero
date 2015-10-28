#include "core/Common.h"
#include "rtneat/factor.h"
#include <string>

using namespace NEAT;
using namespace std;

Factor::Factor(const std::string& record, int id)
    : _record(record), _id(id) 
{ }

std::ostream& NEAT::operator<<(std::ostream& out, const FactorPtr& factor) {
    out << "factor ";
    out << factor->_id;
    out << " ";
    out << factor->_record;
    out << std::endl;
    return out;
}
