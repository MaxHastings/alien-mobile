#include "alienmobile/Simulation.h"
#include "alienmobile/Ecology.h"
#include <cassert>
#include <iostream>
using namespace alienmobile;
World specimen(Genome const& g) {
    auto c=depthPlaytestConfig();c.ecosystemSeed=false;c.emissionRate=0;c.hazardStrength=0;
    c.initialCellEnergy=.4f;
    World w(c);w.cells.clear();w.connections.clear();w.angles.clear();w.creatures.clear();w.motes.clear();w.energyLedger={};
    w.addFounder(g,{},0,.5f);return w;
}
int main() {
    auto small=makePrimitiveGenome();DeterministicRng rng(77);
    auto large=applyDevelopmentMutation(small,rng,MutationKind::DuplicateGene).genome;
    assert(isValidDevelopmentGenome(large));assert(measureDevelopment(large).complete());
    auto n=measureDevelopment(small).cells,m=measureDevelopment(large).cells;
    assert(m==2*n+1);
    // The complete original working section survives a duplication unchanged.
    for(unsigned i=0;i<n;++i)assert(small.genes[0].nodes[i]==large.genes[0].nodes[i]);
    assert(large.genes[1].nodes[1].behavior==small.genes[0].nodes[1].behavior);
    assert(large.genes[1].nodes[2].behavior==small.genes[0].nodes[2].behavior);
    auto a=specimen(small),b=specimen(large);
    // Identical finite spatial resource field: motes sit at separated physical
    // interception sites. No size-dependent energy grant enters either world.
    for(auto const& cell:b.cells) {a.addMote(cell.position,{},.1);b.addMote(cell.position,{},.1);}
    updateResources(a,.001f);updateResources(b,.001f);
    assert(b.energyLedger.absorbed>a.energyLedger.absorbed+.1);
    auto sparseSmall=a.energyLedger.absorbed,sparseLarge=b.energyLedger.absorbed;
    a=specimen(small);b=specimen(large);
    a.addMote({}, {},.1);b.addMote({}, {},.1);
    updateResources(a,.001f);updateResources(b,.001f);
    assert(std::abs(a.energyLedger.absorbed-b.energyLedger.absorbed)<1e-9);
    assert(a.cells.size()*a.config().metabolismRate<b.cells.size()*b.config().metabolismRate);
    std::cout<<"cells="<<n<<','<<m<<" dispersed uptake="<<sparseSmall<<','<<sparseLarge
        <<" concentrated uptake="<<a.energyLedger.absorbed<<','<<b.energyLedger.absorbed
        <<" maintenance/s="<<n*a.config().metabolismRate<<','<<m*b.config().metabolismRate<<'\n';
}
