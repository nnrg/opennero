#include "core/Common.h"
#include "neat.h"
#include "organism.h"
#include "network.h"
#include <string>
#include <map>
#include <sstream>
#include <boost/make_shared.hpp>

using namespace NEAT;
using namespace std;

Organism::Organism(double fit, GenomePtr g, int gen, const string &md) :
    fitness(fit), 
    orig_fitness(fitness), 
    error(0),
    winner(false), 
    gnome(g),
    net(gnome->genesis(gnome->genome_id)),
    species(), //Start it in no Species
    expected_offspring(0), 
    generation(gen), 
    eliminate(false), 
    champion(false), 
    super_champ_offspring(0), 
    pop_champ(false),
    pop_champ_child(false),
    high_fit(0), 
    time_alive(0), 
    mut_struct_baby(false),
    mate_baby(false),
    metadata(md),
    modified(true),
    smited(false)
{
}

Organism::Organism(std::istream &in):
    fitness(0),
    orig_fitness(fitness),
    error(0),
    winner(false),
    species(),
    expected_offspring(0),
    generation(1),
    eliminate(false),
    champion(false),
    super_champ_offspring(0),
    pop_champ(false),
    pop_champ_child(false),
    high_fit(0),
    time_alive(0),
    mut_struct_baby(false),
    mate_baby(false),
    modified(true),
    smited(false)
    {
        string curword;
        bool md = false;
        while (in)
        {
            in >> curword;
            if (!in) break;

            //Check for next
            if (curword == "genomestart")
            {
                int idcheck;
                in >> idcheck;

                // If there isn't metadata, set metadata to ""
                if (md == false)
                {
                    metadata = "";
                }
                md = false;

                gnome.reset(new Genome(idcheck,in));
                net = gnome->genesis(gnome->genome_id);
                metadata = metadata;
                return;
            }
            else if (curword == "/*")
            {
                // New metadata possibly, so clear out the metadata
                metadata = "";
                while (curword != "*/")
                {
                    // If we've started to form the metadata, put a space in the front
                    if (md)
                    {
                        metadata += " ";
                    }

                    // Append the next word to the metadata, and say that there is metadata
                    metadata += curword;
                    md = true;

                    in >> curword;
                }
            }
        }
    }

Organism::Organism(const Organism& org) :
    fitness(org.fitness),
    orig_fitness(org.orig_fitness),
    error(org.error), 
    winner(org.winner),
    gnome(new Genome(*(org.gnome))), // Associative relationship
    net(new Network(*(org.net))), // Associative relationship
    species(org.species), // Delegation relationship
    expected_offspring(org.expected_offspring), 
    generation(org.generation),
    eliminate(org.eliminate), 
    champion(org.champion), 
    super_champ_offspring(org.super_champ_offspring), 
    pop_champ(org.pop_champ),
    pop_champ_child(org.pop_champ_child),
    high_fit(org.high_fit), 
    time_alive(org.time_alive), 
    mut_struct_baby(org.mut_struct_baby),
    mate_baby(org.mate_baby),
    metadata(org.metadata),
    modified(false),
    smited(false)
{
}

Organism::~Organism()
{
}

void Organism::update_phenotype()
{
    //Now, recreate the phenotype off the new genotype
    // note: net gets deleted automatically because it is a smart pointer
    net=gnome->genesis(gnome->genome_id);

    modified = true;
}

void Organism::update_genotype()
{
    // Import changes from phenotype into the genotype
    gnome->Lamarck();

    modified = true;
}

bool NEAT::order_orgs(OrganismPtr x, OrganismPtr y)
{
    return (x)->fitness > (y)->fitness;
}

bool NEAT::order_orgs_by_adjusted_fit(OrganismPtr x, OrganismPtr y)
{
    return (x)->fitness / x->species.lock()->organisms.size() > (y)->fitness / y->species.lock()->organisms.size();
}

MetadataParser::MetadataParser(NEAT::OrganismPtr org, const string& delimToken)
{
    mParsedOrganism = org;
    if (mParsedOrganism )
    {
        const string metadata = mParsedOrganism->metadata;

        string::size_type lastPos = metadata.find_first_not_of(delimToken, 0);

        string::size_type pos = metadata.find_first_not_of(delimToken, lastPos);

        string key;
        string value;
        while (string::npos != pos || string::npos != lastPos)
        {
            if (key.empty())
            {
                key = metadata.substr(lastPos, pos - lastPos);
            }
            else if (value.empty())
            {
                value = metadata.substr(lastPos, pos - lastPos);
            }
            else
            {
                SetKeyVal(key, value);
                key.clear();
                value.clear();
            }
        }
    }
}

string MetadataParser::GetKeyVal(const string &key)
{
    map<string,string>::iterator needle = mTokens.find(key);
    if (needle != mTokens.end())
    {
        return needle->second;
    }
    else
    {
        return "";
    }
}

void MetadataParser::SetKeyVal(const string &key, const string &val)
{
    if (!key.empty() && !val.empty())
    {
        mTokens.insert(pair<string, string>(key, val));
    }
}

void MetadataParser::UpdateOrganism()
{
    if (mParsedOrganism )
    {
        ostringstream oss;
        for (map<string,string>::iterator itr = mTokens.begin(); itr
            != mTokens.end(); ++itr)
        {
            oss << itr->first<< " "<< itr->second;
            oss << " ";
        }
        mParsedOrganism->metadata = oss.str();
    }
}

std::ostream& NEAT::operator<<(std::ostream& out, const OrganismPtr& organism) {
    out << "Organism #"<< organism->gnome->genome_id<< " Fitness: "<< organism->fitness << " Time: "<< organism->time_alive
        << endl;
    out << organism->gnome;

    return out;
}
std::ostream& NEAT::operator<<(std::ostream& out, const Organism& organism){
    out << "Organism #"<< organism.gnome->genome_id<< " Fitness: "<< organism.fitness << " Time: "<< organism.time_alive
        << endl;
    out << organism.gnome;
    return out;
}
