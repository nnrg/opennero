#ifndef _EXPERIMENTS_H_
#define _EXPERIMENTS_H_

#include "neat.h"
#include "population.h"

namespace NEAT
{
    class CartPole
    {
        public:
            CartPole(bool randomize, bool velocity);
            virtual ~CartPole() {}
            virtual void simplifyTask();
            virtual void nextTask();
            virtual F64 evalNet(NetworkPtr net, S32 thresh);
            F64 maxFitness;
            bool MARKOV;

            bool last_hundred;
            bool nmarkov_long; // Flag that we are looking at the champ
            bool generalization_test; // Flag we are testing champ's generalization

            F64 state[6];

        protected:
            virtual void init(bool randomize);

        private:

            void performAction(F64 output);
            void step(F64 action, F64 *state, F64 *derivs);
            void rk4(F64 f, F64 y[], F64 dydx[], F64 yout[]);
            bool outsideBounds();

            const static S32 NUM_INPUTS = 7;
            const F64 MUP;
            const F64 MUC;
            const F64 GRAVITY;
            const F64 MASSCART;
            const F64 MASSPOLE_1;

            const F64 LENGTH_1; // actually half the pole's length 

            const F64 FORCE_MAG;
            const F64 TAU; // seconds between state updates 

            const F64 one_degree; // 2pi/360 
            const F64 six_degrees;
            const F64 twelve_degrees;
            const F64 fifteen_degrees;
            const F64 thirty_six_degrees;
            const F64 fifty_degrees;

            F64 LENGTH_2;
            F64 MASSPOLE_2;
            F64 MIN_INC;
            F64 POLE_INC;
            F64 MASS_INC;

            // Queues used for Gruau's fitness which damps oscillations
            S32 balanced_sum;
            F64 cartpos_sum;
            F64 cartv_sum;
            F64 polepos_sum;
            F64 polev_sum;
    };

    // Double pole balacing evolution routines
    PopulationPtr pole2_test(S32 gens, S32 velocity);
    PopulationPtr pole2_testf(S32 gens, S32 velocity,
            const std::string& filename);
    bool pole2_evaluate(OrganismPtr org, bool velocity, CartPole *thecart);
    S32 pole2_epoch(PopulationPtr pop, S32 generation,
            const std::string& filename, bool velocity, CartPole *thecart,
            S32 &champgenes, S32 &champnodes, S32 &winnernum,
            std::ofstream &oFile);
    S32 pole2_epoch_realtime(PopulationPtr pop, S32 generation,
            const std::string& filename, bool velocity, CartPole *thecart,
            S32 &champgenes, S32 &champnodes, S32 &winnernum,
            std::ofstream &oFile);

} // namespace NEAT

#endif
