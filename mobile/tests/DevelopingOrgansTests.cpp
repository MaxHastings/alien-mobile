#include "alienmobile/Simulation.h"
#include "alienmobile/Ecology.h"
#include <cassert>
#include <iostream>
using namespace alienmobile;
int main() {
    auto c=depthPlaytestConfig();c.ecosystemSeed=false;c.activeDevelopment=true;
    c.constructionInterval=.01f;c.initialRootEnergy=1.2f;c.emissionRate=0;c.hazardStrength=0;
    c.metabolismRate=0;c.sensorEnergyCost=0;
    World w(c);w.cells.clear();w.connections.clear();w.angles.clear();w.creatures.clear();w.motes.clear();
    auto genome=makePrimitiveGenome();genome.genes[0].nodes.push_back({0,{-1,0},false});
    w.addDevelopingFounder(genome,{},0,.5f);Simulation sim(w,c);
    sim.step(.011f);sim.step(.011f);
    assert(w.cells.size()==3 && !w.creatures[0].mature);
    w.addMote(w.cells[1].position,{},2);
    for(int i=0;i<3;++i)updateSignals(w,.01f);
    double before=0;for(auto const& cell:w.cells)before+=cell.energy;
    auto spent=updateActuation(w,.01f,c.motorEnergyCost);
    double after=0;for(auto const& cell:w.cells)after+=cell.energy;
    assert(length(w.cells[2].motorThrust)>0 && spent>0 && std::abs(before-after-spent)<1e-9);
    c.activeDevelopment=false;w.setSimulationConfig(c);updateSignals(w,.01f);updateActuation(w,.01f,c.motorEnergyCost);
    assert(length(w.cells[2].motorThrust)==0);
    c.heterogeneousEnvironment=true;w.setSimulationConfig(c);
    auto flow=environmentCurrent(w,{5,0});assert(dot(flow,Vec2{5,0}-w.energySource.position)<1e-5);
    updateActuation(w,.01f,c.motorEnergyCost);
    assert(length(w.cells[0].thrust)>0 && length(w.cells[0].motorThrust)==0);
    std::cout<<"developing organs perform paid physical work; legacy dormancy preserved; fluid force is not rendered as motor exhaust\n";
}
