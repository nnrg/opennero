#include "core/Common.h"
#include "population.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>
#include <fstream>
#include <stdexcept>

using namespace std;
using namespace NEAT;

PopulationPtr Population::copy(PopulationPtr p) {
  //FIXME - size hardcoded
  PopulationPtr np(new Population(p->organisms[0]->gnome,1));
  return p;
}

//used to give population console access
namespace PopulationConsole
{
    void initConsole()
    {
        static bool consoleInit = false;

        if ( !consoleInit )
        {
            consoleInit = true;
        }
    }
}

Population::Population(GenomePtr g, S32 size)
{
    PopulationConsole::initConsole();
    winnergen=0;
    highest_fitness=0.0;
    highest_last_changed=0;
    spawn(g, size);
}

Population::Population(GenomePtr g, S32 size, F32 power)
{
    PopulationConsole::initConsole();
    winnergen=0;
    highest_fitness=0.0;
    highest_last_changed=0;
    clone(g, size, power);
}

//MSC Addition
//Added the ability for a population to be spawned
//off of a vector of Genomes.  Useful when converging.
Population::Population(vector<Genome*> genomeList, F32 power)
{

    PopulationConsole::initConsole();
    winnergen=0;
    highest_fitness=0.0;
    highest_last_changed=0;

    GenomePtr new_genome;
    OrganismPtr new_organism;

    //Create size copies of the Genome
    //Start with perturbed linkweights
    for (vector<Genome*>::iterator iter = genomeList.begin(); iter
        != genomeList.end(); ++iter)
    {

        new_genome.reset(*iter);
        if (power>0)
            new_genome->mutate_link_weights(power, 1.0, GAUSSIAN);
        //new_genome->mutate_link_weights(1.0,1.0,COLDGAUSSIAN);
        new_genome->randomize_traits();
        new_organism.reset(new Organism(0.0,new_genome,1));
        organisms.push_back(new_organism);
    }

    //Keep a record of the innovation and node number we are on
    cur_node_id=new_genome->get_last_node_id();
    cur_innov_num=new_genome->get_last_gene_innovnum();

    //Separate the new Population into species
    speciate();
}

Population::Population(const std::string& filename)
{

    PopulationConsole::initConsole();
    string curword;

    GenomePtr new_genome;

    winnergen=0;

    highest_fitness=0.0;
    highest_last_changed=0;

    cur_node_id=0;
    cur_innov_num=0.0;

    ifstream iFile(filename.c_str());
    if (!iFile)
    {
        throw std::runtime_error("Could not open genome file: " + filename);
    }

		//FIXME - PUT '&& iFile' INTO IF STATEMENTS
    else
    {
        bool md = false;
        string metadata;
        //Loop until file is finished, parsing each line
        while (iFile)
        {
            iFile >> curword;
	    if (!iFile) break;

            //Check for next
            if (curword == "genomestart")
            {
                int idcheck;
                iFile >> idcheck;

                // If there isn't metadata, set metadata to ""
                if (md == false)
                {
                    metadata = "";
                }
                md = false;

                new_genome.reset(new Genome(idcheck,iFile));
                OrganismPtr p(new Organism(0,new_genome,1, metadata));
                organisms.push_back(p);
                if (cur_node_id<(new_genome->get_last_node_id()))
                    cur_node_id=new_genome->get_last_node_id();

                if (cur_innov_num<(new_genome->get_last_gene_innovnum()))
                    cur_innov_num=new_genome->get_last_gene_innovnum();
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

                    iFile >> curword;
                }
            }
        }

        iFile.close();

        speciate();

    }
}

//Creates a population filled to the inputed size.
Population::Population(const std::string& filename, S32 size)
{

    PopulationConsole::initConsole();
    string curword; //max word size of 128 characters

    GenomePtr new_genome;

    winnergen=0;

    highest_fitness=0.0;
    highest_last_changed=0;

    cur_node_id=0;
    cur_innov_num=0.0;

    ifstream iFile(filename.c_str());
    if (!iFile)
    {
        throw std::runtime_error("Could not open genomes file for input: " + filename);
    }
    else
    {
        while (static_cast<S32>(organisms.size()) < size)
        {
            bool md = false;
            string metadata;
            //Loop until file is finished, parsing each line
            //If we have enough guys, then stop reading more in!
            while (iFile && static_cast<S32>(organisms.size()) < size)
            {
                iFile >> curword;
                if (!iFile) break;

                //Check for next
                if (curword == "genomestart")
                {
                    S32 idcheck;
                    iFile >> idcheck;

                    // If there isn't metadata, set metadata to ""
                    if (md == false)
                    {
                        metadata = "";
                    }
                    md = false;

                    new_genome.reset(new Genome(idcheck,iFile));
                    OrganismPtr p(new Organism(0,new_genome,1, metadata));
                    organisms.push_back(p);
                    if (cur_node_id<(new_genome->get_last_node_id()))
                        cur_node_id=new_genome->get_last_node_id();

                    if (cur_innov_num<(new_genome->get_last_gene_innovnum()))
                        cur_innov_num=new_genome->get_last_gene_innovnum();
                }
                else if (curword == "/*")
                {
                    // New metadata possibly, so clear out the metadata
                    metadata = "";
                    iFile >> curword;
                    while (curword.find("*/") == string::npos)
                    {
                        // If we've started to form the metadata, put a space in the front
                        if (md)
                        {
                            metadata += " ";
                        }

                        // Append the next word to the metadata, and say that there is metadata
                        metadata += curword;
                        md = true;

                        iFile >> curword;
                    }
                }
            }
            iFile.clear(); // clear end-of-file state
            iFile.seekg(0, ios::beg); // rewind the file pointer
        }
        iFile.close();
        speciate();
    }
}

