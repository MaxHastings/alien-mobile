#include "alienmobile/Simulation.h"
#include <iostream>
#include <set>
#include <cstdlib>
#include <unordered_map>
using namespace alienmobile;
// Reproducible design instrument, deliberately no equal-share pass threshold.
// argv: profile seed specimen (-1 mixed) ablate mutation seconds
int main(int argc,char** argv) {
 int profile=argc>1?atoi(argv[1]):0,seed=argc>2?atoi(argv[2]):17,which=argc>3?atoi(argv[3]):0;
 bool ablate=argc>4?atoi(argv[4]):0,mutation=argc>5?atoi(argv[5]):0;int seconds=argc>6?atoi(argv[6]):180;
 auto c=evolutionPlaytestConfig();c.randomSeed=seed;c.ecosystemSeed=false;c.catalogSeed=false;
 c.openStructuralMutation=true;c.maxCellCount=240;c.maxMotes=1600;
 c.autonomousResources=false;c.heterogeneousEnvironment=false;c.energySourcePosition={0,0};
 c.energySourceRadius=5;c.emissionRate=7;c.moteDriftSpeedMin=.04f;c.moteDriftSpeedMax=.12f;
 if(profile==1){c.energySourceRadius=2;c.emissionRate=3;c.moteLifetime=30;}
 if(profile==2){c.energySourceRadius=7;c.emissionRate=9;c.moteDriftSpeedMin=.4f;c.moteDriftSpeedMax=.65f;}
 if(profile==3){c.energySourceRadius=3;c.emissionRate=12;}
 if(profile==4){c.energySourceRadius=6;c.emissionRate=8;}
 World w(c);w.cells.clear();w.connections.clear();w.angles.clear();w.creatures.clear();w.motes.clear();
 w.nextCellId=w.nextCreatureId=0;w.nextLineageId=1;w.energyLedger={};
 auto catalog=makeCuratedSpecimenCatalog();std::unordered_map<uint32_t,int> ancestry;Genome baseline[6];
 for(int i=0;i<int(catalog.size());++i) {
   if(which>=0 && which!=i)continue;
   auto s=catalog[i];
   if(!mutation)s.genome.mutationRates={0,0,0,0,0,0,0,0,0,0,0,0,1,1,1};
   if(ablate)for(auto& g:s.genome.genes)for(auto& n:g.nodes) {
      auto& b=n.behavior;
      if(i==0 && b.role==CellRole::EnergySensor)b.role=CellRole::Structural;
      if(i==1 && n.parentNode>=4)n.relativePosition=n.relativePosition*.4f;
      if((i==2) && b.role==CellRole::Motor)b.motorStrength=0;
      if(i==3 && b.role==CellRole::Depot)b.role=CellRole::Structural;
      if(i==4 && b.role==CellRole::Attacker)b.role=CellRole::Structural;
   }
   float angle=(i+seed%6)*1.04719755f;
   Vec2 pos=which<0?Vec2{float(cos(angle))*4,float(sin(angle))*4}:Vec2{};
   auto id=w.addSpecimen(s,pos,which<0?angle:float(seed%6)*1.04719755f);
   if(id==kInvalidId){std::cerr<<"placement failed "<<i<<'\n';return 2;}
   ancestry[id]=i;baseline[i]=s.genome;w.findCreature(id)->lineageId=i+1;w.nextLineageId=i+2;
 }
 // A prey-rich setting uses ordinary low-cost, noncombat founders.
 if(profile==4)for(int n=0;n<6;++n){auto s=catalog[3];s.genome.mutationRates={0,0,0,0,0,0,0,0,0,0,0,0,1,1,1};
   float a=n*1.04719755f;auto id=w.addSpecimen(s,{float(cos(a))*3.5f,float(sin(a))*3.5f},a);if(id!=kInvalidId)w.findCreature(id)->lineageId=99;}
 Simulation sim(w,c);std::set<uint32_t> births[6],variants[6];unsigned gen[6]={}, minBody[6]={999,999,999,999,999,999},maxBody[6]={};std::set<uint32_t> recognizable[6];double alive[6]={},food[6]={},attack[6]={};
 for(int step=0;step<seconds*120;++step){
   if(profile==1){w.energySource.position={float(5*sin(step/120.0*.06)),float(5*cos(step/120.0*.06))};}
   // Finite feast/famine pulses, identical for all genomes.
   if(profile==3 && step%(120*80)==120*20){c.emissionRate=0;w.setSimulationConfig(c);}
   if(profile==3 && step>0 && step%(120*80)==0){c.emissionRate=12;w.setSimulationConfig(c);}
   sim.step();
   for(auto const& o:w.creatures)if(!ancestry.count(o.id) && ancestry.count(o.parentId))ancestry[o.id]=ancestry[o.parentId];
   if(step%120==0){if(!w.allValuesFinite()||!w.allConnectionsValid())return 3;
    for(auto const& o:w.creatures)if(ancestry.count(o.id)&&!o.fragment&&o.mature){int i=ancestry[o.id];alive[i]++;
      gen[i]=std::max(gen[i],o.generation);if(o.parentId!=kInvalidId){births[i].insert(o.id);if(o.genome!=baseline[i])variants[i].insert(o.id);
       auto count=unsigned(measureDevelopment(o.genome).cells);minBody[i]=std::min(minBody[i],count);maxBody[i]=std::max(maxBody[i],count);
       bool same=o.genome.genes.size()==baseline[i].genes.size();
       if(same)for(unsigned g=0;g<o.genome.genes.size();++g){auto const& a=o.genome.genes[g];auto const& b=baseline[i].genes[g];
         if(a.nodes.size()!=b.nodes.size()){same=false;break;}
         for(unsigned n=0;n<a.nodes.size();++n)if(a.nodes[n].parentNode!=b.nodes[n].parentNode || a.nodes[n].behavior.role!=b.nodes[n].behavior.role || !(a.nodes[n].construction==b.nodes[n].construction))same=false;
       }
       if(same)recognizable[i].insert(o.id);}}
   }
 }
 unsigned mature[6]={},cells[6]={};for(auto const& o:w.creatures)if(ancestry.count(o.id)&&!o.fragment){int i=ancestry[o.id];mature[i]+=o.mature;cells[i]+=w.cellIndicesForCreature(o.id).size();}
 for(int i=0;i<int(catalog.size());++i)if(which<0||which==i)std::cout<<profile<<','<<seed<<','<<catalog[i].name<<','<<ablate<<','<<mutation<<','<<seconds<<','<<mature[i]<<','<<cells[i]<<','<<births[i].size()<<','<<alive[i]<<','<<gen[i]<<','<<variants[i].size()<<','<<w.energyLedger.absorbed<<','<<w.energyLedger.attacked<<','<<w.energyLedger.digested<<','<<recognizable[i].size()<<','<<(minBody[i]==999?0:minBody[i])<<','<<maxBody[i]<<'\n';
}
