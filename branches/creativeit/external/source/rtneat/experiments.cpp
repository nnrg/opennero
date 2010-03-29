#include "core/Common.h"
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "experiments.h"
#include "network.h"

using namespace NEAT;
using namespace std;

// ------------------------------------------------------------------ 
// Double pole balacing                                               
// ------------------------------------------------------------------ 

//Perform evolution on double pole balacing, for gens generations
//If velocity is false, then velocity information will be withheld from the 
//network population (non-Markov)

CartPole::CartPole(bool randomize,bool velocity)
:
        //NUM_INPUTS(7),
        MUP(0.000002), MUC(0.0005), GRAVITY(-9.8), MASSCART(1.0),
        MASSPOLE_1(0.1),

        LENGTH_1(0.5),

        FORCE_MAG(10.0), TAU(0.01),

        one_degree(0.0174532), six_degrees(0.1047192),
        twelve_degrees(0.2094384), fifteen_degrees(0.2617993),
        thirty_six_degrees(0.628329), fifty_degrees(0.87266)
{
    maxFitness = 100; //PFHACK
    //maxFitness = 100000;

    MARKOV=velocity;

    MIN_INC = 0.001;
    POLE_INC = 0.05;
    MASS_INC = 0.01;

    LENGTH_2 = 0.05;
    MASSPOLE_2 = 0.01;
}

// Faustino Gomez wrote this physics code using the differential equations from 
// Alexis Weiland's paper and added the Runge-Kutta himself.
double CartPole::evalNet(NetworkPtr net, int thresh)
{
    int steps=0;
    int tempnum = 0;
    //tempnum += NUM_INPUTS;
    tempnum += 1;
    tempnum -= 1;
    double input[NUM_INPUTS];
    //double input[(int)1];
    double output;

    int nmarkovmax;

    double nmarkov_fitness;

    //init(randomize);		// restart at some point

    if (nmarkov_long)
        nmarkovmax=100;
    //if (nmarkov_long) nmarkovmax=100000; //PFHACK
    else if (generalization_test)
        nmarkovmax=1000;
    else
        nmarkovmax=1000;

    init(0);

    if (MARKOV)
    {
        while (steps++ < maxFitness)
        {

            input[0] = state[0] / 4.8;
            input[1] = state[1] /2;
            input[2] = state[2] / 0.52;
            input[3] = state[3] /2;
            input[4] = state[4] / 0.52;
            input[5] = state[5] /2;
            input[6] = .5;

            net->load_sensors(input);

            //Activate the net
            //If it loops, exit returning only fitness of 1 step
            if (!(net->activate()))
                return 1.0;

            output=(*(net->outputs.begin()))->activation;

            performAction(output);

            if (outsideBounds()) // if failure
                break; // stop it now
        }
        return (double) steps;
    }
    else
    { //NON MARKOV CASE

        while (steps++ < nmarkovmax)
        {

            //Do special parameter summing on last hundred
            if ((steps==900)&&(!nmarkov_long))
                last_hundred=true;

            //input[0] = state[0] / 4.8;
            //input[1] = 0.0;
            //input[2] = state[2]  / 0.52;
            //input[3] = 0.0;
            //input[4] = state[4] / 0.52;
            //input[5] = 0.0;
            //input[6] = .5;


            input[0] = state[0] / 4.8;
            input[1] = state[2] / 0.52;
            input[2] = state[4] / 0.52;
            input[3] = .5;

            net->load_sensors(input);

            //Activate the net
            //If it loops, exit returning only fitness of 1 step
            if (!(net->activate()))
                return 0.0001;

            output=(*(net->outputs.begin()))->activation;

            performAction(output);

            if (nmarkov_long&&(outsideBounds())) // if failure
                break; // stop it now
        }

        //If we are generalizing we just need to balance it a while
        if (generalization_test)
            return (double) balanced_sum;

        if (!nmarkov_long)
        {
            if (balanced_sum>100)
                nmarkov_fitness=((0.1*(((double) balanced_sum)/1000.0))+(0.9
                    *(0.75/(cartpos_sum+cartv_sum+polepos_sum+polev_sum))));
            else
                nmarkov_fitness=(0.1*(((double) balanced_sum)/1000.0));

            //cout<<"Balanced:  "<<balanced_sum<<" jiggle: "<<cartpos_sum+cartv_sum+polepos_sum+polev_sum<<" ***";

            return nmarkov_fitness;
        }
        else
            return (double) steps;

    }

}