Population::~Population()
{
}

bool Population::verify()
{
    vector<OrganismPtr>::iterator curorg;

    bool verification = true;

    for (curorg=organisms.begin(); curorg!=organisms.end(); ++curorg)
    {
        verification = verification && ((*curorg)->gnome)->verify();
    }

    return verification;
}

bool Population::clone(GenomePtr g, S32 size, F32 power)
{
    S32 count;
    GenomePtr new_genome;
    OrganismPtr new_organism;

    //Create size copies of the Genome
    //Start with perturbed linkweights
    for (count=1; count<=size; count++)
    {
        //cout<<"CREATING ORGANISM "<<count<<endl;

        new_genome=g->duplicate(count);
        if (power>0)
            new_genome->mutate_link_weights(power, 1.0, GAUSSIAN);
        //new_genome->mutate_link_weights(1.0,1.0,COLDGAUSSIAN);
        new_genome->randomize_traits();
        new_organism.reset(new Organism(0.0,new_genome,1));
        organisms.push_back(new_organism);
    }

    //Keep a record of the innovation and node number we are on
    cur_node_id=new_genome->get_last_node_id();
    cur_innov_num=new_genome->get_last_gene_innovnum();

    //Separate the new Population into species
    speciate();

    return true;
}

bool Population::spawn(GenomePtr g, S32 size)
{
    S32 count;
    GenomePtr new_genome;
    OrganismPtr new_organism;

    //Create size copies of the Genome
    //Start with perturbed linkweights
    for (count=1; count<=size; count++)
    {
        //cout<<"CREATING ORGANISM "<<count<<endl;

        new_genome=g->duplicate(count);
        //new_genome->mutate_link_weights(1.0,1.0,GAUSSIAN);
        new_genome->mutate_link_weights(1.0, 1.0, COLDGAUSSIAN);
        new_genome->randomize_traits();
        new_organism.reset(new Organism(0.0,new_genome,1));
        organisms.push_back(new_organism);
    }

    //Keep a record of the innovation and node number we are on
    cur_node_id=new_genome->get_last_node_id();
    cur_innov_num=new_genome->get_last_gene_innovnum();

    //Separate the new Population into species
    speciate();

    return true;
}

bool Population::speciate()
{
    vector<OrganismPtr>::iterator curorg; //For stepping through Population
    vector<SpeciesPtr>::iterator curspecies; //Steps through species
    OrganismPtr comporg; //Organism for comparison 
    SpeciesPtr newspecies; //For adding a new species

    S32 counter=0; //Species counter

    //Step through all existing organisms
    for (curorg=organisms.begin(); curorg!=organisms.end(); ++curorg)
    {

        //For each organism, search for a species it is compatible to
        curspecies=species.begin();
        if (curspecies==species.end())
        {
            //Create the first species
            newspecies.reset(new Species(++counter));
            species.push_back(newspecies);
            newspecies->add_Organism(*curorg); //Add the current organism
            (*curorg)->species=newspecies; //Point organism to its species
        }
        else
        {
            comporg=(*curspecies)->first();
            while ((comporg.get()!=0)&&(curspecies!=species.end()))
            {

                if ((((*curorg)->gnome)->compatibility(comporg->gnome))
                    <NEAT::compat_threshold)
                {

                    //Found compatible species, so add this organism to it
                    (*curspecies)->add_Organism(*curorg);
                    (*curorg)->species=(*curspecies); //Point organism to its species
                    comporg.reset(); //Note the search is over
                }
                else
                {

                    //Keep searching for a matching species
                    ++curspecies;
                    if (curspecies!=species.end())
                        comporg=(*curspecies)->first();
                }
            }

            //If we didn't find a match, create a new species
            if (comporg!=0)
            {
                newspecies.reset(new Species(++counter));
                species.push_back(newspecies);
                newspecies->add_Organism(*curorg); //Add the current organism
                (*curorg)->species=newspecies; //Point organism to its species
            }

        } //end else 

    } //end for

    last_species=counter; //Keep track of highest species

    return true;
}

bool Population::print_to_file_by_species(ofstream& outFile)
{

    vector<SpeciesPtr>::iterator curspecies;

    //Step through the Species and print them to the file
    for (curspecies=species.begin(); curspecies!=species.end(); ++curspecies)
    {
        (*curspecies)->print_to_file(outFile);
    }

    return true;
}

// Print all the genomes in the population to a file
bool Population::print_to_file(std::ofstream &outFile)
{
    vector<OrganismPtr>::iterator curorg;
    
    for (curorg = organisms.begin(); curorg != organisms.end(); ++curorg)
    {
        (*curorg)->gnome->print_to_file(outFile);
    }
    
    return true;
}


