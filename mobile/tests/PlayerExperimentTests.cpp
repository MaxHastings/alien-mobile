#include "alienmobile/Simulation.h"
#include <cassert>
#include <iostream>
using namespace alienmobile;
int main() {
    for(unsigned seed:{913u,1913u,2913u}) {
        auto c=evolutionPlaytestConfig();c.randomSeed=seed;
        World w(c);Simulation sim(w,c);
        unsigned exposures=0;
        for(unsigned tick=0;tick<360*120;++tick) {
            // Repeatable explicit experiments, not rescue: fixed times and
            // patch location regardless of population health or ancestry.
            if(tick==30*120 || tick==120*120 || tick==210*120) {
                auto p=w.resourcePatchPosition(3);
                assert(w.scatterFood(p));assert(w.applyMutagen(p));++exposures;
            }
            sim.step();
            if(tick%120==0)assert(w.allValuesFinite() && w.allConnectionsValid());
        }
        assert(exposures==3 && w.mutagen.remaining==0);
        auto const& s=sim.stats();
        std::cout<<"seed="<<seed<<" seconds=360 adults="<<w.matureCreatureCount()
            <<" cells="<<w.cells.size()<<" births="<<s.births<<" mutations="<<s.mutations
            <<" deaths="<<s.deaths<<" generation="<<s.maximumMatureGeneration<<'\n';
    }
}
