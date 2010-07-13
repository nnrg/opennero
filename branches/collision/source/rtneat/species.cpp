#include "core/Common.h"
#include "species.h"
#include <vector>
#include <cmath>
#include <fstream>
#include <string>

using namespace NEAT;
using namespace std;

//used to give species console access
namespace SpeciesConsole
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

Species::Species(int i)
{
    SpeciesConsole::initConsole();
    id=i;
    age=1;
    ave_fitness=0.0;
    expected_offspring=0;
    checked=false;
    novel=false;
    age_of_last_improvement=0;
    max_fitness=0;
    max_fitness_ever=0;
    obliterate=false;

    average_est=0;
}

Species::Species(int i, bool n)
{
    SpeciesConsole::initConsole();
    id=i;
    age=1;
    ave_fitness=0.0;
    expected_offspring=0;
    checked=false;
    novel=n;
    age_of_last_improvement=0;
    max_fitness=0;
    max_fitness_ever=0;
    obliterate=false;

    average_est=0;
}

Species::~Species()
{
}

bool Species::rank()
{
    sort(organisms.begin(), organisms.end(), order_orgs);
    return true;
}

F64 Species::estimate_average()
{
    vector<OrganismPtr>::iterator curorg;
    F64 total = 0.0; //running total of fitnesses

    //Note: Since evolution is happening in real-time, some organisms may not
    //have been around long enough to count them in the fitness evaluation

    F64 num_orgs = 0; //counts number of orgs above the time_alive threshold


    for (curorg = organisms.begin(); curorg != organisms.end(); ++curorg)
    {
        //New variable time_alive
        if (((*curorg)->time_alive) >= static_cast<S32>(NEAT::time_alive_minimum))
        {
            total += (*curorg)->fitness;
            ++num_orgs;
            //Con::printf("Est.: Organism %d fitness %f",(*curorg)->gnome->genome_id, (*curorg)->fitness);
        }
    }

    if (num_orgs > 0)
        average_est = total / num_orgs;
    else
    {
        average_est = 0;
    }

    return average_est;
}

