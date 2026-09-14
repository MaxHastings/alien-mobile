#include "BehaviorFixture.h"
#include "alienmobile/Ecology.h"
#include <cassert>
#include <algorithm>
#include <iostream>
using namespace alienmobile;
int main() {
    auto c=depthPlaytestConfig();c.ecosystemSeed=false;c.emissionRate=0;c.hazardStrength=c.metabolismRate=0;c.constructionEnergy=10;
    auto g=makeLocomotionGenome();g.genes[0].nodes[1].behavior.role=CellRole::Depot;
    auto w=specimen(g,c);w.motes.clear();
    w.addMote(w.cells[1].position,{},5);updateResources(w,.01f);
    assert(w.cells[1].energy>c.maxCellEnergy && w.cells[1].energy<=w.cellCapacity(w.cells[1]));
    // Diffusion transfers stored energy, respects capacity, and conserves it.
    for(auto& cell:w.cells){cell.energy=0;cell.behavior.role=CellRole::Structural;}
    w.cells[1].behavior.role=CellRole::Depot;w.cells[1].energy=4;w.motes.clear();
    Simulation sim(w,c);sim.step(.1f);double total=0;
    for(auto const& cell:w.cells){total+=cell.energy;assert(cell.energy<=w.cellCapacity(cell));}
    assert(std::abs(total-4)<1e-5 && w.cells[0].energy>0);
    // A delay and leaky integrator process incoming physical-edge signals.
    for(auto mode:{MemoryMode::Delay,MemoryMode::Integrate}) {
        auto b=specimen(makeLocomotionGenome(),c);
        for(auto& cell:b.cells){cell.behavior.role=CellRole::Structural;cell.behavior.signalWeight=0;}
        auto& memory=b.cells[1];memory.behavior.role=CellRole::Memory;
        memory.behavior.memoryMode=mode;memory.behavior.memoryTime=.1f;memory.behavior.signalWeight=1;
        for(int i=0;i<10;++i){b.cells[0].currentSignals.fill(1);updateSignals(b,.01f);}
        float value=memory.currentSignals[Activation];
        if(mode==MemoryMode::Delay) {assert(value==0);b.cells[0].currentSignals.fill(0);updateSignals(b,.01f);assert(memory.currentSignals[Activation]==1);}
        else assert(value>.6f && value<.7f);
    }
    auto extraction=[&](bool defense,bool attached) {
        auto b=specimen(makeLocomotionGenome(),c);
        for(auto& cell:b.cells)cell.position={-10-float(cell.id),0};
        b.addFounder(makeFeederGenome(),{0,0},0,.5f);
        auto& attacker=b.cells[0];attacker.position={-.5f,0};attacker.behavior.role=CellRole::Attacker;
        attacker.currentSignals[Activation]=1;
        auto& defender=b.cells[5];defender.behavior.role=defense ? CellRole::Defender : CellRole::Structural;
        if(!attached)b.connections.erase(std::remove_if(b.connections.begin(),b.connections.end(),[](auto e){return e.cellA==4&&e.cellB==5;}),b.connections.end());
        updateTrophicOrgans(b,.1f);return b.energyLedger.attacked;
    };
    auto plain=extraction(false,true),shielded=extraction(true,true),distant=extraction(true,false);
    assert(plain>0 && shielded<plain && std::abs(distant-plain)<1e-6);
    // Communication is foreign, range-limited, paid and committed synchronously.
    auto radio=specimen(makeFeederGenome(),c);
    radio.addFounder(makeFeederGenome(),{2,0},0,.4f);
    radio.cells[0].behavior.role=CellRole::Sender;radio.cells[0].currentSignals[Activation]=1;
    radio.cells[4].behavior.role=CellRole::Receiver;
    updateSignals(radio,.01f);assert(radio.cells[4].currentSignals[Activation]>.4f);
    assert(radio.energyLedger.organCost>0);
    radio.cells[0].energy=0;radio.cells[0].currentSignals[Activation]=1;
    for(auto& cell:radio.cells)if(cell.id!=radio.cells[0].id)cell.currentSignals={};
    updateSignals(radio,.01f);assert(radio.cells[4].currentSignals[Activation]==0);
    radio.cells[0].energy=1;radio.cells[0].currentSignals[Activation]=1;radio.cells[4].position={10,0};
    updateSignals(radio,.01f);assert(radio.cells[4].currentSignals[Activation]==0);
    std::cout<<"Depot capacity and conservation, timed memory, local paid defense passed\n";
}