void CartPole::init(bool randomize)
{
    static int first_time = 1;

    if (!MARKOV)
    {
        //Clear all fitness records
        cartpos_sum=0.0;
        cartv_sum=0.0;
        polepos_sum=0.0;
        polev_sum=0.0;
    }

    balanced_sum=0; //Always count # balanced

    last_hundred=false;

    //if (randomize) {
    //state[0] = (lrand48()%4800)/1000.0 - 2.4;
    //state[1] = (lrand48()%2000)/1000.0 - 1;
    //state[2] = (lrand48()%400)/1000.0 - 0.2;
    //state[3] = (lrand48()%400)/1000.0 - 0.2;
    //state[4] = (lrand48()%3000)/1000.0 - 1.5;
    //state[5] = (lrand48()%3000)/1000.0 - 1.5;
    //}
    //else {


    if (!generalization_test)
    {
        state[0] = state[1] = state[3] = state[4] = state[5] = 0;
        state[2] = 0.07; // one_degree;
    }
    else
    {
        state[4] = state[5] = 0;
    }

    //}
    if (first_time)
    {
        //cout<<"Initial Long pole angle = %f\n"<<state[2]<<endl;
        //cout<<"Initial Short pole length = %f\n"<<LENGTH_2<<endl;
        first_time = 0;
    }
}

void CartPole::performAction(double output)
{

    int i;
    double dydx[6];

    const bool RK4=true; //Set to Runge-Kutta 4th order integration method
    const double EULER_TAU= TAU/4;

    //random start state for long pole
    //state[2]= drand48();   

    //--- Apply action to the simulated cart-pole ---

    if (RK4)
    {
        for (i=0; i<2; ++i)
        {
            dydx[0] = state[1];
            dydx[2] = state[3];
            dydx[4] = state[5];
            step(output, state, dydx);
            rk4(output, state, dydx, state);
        }
    }
    else
    {
        for (i=0; i<8; ++i)
        {
            step(output, state, dydx);
            state[0] += EULER_TAU * dydx[0];
            state[1] += EULER_TAU * dydx[1];
            state[2] += EULER_TAU * dydx[2];
            state[3] += EULER_TAU * dydx[3];
            state[4] += EULER_TAU * dydx[4];
            state[5] += EULER_TAU * dydx[5];
        }
    }

    //Record this state
    if (last_hundred)
    {
        cartpos_sum+=fabs(state[0]);
        cartv_sum+=fabs(state[1]);
        polepos_sum+=fabs(state[2]);
        polev_sum+=fabs(state[3]);
    }

    if (false)
    {
        //cout<<"[ x: "<<state[0]<<" xv: "<<state[1]<<" t1: "<<state[2]<<" t1v: "<<state[3]<<" t2:"<<state[4]<<" t2v: "<<state[5]<<" ] "<<
        //cartpos_sum+cartv_sum+polepos_sum+polepos_sum+polev_sum<<endl;
        if (!(outsideBounds()))
        {
            if (balanced_sum<1000)
            {
                //cout<<".";
                ++balanced_sum;
            }
        }
        else
        {
            if (balanced_sum==1000)
                balanced_sum=1000;
            else
                balanced_sum=0;
        }
    }
    else if (!(outsideBounds()))
        ++balanced_sum;

}