OrganismPtr Species::reproduce_one(S32 generation, PopulationPtr& pop,
                                   vector<SpeciesPtr> &sorted_species,
                                   bool addAdvice, Genome* adv)
{
    S32 count=generation; //This will assign genome id's according to the generation
    vector<OrganismPtr>::iterator curorg;

    vector<OrganismPtr> elig_orgs; //This list contains the eligible organisms (KEN)

    S32 poolsize; //The number of Organisms in the old generation

    OrganismPtr mom; //Parent Organisms
    OrganismPtr dad;
    OrganismPtr baby; //The new Organism

    GenomePtr new_genome; //For holding baby's genes

    vector<SpeciesPtr>::iterator curspecies; //For adding baby
    SpeciesPtr newspecies; //For babies in new Species
    OrganismPtr comporg; //For Species determination through comparison

    SpeciesPtr randspecies; //For mating outside the Species
    F64 randmult;
    S32 randspeciesnum;
    S32 spcount;
    vector<SpeciesPtr>::iterator cursp;

    bool outside;

    bool found; //When a Species is found

    S32 giveup; //For giving up finding a mate outside the species

    bool mut_struct_baby;
    bool mate_baby;

    //The weight mutation power is species specific depending on its age
    F64 mut_power=NEAT::weight_mut_power;

    //Roulette wheel variables
    F64 total_fitness=0.0;
    F64 marble; //The marble will have a number between 0 and total_fitness
    F64 spin; //Fitness total while the wheel is spinning

    //Check for a mistake
    assert(organisms.size() > 0);

    rank(); //Make sure organisms are ordered by rank

    //ADDED CODE (Ken) 
    //Now transfer the list to elig_orgs without including the ones that are too young (Ken)
    for (curorg=organisms.begin(); curorg!=organisms.end(); ++curorg)
    {
        if ((*curorg)->time_alive >= static_cast<S32>(NEAT::time_alive_minimum))
            elig_orgs.push_back(*curorg);
    }

    //Now elig_orgs should be an ordered list of mature organisms
    //Special case: if it's empty, then just include all the organisms (age doesn't matter in this case) (Ken)
    if (elig_orgs.size()==0)
    {
        for (curorg=organisms.begin(); curorg!=organisms.end(); ++curorg)
        {
            elig_orgs.push_back(*curorg);
        }
    }

    //Now elig_orgs is guaranteed to contain either an ordered list of mature orgs or all the orgs (Ken)
    //We may also want to check to see if we are getting pools of >1 organism (to make sure our survival_thresh is sensible) (Ken)

    //Only choose from among the top ranked orgs
    poolsize=static_cast<S32>((elig_orgs.size() - 1) * NEAT::survival_thresh);
    //Con::printf("elig_orgs = %i, poolsize = %i", elig_orgs.size(), poolsize);
    //poolsize=(organisms.size()-1)*.9;

    //Compute total fitness of species for a roulette wheel
    //Note: You don't get much advantage from a roulette here
    // because the size of a species is relatively small.
    // But you can use it by using the roulette code here
    for (curorg=elig_orgs.begin(); curorg!=elig_orgs.end(); ++curorg)
    {
        total_fitness+=(*curorg)->fitness;
    }

    //In reproducing only one offspring, the champ shouldn't matter  
    //thechamp=(*(organisms.begin()));

    //Create one offspring for the Species

    mut_struct_baby=false;
    mate_baby=false;

    outside=false;

    //First, decide whether to mate or mutate
    //If there is only one organism in the pool, then always mutate
    if ((randfloat()<NEAT::mutate_only_prob)||poolsize == 0)
    {

        //Choose the random parent

        ////Roulette Wheel
        marble=randfloat()*total_fitness;
        curorg=elig_orgs.begin();
        spin=(*curorg)->fitness;
        while (spin<marble)
        {
            ++curorg;

            //Keep the wheel spinning
            spin+=(*curorg)->fitness;
        }
        //Finished roulette


        mom=(*curorg);

        new_genome=(mom->gnome)->duplicate(count);

        //Do the mutation depending on probabilities of 
        //various mutations

        if (randfloat()<NEAT::mutate_add_node_prob)
        {
            //cout<<"mutate add node"<<endl;
            new_genome->mutate_add_node(pop->innovations, pop->cur_node_id,
                                        pop->cur_innov_num);
            mut_struct_baby=true;
        }
        else if (randfloat()<NEAT::mutate_add_link_prob)
        {
            //cout<<"mutate add link"<<endl;
            NetworkPtr net_analogue=new_genome->genesis(generation);
            new_genome->mutate_add_link(pop->innovations, pop->cur_innov_num,
                                        NEAT::newlink_tries);
            mut_struct_baby=true;
        }
        //NOTE:  A link CANNOT be added directly after a node was added because the phenotype
        //       will not be appropriately altered to reflect the change
        else
        {
            //If we didn't do a structural mutation, we do the other kinds

            if (randfloat()<NEAT::mutate_random_trait_prob)
            {
                //cout<<"mutate random trait"<<endl;
                new_genome->mutate_random_trait();
            }
            if (randfloat()<NEAT::mutate_link_trait_prob)
            {
                //cout<<"mutate_link_trait"<<endl;
                new_genome->mutate_link_trait(1);
            }
            if (randfloat()<NEAT::mutate_node_trait_prob)
            {
                //cout<<"mutate_node_trait"<<endl;
                new_genome->mutate_node_trait(1);
            }
            if (randfloat()<NEAT::mutate_link_weights_prob)
            {
                //cout<<"mutate_link_weights"<<endl;
                new_genome->mutate_link_weights(mut_power, 1.0, GAUSSIAN);
            }
            if (randfloat()<NEAT::mutate_toggle_enable_prob)
            {
                //cout<<"mutate toggle enable"<<endl;
                new_genome->mutate_toggle_enable(1);

            }
            if (randfloat()<NEAT::mutate_gene_reenable_prob)
            {
                //cout<<"mutate gene reenable"<<endl;
                new_genome->mutate_gene_reenable();
            }
        }

        baby.reset(new Organism(0.0,new_genome,generation));

    }

    //Otherwise we should mate 
    else
    {

        ////Roulette Wheel
        marble=randfloat()*total_fitness;
        curorg=elig_orgs.begin();
        spin=(*curorg)->fitness;
        while (spin<marble)
        {
            ++curorg;

            //Keep the wheel spinning
            spin+=(*curorg)->fitness;
        }
        //Finished roulette


        mom=(*curorg);

        //Choose random dad

        if ((randfloat()>NEAT::interspecies_mate_rate))
        {
            //Mate within Species


            ////Use a roulette wheel
            marble=randfloat()*total_fitness;
            curorg=elig_orgs.begin();
            spin=(*curorg)->fitness;
            while (spin<marble)
            {
                ++curorg;

                //Keep the wheel spinning
                spin+=(*curorg)->fitness;
            }
            ////Finished roulette


            dad=(*curorg);
        }
        else
        {

            //Mate outside Species  
            randspecies = shared_from_this();

            //Select a random species
            giveup=0; //Give up if you cant find a different Species
            while (randspecies.get() == this && giveup < 5)
            {

                //This old way just chose any old species
                //randspeciesnum=randint(0,(pop->species).size()-1);

                //Choose a random species tending towards better species
                randmult=gaussrand()/4;
                if (randmult>1.0)
                    randmult=1.0;
                //This tends to select better species
                randspeciesnum=(int) floor((randmult
                    *(sorted_species.size()-1.0))+0.5);
                cursp=(sorted_species.begin());
                for (spcount=0; spcount<randspeciesnum; spcount++)
                    ++cursp;
                randspecies=(*cursp);

                ++giveup;
            }

            //New way: Make dad be a champ from the random species
            dad=(*((randspecies->organisms).begin()));

            outside=true;
        }

        //Perform mating based on probabilities of differrent mating types
        if (randfloat()<NEAT::mate_multipoint_prob)
        {
            new_genome=(mom->gnome)->mate_multipoint(dad->gnome, count, mom->fitness,
                                             dad->fitness, outside);
        }
        else if (randfloat()<(NEAT::mate_multipoint_avg_prob
            /(NEAT::mate_multipoint_avg_prob+NEAT::mate_singlepoint_prob)))
        {
            new_genome=(mom->gnome)->mate_multipoint_avg(dad->gnome, count,
                                                 mom->fitness, dad->fitness,
                                                 outside);
        }
        else
        {
            new_genome=(mom->gnome)->mate_singlepoint(dad->gnome, count);
        }

        mate_baby=true;

        //Determine whether to mutate the baby's Genome
        //This is done randomly or if the mom and dad are the same organism
        if ((randfloat()>NEAT::mate_only_prob)||((dad->gnome)->genome_id==(mom->gnome)->genome_id)||(((dad->gnome)->compatibility(mom->gnome))==0.0))
        {

            //Do the mutation depending on probabilities of 
            //various mutations
            if (randfloat()<NEAT::mutate_add_node_prob)
            {
                new_genome->mutate_add_node(pop->innovations, pop->cur_node_id,
                                            pop->cur_innov_num);
                //  cout<<"mutate_add_node: "<<new_genome<<endl;
                mut_struct_baby=true;
            }
            else if (randfloat()<NEAT::mutate_add_link_prob)
            {
                NetworkPtr net_analogue=new_genome->genesis(generation);
                new_genome->mutate_add_link(pop->innovations,
                                            pop->cur_innov_num,
                                            NEAT::newlink_tries);
                mut_struct_baby=true;
            }
            else
            {
                //Only do other mutations when not doing strurctural mutations

                if (randfloat()<NEAT::mutate_random_trait_prob)
                {
                    new_genome->mutate_random_trait();
                    //cout<<"..mutate random trait: "<<new_genome<<endl;
                }
                if (randfloat()<NEAT::mutate_link_trait_prob)
                {
                    new_genome->mutate_link_trait(1);
                    //cout<<"..mutate link trait: "<<new_genome<<endl;
                }
                if (randfloat()<NEAT::mutate_node_trait_prob)
                {
                    new_genome->mutate_node_trait(1);
                    //cout<<"mutate_node_trait: "<<new_genome<<endl;
                }
                if (randfloat()<NEAT::mutate_link_weights_prob)
                {
                    new_genome->mutate_link_weights(mut_power, 1.0, GAUSSIAN);
                    //cout<<"mutate_link_weights: "<<new_genome<<endl;
                }
                if (randfloat()<NEAT::mutate_toggle_enable_prob)
                {
                    new_genome->mutate_toggle_enable(1);
                    //cout<<"mutate_toggle_enable: "<<new_genome<<endl;
                }
                if (randfloat()<NEAT::mutate_gene_reenable_prob)
                {
                    new_genome->mutate_gene_reenable();
                    //cout<<"mutate_gene_reenable: "<<new_genome<<endl;
                }
            }

            //Create the baby
            baby.reset(new Organism(0.0,new_genome,generation));

        }
        else
        {
            //Create the baby without mutating first
            baby.reset(new Organism(0.0,new_genome,generation));
        }

    }

    //Add the baby to its proper Species
    //If it doesn't fit a Species, create a new one

    baby->mut_struct_baby=mut_struct_baby;
    baby->mate_baby=mate_baby;

    curspecies = pop->species.begin();
    if (curspecies == pop->species.end())
    {
        //Create the first species
        newspecies.reset(new Species(++(pop->last_species),true));
        (pop->species).push_back(newspecies);
        newspecies->add_Organism(baby); //Add the baby
        baby->species=newspecies; //Point the baby to its species
    }
    else
    {
        comporg=(*curspecies)->first();
        assert(comporg->gnome);
        found=false;

        while ( curspecies != pop->species.end() && (!found) )
        {
            if (!comporg)
            {
                //Keep searching for a matching species
                ++curspecies;
                if (curspecies!=(pop->species).end())
                    comporg=(*curspecies)->first();
            }
            else if (baby->gnome->compatibility(comporg->gnome) < NEAT::compat_threshold )
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
                if (curspecies!=(pop->species).end())
                    comporg=(*curspecies)->first();
            }
        }

        //If we didn't find a match, create a new species
        if (found==false)
        {
            newspecies.reset(new Species(++(pop->last_species),true));
            (pop->species).push_back(newspecies);
            newspecies->add_Organism(baby); //Add the baby
            baby->species=newspecies; //Point baby to its species
        }

    } //end else     

    //Put the baby also in the master organism list
    (pop->organisms).push_back(baby);

    return baby; //Return a pointer to the baby
}