bool Population::epoch(S32 generation)
{

    vector<SpeciesPtr>::iterator curspecies;
    vector<SpeciesPtr>::iterator deadspecies; //For removing empty Species

    vector<OrganismPtr>::iterator curorg;
    vector<OrganismPtr>::iterator deadorg;

    vector<InnovationPtr>::iterator curinnov;
    vector<InnovationPtr>::iterator deadinnov; //For removing old Innovs

    F64 total=0.0; //Used to compute average fitness over all Organisms

    F64 overall_average; //The average modified fitness among ALL organisms

    S32 orgcount;

    //The fractional parts of expected offspring that can be 
    //Used only when they accumulate above 1 for the purposes of counting
    //Offspring
    F64 skim;
    S32 total_expected; //precision checking
    S32 total_organisms=static_cast<S32>(organisms.size());
    S32 max_expected;
    SpeciesPtr best_species;
    S32 final_expected;

    //Rights to make babies can be stolen from inferior species
    //and given to their superiors, in order to concentrate exploration on
    //the best species
    S32 NUM_STOLEN=NEAT::babies_stolen; //Number of babies to steal
    S32 one_fifth_stolen;
    S32 one_tenth_stolen;

    vector<SpeciesPtr> sorted_species; //Species sorted by max fit org in Species
    S32 stolen_babies; //Babies taken from the bad species and given to the champs

    S32 half_pop;

    S32 best_species_num; //Used in debugging to see why (if) best species dies
    bool best_ok;

    //We can try to keep the number of species constant at this number
    S32 num_species_target=4;
    S32 num_species=static_cast<S32>(species.size());
    F64 compat_mod=0.3; //Modify compat thresh to control speciation


    //Keeping species diverse
    //This commented out code forces the system to aim for 
    // num_species species at all times, enforcing diversity
    //This tinkers with the compatibility threshold, which
    // normally would be held constant

    if (generation>1)
    {
        if (num_species<num_species_target)
            NEAT::compat_threshold-=compat_mod;
        else if (num_species>num_species_target)
            NEAT::compat_threshold+=compat_mod;

        if (NEAT::compat_threshold<0.3)
            NEAT::compat_threshold=0.3;

    }

    //Stick the Species pointers into a new Species list for sorting
    for (curspecies=species.begin(); curspecies!=species.end(); ++curspecies)
    {
        sorted_species.push_back(*curspecies);
    }

    //Sort the Species by max fitness (Use an extra list to do this)
    //These need to use ORIGINAL fitness
    sort(sorted_species.begin(), sorted_species.end(), order_species);

    //Flag the lowest performing species over age 20 every 30 generations 
    //NOTE: THIS IS FOR COMPETITIVE COEVOLUTION STAGNATION DETECTION

    curspecies=sorted_species.end();
    curspecies--;
    while ((curspecies!=sorted_species.begin())&&((*curspecies)->age<20))
        --curspecies;
    if ((generation%30)==0)
        (*curspecies)->obliterate=true;

    // cout<<"Number of Species: "<<num_species<<endl;
    // cout<<"compat_thresh: "<<compat_threshold<<endl;

    //Use Species' ages to modify the objective fitness of organisms
    // in other words, make it more fair for younger species
    // so they have a chance to take hold
    //Also penalize stagnant species
    //Then adjust the fitness using the species size to "share" fitness
    //within a species.
    //Then, within each Species, mark for death 
    //those below survival_thresh*average
    for (curspecies=species.begin(); curspecies!=species.end(); ++curspecies)
    {
        (*curspecies)->adjust_fitness();
    }

    //Go through the organisms and add up their fitnesses to compute the
    //overall average
    for (curorg=organisms.begin(); curorg!=organisms.end(); ++curorg)
    {
        total+=(*curorg)->fitness;
    }
    overall_average=total/total_organisms;
    cout << "overall_average: " << overall_average << endl;
    //  cout<<"Generation "<<generation<<": "<<"overall_average = "<<overall_average<<endl;

    //Now compute expected number of offspring for each individual organism
    for (curorg=organisms.begin(); curorg!=organisms.end(); ++curorg)
    {
        (*curorg)->expected_offspring=(((*curorg)->fitness)/overall_average);
    }

    //Now add those offspring up within each Species to get the number of
    //offspring per Species
    skim=0.0;
    total_expected=0;
    for (curspecies=species.begin(); curspecies!=species.end(); ++curspecies)
    {
        skim=(*curspecies)->count_offspring(skim);
        total_expected+=(*curspecies)->expected_offspring;
    }

    //Need to make up for lost foating point precision in offspring assignment
    //If we lost precision, give an extra baby to the best Species
    if (total_expected<total_organisms)
    {
        //Find the Species expecting the most
        max_expected=0;
        final_expected=0;
        for (curspecies=species.begin(); curspecies!=species.end(); ++curspecies)
        {
            if ((*curspecies)->expected_offspring>=max_expected)
            {
                max_expected=(*curspecies)->expected_offspring;
                best_species=(*curspecies);
            }
            final_expected+=(*curspecies)->expected_offspring;
        }
        //Give the extra offspring to the best species
        ++(best_species->expected_offspring);
        final_expected++;

        //If we still arent at total, there is a problem
        //Note that this can happen if a stagnant Species
        //dominates the population and then gets killed off by its age
        //Then the whole population plummets in fitness
        //If the average fitness is allowed to hit 0, then we no longer have 
        //an average we can use to assign offspring.
        if (final_expected<total_organisms)
        {
            //      cout<<"Population died!"<<endl;
            //cin>>pause;
            for (curspecies=species.begin(); curspecies!=species.end(); ++curspecies)
            {
                (*curspecies)->expected_offspring=0;
            }
            best_species->expected_offspring=total_organisms;
        }
    }

    //Sort the Species by max fitness (Use an extra list to do this)
    //These need to use ORIGINAL fitness
    sort(sorted_species.begin(), sorted_species.end(), order_species);

    best_species_num=(*(sorted_species.begin()))->id;

    for (curspecies=sorted_species.begin(); curspecies!=sorted_species.end(); ++curspecies)
    {

        //Print out for Debugging/viewing what's going on 
        //    cout<<"orig fitness of Species"<<(*curspecies)->id<<"(Size "<<(*curspecies)->organisms.size()<<"): "<<(*((*curspecies)->organisms).begin())->orig_fitness<<" last improved "<<((*curspecies)->age-(*curspecies)->age_of_last_improvement)<<endl;
    }

    //Check for Population-level stagnation
    curspecies=sorted_species.begin();
    (*(((*curspecies)->organisms).begin()))->pop_champ=true; //DEBUG marker of the best of pop
    if (((*(((*curspecies)->organisms).begin()))->orig_fitness)>highest_fitness)
    {
        highest_fitness=((*(((*curspecies)->organisms).begin()))->orig_fitness);
        highest_last_changed=0;
        //    cout<<"NEW POPULATION RECORD FITNESS: "<<highest_fitness<<endl;
    }
    else
    {
        ++highest_last_changed;
        //    cout<<highest_last_changed<<" generations since last population fitness record: "<<highest_fitness<<endl;
    }

    //Check for stagnation- if there is stagnation, perform delta-coding
    if (highest_last_changed>=NEAT::dropoff_age+5)
    {

        //    cout<<"PERFORMING DELTA CODING"<<endl;

        highest_last_changed=0;

        half_pop=NEAT::pop_size/2;

        //    cout<<"half_pop"<<half_pop<<" pop_size-halfpop: "<<pop_size-half_pop<<endl;

        curspecies=sorted_species.begin();

        (*(((*curspecies)->organisms).begin()))->super_champ_offspring=half_pop;
        (*curspecies)->expected_offspring=half_pop;
        (*curspecies)->age_of_last_improvement=(*curspecies)->age;

        ++curspecies;

        if (curspecies!=sorted_species.end())
        {

            (*(((*curspecies)->organisms).begin()))->super_champ_offspring=NEAT::pop_size-half_pop;
            (*curspecies)->expected_offspring=NEAT::pop_size-half_pop;
            (*curspecies)->age_of_last_improvement=(*curspecies)->age;

            ++curspecies;

            //Get rid of all species under the first 2
            while (curspecies!=sorted_species.end())
            {
                (*curspecies)->expected_offspring=0;
                ++curspecies;
            }
        }
        else
        {
            curspecies=sorted_species.begin();
            (*(((*curspecies)->organisms).begin()))->super_champ_offspring+=NEAT::pop_size-half_pop;
            (*curspecies)->expected_offspring=NEAT::pop_size-half_pop;
        }

    }
    //STOLEN BABIES:  The system can take expected offspring away from
    //  worse species and give them to superior species depending on
    //  the system parameter babies_stolen (when babies_stolen > 0)
    else if (NEAT::babies_stolen>0)
    {
        //Take away a constant number of expected offspring from the worst few species

        stolen_babies=0;
        curspecies=sorted_species.end();
        curspecies--;
        while ((stolen_babies<NUM_STOLEN)&&(curspecies!=sorted_species.begin()))
        {

            //cout<<"Considering Species "<<(*curspecies)->id<<": age "<<(((*curspecies)->age))<<" expected offspring "<<(((*curspecies)->expected_offspring))<<endl;

            if ((((*curspecies)->age)>5)&&(((*curspecies)->expected_offspring)>2))
            {
                //cout<<"STEALING!"<<endl;

                //This species has enough to finish off the stolen pool
                if (((*curspecies)->expected_offspring-1)>=(NUM_STOLEN
                    -stolen_babies))
                {
                    (*curspecies)->expected_offspring-=(NUM_STOLEN
                        -stolen_babies);
                    stolen_babies=NUM_STOLEN;
                }
                //Not enough here to complete the pool of stolen
                else
                {
                    stolen_babies+=(*curspecies)->expected_offspring-1;
                    (*curspecies)->expected_offspring=1;

                }
            }

            curspecies--;

            //if (stolen_babies>0)
            //cout<<"stolen babies so far: "<<stolen_babies<<endl;
        }

        //cout<<"STOLEN BABIES: "<<stolen_babies<<endl;

        //Mark the best champions of the top species to be the super champs
        //who will take on the extra offspring for cloning or mutant cloning
        curspecies=sorted_species.begin();

        //Determine the exact number that will be given to the top three
        //They get , in order, 1/5 1/5 and 1/10 of the stolen babies
        one_fifth_stolen=NEAT::babies_stolen/5;
        one_tenth_stolen=NEAT::babies_stolen/10;

        //Don't give to dying species even if they are champs
        while (((*curspecies)->last_improved()>NEAT::dropoff_age)&&(curspecies
            !=sorted_species.end()))
            ++curspecies;

        //Concentrate A LOT on the number one species
        if ((stolen_babies>=one_fifth_stolen)&&(curspecies
            !=sorted_species.end()))
        {
            (*(((*curspecies)->organisms).begin()))->super_champ_offspring=one_fifth_stolen;
            (*curspecies)->expected_offspring+=one_fifth_stolen;
            stolen_babies-=one_fifth_stolen;
            //cout<<"Gave "<<one_fifth_stolen<<" babies to Species "<<(*curspecies)->id<<endl;
            //      cout<<"The best superchamp is "<<(*(((*curspecies)->organisms).begin()))->gnome->genome_id<<endl;

            //Print this champ to file "champ" for observation if desired
            //IMPORTANT:  This causes generational file output 
            //print_Genome_tofile((*(((*curspecies)->organisms).begin()))->gnome,"champ");

            curspecies++;

        }

        //Don't give to dying species even if they are champs
        while (((*curspecies)->last_improved()>NEAT::dropoff_age)&&(curspecies
            !=sorted_species.end()))
            ++curspecies;

        if ((curspecies!=sorted_species.end()))
        {
            if (stolen_babies>=one_fifth_stolen)
            {
                (*(((*curspecies)->organisms).begin()))->super_champ_offspring=one_fifth_stolen;
                (*curspecies)->expected_offspring+=one_fifth_stolen;
                stolen_babies-=one_fifth_stolen;
                //cout<<"Gave "<<one_fifth_stolen<<" babies to Species "<<(*curspecies)->id<<endl;
                curspecies++;

            }
        }

        //Don't give to dying species even if they are champs
        while (((*curspecies)->last_improved()>NEAT::dropoff_age)&&(curspecies
            !=sorted_species.end()))
            ++curspecies;

        if (curspecies!=sorted_species.end())
            if (stolen_babies>=one_tenth_stolen)
            {
                (*(((*curspecies)->organisms).begin()))->super_champ_offspring=one_tenth_stolen;
                (*curspecies)->expected_offspring+=one_tenth_stolen;
                stolen_babies-=one_tenth_stolen;

                //cout<<"Gave "<<one_tenth_stolen<<" babies to Species "<<(*curspecies)->id<<endl;
                curspecies++;

            }

        //Don't give to dying species even if they are champs
        while (((*curspecies)->last_improved()>NEAT::dropoff_age)&&(curspecies
            !=sorted_species.end()))
            ++curspecies;

        while ((stolen_babies>0)&&(curspecies!=sorted_species.end()))
        {
            //Randomize a little which species get boosted by a super champ

            if (randfloat()>0.1)
                if (stolen_babies>3)
                {
                    (*(((*curspecies)->organisms).begin()))->super_champ_offspring=3;
                    (*curspecies)->expected_offspring+=3;
                    stolen_babies-=3;
                    //cout<<"Gave 3 babies to Species "<<(*curspecies)->id<<endl;
                }
                else
                {
                    //cout<<"3 or less babies available"<<endl;
                    (*(((*curspecies)->organisms).begin()))->super_champ_offspring=stolen_babies;
                    (*curspecies)->expected_offspring+=stolen_babies;
                    //cout<<"Gave "<<stolen_babies<<" babies to Species "<<(*curspecies)->id<<endl;
                    stolen_babies=0;

                }

            curspecies++;

            //Don't give to dying species even if they are champs
            while ((curspecies!=sorted_species.end())&&((*curspecies)->last_improved()>NEAT::dropoff_age))
                ++curspecies;

        }

        //cout<<"Done giving back babies"<<endl;

        //If any stolen babies aren't taken, give them to species #1's champ
        if (stolen_babies>0)
        {

            //cout<<"Not all given back, giving to best Species"<<endl;

            curspecies=sorted_species.begin();
            (*(((*curspecies)->organisms).begin()))->super_champ_offspring+=stolen_babies;
            (*curspecies)->expected_offspring+=stolen_babies;
            stolen_babies=0;
        }

    }

    //Kill off all Organisms marked for death.  The remainder
    //will be allowed to reproduce.
    curorg=organisms.begin();
    while (curorg!=organisms.end())
    {
        if (((*curorg)->eliminate))
        {
            //Remove the organism from its Species
            (*curorg)->species.lock()->remove_org(*curorg);

            //Delete the organism from memory
            curorg->reset();

            //Remember where we are
            deadorg=curorg;
            ++curorg;

            //Remove the organism from the master list
            organisms.erase(deadorg);

        }
        else
        {
            ++curorg;
        }

    }

    //Perform reproduction.  Reproduction is done on a per-Species
    //basis.  (So this could be paralellized potentially.)
    for (curspecies=species.begin(); curspecies!=species.end(); ++curspecies)
    {
        (*curspecies)->reproduce(generation, shared_from_this(),
                                    sorted_species);
    }

    //cout<<"Reproduction Complete"<<endl;

    //Destroy and remove the old generation from the organisms and species  
    curorg=organisms.begin();
    while (curorg!=organisms.end())
    {

        //Remove the organism from its Species
        (*curorg)->species.lock()->remove_org(*curorg);

        //Delete the organism from memory
        curorg->reset();

        //Remember where we are
        deadorg=curorg;
        ++curorg;

        //Remove the organism from the master list
        organisms.erase(deadorg);

    }

    //Remove all empty Species and age ones that survive
    //As this happens, create master organism list for the new generation
    curspecies=species.begin();
    orgcount=0;
    while (curspecies!=species.end())
    {
        if (((*curspecies)->organisms.size())==0)
        {
            deadspecies=curspecies;
            ++curspecies;
            species.erase(deadspecies);
        }
        //Age surviving Species and 
        //Rebuild master Organism list: NUMBER THEM as they are added to the list
        else
        {
            //Age any Species that is not newly created in this generation
            if ((*curspecies)->novel)
            {
                (*curspecies)->novel=false;
            }
            else
                ++((*curspecies)->age);

            //Go through the organisms of the curspecies and add them to 
            //the master list
            for (curorg=((*curspecies)->organisms).begin(); curorg!=((*curspecies)->organisms).end(); ++curorg)
            {
                ((*curorg)->gnome)->genome_id=orgcount++;
                organisms.push_back(*curorg);
            }
            ++curspecies;

        }
    }

    //Remove the innovations of the current generation
    innovations.clear();

    //DEBUG: Check to see if the best species died somehow
    // We don't want this to happen
    curspecies=species.begin();
    best_ok=false;
    while (curspecies!=species.end())
    {
        if (((*curspecies)->id)==best_species_num)
            best_ok=true;
        ++curspecies;
    }
    if (!best_ok)
    {
        //cout<<"ERROR: THE BEST SPECIES DIED!"<<endl;
    }
    else
    {
        //cout<<"The best survived: "<<best_species_num<<endl;
    }

    //DEBUG: Checking the top organism's duplicate in the next gen
    //This prints the champ's child to the screen
    for (curorg=organisms.begin(); curorg!=organisms.end(); ++curorg)
    {
        if ((*curorg)->pop_champ_child)
        {
            //cout<<"At end of reproduction cycle, the child of the pop champ is: "<<(*curorg)->gnome<<endl;
        }
    }

    //cout<<"babies_stolen at end: "<<babies_stolen<<endl;

    //cout<<"Epoch complete"<<endl; 

    return true;

}

