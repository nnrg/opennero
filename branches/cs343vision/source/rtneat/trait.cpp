#include "core/Common.h"
#include "trait.h"

using namespace NEAT;
using namespace std;

Trait::Trait()
{
    for (S32 count=0; count<NEAT::num_trait_params; count++)
        params[count]=0;
    trait_id=0;
}

Trait::Trait(S32 id, F64 p1, F64 p2, F64 p3, F64 p4, F64 p5, F64 p6, F64 p7,
             F64 p8, F64 p9)
{
    trait_id=id;
    params[0]=p1;
    params[1]=p2;
    params[2]=p3;
    params[3]=p4;
    params[4]=p5;
    params[5]=p6;
    params[6]=p7;
    params[7]=0;
}

Trait::Trait(const Trait& t)
{
    for (S32 count=0; count < NEAT::num_trait_params; count++)
        params[count]=(t.params)[count];

    trait_id = t.trait_id;
}

Trait::Trait(TraitPtr t)
{
    for (S32 count=0; count<NEAT::num_trait_params; count++)
        params[count]=(t->params)[count];

    trait_id=t->trait_id;
}

Trait::Trait(istream &argline)
{

    //Read in trait id
    argline >> trait_id;

    //IS THE STOPPING CONDITION CORRECT?  ALERT
    for (S32 count=0; count<NEAT::num_trait_params; count++)
    {
        argline >> params[count];
    }

}

Trait::Trait(TraitPtr t1, TraitPtr t2)
{
    for (S32 count=0; count<NEAT::num_trait_params; count++)
        params[count]=(((t1->params)[count])+((t2->params)[count]))/2.0;
    trait_id=t1->trait_id;
}

void Trait::print_to_file(std::ofstream &outFile)
{

    outFile << "trait "<< trait_id << " ";
    for (S32 count=0; count < NEAT::num_trait_params; count++)
        outFile<<params[count]<<" ";
    outFile<<endl;
}

void Trait::mutate()
{
    for (S32 count=0; count<NEAT::num_trait_params; count++)
    {
        if (randfloat()>NEAT::trait_param_mut_prob)
        {
            params[count]+=(randposneg()*randfloat())
                *NEAT::trait_mutation_power;
            if (params[count]<0)
                params[count]=0;
            if (params[count]>1.0)
                params[count]=1.0;
        }
    }
}