bool Species::add_Organism(OrganismPtr o)
{
    assert(o);
    assert(o->gnome);
    organisms.push_back(o);
    return true;
}

OrganismPtr Species::get_champ()
{
    F64 champ_fitness=-1.0;
    OrganismPtr thechamp;
    vector<OrganismPtr>::iterator curorg;

    for (curorg=organisms.begin(); curorg!=organisms.end(); ++curorg)
    {
        if (((*curorg)->fitness)>champ_fitness)
        {
            thechamp=(*curorg);
            champ_fitness=thechamp->fitness;
        }
    }

    return thechamp;

}

bool Species::remove_org(OrganismPtr org)
{
    vector<OrganismPtr>::iterator curorg;

    curorg=organisms.begin();
    while ((curorg!=organisms.end())&&((*curorg)!=org))
        ++curorg;

    assert(curorg != organisms.end());
    organisms.erase(curorg);
    return true;

}

OrganismPtr Species::first()
{
	assert(organisms.size() > 0);
    return *(organisms.begin());
}

bool Species::print_to_file(std::ofstream &outFile)
{
    vector<OrganismPtr>::iterator curorg;

    //Print a comment on the Species info
    outFile<<endl<<"/* Species #"<<id<<" : (Size "<<organisms.size()<<") (AF "
        <<ave_fitness<<") (Age "<<age<<")  */"<<endl<<endl;

    //Print all the Organisms' Genomes to the outFile
    for (curorg=organisms.begin(); curorg!=organisms.end(); ++curorg)
    {

        //Put the fitness for each organism in a comment
        outFile<<endl<<"/* Organism #"<<((*curorg)->gnome)->genome_id<<" Fitness: "<<(*curorg)->fitness<<" Error: "<<(*curorg)->error<<" */"<<endl;

        //If it is a winner, mark it in a comment
        if ((*curorg)->winner)
        {
            outFile<<"/* ##------$ WINNER "<<((*curorg)->gnome)->genome_id<<" SPECIES #"<<id
                <<" $------## */"<<endl;
        }

        ((*curorg)->gnome)->print_to_file(outFile);
    }

    outFile << endl << endl;

    return true;

}