bool Population::rank_within_species()
{
    vector<SpeciesPtr>::iterator curspecies;

    //Add each Species in this generation to the snapshot
    for (curspecies=species.begin(); curspecies!=species.end(); ++curspecies)
    {
        (*curspecies)->rank();
    }

    return true;
}

void Population::estimate_all_averages()
{
    vector<SpeciesPtr>::iterator curspecies;

    for (curspecies=species.begin(); curspecies!=species.end(); ++curspecies)
    {
        (*curspecies)->estimate_average();
    }

}

SpeciesPtr Population::choose_parent_species()
{

    F64 total_fitness=0;
    vector<SpeciesPtr>::iterator curspecies;
    F64 marble; //The roulette marble
    F64 spin; //Spins until the marble reaches its chosen point

    //We can try to keep the number of species constant at this number
    // TODO: this is not implemented in NERO 2.0 - it might be nice to implement it
    // S32 num_species_target=4;
    // S32 num_species=species.size();
    // F64 compat_mod=0.3;  //Modify compat thresh to control speciation

    //Keeping species diverse
    //This commented out code forces the system to aim for 
    // num_species species at all times, enforcing diversity
    //This tinkers with the compatibility threshold, which
    // normally would be held constant

    //if (num_species<num_species_target)
    //	NEAT::compat_threshold-=compat_mod;
    //else if (num_species>num_species_target)
    //	NEAT::compat_threshold+=compat_mod;

    //if (NEAT::compat_threshold<0.3) NEAT::compat_threshold=0.3;


    //Use the roulette method to choose the species 

    //Sum all the average fitness estimates of the different species
    //for the purposes of the roulette
    for (curspecies=species.begin(); curspecies!=species.end(); ++curspecies)
    {
        total_fitness+=(*curspecies)->average_est;
    }

    marble=randfloat()*total_fitness;
    curspecies=species.begin();
    spin=(*curspecies)->average_est;
    while (spin<marble && (curspecies + 1) != species.end())
    {
        ++curspecies;

        //Keep the wheel spinning
        spin+=(*curspecies)->average_est;
    }
    //Finished roulette

    //  cout<<"Chose random species "<<(*curspecies)->id<<endl;
    //Con::printf("Chose random species %d.",(*curspecies)->id);

    //Return the chosen species
    return (*curspecies);
}

