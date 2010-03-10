#include "core/Common.h"
#include "pool.h"
#include <vector>
#include <cmath>
#include <fstream>

using namespace NEAT;
using namespace std;

//used to give pool console access
namespace PoolConsole
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

Pool::Pool() :
    id(0), age(1), organisms(0), average_est(0)
{
}

Pool::~Pool()
{
}

bool Pool::rank()
{
    sort(organisms.begin(), organisms.end(), order_orgs);
    return true;
}

OrganismPtr Pool::reproduce_one(S32 generation, PopulationPtr pop,
                                vector<SpeciesPtr> &sorted_species)
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

    NetworkPtr net_analogue; //For adding link to test for recurrency

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
    if ((organisms.size()==0))
    {
        return baby;
    }

    rank(); //Make sure organisms are ordered by rank

    //ADDED CODE (Ken) 
    //Now transfer the list to elig_orgs without including the ones that are too young (Ken)
    //for(curorg=organisms.begin();curorg!=organisms.end();++curorg) {
    //	if ((*curorg)->time_alive >= NEAT::time_alive_minimum)
    //		elig_orgs.push_back(*curorg);
    //}

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
    if ((false)&&poolsize == 0)
    {

        //Choose the random parent

        //RANDOM PARENT CHOOSER
        //orgnum=randint(0,poolsize);
        //curorg=elig_orgs.begin();
        //for(orgcount=0;orgcount<orgnum;orgcount++)
        //	++curorg;                       


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


        mom = *curorg;

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
            net_analogue=new_genome->genesis(generation);
            new_genome->mutate_add_link(pop->innovations, pop->cur_innov_num,
                                        NEAT::newlink_tries);
            net_analogue.reset();
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
        //orgnum=randint(0,poolsize);
        //curorg=elig_orgs.begin();
        //for(orgcount=0;orgcount<orgnum;orgcount++)
        //	++curorg;


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


        mom = *curorg;

        //Choose random dad

        if ((false))
        {
            //Mate within Species

            //orgnum=randint(0,poolsize);
            //curorg=elig_orgs.begin();
            //for(orgcount=0;orgcount<orgnum;orgcount++)
            //	++curorg;


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


            dad = *curorg;
        }
        else
        {

            //Mate outside Species  
            randspecies=*sorted_species.begin();

            //Select a random species
            giveup=0; //Give up if you cant find a different Species
            while ((randspecies==*(sorted_species.begin()))&&(giveup<5))
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
            dad = *((randspecies->organisms).begin());

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
                net_analogue=new_genome->genesis(generation);
                new_genome->mutate_add_link(pop->innovations,
                                            pop->cur_innov_num,
                                            NEAT::newlink_tries);
                net_analogue.reset();
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

        while ((curspecies!=(pop->species).end()) && (!found))
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

    //Put the baby also in the master organism list
    (pop->organisms).push_back(baby);

    return baby; //Return a pointer to the baby
}

bool Pool::add_Organism(OrganismPtr o)
{
    organisms.push_back(o);
    return true;
}

bool Pool::remove_org(OrganismPtr org)
{
    vector<OrganismPtr>::iterator curorg;

    curorg=organisms.begin();
    while ((curorg!=organisms.end())&&((*curorg)!=org))
        ++curorg;

    if (curorg==organisms.end())
    {
        //cout<<"ALERT: Attempt to remove nonexistent Organism from Species"<<endl;
        return false;
    }
    else
    {
        organisms.erase(curorg);
        return true;
    }

}

OrganismPtr Pool::first()
{
    return *(organisms.begin());
}

bool Pool::print_to_file(std::ofstream &outFile)
{
    vector<OrganismPtr>::iterator curorg;

    //Print a comment on the Species info
    outFile << endl<<"/* Species #"<<id<<" : (Size "<<organisms.size()
        <<") (AF "<<average_est<<") (Age "<<age<<")  */"<<endl<<endl;

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
        //We can confirm by writing the genome #'s to the screen
        cout<<((*curorg)->gnome)->genome_id<<endl;
    }
    outFile << endl << endl;

    return true;

}

