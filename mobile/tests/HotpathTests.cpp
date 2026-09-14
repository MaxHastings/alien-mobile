#include "alienmobile/Simulation.h"
#include "alienmobile/Ecology.h"
#include "alienmobile/SpatialGrid.h"
#include "alienmobile/FrameStepBudget.h"
#include <cassert>
#include <iostream>
using namespace alienmobile;

void frameBudget() {
    double dt=1.0/120;
    FrameStepBudget normal(.008);
    assert(!normal.canStep(0,dt,0));
    assert(normal.canStep(2*dt,dt,0));normal.didStep();
    assert(normal.canStep(dt,dt,.001));normal.didStep();
    assert(!normal.canStep(dt,dt,.009));
    FrameStepBudget overloaded(.008);
    assert(overloaded.canStep(1,dt,.020)); // One tick can always make progress.
    overloaded.didStep();assert(!overloaded.canStep(1,dt,.020));
    FrameStepBudget fast(.008);
    for(int i=0;i<8;++i){assert(fast.canStep(1,dt,0));fast.didStep();}
    assert(!fast.canStep(1,dt,0));
    double remainder=FrameStepBudget::discardBacklog(12.5*dt,dt);
    assert(std::abs(remainder-.5*dt)<1e-12);
}

void ownerLookup() {
    World w;w.creatures.clear();
    for(int i=0;i<20;++i)w.addCreature(i,true);
    auto first=w.creatures[0].id,last=w.creatures.back().id;
    assert(w.findCreature(first)==&w.creatures[0]);
    std::swap(w.creatures.front(),w.creatures.back());
    assert(w.findCreature(first)==&w.creatures.back());
    assert(w.findCreature(last)==&w.creatures.front());
    w.creatures.erase(w.creatures.begin());
    assert(w.findCreature(last)==nullptr);
    auto copy=w;w.creatures.clear();
    assert(copy.findCreature(first)==&copy.creatures.back());
    auto old=copy.creatures[0].id;copy.creatures[0].id=1000000;
    assert(copy.findCreature(old)==nullptr);
    assert(copy.findCreature(1000000)==&copy.creatures[0]);
    w.reset();for(auto const& owner:w.creatures)assert(w.findCreature(owner.id)==&owner);
}

// Exhaustive pair scan as an independent oracle for the indexed CPU path.
void denseRepulsion() {
    for(bool wrapped:{false,true}) {
        auto c=depthPlaytestConfig();c.toroidal=wrapped;c.linearDrag=0;c.springDamping=0;
        World w(c);w.cells.clear();w.creatures.clear();w.connections.clear();w.angles.clear();
        auto owner=w.addCreature(0,true);auto id=w.creatures[owner].id;
        DeterministicRng rng(21);
        for(unsigned i=0;i<160;++i) {
            w.addCell(id,{rng.nextUnit()*2-1,rng.nextUnit()*2-1},{},1,false);
            if(i)w.addConnection(i-1,i,.7f,0);
        }
        // Coincident and disconnected pairs, plus seam interactions.
        w.cells[2].position=w.cells[0].position;
        w.cells[4].position={c.worldMinX+.1f,0};w.cells[6].position={c.worldMaxX-.1f,0};
        auto expected=w;
        for(unsigned i=0;i<w.cells.size();++i)for(unsigned j=i+1;j<w.cells.size();++j) {
            auto delta=w.displacement(w.cells[i].position,w.cells[j].position);auto distance=length(delta);
            if(distance>=c.repulsionDistance || w.hasConnection(i,j))continue;
            auto force=normalizedOr(delta)*(c.repulsionStrength*(c.repulsionDistance-distance));
            expected.cells[i].velocity-=force*c.fixedTimeStep;
            expected.cells[j].velocity+=force*c.fixedTimeStep;
        }
        CpuPhysicsBackend physics(c);physics.step(w,c.fixedTimeStep);
        for(unsigned i=0;i<w.cells.size();++i)assert(w.cells[i].velocity==expected.cells[i].velocity);
    }
}

// Original index-order selection oracle, including occlusion and exact ties.
std::size_t referenceTarget(World const& w) {
    auto c=w.config();auto const& organ=w.cells[0];float best=c.attackRange;
    std::size_t nearest=w.cells.size();
    auto contacts=SpatialGrid(w.cells,c).query(organ.position,c.attackRange+c.cellRadius);
    for(auto j:contacts) {
        auto const& victim=w.cells[j];
        if(!victim.alive || victim.creatureId==organ.creatureId)continue;
        auto ray=w.displacement(organ.position,victim.position);float distance=length(ray);
        if(distance>=best || victim.energy+victim.rawEnergy+victim.embodiedEnergy<=0)continue;
        bool blocked=false;
        for(auto k:contacts) {
            if(k==0 || k==j || !w.cells[k].alive)continue;
            auto offset=w.displacement(organ.position,w.cells[k].position);
            float t=dot(offset,ray)/std::max<double>(1e-8f,lengthSquared(ray));
            if(t>.05f && t<.95f && length(offset-ray*t)<c.cellRadius*.8f){blocked=true;break;}
        }
        if(!blocked){nearest=j;best=distance;}
    }
    return nearest;
}
void attackSelection() {
    for(bool wrapped:{false,true})for(unsigned seed=0;seed<100;++seed) {
        auto c=depthPlaytestConfig();c.toroidal=wrapped;
        World w(c);w.cells.clear();w.creatures.clear();w.connections.clear();w.angles.clear();
        auto a=w.addCreature(0,true),b=w.addCreature(0,true);
        auto aid=w.creatures[a].id,bid=w.creatures[b].id;
        w.addCell(aid,{},{},1,false);w.cells[0].behavior.role=CellRole::Attacker;
        w.cells[0].currentSignals[Activation]=1;
        DeterministicRng rng(seed+1);
        for(unsigned i=0;i<80;++i) {
            w.addCell(bid,{rng.nextUnit()*2-1,rng.nextUnit()*2-1},{},1,false);
            w.cells.back().behavior.role=CellRole::Structural;
        }
        // Equal-distance candidates must keep the smaller index as winner.
        if(seed==0){w.cells[1].position={.01f,0};w.cells[2].position={-.01f,0};}
        auto target=referenceTarget(w);
        updateTrophicOrgans(w,c.fixedTimeStep);
        for(unsigned i=1;i<w.cells.size();++i)assert((w.cells[i].energy<1)==(i==target));
    }
}
int main(){frameBudget();ownerLookup();denseRepulsion();attackSelection();std::cout<<"Hotpath equivalence and frame-budget checks passed\n";}
