// Offline natural selection and replay; no diagnostic feeds into Simulation.
#include "alienmobile/Simulation.h"
#include "DiscoveryGenomeIO.h"
#include <filesystem>
#include <map>
#include <set>
#include <iostream>
#include <sstream>
#include <algorithm>
using namespace alienmobile;
struct Life {int family=-1;unsigned generation=0,children=0;bool mature=false;Genome genome;unsigned parent=kInvalidId,lastSeen=0;int matureAt=-1;};
int main(int argc,char** argv){
 if(argc<8){std::cerr<<"mode seed profile seconds input output ablation [trace]\n";return 2;}
 std::string mode=argv[1],input=argv[5],output=argv[6],ablation=argv[7];unsigned seed=std::stoul(argv[2]),profile=std::stoul(argv[3]),seconds=std::stoul(argv[4]);
 std::filesystem::create_directories(output);
 auto historical=makeGardenFounders();std::vector<SpecimenSnapshot> seeds;char const* seedNames[]={"Dart","Ribbon","Crown","Vault","Lancer"};for(unsigned i=0;i<historical.size();++i)seeds.push_back({seedNames[i],historical[i],.4f,1,""});seeds.push_back({"Primitive",makePrimitiveGenome(),.4f,1,""});seeds.push_back({"Contractile",makeContractileFeederGenome(),.5f,1,""});
 if(input!="seeds") {seeds.clear();std::ifstream in(input);if(!in)throw std::runtime_error("cannot open genome manifest");std::string file;while(in>>file){auto path=std::filesystem::path(file);if(path.is_relative())path=std::filesystem::path(input).parent_path()/path;std::ifstream dna(path);seeds.push_back({std::filesystem::path(file).stem().string(),discovery::readGenome(dna),.4f,1,""});}}
 bool playerWorld=mode=="long-game" || mode=="long-fixed";
 auto c=evolutionPlaytestConfig();c.randomSeed=seed;c.catalogSeed=playerWorld;c.ecosystemSeed=playerWorld;
 // All profiles retain actual autonomous finite-particle ecology.
 if(profile==1){c.emissionRate=6;c.autonomousPatchRadius=2;c.autonomousPatchDrift=2.5f;}
 if(profile==2){c.emissionRate=12;c.autonomousPatchRadius=4;c.moteDriftSpeedMin=.25f;c.moteDriftSpeedMax=.6f;}
 if(profile==3){c.autonomousPatchCycleSeconds=55;c.autonomousPatchDrift=3;}
 if(profile==7){c.autonomousPatchRadius=2.5f;}
 if(profile==8)c.maxCellCount=960;
 if(profile==9)c.autonomousPatchDrift=6;
 if(profile==4){c.worldMinX=-21;c.worldMaxX=21;c.worldMinY=-22;c.worldMaxY=22;}
 if(seed>=4000 && !playerWorld){DeterministicRng env(seed+73);c.emissionRate*=.85f+.3f*env.nextUnit();c.autonomousPatchRadius*=.85f+.3f*env.nextUnit();c.autonomousPatchDrift*=.85f+.3f*env.nextUnit();}
 World w(c);if(seed>=4000 && !playerWorld){DeterministicRng env(seed+97);for(auto& p:w.resourcePatches)p.anchor+=Vec2{env.nextUnit()-.5f,env.nextUnit()-.5f}*2;}
 if(!playerWorld){w.cells.clear();w.connections.clear();w.angles.clear();w.creatures.clear();w.nextCellId=w.nextCreatureId=0;w.energyLedger.organismSeeded=0;}
 if(profile==7)for(auto& p:w.resourcePatches)p.anchor=p.anchor*.45f;
 DeterministicRng sampling(seed+991);std::map<unsigned,Life> lives;std::vector<unsigned> births(seeds.size()),gen(seeds.size()),changed(seeds.size()),late(seeds.size());
 if(playerWorld){
 for(auto& o:w.creatures){int family=-1;for(unsigned j=0;j<seeds.size();++j)if(o.genome==seeds[j].genome)family=j;
 if(family<0)throw std::runtime_error("opening genome not in manifest");
 if(mode=="long-fixed")o.genome.mutationRates={0,0,0,0,0,0,0,0,0,0,0,0,1,1,1};
 lives[o.id]={family,0,0,true,o.genome};}
 if(mode=="long-fixed")for(auto& s:seeds)s.genome.mutationRates={0,0,0,0,0,0,0,0,0,0,0,0,1,1,1};
 }
 if(!playerWorld)for(unsigned j=0;j<seeds.size();++j){auto s=seeds[j];s.initialEnergy=1;
 if(ablation=="length")for(auto& g:s.genome.genes)for(auto& n:g.nodes)if(n.behavior.role==CellRole::Structural)n.relativePosition=normalizedOr(n.relativePosition)*.45001f;
 if(ablation=="tail")for(auto& g:s.genome.genes)for(auto& n:g.nodes)if(n.behavior.role==CellRole::Motor&&n.behavior.motorMode!=MotorMode::Thrust)n.behavior.motorStrength=0;
 if(ablation=="defense")for(auto& g:s.genome.genes)for(auto& n:g.nodes)if(n.behavior.role==CellRole::Defender)n.behavior.role=CellRole::Structural;
 if(ablation=="arms")for(auto& g:s.genome.genes)for(auto& n:g.nodes)if(n.construction.targetGene>=0)n.construction.branches=1;
 if(ablation=="motor")for(auto& g:s.genome.genes)for(auto& n:g.nodes)if(n.behavior.role==CellRole::Motor)n.behavior.motorStrength=0;
 if(ablation=="sensor")for(auto& g:s.genome.genes)for(auto& n:g.nodes)if(n.behavior.role==CellRole::EnergySensor||n.behavior.role==CellRole::CreatureSensor)n.behavior.role=CellRole::Structural;
 if(ablation=="depot")for(auto& g:s.genome.genes)for(auto& n:g.nodes)if(n.behavior.role==CellRole::Depot)n.behavior.role=CellRole::Structural;
 if(ablation=="attack")for(auto& g:s.genome.genes)for(auto& n:g.nodes)if(n.behavior.role==CellRole::Attacker)n.behavior.role=CellRole::Structural;
 if(mode=="evolve")s.genome.mutationRates=MutationRates{};
 if(mode=="fixed")s.genome.mutationRates={0,0,0,0,0,0,0,0,0,0,0,0,1,1,1};
 seeds[j].genome=s.genome;
 // Common starting reserve; placement phase/patch/offset vary independently of DNA.
 unsigned copies=seeds.size()==1?3:1;
 for(unsigned k=0;k<copies;++k){float a=sampling.nextUnit()*6.2831853f;auto pos=w.resourcePatchPosition((j+k+seed)%w.resourcePatches.size());pos+=Vec2{sampling.nextUnit()-.5f,sampling.nextUnit()-.5f}*2;
 auto id=w.addSpecimen(s,pos,a);if(id==kInvalidId)continue;lives[id]={int(j),0,0,true,s.genome};}
 }
 if(profile==5){SpecimenSnapshot prey{"prey",makeGardenFounders()[3],.7f,1,""};
 for(unsigned k=0;k<6;++k){auto pos=w.resourcePatchPosition(k);w.addSpecimen(prey,pos+Vec2{2.5f,2.5f},0);}}
 Simulation sim(w,c);double maxEnergyError=0;std::ofstream history(output+"/history.csv"),trace;
 history<<"second,family,mature,births,max_generation,changed_births,late_births\n";
 if(argc>8){trace.open(output+"/bodies.csv");trace<<"second,id,parent,node,x,y,role,energy\n";}
 std::set<unsigned> archived;unsigned archiveCount=0;
 std::ofstream longevity,poses;std::set<std::string> savedArchitectures;
 auto architecture=[](Genome const& dna){std::ostringstream out;out<<dna.entryGene<<':'<<dna.genes.size();for(auto const& g:dna.genes){out<<'[';for(auto const& n:g.nodes)out<<n.parentNode<<':'<<int(n.behavior.role)<<':'<<int(n.behavior.motorMode)<<':'<<n.construction.targetGene<<':'<<n.construction.branches<<':'<<n.construction.repetitions<<';';out<<']';}return out.str();};
 if(playerWorld){longevity.open(output+"/longevity.csv");longevity<<"second,family,mature,physical_cells,recognizable,tiny,intact,motors,sensors,depots,architectures,births,max_generation\n";
 poses.open(output+"/poses.csv");poses<<"second,id,family,generation,node,parent,x,y,role,vx,vy,thrust\n";}

 for(unsigned step=0;step<seconds*120;++step){
 if(profile==6 && step%28800==21600){auto cc=c;cc.emissionRate=0;w.setSimulationConfig(cc);}
 if(profile==6 && step%28800==0)w.setSimulationConfig(c);
 if(profile==3 && step%7200==2400){auto cc=c;cc.emissionRate=0;w.setSimulationConfig(cc);}
 if(profile==3 && step%7200==0)w.setSimulationConfig(c);
 if(profile==4 && step%3600==0)w.addPlayerCurrent(w.resourcePatchPosition((step/3600)%6),{1,.4f});
 sim.step();
 for(auto const& o:w.creatures)if(!o.fragment){
 auto it=lives.find(o.id);if(it==lives.end()){auto p=lives.find(o.parentId);if(p==lives.end())continue;it=lives.emplace(o.id,Life{p->second.family,o.generation,0,false,o.genome}).first;}
 auto& l=it->second;auto f=l.family;l.parent=o.parentId;l.lastSeen=step;
 if(o.mature && !l.mature){l.mature=true;l.matureAt=step/120;++births[f];gen[f]=std::max(gen[f],o.generation);changed[f]+=o.genome!=seeds[f].genome;late[f]+=step>seconds*60;
 auto p=lives.find(o.parentId);if(p!=lives.end())++p->second.children;}
 // Save actual reproducing descendants, including novel structures. No genome edits.
 if(mode=="evolve" && step%120==0 && o.mature && l.children>=2 && o.generation>=2 && !archived.count(o.id)){
 archived.insert(o.id);if(archiveCount<40){std::string name=output+"/g"+std::to_string(o.id)+".dna";std::ofstream dna(name);discovery::writeGenome(dna,o.genome);dna.close();std::ifstream check(name);if(discovery::readGenome(check)!=o.genome)throw std::runtime_error("DNA roundtrip");
 std::ofstream meta(output+"/archive.csv",std::ios::app);meta<<o.id<<','<<f<<','<<o.generation<<','<<l.children<<','<<step/120<<','<<measureDevelopment(o.genome).cells<<'\n';++archiveCount;}}
 }
 if(step%1200==0 || step+1==seconds*120){if(!w.allValuesFinite()||!w.allConnectionsValid())return 3;
 double retained=w.emissionAccumulator;for(auto const& cell:w.cells)retained+=cell.energy+cell.rawEnergy+cell.embodiedEnergy;for(auto const& m:w.motes)retained+=m.energy;
 auto const& e=w.energyLedger;double error=e.emitted+e.seeded+e.organismSeeded-retained-e.expired-e.dissipated-e.organCost-e.digestionLoss;maxEnergyError=std::max(maxEnergyError,std::abs(error));std::vector<unsigned> mature(seeds.size());for(auto const& o:w.creatures)if(o.mature&&!o.fragment&&lives.count(o.id))++mature[lives[o.id].family];
 for(unsigned f=0;f<seeds.size();++f)history<<step/120<<','<<f<<','<<mature[f]<<','<<births[f]<<','<<gen[f]<<','<<changed[f]<<','<<late[f]<<'\n';}
 if(playerWorld && (step%7200==0 || step+1==seconds*120)){
 struct Snapshot{unsigned adults=0,physical=0,recognizable=0,tiny=0,intact=0,motors=0,sensors=0,depots=0;std::set<std::string> forms;};
 std::vector<Snapshot> snapshot(seeds.size());
 for(auto const& o:w.creatures)if(!o.fragment&&lives.count(o.id)){
 auto family=lives[o.id].family;auto& a=snapshot[family];auto indices=w.cellIndicesForCreature(o.id);a.physical+=indices.size();if(!o.mature)continue;
 ++a.adults;auto form=architecture(o.genome);a.forms.insert(form);a.recognizable+=form==architecture(seeds[family].genome);
 a.tiny+=measureDevelopment(o.genome).cells<=2;a.intact+=indices.size()==measureDevelopment(o.genome).cells;
 for(auto i:indices){auto const& cell=w.cells[i];a.motors+=cell.behavior.role==CellRole::Motor;a.sensors+=cell.behavior.role==CellRole::EnergySensor||cell.behavior.role==CellRole::CreatureSensor;a.depots+=cell.behavior.role==CellRole::Depot;}
 if(step>=72000 && lives[o.id].children>0 && savedArchitectures.size()<64 && savedArchitectures.insert(std::to_string(family)+form).second){std::ofstream dna(output+"/late-"+std::to_string(step/120)+"-g"+std::to_string(o.id)+".dna");discovery::writeGenome(dna,o.genome);}
 }
 for(unsigned f=0;f<seeds.size();++f){auto const& a=snapshot[f];longevity<<step/120<<','<<f<<','<<a.adults<<','<<a.physical<<','<<a.recognizable<<','<<a.tiny<<','<<a.intact<<','<<a.motors<<','<<a.sensors<<','<<a.depots<<','<<a.forms.size()<<','<<births[f]<<','<<gen[f]<<'\n';}longevity.flush();
 }
 if(playerWorld && step%36000==0)for(auto const& cell:w.cells){auto o=w.findCreature(cell.creatureId);if(o->fragment||!lives.count(o->id))continue;
 poses<<step/120<<','<<o->id<<','<<lives[o->id].family<<','<<o->generation<<','<<cell.genomeNode<<','<<o->bodyNodes[cell.genomeNode].parentNode<<','<<cell.position.x<<','<<cell.position.y<<','<<int(cell.behavior.role)<<','<<cell.velocity.x<<','<<cell.velocity.y<<','<<length(cell.motorThrust)<<'\n';}
 if(trace && step%120==0)for(auto const& cell:w.cells){auto o=w.findCreature(cell.creatureId);trace<<step/120<<','<<cell.creatureId<<','<<o->bodyNodes[cell.genomeNode].parentNode<<','<<cell.genomeNode<<','<<cell.position.x<<','<<cell.position.y<<','<<int(cell.behavior.role)<<','<<cell.energy<<'\n';}
 if(w.creatures.empty())break;
 }
 std::ofstream pedigree(output+"/pedigree.csv");pedigree<<"id,family,generation,mature,mature_children,cells,changed,recognizable,parent,last_seen,mature_at\n";
 for(auto const& [id,l]:lives){auto const& base=seeds[l.family].genome;bool recognizable=l.genome.genes.size()==base.genes.size();
 if(recognizable)for(unsigned g=0;g<base.genes.size();++g){auto const& a=base.genes[g].nodes;auto const& b=l.genome.genes[g].nodes;if(a.size()!=b.size()){recognizable=false;break;}for(unsigned n=0;n<a.size();++n)if(a[n].parentNode!=b[n].parentNode||a[n].behavior.role!=b[n].behavior.role||a[n].behavior.motorMode!=b[n].behavior.motorMode||a[n].construction.targetGene!=b[n].construction.targetGene||a[n].construction.branches!=b[n].construction.branches||a[n].construction.repetitions!=b[n].construction.repetitions)recognizable=false;}
 pedigree<<id<<','<<l.family<<','<<l.generation<<','<<l.mature<<','<<l.children<<','<<measureDevelopment(l.genome).cells<<','<<(l.genome!=seeds[l.family].genome)<<','<<recognizable<<','<<l.parent<<','<<l.lastSeen/120<<','<<l.matureAt<<'\n';}
 uint64_t hash=1469598103934665603ULL;auto hashValue=[&](auto const& v){auto bytes=reinterpret_cast<unsigned char const*>(&v);for(unsigned i=0;i<sizeof(v);++i){hash^=bytes[i];hash*=1099511628211ULL;}};
 hashValue(w.rng.state());for(auto const& cell:w.cells){hashValue(cell.id);hashValue(cell.creatureId);hashValue(cell.position);hashValue(cell.velocity);hashValue(cell.energy);hashValue(cell.rawEnergy);hashValue(cell.currentSignals);}
 std::ofstream stateHash(output+"/state-hash.txt");stateHash<<hash<<'\n';
 std::ofstream integrity(output+"/integrity.txt");integrity<<"maximum_energy_error="<<maxEnergyError<<'\n';if(maxEnergyError>.1)return 4;
 for(unsigned f=0;f<seeds.size();++f){unsigned alive=0;for(auto const& o:w.creatures)if(o.mature&&!o.fragment&&lives.count(o.id)&&lives[o.id].family==int(f))++alive;
 std::cout<<seed<<','<<profile<<','<<seeds[f].name<<','<<alive<<','<<births[f]<<','<<gen[f]<<','<<changed[f]<<','<<late[f];unsigned physical=0;for(auto const& cell:w.cells)if(lives.count(cell.creatureId)&&lives[cell.creatureId].family==int(f))++physical;std::cout<<','<<physical<<'\n';}
 if(mode=="seeds")for(unsigned f=0;f<seeds.size();++f){std::ofstream dna(output+"/"+seeds[f].name+".dna");discovery::writeGenome(dna,seeds[f].genome);}
}
