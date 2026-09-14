#include "CreatorCompetenceFixture.h"
#include <sstream>

#include <algorithm>
// Fixed anatomy and a finite receptor stimulus isolate frame and routing errors
// from growth, contact, consumption and ecological selection.
double torque(Genome g,Vec2 target,float rotation=0){
 auto c=evolutionPlaytestConfig();World w(c);clear(w);w.motes.clear();
 SpecimenSnapshot s;s.genome=g;s.initialEnergy=1;auto id=w.addSpecimen(s,{},rotation);assert(id!=kInvalidId);
 auto rotate=[&](Vec2 v){return Vec2{v.x*std::cos(rotation)-v.y*std::sin(rotation),v.x*std::sin(rotation)+v.y*std::cos(rotation)};};
 bool creatureTarget=false;for(auto const& n:g.genes[0].nodes)creatureTarget|=n.behavior.role==CellRole::CreatureSensor;
 if(creatureTarget){Genome prey;prey.genes[0].nodes={{-1,{},true}};w.addFounder(prey,rotate(target),0,0);}else w.addMote(rotate(target),{},2);
 for(int n=0;n<240;++n)updateSignals(w,c.fixedTimeStep);
 updateActuation(w,c.fixedTimeStep,c.motorEnergyCost);
 Vec2 center{};unsigned count=0;for(auto const& cell:w.cells)if(cell.creatureId==id){center+=cell.position;++count;}center=center/float(count);
 double t=0;for(auto const& cell:w.cells)if(cell.creatureId==id){auto r=cell.position-center;t+=r.x*cell.motorThrust.y-r.y*cell.motorThrust.x;}return t;
}
int main(){
 auto fixtures=bodies();auto held=holdouts();fixtures.insert(fixtures.end(),held.begin(),held.end());
 Genome singleton;singleton.genes[0].nodes={{-1,{},true}};assert(compileCreatorBody(singleton)==singleton);
 for(auto const& s:fixtures){
  auto g=compileCreatorBody(s.genome);assert(validCreatorBody(g));assert(g==compileCreatorBody(g));
  auto anatomical=g;
  for(size_t i=0;i<g.genes[0].nodes.size();++i){auto& b=anatomical.genes[0].nodes[i].behavior;auto const& before=s.genome.genes[0].nodes[i].behavior;
   b.weights=before.weights;b.biases=before.biases;b.neural=before.neural;b.signalWeight=before.signalWeight;b.selfWeight=before.selfWeight;b.motorChannel=before.motorChannel;}
  assert(anatomical==s.genome);
  std::stringstream io;genomeio::writeGenome(io,g);assert(genomeio::readGenome(io)==g);
 }
 for(auto s:makeCuratedSpecimenCatalog()){
  assert(prepareCreatorRelease(editableBody(s.genome),s.genome)==s.genome);
  auto edit=editableBody(s.genome);assert(addBodyCell(edit,0,{0,1.1f}));
  assert(prepareCreatorRelease(edit,s.genome)==edit); // A body edit is not permission to replace all control.
 }
 auto compact=compileCreatorBody(fixtures[0].genome);
 auto left=torque(compact,{2,2}),right=torque(compact,{2,-2});
 std::cout<<"left torque="<<left<<" right torque="<<right<<'\n';assert(left>.05&&right<-.05);
 assert(std::abs(torque(compact,{2,2},1.3f)-left)<1e-5);
 assert(std::abs(torque(compact,{2,-2},-2.1f)-right)<1e-5);
 auto hunter=compileCreatorBody(fixtures[9].genome);
 assert(torque(hunter,{2,2})>.05 && torque(hunter,{2,-2})<-.05);
 std::cout<<"creature receptor turns toward either side: passed\n";
 auto blind=compact;blind.genes[0].nodes[1].behavior.role=CellRole::Structural;
 assert(std::abs(torque(blind,{2,2})-torque(blind,{2,-2}))<1e-6);
 // A sensor beyond a replacement-mode motor used to be disconnected. It now
 // reaches the other actuator through ordinary inherited sensory relay rows.
 auto serial=fixtures[0];serial.genome.genes[0].nodes[1].behavior=seededOrgan(CellRole::Structural);
 serial.genome.genes[0].nodes.push_back({2,{0,1.1f},false,seededOrgan(CellRole::EnergySensor)});
 auto oldResponse=response(serial);serial.genome=compileCreatorBody(serial.genome);auto newResponse=response(serial);
 std::cout<<"sensor beyond motor: baseline="<<oldResponse<<" compiled="<<newResponse<<'\n';assert(newResponse>oldResponse+.2);
 // Inheritance through real construction with mutation disabled; ordinary
 // mutation is then explicitly required to alter the compiled neural data.
 auto c=evolutionPlaytestConfig();c.randomSeed=42;World w(c);Simulation sim(w,c);
 auto s=fixtures[0];s.genome=compact;s.genome.mutationRates={0,0,0,0,0,0,0,0,0,0,0,0,1,1,1};
 auto id=w.addSpecimenNearby(s,w.resourcePatchPosition(2));assert(id!=kInvalidId);sim.notifyTopologyChanged();bool child=false;
 for(int n=0;n<120*120&&!child;++n){sim.step();for(auto const& cr:w.creatures)if(cr.parentId==id&&cr.mature&&!cr.fragment){assert(cr.genome==s.genome);child=true;}}
 assert(child);
 auto mutableDNA=compact;mutableDNA.mutationRates={.5f,0,0,0,0,0,0,0,0,0,0,0,1,1,1};DeterministicRng rng(13);bool neuralChange=false;
 for(int n=0;n<100;++n){auto m=mutateDevelopmentGenome(mutableDNA,rng);if(m.kind==MutationKind::Behavior && m.genome!=mutableDNA)neuralChange=true;}assert(neuralChange);
 std::vector<double> times;Genome largest=fixtures[1].genome;
 while(largest.genes[0].nodes.size()<64)largest.genes[0].nodes.push_back({int(largest.genes[0].nodes.size()-1),{-1,0},false});
 for(int n=0;n<1000;++n){auto start=std::chrono::steady_clock::now();auto g=prepareCreatorRelease(largest,fixtures[0].genome);assert(g.genes[0].nodes.size()==64);times.push_back(std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count());}
 std::sort(times.begin(),times.end());std::cout<<"64-cell prepare latency ms median="<<times[500]<<" p95="<<times[950]<<" max="<<times.back()<<'\n';
 assert(times[950]<50);
}