void Species::adjust_fitness()
{
    vector<OrganismPtr>::iterator curorg;

    S32 num_parents;
    S32 count;

    S32 age_debt;

    //cout<<"Species "<<id<<" last improved "<<(age-age_of_last_improvement)<<" steps ago when it moved up to "<<max_fitness_ever<<endl;

    age_debt=(age-age_of_last_improvement+1)-NEAT::dropoff_age;

    if (age_debt==0)
        age_debt=1;

    for (curorg=organisms.begin(); curorg!=organisms.end(); ++curorg)
    {

        //Remember the original fitness before it gets modified
        (*curorg)->orig_fitness=(*curorg)->fitness;

        //Make fitness decrease after a stagnation point dropoff_age
        //Added an if to keep species pristine until the dropoff point
        //obliterate is used in competitive coevolution to mark stagnation
        //by obliterating the worst species over a certain age
        if ((age_debt>=1)||obliterate)
        {

            //Possible graded dropoff
            //((*curorg)->fitness)=((*curorg)->fitness)*(-atan(age_debt));

            //Extreme penalty for a long period of stagnation (divide fitness by 100)
            ((*curorg)->fitness)=((*curorg)->fitness)*0.01;
            //cout<<"OBLITERATE Species "<<id<<" of age "<<age<<endl;
            //cout<<"dropped fitness to "<<((*curorg)->fitness)<<endl;
        }

        //Give a fitness boost up to some young age (niching)
        //The age_significance parameter is a system parameter
        //  if it is 1, then young species get no fitness boost
        if (age<=10)
            ((*curorg)->fitness)=((*curorg)->fitness)*NEAT::age_significance;

        //Do not allow negative fitness
        if (((*curorg)->fitness)<0.0)
            (*curorg)->fitness=0.0001;

        //Share fitness with the species
        (*curorg)->fitness=((*curorg)->fitness)/(organisms.size());

    }

    //Sort the population and mark for death those after survival_thresh*pop_size
    sort(organisms.begin(), organisms.end(), order_orgs);

    //Update age_of_last_improvement here
    if (((*(organisms.begin()))->orig_fitness)>max_fitness_ever)
    {
        age_of_last_improvement=age;
        max_fitness_ever=((*(organisms.begin()))->orig_fitness);
    }

    //Decide how many get to reproduce based on survival_thresh*pop_size
    //Adding 1.0 ensures that at least one will survive
    num_parents=(int) floor((NEAT::survival_thresh*((double) organisms.size()))
        +1.0);

    //Mark for death those who are ranked too low to be parents
    curorg=organisms.begin();
    (*curorg)->champion=true; //Mark the champ as such
    for (count=1; count<=num_parents; count++)
    {
        if (curorg!=organisms.end())
            ++curorg;
    }
    while (curorg!=organisms.end())
    {
        (*curorg)->eliminate=true; //Mark for elimination
        ++curorg;
    }

}

