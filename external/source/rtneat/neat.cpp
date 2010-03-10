#include "core/Common.h"
#include <fstream>
#include <cmath>
#include <iostream>
#include <string>
#include "neat.h"
#include "XMLSerializable.h"

using namespace std;

namespace NEAT
{
    U32 time_alive_minimum = 20;
    F64 trait_param_mut_prob = 0;
    F64 trait_mutation_power = 0; // Power of mutation on a signle trait param 
    F64 linktrait_mut_sig = 0; // Amount that mutation_num changes for a trait change inside a link
    F64 nodetrait_mut_sig = 0; // Amount a mutation_num changes on a link connecting a node that changed its trait 
    F64 weight_mut_power = 0; // The power of a linkweight mutation 
    F64 recur_prob = 0; // Prob. that a link mutation which doesn't have to be recurrent will be made recurrent 
    F64 disjoint_coeff = 0;
    F64 excess_coeff = 0;
    F64 mutdiff_coeff = 0;
    F64 compat_threshold = 0;
    F64 age_significance = 0; // How much does age matter? 
    F64 survival_thresh = 0; // Percent of ave fitness for survival 
    F64 mutate_only_prob = 0; // Prob. of a non-mating reproduction 
    F64 mutate_random_trait_prob = 0;
    F64 mutate_link_trait_prob = 0;
    F64 mutate_node_trait_prob = 0;
    F64 mutate_link_weights_prob = 0;
    F64 mutate_toggle_enable_prob = 0;
    F64 mutate_gene_reenable_prob = 0;
    F64 mutate_add_node_prob = 0;
    F64 mutate_add_link_prob = 0;
    F64 interspecies_mate_rate = 0; // Prob. of a mate being outside species 
    F64 mate_multipoint_prob = 0;
    F64 mate_multipoint_avg_prob = 0;
    F64 mate_singlepoint_prob = 0;
    F64 mate_only_prob = 0; // Prob. of mating without mutation 
    F64 recur_only_prob = 0; // Probability of forcing selection of ONLY links that are naturally recurrent 
    S32 pop_size = 0; // Size of population 
    S32 dropoff_age = 0; // Age where Species starts to be penalized 
    S32 newlink_tries = 0; // Number of tries mutate_add_link will attempt to find an open link 
    S32 print_every = 0; // Tells to print population to file every n generations 
    S32 babies_stolen = 0; // The number of babies to siphen off to the champions 
    F64 backprop_learning_rate = 0; // Learning rate of back-propagation algorithm
    MTRand NEATRandGen((U64)0); //TODO: we should probably move the Mersenne Twister random generator to OpenNero common