bool Population::remove_species(SpeciesPtr spec)
{
    vector<SpeciesPtr>::iterator curspec;

    curspec=species.begin();
    while ((curspec!=species.end())&&((*curspec)!=spec))
        ++curspec;

    if (curspec==species.end())
    {
        //   cout<<"ALERT: Attempt to remove nonexistent Species from Population"<<endl;
        return false;
    }
    else
    {
        species.erase(curspec);
        return true;
    }
}

OrganismPtr Population::remove_worst()
{

    F64 adjusted_fitness;
    F64 min_fitness=999999;
    vector<OrganismPtr>::iterator curorg;
    OrganismPtr org_to_kill;
    vector<OrganismPtr>::iterator deadorg;
    SpeciesPtr orgs_species; //The species of the dead organism

    //Make sure the organism is deleted from its species and the population

    //First find the organism with minimum *adjusted* fitness
    for (curorg=organisms.begin(); curorg!=organisms.end(); ++curorg)
    {
        adjusted_fitness=((*curorg)->fitness)/((*curorg)->species.lock()->organisms.size());

        if ((*curorg)->smited)
        {
            //get the next time multiple
            U32 nextMultiple;
            if ((*curorg)->time_alive % NEAT::time_alive_minimum == 0)
                nextMultiple = (*curorg)->time_alive;
            else
                nextMultiple = NEAT::time_alive_minimum * ((*curorg)->time_alive / NEAT::time_alive_minimum + 1);

            adjusted_fitness=-9999;
            (*curorg)->time_alive = nextMultiple;
        }

        if ( (adjusted_fitness<min_fitness)
            &&((*curorg)->time_alive
                >= static_cast<S32>(NEAT::time_alive_minimum) ))
        {
            min_fitness=adjusted_fitness;
            org_to_kill=(*curorg);
            deadorg=curorg;
            orgs_species = (*curorg)->species.lock();
        }
    }

    if (org_to_kill)
    {

        //Remove the organism from its species and the population
        orgs_species->remove_org(org_to_kill); //Remove from species
        organisms.erase(deadorg); //Remove from population list

        //Did the species become empty?
        if (orgs_species->organisms.size()==0)
        {
            remove_species(orgs_species);
        }
        //If not, re-estimate the species average after removing the organism
        else
        {
            orgs_species->estimate_average();
        }
    }

    return org_to_kill;
}

