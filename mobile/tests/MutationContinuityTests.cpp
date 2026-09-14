#include "alienmobile/Development.h"
#include "alienmobile/World.h"
#include "alienmobile/Simulation.h"
#include <cassert>
#include <iostream>
using namespace alienmobile;
int main() {
    auto parent=makeFeederGenome(); DeterministicRng rng(142);
    bool waveform=false,amplitude=false,channel=false,self=false,mode=false,orientation=false;
    unsigned unchanged=0;
    for(unsigned n=0;n<20000;++n) {
        auto kind=n%3==0 ? MutationKind::Behavior : n%3==1 ? MutationKind::Property : MutationKind::Geometry;
        auto result=applyDevelopmentMutation(parent,rng,kind);
        assert(isValidDevelopmentGenome(result.genome));
        assert(result.genome.genes[0].nodes.size()==parent.genes[0].nodes.size());
        for(unsigned i=0;i<parent.genes[0].nodes.size();++i) {
            auto const& a=parent.genes[0].nodes[i].behavior;
            auto const& b=result.genome.genes[0].nodes[i].behavior;
            if(kind==MutationKind::Behavior) assert(a.neural==b.neural);
            waveform|=a.waveform!=b.waveform;amplitude|=a.amplitude!=b.amplitude;
            channel|=a.motorChannel!=b.motorChannel;self|=a.selfWeight!=b.selfWeight;mode|=a.neural!=b.neural;
        }
        orientation|=result.genome.genes[0].orientation!=parent.genes[0].orientation;
        unchanged+=!result.mutated();
    }
    assert(waveform && amplitude && channel && self && mode && orientation);
    // The dangerous relay case: a neural weight/bias edit in dormant DNA must
    // not silence the sensor's currently expressed pass-through processing.
    auto c=SimulationConfig{};c.behavioralSeed=true;
    for(unsigned n=0;n<200;++n) {
        auto child=applyDevelopmentMutation(parent,rng,MutationKind::Behavior).genome;
        assert(!child.genes[0].nodes[1].behavior.neural);
        auto frozen=child;frozen.mutationRates={0,0,0,0,0,0,0,0,0,0,0,0};
        assert(mutateDevelopmentGenome(frozen,rng).genome==frozen);
    }
    auto g=makeDevelopmentFounders()[0];bool moduleOrientation=false;
    for(unsigned n=0;n<200;++n) moduleOrientation|=applyDevelopmentMutation(g,rng,MutationKind::Geometry).genome.genes[1].orientation!=g.genes[1].orientation;
    assert(moduleOrientation);
    // Capacity must not sample repeatedly until a small child happens to fit.
    auto limited=ecosystemPlaytestConfig();limited.openStructuralMutation=true;
    limited.ecosystemSeed=false;limited.maxCellCount=5;limited.metabolismRate=0;
    limited.hazardStrength=0;limited.emissionRate=0;limited.sensorEnergyCost=0;
    World w(limited);w.cells.clear();w.connections.clear();w.angles.clear();w.creatures.clear();w.motes.clear();
    w.addFounder(parent,{},0,.5f);auto id=w.creatures[0].id;
    for(auto& cell:w.cells)cell.energy=1.2f;
    Simulation sim(w,limited);sim.step();
    assert(w.findCreature(id)->proposedOffspring);
    auto proposal=w.findCreature(id)->proposedOffspring->genome;
    for(unsigned n=0;n<100;++n)sim.step();
    assert(w.findCreature(id)->proposedOffspring->genome==proposal);
    assert(w.creatures.size()==1);
    limited.maxCellCount=128;Simulation expanded(w,limited);expanded.step();
    auto offspring=w.findCreature(id)->constructor.offspringCreatureId;
    assert(offspring!=kInvalidId && w.findCreature(offspring)->genome==proposal);
    std::cout<<"20000 localized mutations: bounded, formerly frozen fields reachable; small edits preserve controller mode; no-op="<<unchanged<<'\n';
}