void CartPole::step(double action, double *st, double *derivs)
{
    double force, costheta_1, costheta_2, sintheta_1, sintheta_2, gsintheta_1,
        gsintheta_2, temp_1, temp_2, ml_1, ml_2, fi_1, fi_2, mi_1, mi_2;

    force = (action - 0.5) * FORCE_MAG * 2;
    costheta_1 = cos(st[2]);
    sintheta_1 = sin(st[2]);
    gsintheta_1 = GRAVITY * sintheta_1;
    costheta_2 = cos(st[4]);
    sintheta_2 = sin(st[4]);
    gsintheta_2 = GRAVITY * sintheta_2;

    ml_1 = LENGTH_1 * MASSPOLE_1;
    ml_2 = LENGTH_2 * MASSPOLE_2;
    temp_1 = MUP * st[3]/ ml_1;
    temp_2 = MUP * st[5]/ ml_2;
    fi_1 = (ml_1 * st[3]* st[3]* sintheta_1) +(0.75 * MASSPOLE_1 * costheta_1
        * (temp_1 + gsintheta_1));
    fi_2 = (ml_2 * st[5]* st[5]* sintheta_2) +(0.75 * MASSPOLE_2 * costheta_2
        * (temp_2 + gsintheta_2));
    mi_1 = MASSPOLE_1 * (1 - (0.75 * costheta_1 * costheta_1));
    mi_2 = MASSPOLE_2 * (1 - (0.75 * costheta_2 * costheta_2));

    derivs[1] = (force + fi_1 + fi_2)/ (mi_1 + mi_2 + MASSCART);

    derivs[3] = -0.75 * (derivs[1] * costheta_1 + gsintheta_1 + temp_1)
        / LENGTH_1;
    derivs[5] = -0.75 * (derivs[1] * costheta_2 + gsintheta_2 + temp_2)
        / LENGTH_2;

}

void CartPole::rk4(double f, double y[], double dydx[], double yout[])
{

    int i;

    double hh, h6, dym[6], dyt[6], yt[6];

    hh=TAU*0.5;
    h6=TAU/6.0;
    for (i=0; i<=5; i++)
        yt[i]=y[i]+hh*dydx[i];
    step(f, yt, dyt);
    dyt[0] = yt[1];
    dyt[2] = yt[3];
    dyt[4] = yt[5];
    for (i=0; i<=5; i++)
        yt[i]=y[i]+hh*dyt[i];
    step(f, yt, dym);
    dym[0] = yt[1];
    dym[2] = yt[3];
    dym[4] = yt[5];
    for (i=0; i<=5; i++)
    {
        yt[i]=y[i]+TAU*dym[i];
        dym[i] += dyt[i];
    }
    step(f, yt, dyt);
    dyt[0] = yt[1];
    dyt[2] = yt[3];
    dyt[4] = yt[5];
    for (i=0; i<=5; i++)
        yout[i]=y[i]+h6*(dydx[i]+dyt[i]+2.0*dym[i]);
}

bool CartPole::outsideBounds()
{
    const double failureAngle = thirty_six_degrees;

    return state[0] < -2.4||state[0] > 2.4||state[2] < -failureAngle ||state[2]
        > failureAngle ||state[4] < -failureAngle ||state[4] > failureAngle;
}

void CartPole::nextTask()
{

    LENGTH_2 += POLE_INC; // LENGTH_2 * INCREASE; 
    MASSPOLE_2 += MASS_INC; // MASSPOLE_2 * INCREASE;
    //  ++new_task;
    //cout<<"#Pole Length %2.4f\n"<<LENGTH_2<<endl;
}