OrganismPtr Population::remove_worst_probabilistic()
{

    vector<OrganismPtr>::iterator curorg;
    OrganismPtr org_to_kill;
    vector<OrganismPtr>::iterator deadorg;
    SpeciesPtr orgs_species; //The species of the dead organism

    //Make sure the organism is deleted from its species and the population

    vector<OrganismPtr> sorted_adjusted_orgs;

    for (curorg = organisms.begin(); curorg != organisms.end(); ++curorg)
    {
        if ((*curorg)->time_alive >= static_cast<S32>(NEAT::time_alive_minimum) )
            sorted_adjusted_orgs.push_back(*curorg);
    }

    if (sorted_adjusted_orgs.size() == 0)
        return org_to_kill;

    sort(sorted_adjusted_orgs.begin(), sorted_adjusted_orgs.end(),
         order_orgs_by_adjusted_fit);

    S32
        size_bottom_10_percent = static_cast<S32>(ceil((F32)sorted_adjusted_orgs.size()
            * 0.10));
    S32 randorgnum = NEAT::randint(
        static_cast<S32>(sorted_adjusted_orgs.size() - size_bottom_10_percent), 
        static_cast<S32>(sorted_adjusted_orgs.size() - 1));

    curorg = sorted_adjusted_orgs.begin();
    curorg += randorgnum;
    org_to_kill = *curorg;
    orgs_species = (org_to_kill)->species.lock();

    curorg = organisms.begin();
    while (*curorg != org_to_kill)
    {
        ++curorg;
    }
    deadorg = curorg;

    if (org_to_kill)
    {
        //Remove the organism from its species and the population
        orgs_species->remove_org(org_to_kill); //Remove from species
        organisms.erase(deadorg); //Remove from population list

        //Did the species become empty?
        if ((orgs_species->organisms.size())==0)
        {

            remove_species(orgs_species);
        }
        //If not, re-estimate the species average after removing the organism
        else
        {
            orgs_species->estimate_average();
        }
    }

    return org_to_kill;
}

