#pragma once
#include "alienmobile/Creator.h"
#include "alienmobile/Simulation.h"
#include "alienmobile/GenomeIO.h"
#include <iostream>
#include <map>
#include <set>
#include <chrono>
#include <cassert>
using namespace alienmobile;
inline std::vector<SpecimenSnapshot> bodies(){
 std::vector<SpecimenSnapshot> out;
 auto add=[&](std::string name,std::vector<GenomeNode> nodes){SpecimenSnapshot s;s.name=name;s.initialEnergy=1; s.genome.genes[0].nodes=nodes;assert(validCreatorBody(s.genome));out.push_back(s);};
 auto node=[](int parent,Vec2 edge,CellRole role,float angle=0){GenomeNode n(parent,edge,false,seededOrgan(role));n.behavior.axisAngle=angle;return n;};
 auto R=CellRole::Structural,S=CellRole::EnergySensor,M=CellRole::Motor;
 for(int type=0;type<12;++type){
  std::vector<GenomeNode> n={{-1,{},true},node(0,{1,0},S),node(0,{-.8f,.85f},M,-2.325f),node(0,{-.8f,-.85f},M,2.325f)};
  std::string name;
  if(type==0) name="compact";
  if(type==1){name="long";for(int i=0;i<6;++i)n.push_back(node(i?int(n.size()-1):0,{-1.1f,.15f},R));}
  if(type==2){name="asymmetric";n[3].relativePosition={-.6f,-1.5f};n.push_back(node(2,{0,1.1f},R));}
  if(type==3){name="branched";for(int i=0;i<3;++i)n.push_back(node(i+1,{.3f,1.1f},i==0?S:R));}
  if(type==4){name="many-motor";for(int i=0;i<4;++i)n.push_back(node(i?int(n.size()-1):0,{-1.f,0},M,-3.14159265f));}
  if(type==5){name="minimal-motor";n.erase(n.begin()+3);n[2].relativePosition={-1,0};n[2].behavior.axisAngle=3.14159265f;}
  if(type==6){name="multiple-sensors";n.push_back(node(2,{0,1.1f},S,1.2f));n.push_back(node(3,{0,-1.1f},S,-.7f));}
  if(type==7){name="opposed-motors";n[2].behavior.axisAngle=0;n[3].behavior.axisAngle=3.14159265f;}
  if(type==8){name="storage";n.push_back(node(0,{-1.1f,0},CellRole::Depot));}
  if(type==9){name="hunter";n[1].behavior=seededOrgan(CellRole::CreatureSensor);n.push_back(node(1,{.8f,0},CellRole::Attacker));n.push_back(node(4,{.7f,.5f},CellRole::Digestor));}
  if(type==10){name="awkward-spinner";n[2].behavior.axisAngle=-.75f;n[3].behavior.axisAngle=2.4f;for(int i=0;i<5;++i)n.push_back(node(i?int(n.size()-1):0,{-1.2f,0},R));}
  if(type==11){name="no-motor";n[2].behavior=seededOrgan(CellRole::Depot);n[3].behavior=seededOrgan(R);}
  add(name,n);
 }
 return out;
}
// Held out until the compiler and its constants were fixed. Distinct serial,
// radial and irregular topologies, rather than resized copies of the fork.
inline std::vector<SpecimenSnapshot> holdouts(){
 std::vector<SpecimenSnapshot> out;
 auto make=[&](std::string name,std::vector<GenomeNode> n){SpecimenSnapshot s;s.name=name;s.initialEnergy=1;s.genome.genes[0].nodes=n;assert(validCreatorBody(s.genome));out.push_back(s);};
 auto n=[](int parent,Vec2 edge,CellRole role,float a=0){auto x=GenomeNode(parent,edge,false,seededOrgan(role));x.behavior.axisAngle=a;return x;};
 auto M=CellRole::Motor,S=CellRole::EnergySensor,B=CellRole::Structural;
 make("holdout-zigzag",{{-1,{},true},n(0,{1,.4f},M,-.38f),n(1,{1,-.5f},B),n(2,{1,.6f},S,1.2f),n(0,{0,-1.2f},M,1.3f)});
 make("holdout-radial",{{-1,{},true},n(0,{1.2f,0},M,1.57f),n(0,{-.6f,1.04f},M,1.57f),n(0,{-.6f,-1.04f},M,1.57f),n(1,{1.1f,0},S),n(2,{-.55f,.95f},S)});
 make("holdout-hammer",{{-1,{},true},n(0,{0,1.2f},B),n(1,{0,1.2f},S,-1.57f),n(2,{1.2f,0},M,0),n(2,{-1.2f,0},M,3.14f),n(0,{0,-1.1f},CellRole::Depot)});
 make("holdout-rear-eye",{{-1,{},true},n(0,{-1.1f,0},M,3.14f),n(1,{-1.1f,0},S,.8f),n(0,{.7f,1.1f},M,-1),n(0,{.7f,-1.1f},B)});
 make("holdout-life-eye",{{-1,{},true},n(0,{0,1.1f},CellRole::CreatureSensor,-.7f),n(1,{.9f,.5f},CellRole::Attacker),n(2,{.8f,-.6f},CellRole::Digestor),n(0,{-.9f,-.6f},M,2.5f),n(0,{.8f,-.7f},M,.7f)});
 std::vector<GenomeNode> comb={{-1,{},true}};
 for(int k=0;k<7;++k){int p=comb.size()-1;comb.push_back(n(p,{1.05f,0},k==6?S:B));comb.push_back(n(p+1,{0,k%2?1.1f:-1.1f},k%3?M:CellRole::Depot,k%2?-1.57f:1.57f));}
 make("holdout-comb",comb);
 return out;
}
inline void clear(World& w){w.cells.clear();w.creatures.clear();w.connections.clear();w.angles.clear();}
inline double response(SpecimenSnapshot const& s){
 auto c=evolutionPlaytestConfig();World w(c);clear(w);w.motes.clear();auto id=w.addSpecimen(s,{0,0});assert(id!=kInvalidId);
 Vec2 receptor{};bool creature=false;
 for(auto const& cell:w.cells)if(cell.behavior.role==CellRole::EnergySensor || cell.behavior.role==CellRole::CreatureSensor){receptor=cell.position;creature=cell.behavior.role==CellRole::CreatureSensor;break;}
 Genome target;target.genes[0].nodes={{-1,{},true}};unsigned targetCell=kInvalidId;
 if(creature){w.addFounder(target,{10,10},0,0);targetCell=w.cells.size()-1;}
 std::vector<std::vector<float>> outputs;
 for(Vec2 p:std::vector<Vec2>{{2,2},{2,-2},{-2,2},{-2,-2}}){
  w.motes.clear();if(creature)w.cells[targetCell].position=receptor+p;else w.addMote(receptor+p,{},2);
  for(auto& cell:w.cells){cell.currentSignals={};cell.nextSignals={};}
  for(int i=0;i<240;++i)updateSignals(w,c.fixedTimeStep);
  std::vector<float> o;for(auto const& cell:w.cells)if(cell.creatureId==id&&cell.behavior.role==CellRole::Motor)o.push_back(cell.currentSignals[cell.behavior.motorChannel]);outputs.push_back(o);
 }
 double spread=0;for(size_t j=0;j<outputs[0].size();++j){float lo=1,hi=-1;for(auto const& o:outputs){lo=std::min(lo,o[j]);hi=std::max(hi,o[j]);}spread+=hi-lo;}return spread;
}