void CartPole::simplifyTask()
{
    if (POLE_INC > MIN_INC)
    {
        POLE_INC = POLE_INC/2;
        MASS_INC = MASS_INC/2;
        LENGTH_2 -= POLE_INC;
        MASSPOLE_2 -= MASS_INC;
        //cout<<"#SIMPLIFY\n"<<endl;
        //cout<<"#Pole Length %2.4f\n"<<LENGTH_2<<endl;
    }
    else
    {
        //cout<<"#NO TASK CHANGE\n"<<endl;
    }
}
PopulationPtr NEAT::pole2_test(int gens, int velocity)
{

    PopulationPtr pop;
    GenomePtr start_genome;
    string curword;
    int id;

    int gen;
    CartPole *thecart;

    //control # of times experiment is run
    static const int RUNS=1;

    //Stat collection variables
    int highscore;
    int record[RUNS][200];
    double recordave[200];
    int genesrec[RUNS][200];
    double genesave[200];
    int nodesrec[RUNS][200];
    double nodesave[200];
    int winnergens[RUNS];
    int initcount;
    int champg, champn, winnernum; //Record number of genes and nodes in champ
    int run;
    int curtotal; //For averaging
    int samples; //For averaging

    ofstream oFile("NERO/data/neat/statout.rtf");

    champg=0;
    champn=0;

    //Initialize the stat recording arrays
    for (initcount=0; initcount<200; initcount++)
    {
        recordave[initcount]=0;
        genesave[initcount]=0;
        nodesave[initcount]=0;
        for (run=0; run<RUNS; ++run)
        {
            record[run][initcount]=0;
            genesrec[run][initcount]=0;
            nodesrec[run][initcount]=0;
        }
    }

    ifstream iFile("NERO/data/neat/pole2startgenes");

    iFile >> curword;
    iFile >> id;
    cout << "Reading in Genome id "<< id << endl;
    start_genome.reset(new Genome(id,iFile));
    iFile.close();

    //cout<<"Start Genome: "<<start_genome<<endl;

    //cin>>pause;  

    for (run=0; run<RUNS; run++)
    {

        //cout<<"RUN #"<<run<<endl;

        //Spawn the Population from starter gene
        //cout<<"Spawning Population off Genome"<<endl;
        pop.reset(new Population(start_genome,pop_size));

        // Open files by using the resource manager
        ofstream outFile("nero/data/neat/gen_1.rtf");
        pop->print_to_file_by_species(outFile);

        //Alternative way to start off of randomly connected genomes
        pop->verify();

        //Create the Cart
        thecart=new CartPole(true,velocity != 0);

        for (gen = 1; gen < 2; ++gen)
        {

            ostringstream fnamebuf;
            fnamebuf << "gen_"<< gen << ends; //needs end marker

            highscore=pole2_epoch_realtime(pop, gen, fnamebuf.str().c_str(), velocity != 0, thecart, champg, champn, winnernum,
                                           oFile);

            //cout<<"GOT HIGHSCORE FOR GEN "<<gen<<": "<<highscore-1<<endl;

            record[run][gen-1]=highscore-1;
            genesrec[run][gen-1]=champg;
            nodesrec[run][gen-1]=champn;

            //fnamebuf->clear();
            //delete fnamebuf;

            //Stop right at the winnergen
            if (((pop->winnergen)!=0)&&(gen==(pop->winnergen)))
            {
                winnergens[run]=pop_size*(gen-1)+winnernum;
                gen=gens+1;
            }

            //In non-MARKOV, stop right at winning (could go beyond if desired)
            if ((!(thecart->MARKOV))&&((pop->winnergen)!=0))
                gen=gens+1;

            //cout<<"gen = "<<gen<<" gens = "<<gens<<endl;

            if (gen==(gens-1))
            {
                oFile << "FAIL: Last gen on run "<< run << endl;
            }

        }

        if (run<RUNS-1)
            pop.reset();
        delete thecart;

    }

    oFile<<"Generation highs: "<<endl;
    for (gen=0; gen<=gens-1; gen++)
    {
        curtotal=0;
        for (run=0; run<RUNS; ++run)
        {
            if (record[run][gen]>0)
            {
                cout<<setw(8)<<record[run][gen]<<" ";
                oFile<<setw(8)<<record[run][gen]<<" ";

                curtotal+=record[run][gen];
            }
            else
            {
                cout<<"         ";
                oFile<<"         ";
                curtotal+=100000;
            }
            recordave[gen]=(double) curtotal/RUNS;

        }
        cout<<endl;
        oFile<<endl;
    }

    //cout<<"Generation genes in champ: "<<endl;
    for (gen=0; gen<=gens-1; gen++)
    {
        curtotal=0;
        samples=0;
        for (run=0; run<RUNS; ++run)
        {
            if (genesrec[run][gen]>0)
            {
                cout<<setw(4)<<genesrec[run][gen]<<" ";
                oFile<<setw(4)<<genesrec[run][gen]<<" ";

                curtotal+=genesrec[run][gen];
                samples++;
            }
            else
            {
                cout<<setw(4)<<"     ";
                oFile<<setw(4)<<"     ";
            }
        }
        genesave[gen]=(double) curtotal/samples;

        cout<<endl;
        oFile<<endl;
    }

    cout<<"Generation nodes in champ: "<<endl;
    oFile<<"Generation nodes in champ: "<<endl;

    for (gen=0; gen<=gens-1; gen++)
    {
        curtotal=0;
        samples=0;
        for (run=0; run<RUNS; ++run)
        {
            if (nodesrec[run][gen]>0)
            {
                cout<<setw(4)<<nodesrec[run][gen]<<" ";
                oFile<<setw(4)<<nodesrec[run][gen]<<" ";

                curtotal+=nodesrec[run][gen];
                samples++;
            }
            else
            {
                cout<<setw(4)<<"     ";
                oFile<<setw(4)<<"     ";
            }
        }
        nodesave[gen]=(double) curtotal/samples;

        cout<<endl;
        oFile<<endl;
    }

    cout<<"Generational record fitness averages: "<<endl;
    oFile<<"Generational record fitness averages: "<<endl;
    for (gen=0; gen<gens-1; gen++)
    {
        cout<<recordave[gen]<<endl;
        oFile<<recordave[gen]<<endl;
    }

    cout<<"Generational number of genes in champ averages: "<<endl;
    oFile<<"Generational number of genes in champ averages: "<<endl;

    for (gen=0; gen<gens-1; gen++)
    {
        cout<<genesave[gen]<<endl;
        oFile<<genesave[gen]<<endl;
    }

    cout<<"Generational number of nodes in champ averages: "<<endl;
    oFile<<"Generational number of nodes in champ averages: "<<endl;

    for (gen=0; gen<gens-1; gen++)
    {
        cout<<nodesave[gen]<<endl;
        oFile<<nodesave[gen]<<endl;
    }

    cout<<"Winner evals: "<<endl;
    oFile<<"Winner evals: "<<endl;

    curtotal=0;
    samples=0;
    for (run=0; run<RUNS; ++run)
    {
        cout<<winnergens[run]<<endl;
        oFile<<winnergens[run]<<endl;

        curtotal+=winnergens[run];
        samples++;
    }
    cout<<"Average # evals: "<<((double) curtotal/samples)<<endl;
    oFile<<"Average # evals: "<<((double) curtotal/samples)<<endl;

    oFile.close();

    return pop;

}

