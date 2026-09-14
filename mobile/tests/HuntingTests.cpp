#include "BehaviorFixture.h"
#include <cassert>
#include <iostream>
using namespace alienmobile;
struct HuntResult { double captured,digested;uint64_t births; };
HuntResult trial(int disabled,bool rich) {
    auto c=ecosystemPlaytestConfig();c.ecosystemSeed=false;c.emissionRate=0;c.hazardStrength=0;
    c.initialCellEnergy=0.4f;c.maxCellCount=100;c.maxCellEnergy=rich?3:1.2f;
    c.behaviorMutationProbability=c.geometryMutationProbability=c.nodeAdditionProbability=c.nodeRemovalProbability=c.oscillatorMutationProbability=0;
    World w(c);w.cells.clear();w.connections.clear();w.angles.clear();w.creatures.clear();w.motes.clear();
    auto g=makeHunterGenome();
    for(auto& node:g.genes[0].nodes) {
        auto& b=node.behavior;
        if(disabled==1 && b.role==CellRole::CreatureSensor) b.role=CellRole::Structural;
        if(disabled==2 && b.role==CellRole::Motor) b.motorStrength=0;
        if(disabled==3 && b.role==CellRole::Attacker) {b.biases={};b.weights={};}
        if(disabled==4 && b.role==CellRole::Digestor) b.role=CellRole::Structural;
    }
    w.addFounder(g,{},0,0.8f);
    auto feeder=makeFeederGenome();for(auto& node:feeder.genes[0].nodes)
        if(node.behavior.role==CellRole::Motor)node.behavior.motorStrength=0;
    for(int j=0;j<12;++j)w.addFounder(feeder,{3.f+(j%3)*1.6f,-2.f+(j/3)*1.4f},0,0.5f);
    if(rich) {
        // Finite energy-rich stationary food organisms isolate the complete
        // trophic pathway. No emitter, replenishment, or target movement.
        for(auto& cell:w.cells)if(cell.creatureId!=w.creatures[0].id)cell.energy=2.5f;
        for(std::size_t i=1;i<w.creatures.size();++i) {
            w.creatures[i].constructor.status=ConstructorState::Cooldown;
            w.creatures[i].constructor.timer=100;
        }
    }
    Simulation sim(w,c);
    for(int n=0;n<120*60;++n) {sim.step();if(n%120==0)assert(w.allValuesFinite()&&w.allConnectionsValid());}
    std::cout<<"hunting fixture rich="<<rich<<" disabled="<<disabled<<" captured="<<w.energyLedger.attacked
        <<" digested="<<w.energyLedger.digested<<" births="<<sim.stats().births<<'\n';
    assert(w.energyLedger.emitted==0);
    return {w.energyLedger.attacked,w.energyLedger.digested,sim.stats().births};
}
HuntResult environmentalTrial(bool machinery) {
    auto c=ecosystemPlaytestConfig();c.ecosystemSeed=false;
    c.behaviorMutationProbability=c.geometryMutationProbability=c.nodeAdditionProbability=c.nodeRemovalProbability=c.oscillatorMutationProbability=0;
    World w(c);w.cells.clear();w.connections.clear();w.angles.clear();w.creatures.clear();w.motes.clear();
    auto g=machinery?makeHunterGenome():makeFeederGenome();
    // Disable attack to exclude cannibalism while retaining all organ upkeep.
    if(machinery)for(auto& node:g.genes[0].nodes)if(node.behavior.role==CellRole::Attacker)node.behavior.biases={};
    w.addFounder(g,{-3,-1},0,0.5f);Simulation sim(w,c);
    for(int n=0;n<120*120;++n)sim.step();
    assert(w.energyLedger.attacked==0);
    // Survival is an ecological result, not an authored hunter requirement.
    // The paired trial below still requires a measured reproductive cost.
    std::cout<<"environment-only machinery="<<machinery<<" births="<<sim.stats().births<<" final population="<<w.creatures.size()<<'\n';
    return {0,0,sim.stats().births};
}
int main() {
    std::cout<<std::unitbuf;
    auto active=trial(0,false),noSensor=trial(1,false),noMotor=trial(2,false),noAttack=trial(3,false),noDigestion=trial(4,false);
    assert(active.captured>noSensor.captured && active.captured>noMotor.captured);
    assert(noAttack.captured==0 && noDigestion.digested==0);
    auto fed=trial(0,true),unarmed=trial(3,true),undigested=trial(4,true);
    assert(fed.births>0 && fed.digested>0);
    assert(unarmed.births==0 && undigested.births==0);
    auto feeder=environmentalTrial(false),machinery=environmentalTrial(true);
    assert(machinery.births*4<feeder.births);
}