OrganismPtr Population::reproduce_champ(S32 generation)
{
    vector<OrganismPtr>::iterator curorg;
    F64 max_fitness=0;
    OrganismPtr champ;
    OrganismPtr baby;
    GenomePtr new_genome;
    vector<SpeciesPtr>::iterator curspecies;
    SpeciesPtr newspecies;
    OrganismPtr comporg;
    bool found;

    //The weight mutation power is species specific depending on its age
    F64 mut_power=NEAT::weight_mut_power;

    champ=*(organisms.begin()); //Make sure at least something is chosen
    //Find the population champ
    for (curorg = organisms.begin(); curorg != organisms.end(); ++curorg)
    {
        if (((*curorg)->fitness>max_fitness)&&((*curorg)->time_alive >= static_cast<S32>(NEAT::time_alive_minimum)))
        {
            champ=(*curorg);
            max_fitness=champ->fitness;
        }
    }

    //Now reproduce the pop champ as a new org	
    new_genome=(champ->gnome)->duplicate(generation);

    //Maybe mutate its link weights
    //if (randfloat()<NEAT::mutate_link_weights_prob) {  //KENHACK
    if (randfloat()<0.5)
    {
        //cout<<"mutate_link_weights"<<endl;
        new_genome->mutate_link_weights(mut_power, 1.0, GAUSSIAN);
    }

    baby.reset(new Organism(0.0,new_genome,generation));

    curspecies=(species).begin();
    if (curspecies==(species).end())
    {
        //Create the first species
        newspecies.reset(new Species(++(last_species),true));
        (species).push_back(newspecies);
        newspecies->add_Organism(baby); //Add the baby
        baby->species=newspecies; //Point the baby to its species
    }
    else
    {
        comporg=(*curspecies)->first();
        found=false;

        while ((curspecies!=(species).end()) && (!found))
        {
            if (!comporg)
            {
                //Keep searching for a matching species
                ++curspecies;
                if (curspecies!=(species).end())
                    comporg=(*curspecies)->first();
            }
            else if (((baby->gnome)->compatibility(comporg->gnome))
                <NEAT::compat_threshold)
            {
                //Found compatible species, so add this organism to it
                (*curspecies)->add_Organism(baby);
                baby->species=(*curspecies); //Point organism to its species
                found=true; //Note the search is over
            }
            else
            {
                //Keep searching for a matching species
                ++curspecies;
                if (curspecies!=(species).end())
                    comporg=(*curspecies)->first();
            }
        }

        //If we didn't find a match, create a new species
        if (found==false)
        {
            newspecies.reset(new Species(++(last_species),true));
            (species).push_back(newspecies);
            newspecies->add_Organism(baby); //Add the baby
            baby->species=newspecies; //Point baby to its species
        }

    } //end else     

    //Put the baby also in the master organism list
    (organisms).push_back(baby);

    cout << "CHAMPBABY --- species: "<< champ->species.lock()->id<< "\t fitness: "<< champ->fitness<< endl;

    cout << "----------------------------"<< endl;

    return baby; //Return a pointer to the baby

}