F64 Species::compute_average_fitness()
{
    vector<OrganismPtr>::iterator curorg;

    F64 total=0.0;

    for (curorg=organisms.begin(); curorg!=organisms.end(); ++curorg)
    {
        total+=(*curorg)->fitness;
    }

    ave_fitness=total/(organisms.size());

    return ave_fitness;

}

F64 Species::compute_max_fitness()
{
    F64 max=0.0;
    vector<OrganismPtr>::iterator curorg;

    for (curorg=organisms.begin(); curorg!=organisms.end(); ++curorg)
    {
        if (((*curorg)->fitness)>max)
            max=(*curorg)->fitness;
    }

    max_fitness=max;

    return max;
}

F64 Species::count_offspring(F64 skim)
{
    vector<OrganismPtr>::iterator curorg;
    S32 e_o_intpart; //The floor of an organism's expected offspring
    F64 e_o_fracpart; //Expected offspring fractional part
    F64 skim_intpart; //The whole offspring in the skim

    expected_offspring=0;

    for (curorg=organisms.begin(); curorg!=organisms.end(); ++curorg)
    {
        e_o_intpart=(int) floor((*curorg)->expected_offspring);
        e_o_fracpart=fmod((*curorg)->expected_offspring, 1.0);

        expected_offspring+=e_o_intpart;

        //Skim off the fractional offspring
        skim+=e_o_fracpart;

        //NOTE:  Some precision is lost by computer
        //       Must be remedied later
        if (skim>1.0)
        {
            skim_intpart=floor(skim);
            expected_offspring+=(int) skim_intpart;
            skim-=skim_intpart;
        }
    }

    return skim;

}