int NEAT::pole2_epoch(PopulationPtr pop, int generation,
                      const string& filename, bool velocity, CartPole *thecart,
                      int &champgenes, int &champnodes, int &winnernum,
                      ofstream &oFile)
{
    vector<OrganismPtr>::iterator curorg;
    vector<SpeciesPtr>::iterator curspecies;

    vector<SpeciesPtr> sorted_species; //Species sorted by max fit org in Species

    bool win=false;

    double champ_fitness;
    OrganismPtr champ;

    //double statevals[5]={-0.9,-0.5,0.0,0.5,0.9};
    double statevals[5]=
    { 0.05, 0.25, 0.5, 0.75, 0.95 };

    int s0c, s1c, s2c, s3c;

    int score;

    thecart->nmarkov_long=false;
    thecart->generalization_test=false;

    //Evaluate each organism on a test
    for (curorg=(pop->organisms).begin(); curorg!=(pop->organisms).end(); ++curorg)
    {

    	assert((*curorg)->gnome);

        if (pole2_evaluate(OrganismPtr(*curorg), velocity, thecart))
            win=true;

    }

    //Average and max their fitnesses for dumping to file and snapshot
    for (curspecies=(pop->species).begin(); curspecies!=(pop->species).end(); ++curspecies)
    {

        //This experiment control routine issues commands to collect ave
        //and max fitness, as opposed to having the snapshot do it, 
        //because this allows flexibility in terms of what time
        //to observe fitnesses at

        (*curspecies)->compute_average_fitness();
        (*curspecies)->compute_max_fitness();
    }

    //Find the champion in the markov case simply for stat collection purposes
    if (thecart->MARKOV)
    {
        champ_fitness=0.0;
        for (curorg=(pop->organisms).begin(); curorg!=(pop->organisms).end(); ++curorg)
        {
            if (((*curorg)->fitness)>champ_fitness)
            {
                champ = *curorg;
                champ_fitness=champ->fitness;
                champgenes=static_cast<int>(champ->gnome->genes.size());
                champnodes=static_cast<int>(champ->gnome->nodes.size());
                winnernum=champ->gnome->genome_id;
            }
        }
    }

    //Check for winner in Non-Markov case
    if (!(thecart->MARKOV))
    {

        //Sort the species
        for (curspecies=(pop->species).begin(); curspecies!=(pop->species).end(); ++curspecies)
        {
            sorted_species.push_back(*curspecies);
        }
        sort(sorted_species.begin(), sorted_species.end(), order_new_species);

        //cout<<"Number of species sorted: "<<sorted_species.size()<<endl;

        //First update what is checked and unchecked
        for (curspecies=sorted_species.begin(); curspecies
            !=sorted_species.end(); ++curspecies)
        {
            if (((*curspecies)->compute_max_fitness())>((*curspecies)->max_fitness_ever))
                (*curspecies)->checked=false;

        }

        //Now find a species that is unchecked
        curspecies=sorted_species.begin();
        //cout<<"Is the first species checked? "<<(*curspecies)->checked<<endl;
        while ((curspecies!=(sorted_species.end()))&&((*curspecies)->checked))
        {
            //cout<<"Species #"<<(*curspecies)->id<<" is checked"<<endl;
            ++curspecies;
        }

        if (curspecies==(sorted_species.end()))
            curspecies=sorted_species.begin();

        //Remember it was checked
        (*curspecies)->checked=true;
        //cout<<"Is the species now checked? "<<(*curspecies)->checked<<endl;

        //Extract the champ
        //cout<<"Champ chosen from Species "<<(*curspecies)->id<<endl;
        champ=(*curspecies)->get_champ();
        champ_fitness=champ->fitness;
        //cout<<"Champ is organism #"<<champ->gnome->genome_id<<endl;
        //cout<<"Champ fitness: "<<champ_fitness<<endl;
        winnernum=champ->gnome->genome_id;

        //cout<<champ->gnome<<endl;

        //Now check to make sure the champ can do 100,000
        thecart->nmarkov_long=true;
        thecart->generalization_test=false;

        //The champ needs tp be flushed here because it may have
        //leftover activation from its last test run that could affect
        //its recurrent memory
        (champ->net)->flush();

        if (pole2_evaluate(champ, velocity, thecart))
        {
            //cout<<"The champ passed the 100,000 test!"<<endl;

            thecart->nmarkov_long=false;

            //Given that the champ passed, now run it on generalization tests
            score=0;
            for (s0c=0; s0c<=4; ++s0c)
                for (s1c=0; s1c<=4; ++s1c)
                    for (s2c=0; s2c<=4; ++s2c)
                        for (s3c=0; s3c<=4; ++s3c)
                        {
                            thecart->state[0] = statevals[s0c] * 4.32- 2.16;
                            thecart->state[1] = statevals[s1c] * 2.70- 1.35;
                            thecart->state[2] = statevals[s2c] * 0.12566304
                                - 0.06283152;
                            // 0.06283152 =  3.6 degrees 
                            thecart->state[3] = statevals[s3c] * 0.30019504
                                - 0.15009752;
                            // 00.15009752 =  8.6 degrees 
                            thecart->state[4]=0.0;
                            thecart->state[5]=0.0;

                            //cout<<"On combo "<<thecart->state[0]<<" "<<thecart->state[1]<<" "<<thecart->state[2]<<" "<<thecart->state[3]<<endl;
                            thecart->generalization_test=true;

                            (champ->net)->flush(); //Reset the champ for each eval

                            if (pole2_evaluate(champ, velocity, thecart))
                            {
                                //cout<<"----------------------------The champ passed its "<<score<<"th test"<<endl;
                                score++;
                            }

                        }

            if (score>=200)
            {
                cout<<"The champ wins!!! (generalization = "<<score<<" )"<<endl;
                oFile<<"(generalization = "<<score<<" )"<<endl;
                oFile<<"generation= "<<generation<<endl;
                (champ->gnome)->print_to_file(oFile);
                win=true;
            }
            else
            {
                //cout<<"The champ couldn't generalize"<<endl;
                champ->fitness=champ_fitness; //Restore the champ's fitness
            }
        }
        else
        {
            //cout<<"The champ failed the 100,000 test :("<<endl;
            champ->fitness=champ_fitness; //Restore the champ's fitness
        }
    }

    //Only print to file every print_every generations
    if (win || ((generation%(print_every))==0))
    {
        // Open files by using the resource manager
        ofstream outFile(filename.c_str());
        pop->print_to_file_by_species(outFile);
    }

    if ((win)&&((pop->winnergen)==0))
        pop->winnergen=generation;

    //Prints a champion out on each generation
    //IMPORTANT: This causes generational file output!
    //print_Genome_tofile(champ->gnome,"champ");
    (champ->gnome)->print_to_filename("champ");

    //Create the next generation
    pop->epoch(generation);

    oFile.close();
    return static_cast<int>(champ_fitness);

}