    bool load_neat_params(const string& filename)
    {

        ifstream paramFile(filename.c_str(), ios::in);
        if (!paramFile)
        {
            return false;
        }
        // **********LOAD IN PARAMETERS*************** //
        cout << "NEAT READING IN "<< filename << endl;
	paramFile.ignore(256,' ');
        paramFile >> trait_param_mut_prob;
	paramFile.ignore(256,' ');
        paramFile >> trait_mutation_power;
	paramFile.ignore(256,' ');
        paramFile >> linktrait_mut_sig;
	paramFile.ignore(256,' ');
        paramFile >> nodetrait_mut_sig;
	paramFile.ignore(256,' ');
        paramFile >> weight_mut_power;
	paramFile.ignore(256,' ');
        paramFile >> recur_prob;
	paramFile.ignore(256,' ');
        paramFile >> disjoint_coeff;
	paramFile.ignore(256,' ');
        paramFile >> excess_coeff;
	paramFile.ignore(256,' ');
	paramFile >> mutdiff_coeff;
	paramFile.ignore(256,' ');
        paramFile >> compat_threshold;
	paramFile.ignore(256,' ');
        paramFile >> age_significance;
	paramFile.ignore(256,' ');
        paramFile >> survival_thresh;
	paramFile.ignore(256,' ');
        paramFile >> mutate_only_prob;
	paramFile.ignore(256,' ');
        paramFile >> mutate_random_trait_prob;
	paramFile.ignore(256,' ');
        paramFile >> mutate_link_trait_prob;
	paramFile.ignore(256,' ');
        paramFile >> mutate_node_trait_prob;
	paramFile.ignore(256,' ');
        paramFile >> mutate_link_weights_prob;
	paramFile.ignore(256,' ');
        paramFile >> mutate_toggle_enable_prob;
	paramFile.ignore(256,' ');
        paramFile >> mutate_gene_reenable_prob;
	paramFile.ignore(256,' ');
        paramFile >> mutate_add_node_prob;
	paramFile.ignore(256,' ');
        paramFile >> mutate_add_link_prob;
	paramFile.ignore(256,' ');
        paramFile >> interspecies_mate_rate;
	paramFile.ignore(256,' ');
        paramFile >> mate_multipoint_prob;
	paramFile.ignore(256,' ');
        paramFile >> mate_multipoint_avg_prob;
	paramFile.ignore(256,' ');
        paramFile >> mate_singlepoint_prob;
	paramFile.ignore(256,' ');
        paramFile >> mate_only_prob;
	paramFile.ignore(256,' ');
        paramFile >> recur_only_prob;
	paramFile.ignore(256,' ');
        paramFile >> pop_size;
	paramFile.ignore(256,' ');
        paramFile >> dropoff_age;
	paramFile.ignore(256,' ');
        paramFile >> newlink_tries;
	paramFile.ignore(256,' ');
        paramFile >> print_every;
	paramFile.ignore(256,' ');
        paramFile >> babies_stolen;
	paramFile.ignore(256,' ');
        paramFile >> backprop_learning_rate;
        cout << "trait_param_mut_prob="<< trait_param_mut_prob << endl;
        cout << "trait_mutation_power="<< trait_mutation_power << endl;
        cout << "linktrait_mut_sig="<< linktrait_mut_sig << endl;
        cout << "nodetrait_mut_sig="<< nodetrait_mut_sig << endl;
        cout << "weight_mut_power="<< weight_mut_power << endl;
        cout << "recur_prob="<< recur_prob << endl;
        cout << "disjoint_coeff="<< disjoint_coeff << endl;
        cout << "excess_coeff="<< excess_coeff << endl;
        cout << "mutdiff_coeff="<< mutdiff_coeff << endl;
        cout << "compat_threshold="<< compat_threshold << endl;
        cout << "age_significance="<< age_significance << endl;
        cout << "survival_thresh="<< survival_thresh << endl;
        cout << "mutate_only_prob="<< mutate_only_prob << endl;
        cout << "mutate_random_trait_prob="<< mutate_random_trait_prob << endl;
        cout << "mutate_link_trait_prob="<< mutate_link_trait_prob << endl;
        cout << "mutate_node_trait_prob="<< mutate_node_trait_prob << endl;
        cout << "mutate_link_weights_prob="<< mutate_link_weights_prob << endl;
        cout << "mutate_toggle_enable_prob="<< mutate_toggle_enable_prob << endl;
        cout << "mutate_gene_reenable_prob="<< mutate_gene_reenable_prob << endl;
        cout << "mutate_add_node_prob="<< mutate_add_node_prob << endl;
        cout << "mutate_add_link_prob="<< mutate_add_link_prob << endl;
        cout << "interspecies_mate_rate="<< interspecies_mate_rate << endl;
        cout << "mate_multipoint_prob="<< mate_multipoint_prob << endl;
        cout << "mate_multipoint_avg_prob="<< mate_multipoint_avg_prob << endl;
        cout << "mate_singlepoint_prob="<< mate_singlepoint_prob << endl;
        cout << "mate_only_prob="<< mate_only_prob << endl;
        cout << "recur_only_prob="<< recur_only_prob << endl;
        cout << "pop_size="<< pop_size << endl;
        cout << "dropoff_age="<< dropoff_age << endl;
        cout << "newlink_tries="<< newlink_tries << endl;
        cout << "print_every="<< print_every << endl;
        cout << "babies_stolen="<< babies_stolen << endl;
        cout << "backprop_learning_rate="<< backprop_learning_rate << endl;
        paramFile.close();
        return true;
    }

