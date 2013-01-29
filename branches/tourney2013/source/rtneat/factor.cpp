#include "core/Common.h"
#include "rtneat/factor.h"
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace NEAT;
using namespace std;

Factor::Factor(const std::string& record, int id)
    : _record(record), _id(id) 
{ }

void Factor::print_to_file(std::ofstream &outFile)
{
    outFile << "factor ";
    outFile << _id;
    outFile << " ";
    outFile << _record;
    outFile << std::endl;
}