int NEAT::pole2_epoch_realtime(PopulationPtr pop, int generation,
                               const string& filename, bool velocity,
                               CartPole *thecart, int &champgenes,
                               int &champnodes, int &winnernum,
                               ofstream &outFile)
{
    vector<OrganismPtr>::iterator curorg;
    vector<SpeciesPtr>::iterator curspecies;

    vector<SpeciesPtr>::iterator curspec; //used in printing out debug info

    vector<SpeciesPtr> sorted_species; //Species sorted by max fit org in Species

    bool win=false;

    //Real-time evolution variables
    int offspring_count;
    OrganismPtr new_org;

    thecart->nmarkov_long=false;
    thecart->generalization_test=false;

    //Initially, we evaluate the whole population
    //Evaluate each organism on a test
    for (curorg=(pop->organisms).begin(); curorg!=(pop->organisms).end(); ++curorg)
    {

        //shouldn't happen
    	assert((*curorg)->gnome);

        if (pole2_evaluate(OrganismPtr(*curorg), velocity, thecart))
            win=true;

    }

    //Now create offspring one at a time, testing each offspring,
    // and replacing the worst with the new offspring if its better
    pop->rank_within_species(); //Is this necessary?

    pop->estimate_all_averages();

    //NERO should use a similar main loop as the following:
    // (note that somewhere in here we need to update species fitnesses
    //  when an organism becomes mature enough to be judged)
    //Real-time Loop on creating offspring
    for (offspring_count=0; offspring_count<500; offspring_count++)
    {

        if (offspring_count % 5000)
        {
            //For printing only
            for (curspec=(pop->species).begin(); curspec!=(pop->species).end(); curspec++)
            {
                cout<<"Species "<<(*curspec)->id<<" size"<<(*curspec)->organisms.size()<<" average= "<<(*curspec)->average_est<<endl;
            }
        }

        //cout<<"Pop size: "<<pop->organisms.size()<<endl;

        new_org = (pop->choose_parent_species())->reproduce_one(offspring_count, pop, pop->species, 0, 0);

        //cout<<"Evaluating new baby: "<<endl;
        if (pole2_evaluate(new_org, velocity, thecart))
            win = true;

        //Reestimate the baby's species fitness
        new_org->species.lock()->estimate_average();

        //Remove the worst organism
        pop->remove_worst();

    }

    pop->print_to_file_by_species(outFile);
    return 1;
}

