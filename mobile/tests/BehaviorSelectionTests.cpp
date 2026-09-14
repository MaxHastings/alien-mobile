#include "BehaviorFixture.h"
#include <cassert>
#include <iostream>

struct Outcome { uint64_t births; double exposure; };

Outcome trial(unsigned seed, bool degraded) {
    SimulationConfig c;
    c.hazardStrength=0; c.energySourceRadius=1.8f; c.maxCellCount=120;
    c.geometryMutationProbability=c.nodeAdditionProbability=c.nodeRemovalProbability=0;
    c.oscillatorMutationProbability=c.behaviorMutationProbability=0;
    c.randomSeed=100+seed;
    float angle=seed*6.28318530718f/12;
    c.energySourcePosition={4*std::cos(angle),4*std::sin(angle)};
    auto genome=makeFeederGenome();
    // Same anatomy, sensing, biases, motors and costs; remove only the
    // inherited response to sensory input. Motors can still run on bias.
    if(degraded) for(auto& node:genome.genes[0].nodes)
        if(node.behavior.neural) node.behavior.weights={};
    auto world=specimen(genome,c);
    // Total initial energy is less than the cost of one four-cell child.
    for(auto& cell:world.cells) cell.energy=0.3f;
    assert(world.cells.size()*0.3f < genome.genes[0].nodes.size()*c.constructionEnergy);
    Simulation sim(world,c);
    double exposure=0;
    for(int step=0;step<120*90;++step) {
        sim.step();
        // Diagnostic only; never fed back to biology or used as fitness.
        for(auto const& cell:world.cells)
            exposure+=std::max(0.0f,1-length(cell.position-world.energySource.position)
                /world.energySource.radius)*c.fixedTimeStep;
        if(step%120==0) {
            assert(world.allValuesFinite() && world.allConnectionsValid());
            for(auto const& cell:world.cells) assert(cell.energy>=0);
        }
    }
    return {sim.stats().births,exposure};
}

int main() {
    uint64_t aBirths=0,bBirths=0; unsigned aSuccess=0,bSuccess=0,wins=0;
    double aExposure=0,bExposure=0;
    for(unsigned seed=0;seed<12;++seed) {
        auto a=trial(seed,false), b=trial(seed,true);
        aBirths+=a.births; bBirths+=b.births;
        aSuccess+=a.births>0; bSuccess+=b.births>0; wins+=a.births>b.births;
        aExposure+=a.exposure; bExposure+=b.exposure;
        std::cout<<"paired seed="<<seed<<" A births="<<a.births<<" B births="<<b.births
            <<" A exposure="<<a.exposure<<" B exposure="<<b.exposure<<'\n';
    }
    // Aggregate controlled evidence, not a promise that A wins every encounter.
    assert(wins>=6 && aSuccess>bSuccess && aBirths>bBirths);
    assert(aExposure>bExposure+10);
    auto replay=trial(0,false), original=trial(0,false);
    assert(replay.births==original.births && replay.exposure==original.exposure);
    std::cout<<"selection: A births="<<aBirths<<" B births="<<bBirths
        <<" successful trials="<<aSuccess<<"/12 versus "<<bSuccess<<"/12\n";

    // Stronger actuation buys proportionally more force with real cell energy.
    auto g=makeFeederGenome(); auto a=specimen(g),b=a;
    for(auto& cell:a.cells) cell.currentSignals[Activation]=0.5f;
    for(auto& cell:b.cells) { cell.currentSignals[Activation]=0.5f; cell.behavior.motorStrength*=2; }
    auto costA=updateActuation(a,0.1f,0.035f),costB=updateActuation(b,0.1f,0.035f);
    assert(std::abs(costB-2*costA)<1e-6f);
    assert(length(b.cells[2].thrust-a.cells[2].thrust*2)<1e-6f);

    // Range/sensitivity are paid even in an empty environment.
    SimulationConfig c; c.energySourceStrength=c.hazardStrength=c.metabolismRate=0;
    c.constructionEnergy=10;
    for(auto& node:g.genes[0].nodes) if(node.behavior.role==CellRole::Motor) node.behavior.motorStrength=0;
    a=specimen(g,c);g.genes[0].nodes[1].behavior.sensorRange=6;g.genes[0].nodes[1].behavior.sensitivity=3;
    b=specimen(g,c);Simulation sa(a,c),sb(b,c);
    sa.step();sb.step();
    float ea=0,eb=0;for(auto const& cell:a.cells)ea+=cell.energy;
    for(auto const& cell:b.cells)eb+=cell.energy;
    assert(ea>eb);
    std::cout<<"motor proportional cost and sensor upkeep passed\n";
}