    /// Random number from the normal Gaussian distribution with mean 0 and variance 1
    /// Uses the mersenne twister implementation
    F64 gaussrand()
    {
        return NEATRandGen.randNorm(0, 1);
    }

    F64 fsigmoid(F64 activesum, F64 slope, F64 constant)
    {
        //RIGHT SHIFTED ---------------------------------------------------------
        //return (1/(1+(exp(-(slope*activesum-constant))))); //ave 3213 clean on 40 runs of p2m and 3468 on another 40 
        //41394 with 1 failure on 8 runs

        //LEFT SHIFTED ----------------------------------------------------------
        //return (1/(1+(exp(-(slope*activesum+constant))))); //original setting ave 3423 on 40 runs of p2m, 3729 and 1 failure also

        //PLAIN SIGMOID ---------------------------------------------------------
        return (1/(1+(exp(-activesum)))); //3511 and 1 failure

        //LEFT SHIFTED NON-STEEPENED---------------------------------------------
        //return (1/(1+(exp(-activesum-constant)))); //simple left shifted

        //NON-SHIFTED STEEPENED
        //return (1/(1+(exp(-(slope*activesum))))); //Compressed
    }

    F64 flinear(F64 activesum, F64 slope, F64 constant)
    {
        // Linear
        //return (slope*activesum+constant);

        // Identity
        return activesum;
    }

    F64 hebbian(F64 weight, F64 maxweight, F64 active_in, F64 active_out,
                      F64 hebb_rate, F64 pre_rate, F64 post_rate)
    {

        bool neg=false;
        F64 delta;

        F64 topweight;

        if (maxweight<5.0)
            maxweight=5.0;

        if (weight>maxweight)
            weight=maxweight;

        if (weight<-maxweight)
            weight=-maxweight;

        if (weight<0)
        {
            neg=true;
            weight=-weight;
        }

        //if (weight<0) {
        //  weight_mag=-weight;
        //}
        //else weight_mag=weight;


        topweight=weight+2.0;
        if (topweight>maxweight)
            topweight=maxweight;

        if (!(neg))
        {
            //if (true) {
            delta=hebb_rate*(maxweight-weight)*active_in*active_out+pre_rate
                *(topweight)*active_in*(active_out-1.0);
            //post_rate*(weight+1.0)*(active_in-1.0)*active_out;

            //delta=delta-hebb_rate/2; //decay

            //delta=delta+randposneg()*randfloat()*0.01; //noise

            //cout<<"delta: "<<delta<<endl;

            //if (weight+delta>0)
            //  return weight+delta;
            //else return 0.01;

            return weight+delta;

        }
        else
        {
            //In the inhibatory case, we strengthen the synapse when output is low and
            //input is high
            delta=pre_rate*(maxweight-weight)*active_in*(1.0-active_out)+ //"unhebb"
                //hebb_rate*(maxweight-weight)*(1.0-active_in)*(active_out)+
                -hebb_rate*(topweight+2.0)*active_in*active_out+ //anti-hebbian
                //hebb_rate*(maxweight-weight)*active_in*active_out+
                //pre_rate*weight*active_in*(active_out-1.0)+
                //post_rate*weight*(active_in-1.0)*active_out;
                0;

            //delta=delta-hebb_rate; //decay

            //delta=delta+randposneg()*randfloat()*0.01; //noise

            //if (-(weight+delta)<0)
            //  return -(weight+delta);
            //  else return -0.01;

            return -(weight+delta);

        }
    }

    const std::string INDENT = "  ";

} // end NEAT

