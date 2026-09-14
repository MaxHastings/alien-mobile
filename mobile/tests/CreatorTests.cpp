#include "alienmobile/Creator.h"
#include "alienmobile/GenomeIO.h"
#include "alienmobile/Simulation.h"
#include <cassert>
#include <sstream>
#include <iostream>
using namespace alienmobile;
int main(){
 auto catalog=makeCuratedSpecimenCatalog();
 for(auto const& s:catalog){
  std::stringstream io;genomeio::writeGenome(io,s.genome);assert(genomeio::readGenome(io)==s.genome);
  auto body=editableBody(s.genome);assert(validCreatorBody(body));
  DevelopmentCursor a,b;auto original=a.next(s.genome),copy=b.next(body);
  while(original){assert(copy);assert(original->physical.behavior==copy->physical.behavior);assert(original->physical.relativePosition==copy->physical.relativePosition);original=a.next(s.genome);copy=b.next(body);}assert(!copy);
  assert(!removeBodyCell(body,0));assert(!changeOrgan(body,0,CellRole::Motor));
  assert(addBodyCell(body,0,{0,1.2}));auto last=body.genes[0].nodes.size()-1;
  assert(changeOrgan(body,last,CellRole::Motor));assert(body.genes[0].nodes[last].behavior.neural);
  assert(!addBodyCell(body,0,{NAN,1}));assert(removeBodyCell(body,last));
 }
 auto attached=catalog[0].genome;
 attached.genes[attached.entryGene].nodes[0].construction.separateOffspring=false;
 assert(!editableBody(attached).genes[0].nodes[0].construction.separateOffspring);
 // Geometry edits change the physical attachment without rewriting organs.
 auto shape=editableBody(catalog[0].genome);auto before=shape;
 size_t tip=shape.genes[0].nodes.size()-1;
 auto edge=shape.genes[0].nodes[tip].relativePosition;
 assert(moveBodyCell(shape,tip,edge*1.1f));
 assert(shape!=before);
 for(size_t i=0;i<shape.genes[0].nodes.size();++i)
  assert(shape.genes[0].nodes[i].behavior==before.genes[0].nodes[i].behavior);
 auto moved=shape;
 assert(!moveBodyCell(shape,0,{1,1}));
 assert(!moveBodyCell(shape,tip,{NAN,1}));
 assert(!moveBodyCell(shape,tip,{0,0}));
 assert(!moveBodyCell(shape,tip,{20,0}));assert(shape==moved);
 Genome empty;empty.genes.clear();
 assert(!changeOrgan(empty,0,CellRole::Motor));assert(!removeBodyCell(empty,0));
 assert(!addBodyCell(empty,0,{1,1}));assert(!moveBodyCell(empty,0,{1,1}));
 for(auto bad:{"", "0 9999999", "0 1 0"}){std::stringstream io(bad);bool failed=false;try{genomeio::readGenome(io);}catch(...){failed=true;}assert(failed);}
 // Normal ecology, normal finite founder energy, unaltered reproduction costs.
 for(size_t which=0;which<catalog.size();++which){
  auto s=catalog[which];s.genome=editableBody(s.genome);
  auto parent=s.genome.genes[0].nodes.size()-1;
  assert(addBodyCell(s.genome,parent,{0,1.15f}));
  assert(changeOrgan(s.genome,parent+1,which%2?CellRole::Depot:CellRole::Motor));
  auto config=evolutionPlaytestConfig();config.randomSeed=42;
  World world(config);Simulation sim(world,config);
  world.cells.clear();world.creatures.clear();world.connections.clear();world.angles.clear();
  auto id=world.addSpecimen(s,world.resourcePatchPosition(0));assert(id!=kInvalidId);sim.notifyTopologyChanged();
  auto start=world.cells[world.findCreature(id)->rootCell].position;
  double thrust=0,acquired=0;unsigned descendants=0,changed=0;float distance=0;
  for(int step=0;step<120*180;++step){sim.step();assert(world.allValuesFinite());assert(world.allConnectionsValid());
   if(step%120==0){if(auto owner=world.findCreature(id)){distance=std::max(distance,length(world.cells[owner->rootCell].position-start));}for(auto const& c:world.cells){thrust+=length(c.motorThrust);acquired+=c.acquiredEnergy;}}
  }
  for(auto const& c:world.creatures)if(c.generation>0 && c.mature){++descendants;changed+=c.genome!=s.genome;}
  std::cout<<s.name<<" edited: births="<<sim.stats().births<<" mutations="<<sim.stats().mutations<<" alive descendants="<<descendants<<" changed="<<changed<<" max distance="<<distance<<" motor force samples="<<thrust<<" food samples="<<acquired<<"\n";
  assert(sim.stats().births>0);if(which!=3)assert(thrust>0);else assert(thrust==0);assert(acquired>0);
 }
}
