#include "alienmobile/Simulation.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace alienmobile;
int main() {
    auto c=evolutionPlaytestConfig(); World w(c); Simulation sim(w,c);
    assert(!w.applyMutagen({NAN,0}) && !w.applyMutagen({24,0}));
    auto before=w.creatures[0].genome;
    assert(w.applyMutagen({0,0}));
    assert(!w.applyMutagen({10,10})); // no stacking or continuous refresh
    assert(w.mutationExposure({0,0})==1.5f && w.mutationExposure({4.1f,0})==1);
    assert(w.creatures[0].genome==before);
    for(unsigned i=0;i<2402;++i)sim.step();
    assert(w.mutagen.remaining==0 && w.mutationExposure({0,0})==1);
    assert(w.applyMutagen({0,0}));sim.reset();assert(w.mutagen.remaining==0);
    for(auto const& specimen:makeCuratedSpecimenCatalog()) {
        unsigned normal=0,exposed=0;
        for(unsigned i=0;i<10000;++i) {
            DeterministicRng a(i+71),b(i+71),d(i+71);
            auto plain=mutateDevelopmentGenome(specimen.genome,a);
            auto control=mutateDevelopmentGenome(specimen.genome,d,1);
            auto changed=mutateDevelopmentGenome(specimen.genome,b,1.5f);
            assert(plain.genome==control.genome);
            normal+=plain.mutated();exposed+=changed.mutated();
            if(!changed.metaMutated && changed.kind!=MutationKind::Meta)
                assert(changed.genome.mutationRates==specimen.genome.mutationRates);
            assert(isValidDevelopmentGenome(changed.genome));
        }
        assert(exposed>normal);
        std::cout<<specimen.name<<" 10000 draws normal="<<normal<<" exposed="<<exposed<<'\n';
    }
    // A capacity-delayed proposal must not be rerolled by later exposure.
    c.catalogSeed=false;c.ecosystemSeed=false;c.maxCellCount=5;c.metabolismRate=0;
    c.emissionRate=0;c.sensorEnergyCost=0;
    World limited(c);limited.cells.clear();limited.connections.clear();limited.angles.clear();limited.creatures.clear();
    limited.addFounder(makeCuratedSpecimenCatalog()[0].genome,{},0,.5f);
    for(auto& cell:limited.cells)cell.energy=1.2f;
    Simulation wait(limited,c);wait.step();auto id=limited.creatures[0].id;
    assert(limited.findCreature(id)->proposedOffspring);
    auto proposal=limited.findCreature(id)->proposedOffspring->genome;
    assert(limited.applyMutagen({0,0}));
    for(unsigned i=0;i<100;++i)wait.step();
    assert(limited.findCreature(id)->proposedOffspring->genome==proposal);
    std::cout<<"Exposure bounded in space/time; DNA unaltered until ordinary conception; pending DNA preserved.\n";
}
