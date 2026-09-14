#include "CompetitionResearch.h"
#include <cassert>
#include <iostream>
int main() {
    using namespace competition;
    auto c=referenceConfig(17);World fixed(c),small(c),full(c);
    initialRegime(fixed,"fixed");initialRegime(small,"small");initialRegime(full,"full");
    assert(fixed.cells==small.cells && small.cells==full.cells);
    assert(fixed.rng.state()==small.rng.state() && small.rng.state()==full.rng.state());
    for(unsigned i=0;i<fixed.motes.size();++i){assert(fixed.motes[i].position==full.motes[i].position);assert(fixed.motes[i].energy==full.motes[i].energy);}
    auto rates=small.creatures[1].genome.mutationRates;assert(rates.neural==.28f && rates.geometry==.18f && rates.property==0 && rates.meta==0 && rates.duplicateGene==0);
    World observed(c),control(c);initialRegime(observed,"full");initialRegime(control,"full");
    Simulation a(observed,c),b(control,c);Observer observer;observer.observe(observed,0);
    for(unsigned step=1;step<=14400;++step){a.step();observer.observe(observed,step);b.step();}
    assert(observed.cells==control.cells);assert(observed.creatures==control.creatures);assert(observed.connections==control.connections);assert(stateHash(observed)==stateHash(control));
    assert(observer.totals[0].born+observer.totals[1].born==a.stats().births);
    for(auto const& l:observer.lives)if(l.known && l.parent!=kInvalidId)assert(l.ancestry==observer.lives[l.parent].ancestry);
    std::cout<<"identical initial physical states, isolated mutation regimes, unchanged observed trajectory, exact mature-birth and ancestry tracking passed\n";
}