bool Species::reproduce(S32 generation, PopulationPtr pop,
                        vector<SpeciesPtr> &sorted_species)
{
    S32 count;
    vector<OrganismPtr>::iterator curorg;

    S32 poolsize; //The number of Organisms in the old generation

    S32 orgnum; //Random variable
    S32 orgcount;
    OrganismPtr mom; //Parent Organisms
    OrganismPtr dad;
    OrganismPtr baby; //The new Organism

    GenomePtr new_genome; //For holding baby's genes

    vector<SpeciesPtr>::iterator curspecies; //For adding baby
    SpeciesPtr newspecies; //For babies in new Species
    OrganismPtr comporg; //For Species determination through comparison

    SpeciesPtr randspecies; //For mating outside the Species
    F64 randmult;
    S32 randspeciesnum;
    S32 spcount;
    vector<SpeciesPtr>::iterator cursp;

    bool outside;

    bool found; //When a Species is found

    bool champ_done=false; //Flag the preservation of the champion  

    OrganismPtr thechamp;

    S32 giveup; //For giving up finding a mate outside the species

    bool mut_struct_baby;
    bool mate_baby;

    //The weight mutation power is species specific depending on its age
    F64 mut_power=NEAT::weight_mut_power;

    //Check for a mistake
    if ((expected_offspring>0)&&(organisms.size()==0))
    {
        //    cout<<"ERROR:  ATTEMPT TO REPRODUCE OUT OF EMPTY SPECIES"<<endl;
        return false;
    }

    poolsize=static_cast<S32>(organisms.size()-1);

    thechamp=(*(organisms.begin()));

    //Create the designated number of offspring for the Species
    //one at a time
    for (count=0; count<expected_offspring; count++)
    {

        mut_struct_baby=false;
        mate_baby=false;

        outside=false;

        //Debug Trap
        if (expected_offspring>NEAT::pop_size)
        {
            //      cout<<"ALERT: EXPECTED OFFSPRING = "<<expected_offspring<<endl;
            //      cin>>pause;
        }

        //If we have a super_champ (Population champion), finish off some special clones
        if ((thechamp->super_champ_offspring) > 0)
        {
            mom=thechamp;
            new_genome=(mom->gnome)->duplicate(count);

            if ((thechamp->super_champ_offspring) == 1)
            {

            }

            //Most superchamp offspring will have their connection weights mutated only
            //The last offspring will be an exact duplicate of this super_champ
            //Note: Superchamp offspring only occur with stolen babies!
            //      Settings used for published experiments did not use this
            if ((thechamp->super_champ_offspring) > 1)
            {
                if ((randfloat()<0.8)||(NEAT::mutate_add_link_prob==0.0))
                    //ABOVE LINE IS FOR:
                    //Make sure no links get added when the system has link adding disabled
                    new_genome->mutate_link_weights(mut_power, 1.0, GAUSSIAN);
                else
                {
                    //Sometimes we add a link to a superchamp
                    NetworkPtr net_analogue=new_genome->genesis(generation);
                    new_genome->mutate_add_link(pop->innovations,
                                                pop->cur_innov_num,
                                                NEAT::newlink_tries);
                    mut_struct_baby=true;
                }
            }

            baby.reset(new Organism(0.0,new_genome,generation));

            if ((thechamp->super_champ_offspring) == 1)
            {
                if (thechamp->pop_champ)
                {
                    //cout<<"The new org baby's genome is "<<baby->gnome<<endl;
                    baby->pop_champ_child=true;
                    baby->high_fit=mom->orig_fitness;
                }
            }

            thechamp->super_champ_offspring--;
        }
        //If we have a Species champion, just clone it 
        else if ((!champ_done)&&(expected_offspring>5))
        {

            mom=thechamp; //Mom is the champ

            new_genome=(mom->gnome)->duplicate(count);

            baby.reset(new Organism(0.0,new_genome,generation)); //Baby is just like mommy

            champ_done=true;

        }
        //First, decide whether to mate or mutate
        //If there is only one organism in the pool, then always mutate
        else if ((randfloat()<NEAT::mutate_only_prob)||poolsize== 0)
        {

            //Choose the random parent

            //RANDOM PARENT CHOOSER
            orgnum=randint(0, poolsize);
            curorg=organisms.begin();
            for (orgcount=0; orgcount<orgnum; orgcount++)
                ++curorg;

            ////Roulette Wheel
            //marble=randfloat()*total_fitness;
            //curorg=organisms.begin();
            //spin=(*curorg)->fitness;
            //while(spin<marble) {
            //++curorg;

            ////Keep the wheel spinning
            //spin+=(*curorg)->fitness;
            //}
            ////Finished roulette
            //

            mom=(*curorg);

            new_genome=(mom->gnome)->duplicate(count);

            //Do the mutation depending on probabilities of 
            //various mutations

            if (randfloat()<NEAT::mutate_add_node_prob)
            {
                //cout<<"mutate add node"<<endl;
                new_genome->mutate_add_node(pop->innovations, pop->cur_node_id,
                                            pop->cur_innov_num);
                mut_struct_baby=true;
            }
            else if (randfloat()<NEAT::mutate_add_link_prob)
            {
                //cout<<"mutate add link"<<endl;
                NetworkPtr net_analogue(new_genome->genesis(generation));
                new_genome->mutate_add_link(pop->innovations,
                                            pop->cur_innov_num,
                                            NEAT::newlink_tries);
                mut_struct_baby=true;
            }
            //NOTE:  A link CANNOT be added directly after a node was added because the phenotype
            //       will not be appropriately altered to reflect the change
            else
            {
                //If we didn't do a structural mutation, we do the other kinds

                if (randfloat()<NEAT::mutate_random_trait_prob)
                {
                    //cout<<"mutate random trait"<<endl;
                    new_genome->mutate_random_trait();
                }
                if (randfloat()<NEAT::mutate_link_trait_prob)
                {
                    //cout<<"mutate_link_trait"<<endl;
                    new_genome->mutate_link_trait(1);
                }
                if (randfloat()<NEAT::mutate_node_trait_prob)
                {
                    //cout<<"mutate_node_trait"<<endl;
                    new_genome->mutate_node_trait(1);
                }
                if (randfloat()<NEAT::mutate_link_weights_prob)
                {
                    //cout<<"mutate_link_weights"<<endl;
                    new_genome->mutate_link_weights(mut_power, 1.0, GAUSSIAN);
                }
                if (randfloat()<NEAT::mutate_toggle_enable_prob)
                {
                    //cout<<"mutate toggle enable"<<endl;
                    new_genome->mutate_toggle_enable(1);

                }
                if (randfloat()<NEAT::mutate_gene_reenable_prob)
                {
                    //cout<<"mutate gene reenable"<<endl;
                    new_genome->mutate_gene_reenable();
                }
            }

            baby.reset(new Organism(0.0,new_genome,generation));

        }

        //Otherwise we should mate 
        else
        {

            //Choose the random mom
            orgnum=randint(0, poolsize);
            curorg=organisms.begin();
            for (orgcount=0; orgcount<orgnum; orgcount++)
                ++curorg;

            ////Roulette Wheel
            //marble=randfloat()*total_fitness;
            //curorg=organisms.begin();
            //spin=(*curorg)->fitness;
            //while(spin<marble) {
            //++curorg;

            ////Keep the wheel spinning
            //spin+=(*curorg)->fitness;
            //}
            ////Finished roulette
            //

            mom=(*curorg);

            //Choose random dad

            if ((randfloat()>NEAT::interspecies_mate_rate))
            {
                //Mate within Species

                orgnum=randint(0, poolsize);
                curorg=organisms.begin();
                for (orgcount=0; orgcount<orgnum; orgcount++)
                    ++curorg;

                ////Use a roulette wheel
                //marble=randfloat()*total_fitness;
                //curorg=organisms.begin();
                //spin=(*curorg)->fitness;
                //while(spin<marble) {
                //++curorg;
                //}

                ////Keep the wheel spinning
                //spin+=(*curorg)->fitness;
                //}
                ////Finished roulette
                //

                dad=(*curorg);
            }
            else
            {

                //Mate outside Species  
                randspecies = shared_from_this();

                //Select a random species
                giveup=0; //Give up if you cant find a different Species
                while ( randspecies.get() == this && giveup < 5 )
                {

                    //This old way just chose any old species
                    //randspeciesnum=randint(0,(pop->species).size()-1);

                    //Choose a random species tending towards better species
                    randmult=gaussrand()/4;
                    if (randmult>1.0)
                        randmult=1.0;
                    //This tends to select better species
                    randspeciesnum=(int) floor((randmult*(sorted_species.size()
                        -1.0))+0.5);
                    cursp=(sorted_species.begin());
                    for (spcount=0; spcount<randspeciesnum; spcount++)
                        ++cursp;
                    randspecies=(*cursp);

                    ++giveup;
                }

                //New way: Make dad be a champ from the random species
                dad=(*((randspecies->organisms).begin()));

                outside=true;
            }

            //Perform mating based on probabilities of differrent mating types
            if (randfloat()<NEAT::mate_multipoint_prob)
            {
                new_genome=(mom->gnome)->mate_multipoint(dad->gnome, count,
                                                 mom->fitness, dad->fitness,
                                                 outside);
            }
            else if (randfloat()<(NEAT::mate_multipoint_avg_prob
                /(NEAT::mate_multipoint_avg_prob+NEAT::mate_singlepoint_prob)))
            {
                new_genome=(mom->gnome)->mate_multipoint_avg(dad->gnome, count,
                                                     mom->fitness,
                                                     dad->fitness, outside);
            }
            else
            {
                new_genome=(mom->gnome)->mate_singlepoint(dad->gnome, count);
            }

            mate_baby=true;

            //Determine whether to mutate the baby's Genome
            //This is done randomly or if the mom and dad are the same organism
            if ((randfloat()>NEAT::mate_only_prob)||((dad->gnome)->genome_id==(mom->gnome)->genome_id)||(((dad->gnome)->compatibility(mom->gnome))==0.0))
            {

                //Do the mutation depending on probabilities of 
                //various mutations
                if (randfloat()<NEAT::mutate_add_node_prob)
                {
                    new_genome->mutate_add_node(pop->innovations,
                                                pop->cur_node_id,
                                                pop->cur_innov_num);
                    //  cout<<"mutate_add_node: "<<new_genome<<endl;
                    mut_struct_baby=true;
                }
                else if (randfloat()<NEAT::mutate_add_link_prob)
                {
                    NetworkPtr net_analogue=new_genome->genesis(generation);
                    new_genome->mutate_add_link(pop->innovations,
                                                pop->cur_innov_num,
                                                NEAT::newlink_tries);
                    mut_struct_baby=true;
                }
                else
                {
                    //Only do other mutations when not doing sturctural mutations

                    if (randfloat()<NEAT::mutate_random_trait_prob)
                    {
                        new_genome->mutate_random_trait();
                        //cout<<"..mutate random trait: "<<new_genome<<endl;
                    }
                    if (randfloat()<NEAT::mutate_link_trait_prob)
                    {
                        new_genome->mutate_link_trait(1);
                        //cout<<"..mutate link trait: "<<new_genome<<endl;
                    }
                    if (randfloat()<NEAT::mutate_node_trait_prob)
                    {
                        new_genome->mutate_node_trait(1);
                        //cout<<"mutate_node_trait: "<<new_genome<<endl;
                    }
                    if (randfloat()<NEAT::mutate_link_weights_prob)
                    {
                        new_genome->mutate_link_weights(mut_power, 1.0,
                                                        GAUSSIAN);
                        //cout<<"mutate_link_weights: "<<new_genome<<endl;
                    }
                    if (randfloat()<NEAT::mutate_toggle_enable_prob)
                    {
                        new_genome->mutate_toggle_enable(1);
                        //cout<<"mutate_toggle_enable: "<<new_genome<<endl;
                    }
                    if (randfloat()<NEAT::mutate_gene_reenable_prob)
                    {
                        new_genome->mutate_gene_reenable();
                        //cout<<"mutate_gene_reenable: "<<new_genome<<endl;
                    }
                }

                //Create the baby
                baby.reset(new Organism(0.0,new_genome,generation));

            }
            else
            {
                //Create the baby without mutating first
                baby.reset(new Organism(0.0,new_genome,generation));
            }

        }

        //Add the baby to its proper Species
        //If it doesn't fit a Species, create a new one

        baby->mut_struct_baby=mut_struct_baby;
        baby->mate_baby=mate_baby;

        curspecies=(pop->species).begin();
        if (curspecies==(pop->species).end())
        {
            //Create the first species
            newspecies.reset(new Species(++(pop->last_species),true));
            (pop->species).push_back(newspecies);
            newspecies->add_Organism(baby); //Add the baby
            baby->species=newspecies; //Point the baby to its species
        }
        else
        {
            comporg=(*curspecies)->first();
            found=false;
            while ((curspecies!=(pop->species).end())&&(!found))
            {
                if (!comporg)
                {
                    //Keep searching for a matching species
                    ++curspecies;
                    if (curspecies!=(pop->species).end())
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
                    if (curspecies!=(pop->species).end())
                        comporg=(*curspecies)->first();
                }
            }

            //If we didn't find a match, create a new species
            if (found==false)
            {
                newspecies.reset(new Species(++(pop->last_species),true));
                (pop->species).push_back(newspecies);
                newspecies->add_Organism(baby); //Add the baby
                baby->species=newspecies; //Point baby to its species
            }

        } //end else 

    }
    return true;
}

bool NEAT::order_species(SpeciesPtr x, SpeciesPtr y)
{
    return (((*((x->organisms).begin()))->orig_fitness) > ((*((y->organisms).begin()))->orig_fitness));
}

bool NEAT::order_new_species(SpeciesPtr x, SpeciesPtr y)
{
    return (x->compute_max_fitness() > y->compute_max_fitness());
}

