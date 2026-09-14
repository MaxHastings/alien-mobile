#include "BehaviorFixture.h"
#include "alienmobile/Ecology.h"
#include <cassert>
#include <iostream>
using namespace alienmobile;
SimulationConfig damageConfig() {
    auto c=depthPlaytestConfig();c.ecosystemSeed=false;c.emissionRate=0;
    c.metabolismRate=c.hazardStrength=c.sensorEnergyCost=0;c.constructionEnergy=10;
    return c;
}
void valid(World const& w) {
    assert(w.allConnectionsValid() && w.allValuesFinite());
    for(auto const& cell:w.cells)assert(w.findCreature(cell.creatureId));
    for(auto const& owner:w.creatures) {
        assert(owner.rootCell<w.cells.size());
        assert(w.cells[owner.rootCell].creatureId==owner.id);
        if(owner.fragment) assert(!owner.mature);
    }
}
int main() {
    auto c=damageConfig();
    // Cut the central cell. The root component survives; the distal motor
    // remains physical but cannot actuate, absorb resources or reproduce.
    auto w=specimen(makeLocomotionGenome(),c);w.motes.clear();
    auto rootId=w.creatures[0].id;auto cutId=w.cells[1].id;
    double returned=(w.cells[1].energy+w.cells[1].embodiedEnergy)*c.recycleFraction;
    assert(w.removeCellsAndFragment({cutId}));valid(w);
    assert(w.cells.size()==3 && w.creatures.size()==2);
    assert(w.findCreature(rootId)->mature && w.connections.size()==1);
    assert(std::abs(w.energyLedger.recycled-returned)<1e-5);
    for(auto& cell:w.cells)cell.currentSignals.fill(1);
    assert(updateActuation(w,.01f,c.motorEnergyCost)==0);
    auto fragment=w.cells[1].creatureId;
    assert(w.findCreature(fragment)->fragment);
    w.motes.clear();Simulation sim(w,c);
    for(int i=0;i<490;++i)sim.step();
    assert(w.findCreature(fragment)==nullptr && w.cells.size()==2);valid(w);
    // Direct local attack consumes exactly the contacted cell's material.
    auto a=specimen(makeFeederGenome(),c);a.motes.clear();
    for(auto& cell:a.cells)cell.position+=Vec2{-10,0};
    a.addFounder(makeFeederGenome(),{0,0},0,.4f);
    auto victimId=a.cells[4].id;auto victimOwner=a.cells[4].creatureId;
    auto& organ=a.cells[0];organ.position={-.5f,0};organ.behavior.role=CellRole::Attacker;
    organ.energy=1;organ.rawEnergy=0;organ.currentSignals[Activation]=1;
    a.cells[4].energy=.0001f;a.cells[4].embodiedEnergy=0;
    updateTrophicOrgans(a,.1f);assert(a.cells[4].deathRequested);
    assert(a.cells[5].energy==c.initialCellEnergy);
    Simulation attacked(a,c);attacked.step();valid(a);
    for(auto const& cell:a.cells)assert(cell.id!=victimId);
    assert(a.findCreature(victimOwner)->fragment);
    assert(attacked.stats().cellDeaths==1);
    // Sensor loss removes its only local environmental input.
    auto sensing=specimen(makeFeederGenome(),c);sensing.motes.clear();
    sensing.addMote({2,0},{},1);updateSignals(sensing,.01f);
    assert(sensing.cells[1].currentSignals[EnergyIntensity]>0);
    sensing.removeCellsAndFragment({sensing.cells[1].id});
    for(auto& cell:sensing.cells)cell.currentSignals={};
    updateSignals(sensing,.01f);
    for(auto const& cell:sensing.cells)assert(cell.currentSignals[EnergyIntensity]==0);
    // Constructor removal ends reproduction, including a reserved child.
    auto b=specimen(makeFeederGenome(),c);
    auto childIndex=b.addCreature(1,false,makeFeederGenome());auto childId=b.creatures[childIndex].id;
    b.creatures[0].constructor.offspringCreatureId=childId;
    b.removeCellsAndFragment({b.cells[2].id});
    assert(b.findCreature(childId)); // An unrelated injury must not erase reservations.
    b.removeCellsAndFragment({b.cells[0].id});
    assert(b.findCreature(childId)->fragment);
    Simulation orphaned(b,c);for(int i=0;i<500;++i)orphaned.step();
    // Continued development must not brace against a destroyed sibling.
    auto growing=specimen(makeFeederGenome(),c);
    growing.removeCellsAndFragment({growing.cells[2].id});
    growing.braceGenomeNode(growing.creatures[0].id,3);
    valid(growing);
    // Repeated compaction with monotonically increasing identities.
    for(int cycle=0;cycle<100;++cycle) {
        auto body=specimen(makeHunterGenome(),c);
        while(!body.cells.empty()) {
            body.removeCellsAndFragment({body.cells[body.cells.size()/2].id});valid(body);
        }
        assert(body.creatures.empty() && body.connections.empty() && body.angles.empty());
    }
    std::cout<<"Local attack, functional loss, fragmentation, recycling, reservations and 600 compactions passed\n";
}
