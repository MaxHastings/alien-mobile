#include "CreatorCompetenceFixture.h"
void audit(){
 std::cout<<"body,compiled,reproduction,seconds,food,organ_cost,dissipated,death_cause\n";
 for(size_t type:{0u,1u,9u})for(bool compiled:{false,true})for(bool reproduction:{true,false}) {
  auto s=bodies()[type];if(compiled)s.genome=compileCreatorBody(s.genome);
  auto c=evolutionPlaytestConfig();c.randomSeed=44;World w(c);clear(w);Simulation sim(w,c);
  auto id=w.addSpecimenNearby(s,w.resourcePatchPosition(2)+Vec2{8,0});assert(id!=kInvalidId);sim.notifyTopologyChanged();
  if(!reproduction){auto p=w.findCreature(id);p->constructor.status=ConstructorState::Cooldown;p->constructor.timer=1000;}
  double seconds=120,food=0;std::string cause="alive";std::map<unsigned,double> acquired;
  for(int step=0;step<14400;++step){
   auto owner=w.findCreature(id);if(!owner||owner->fragment)break;
   auto root=w.cells[owner->rootCell];
   for(auto idx:w.cellIndicesForCreature(id))acquired[w.cells[idx].id]=w.cells[idx].acquiredEnergy;
   sim.step();owner=w.findCreature(id);
   if(!owner||owner->fragment){seconds=(step+1)/120.;cause=root.deathRequested?"damage":root.starvationTimer>1?"root_starvation":"structural_loss";break;}
  }
  for(auto x:acquired)food+=x.second;
  std::cout<<s.name<<','<<compiled<<','<<reproduction<<','<<seconds<<','<<food<<','<<w.energyLedger.organCost<<','<<w.energyLedger.dissipated<<','<<cause<<std::endl;
 }
}
int main(int argc,char** argv){
 if(argc>1&&std::string(argv[1])=="audit"){audit();return 0;}
 auto set=argc>2 ? holdouts() : bodies();
 std::string mode=argc>1?argv[1]:"baseline";
 if(mode=="catalog-source" || mode=="catalog-flat") {set=makeCuratedSpecimenCatalog();for(auto& s:set){
  if(mode=="catalog-flat")s.genome=editableBody(s.genome);
 }}
 if(mode=="compiled" || mode=="no-direction")for(auto& s:set){s.genome=compileCreatorBody(s.genome);
  if(mode=="no-direction")for(auto& n:s.genome.genes[0].nodes)if(n.behavior.role==CellRole::Motor)
   for(auto ch:{EnergyX,EnergyY,CreatureX,CreatureY})n.behavior.weights[Activation][ch]=0;
 }
 if(mode=="slow-baseline")for(auto& s:set)for(auto& n:s.genome.genes[0].nodes)if(n.behavior.role==CellRole::Motor){n.behavior.biases[Activation]=.3f;n.behavior.weights[Activation][EnergyIntensity]=-.25f;}
 std::cout<<"body,seed,offset,response,survival,food,extracted,converted,births,living_family,path10,rotation10,force10,finite\n";
 for(auto s:set)for(unsigned seed:{42,43,44}){
  auto c=evolutionPlaytestConfig();c.randomSeed=seed;World w(c);Simulation sim(w,c);
  float offset=seed==42?0:seed==43?4:8;
  auto id=w.addSpecimenNearby(s,w.resourcePatchPosition(2)+Vec2{offset,0});if(id==kInvalidId){std::cerr<<"placement failed "<<s.name<<'\n';return 2;}sim.notifyTopologyChanged();
  double survival=120,path=0,rotation=0,force=0;bool alive=true,finite=true;auto old=w.cells[w.findCreature(id)->rootCell].position;Vec2 axis=cellAxis(w,w.findCellForGenomeNode(id,1));
  std::map<unsigned,double> food,extracted,converted;std::set<unsigned> born;
  for(int step=0;step<120*120;++step){sim.step();
   auto owner=w.findCreature(id);if(alive&&(!owner||owner->fragment)){survival=step/120.;alive=false;}
   for(auto i:w.cellIndicesForCreature(id)){auto const& cell=w.cells[i];food[cell.id]=cell.acquiredEnergy;extracted[cell.id]=cell.extractedEnergy;converted[cell.id]=cell.convertedEnergy;if(step<1200)force+=length(cell.motorThrust)/120.;}
   if(step<1200&&owner){auto p=w.cells[owner->rootCell].position;path+=length(p-old);old=p;auto idx=w.findCellForGenomeNode(id,1);if(idx!=kInvalidId){auto a=cellAxis(w,idx);rotation+=std::atan2(axis.x*a.y-axis.y*a.x,dot(axis,a));axis=a;}}
   if(step%120==0){finite &= w.allValuesFinite()&&w.allConnectionsValid();for(auto const& cr:w.creatures)if(cr.ancestorId==id&&cr.generation&&cr.mature&&!cr.fragment)born.insert(cr.id);}
  }
  unsigned living=0;for(auto const& cr:w.creatures)living+=cr.ancestorId==id&&cr.mature&&!cr.fragment;
  auto sum=[](auto const& m){double v=0;for(auto p:m)v+=p.second;return v;};
  std::cout<<s.name<<','<<seed<<','<<offset<<','<<response(s)<<','<<survival<<','<<sum(food)<<','<<sum(extracted)<<','<<sum(converted)<<','<<born.size()<<','<<living<<','<<path<<','<<rotation<<','<<force<<','<<finite<<std::endl;
 }
}
