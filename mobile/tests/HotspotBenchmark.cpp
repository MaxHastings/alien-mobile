#include "alienmobile/Simulation.h"
#include "alienmobile/Behavior.h"
#include "alienmobile/Ecology.h"
#include <chrono>
#include <iostream>
#include <iomanip>
using namespace alienmobile;
int main() {
 for(bool dense:{false,true}) {
  auto c=evolutionPlaytestConfig();c.randomSeed=42;World w(c);
  if(dense)for(auto& cell:w.cells)cell.position=cell.position*.04f;
  Simulation sim(w,c);auto start=std::chrono::steady_clock::now();
  for(int n=0;n<600;++n)sim.step();
  auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
  double checksum=0;for(auto const& cell:w.cells)checksum+=cell.position.x+cell.position.y+cell.energy+cell.rawEnergy;
  auto s=sim.stats();std::cout<<"simulation dense="<<dense<<" ms="<<ms<<" cells="<<w.cells.size()<<" checksum="<<std::setprecision(17)<<checksum<<" resources="<<s.resourcesMs<<" signals="<<s.signalsMs<<" attacks="<<s.attacksMs<<" actuation="<<s.actuationMs<<" development="<<s.developmentMs<<'\n';
 }
 for(int count:{240,1200}) {
  auto c=depthPlaytestConfig(); World w(c);w.cells.clear();w.creatures.clear();w.connections.clear();w.angles.clear();
  auto ci=w.addCreature(0,true);auto id=w.creatures[ci].id;
  DeterministicRng rng(11);
  for(int i=0;i<count;++i){w.addCell(id,{rng.nextUnit()*2-1,rng.nextUnit()*2-1},{},1,false);if(i)w.addConnection(i-1,i,.7f,2.f);}
  CpuPhysicsBackend cpu(c);auto start=std::chrono::steady_clock::now();
  for(int n=0;n<60;++n)cpu.step(w,c.fixedTimeStep);
  auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
  double checksum=0;for(auto const& cell:w.cells)checksum+=cell.position.x+cell.position.y;
  std::cout<<"physics cells="<<count<<" ms="<<ms<<" checksum="<<std::setprecision(17)<<checksum<<'\n';
 }
}