bool NEAT::pole2_evaluate(OrganismPtr org, bool velocity, CartPole *thecart)
{
    NetworkPtr net;
    int numnodes; // Used to figure out how many nodes should be visited during activation 
    int thresh; // How many visits will be allowed before giving up (for loop detection 

    net=org->net;
    numnodes=static_cast<int>(org->gnome->nodes.size());
    thresh=numnodes*2; //this is obsolete

    //DEBUG :  Check flushedness of org
    //org->net->flush_check();

    //Try to balance a pole now
    org->fitness = thecart->evalNet(net, thresh);

    //if (org->pop_champ_child) {
    //cout<<" <<DUPLICATE OF CHAMPION>> ";
    //}

    //Output to screen
    //cout<<"Org "<<(org->gnome)->genome_id<<" fitness: "<<org->fitness;
    //cout<<" ("<<(org->gnome)->genes.size();
    //cout<<" / "<<(org->gnome)->nodes.size()<<")";
    //cout<<"   ";
    //if (org->mut_struct_baby) cout<<" [struct]";
    //if (org->mate_baby) cout<<" [mate]";
    //cout<<endl;

    //Con::printf("Org %d fitness: %f (%d / %d)", (org->gnome)->genome_id, org->fitness, (org->gnome)->genes.size(), 
    //	(org->gnome)->nodes.size());
    //if (org->mut_struct_baby)
    //	Con::printf("                       [struct]");
    //if (org->mate_baby)
    //	Con::printf("                       [mate]");

    if ((!(thecart->generalization_test))&&(!(thecart->nmarkov_long)))
        if (org->pop_champ_child)
        {
            //cout<<org->gnome<<endl;
            //DEBUG CHECK
            if (org->high_fit>org->fitness)
            {
                //cout<<"ALERT: ORGANISM DAMAGED"<<endl;
                (org->gnome)->print_to_filename("failure_champ_genome");
                //cin>>pause;
            }
        }

    //Decide if its a winner, in Markov Case
    if (thecart->MARKOV)
    {
        if (org->fitness>=(thecart->maxFitness))
        {
            org->winner=true;
            return true;
        }
        else
        {
            org->winner=false;
            return false;
        }
    }
    //if doing the long test non-markov 
    else if (thecart->nmarkov_long)
    {
        if (org->fitness>=99999)
        {
            //if (org->fitness>=9000) { 
            org->winner=true;
            return true;
        }
        else
        {
            org->winner=false;
            return false;
        }
    }
    else if (thecart->generalization_test)
    {
        if (org->fitness>=999)
        {
            org->winner=true;
            return true;
        }
        else
        {
            org->winner=false;
            return false;
        }
    }
    else
    {
        org->winner=false;
        return false; //Winners not decided here in non-Markov
    }
}
