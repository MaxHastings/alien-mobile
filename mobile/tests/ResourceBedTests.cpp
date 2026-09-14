#include "alienmobile/Simulation.h"
#include "alienmobile/Ecology.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace alienmobile;
int main() {
 auto c=evolutionPlaytestConfig();c.randomSeed=77;c.hazardStrength=0;
 // The playable preset must never initialize refillable, anchored food.
 assert(c.physicalResources && c.autonomousResources && !c.spatialResources && c.catalogSeed);
 World world(c);
 assert(world.creatures.size()==4);
 auto catalog=makeCuratedSpecimenCatalog();
 assert(world.creatures.front().genome==catalog[3].genome);
 for(auto const& mote:world.motes) assert(mote.resourceBed<0 && mote.energy==c.moteEnergy);

 // Isolate autonomous patch emissions. Moving a historical fixture source
 // must not create a player-controlled food center in the playable world.
 world.cells.clear();world.creatures.clear();world.connections.clear();world.angles.clear();world.motes.clear();
 world.energyLedger={};world.emissionAccumulator=0;world.ecologicalTime=0;
 for(auto& patch:world.resourcePatches) patch.emissionAccumulator=0;
 world.energySource.position={0,0};
 for(int i=0;i<120;++i) updateResources(world,1.f/120);
 assert(!world.motes.empty());
 auto old=world.motes;
 world.energySource.position={12,0};
 for(std::size_t i=0;i<old.size();++i)
     assert(world.motes[i].id==old[i].id && world.motes[i].position==old[i].position);
 for(int i=0;i<120;++i) updateResources(world,1.f/120);
 assert(world.motes.size()>old.size());
 for(std::size_t i=0;i<old.size();++i) {
     assert(world.motes[i].id==old[i].id);
     // It may drift, but cannot teleport with the light.
     assert(length(world.displacement(old[i].position,world.motes[i].position))<1.0f);
 }
 bool freshNearPatch=false;
 for(std::size_t i=old.size();i<world.motes.size();++i) {
     assert(world.motes[i].resourceBed<0 && world.motes[i].energy<=c.moteEnergy);
     for(unsigned patch=0;patch<world.resourcePatches.size();++patch)
         freshNearPatch|=length(world.displacement(world.resourcePatchPosition(patch),world.motes[i].position))
             <=c.autonomousPatchRadius+1.f;
 }
 assert(freshNearPatch);

 // Capture is a local collision, not source ownership or a body-level grant.
 world.setSimulationConfig([&]{auto fixture=c;fixture.emissionRate=0;return fixture;}());
 auto mote=world.motes.front();
 Genome single;single.genes[0].nodes={{-1,{},true}};
 world.motes.clear();world.addMote(mote.position,{},.4);
 world.addFounder(single,world.wrapped(mote.position+Vec2{3,0}),0,.5f);
 world.cells.front().energy=0;updateResources(world,1.f/120);
 assert(world.cells.front().energy==0);
 world.cells.front().position=mote.position;updateResources(world,1.f/120);
 assert(world.cells.front().energy>0 && world.motes.empty());

 // The same temporary field is sampled by resources and individual cells.
 world.addMote({0,0},{},.4);world.addPlayerCurrent({0,0},{1,0});
 auto particleBefore=world.motes.back().position;updateResources(world,1.f/120);
 assert(world.motes.back().position.x>particleBefore.x);
 updateActuation(world,1.f/120,c.motorEnergyCost);
 assert(world.cells.front().thrust.x>0);
 std::cout<<"autonomous patches, local capture, curated founders, and shared physical current passed\n";
}