//KEN: New 2/17/04
//This method takes an Organism and reassigns what Species it belongs to
//It is meant to be used so that we can reasses where Organisms should belong
//as the speciation threshold changes.
void Population::reassign_species(OrganismPtr org)
{

    OrganismPtr comporg;
    bool found=false; //Note we don't really need this flag but it
    //might be useful if we change how this function works
    vector<SpeciesPtr>::iterator curspecies;
    SpeciesPtr newspecies;

    curspecies=(species).begin();

    comporg=(*curspecies)->first();
    assert(comporg->gnome);
    while ((curspecies!=(species).end()) && (!found))
    {
        if (!comporg)
        {
            //Keep searching for a matching species
            ++curspecies;
            if (curspecies!=(species).end())
                comporg=(*curspecies)->first();
        }
        else if (((org->gnome)->compatibility(comporg->gnome))<NEAT::compat_threshold)
        {
            //If we found the same species it's already in, return 0
            if ( *curspecies == org->species.lock() )
            {
                found=true;
                break;
            }
            //Found compatible species
            else
            {
                switch_species(org, org->species.lock(), (*curspecies));
                found=true; //Note the search is over
            }
        }
        else
        {
            //Keep searching for a matching species
            ++curspecies;
            if (curspecies!=(species).end())
                comporg=(*curspecies)->first();
        }
    }

    //If we didn't find a match, create a new species, move the org to
    // that species, check if the old species is empty, 
    //re-estimate averages, and return 0
    if (found==false)
    {

        //Create a new species for the org
        newspecies.reset(new Species(++(last_species),true));
        (species).push_back(newspecies);

        switch_species(org, org->species.lock(), newspecies);
    }
    
}

//Move an Organism from one Species to another
void Population::switch_species(OrganismPtr org, SpeciesPtr orig_species,
                                SpeciesPtr new_species)
{

    //Remove organism from the species we want to remove it from
    if (orig_species) orig_species->remove_org(org);

    //Add the organism to the new species it is being moved to
    new_species->add_Organism(org);
    org->species=new_species;

    //KEN: Delete orig_species if empty, and remove it from pop
    if (orig_species && orig_species->organisms.size() == 0)
    {

        remove_species(orig_species);

        //Re-estimate the average of the species that now has a new member
        new_species->estimate_average();
    }
    //If not, re-estimate the species average after removing the organism
    // AND the new species with the new member
    else
    {
        if (orig_species) orig_species->estimate_average();
        new_species->estimate_average();
    }
}

// Add an organism to the population and to the proper species.
void Population::add_organism(OrganismPtr org)
{
    vector<SpeciesPtr>::iterator curspecies; //For adding org
    SpeciesPtr newspecies; //For orgs in new Species
    OrganismPtr comporg; //For Species determination through comparison
    bool found; //When a Species is found

    curspecies = species.begin();
    if (curspecies == species.end())
    {
        //Create the first species
        newspecies.reset(new Species(++last_species,true));
        species.push_back(newspecies);
        newspecies->add_Organism(org); //Add the org
        org->species=newspecies; //Point the org to its species
    }
    else
    {
        comporg=(*curspecies)->first();
        assert(comporg->gnome);
        found=false;

        while ( curspecies != species.end() && (!found) )
        {
            if (!comporg)
            {
                //Keep searching for a matching species
                ++curspecies;
                if (curspecies!=species.end())
                    comporg=(*curspecies)->first();
            }
            else if (org->gnome->compatibility(comporg->gnome) < NEAT::compat_threshold )
            {
                //Found compatible species, so add this organism to it
                (*curspecies)->add_Organism(org);
                org->species=(*curspecies); //Point organism to its species
                found=true; //Note the search is over
            }
            else
            {
                //Keep searching for a matching species
                ++curspecies;
                if (curspecies!=species.end())
                    comporg=(*curspecies)->first();
            }
        }

        //If we didn't find a match, create a new species
        if (found==false)
        {
            newspecies.reset(new Species(++last_species,true));
            species.push_back(newspecies);
            newspecies->add_Organism(org); //Add the org
            org->species=newspecies; //Point org to its species
        }

    } //end else     

    //Put the org also in the master organism list
    organisms.push_back(org);
}
