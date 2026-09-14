#pragma once
#include "alienmobile/Simulation.h"
#include <string>

namespace alienmobile {
// A finite practice tank. The configuration differs only in its initial
// occupants and food supply. All biology, forces, costs, damage and offspring
// remain the ordinary simulation; there is no training or genome replacement.
inline SimulationConfig rehearsalConfig() {
    auto c=evolutionPlaytestConfig();c.randomSeed=913;
    c.emissionRate=0;
    return c;
}
inline std::string observedLifeMessage(World const& w,uint32_t id) {
    auto owner=w.findCreature(id);
    for(auto it=w.lifeEvents.rbegin();it!=w.lifeEvents.rend();++it) {
        if(it->creature!=id || (owner && !owner->fragment && w.ecologicalTime-it->time>4))continue;
        if(it->kind==LifeEventKind::StarvationLoss)return "Cell lost after its usable energy ran out";
        if(it->kind==LifeEventKind::DamageLoss)return "Cell lost to physical attack";
        if(it->kind==LifeEventKind::InvalidDevelopment)return "Offspring development could not complete";
    }
    if(!owner)return "No living body remains in this observation";
    if(owner->fragment)return "Detached remains; constructor no longer connected";
    for(auto const& cell:w.cells)if(cell.creatureId==id && cell.starvationTimer>.2f)
        return "Red cells have run out of usable energy";
    if(owner->constructor.wait==ConstructorState::Capacity)return "Offspring waiting: tank cell limit reached";
    if(owner->constructor.wait==ConstructorState::Energy)return "Constructor waiting for local usable energy";
    if(owner->constructor.wait==ConstructorState::Development)return "Offspring development could not complete";
    if(owner->constructor.status==ConstructorState::Constructing)return "Constructor is paying for new offspring cells";
    return "";
}
class Rehearsal {
public:
    World world;
    Simulation simulation;
    uint32_t subject=kInvalidId;
    unsigned steps=0;
    static constexpr unsigned durationSteps=12*120;
    Vec2 startCenter{},lastCenter{};
    std::vector<Vec2> trail;
    double acquired=0,forceIntegral=0;
    unsigned completedChildren=0;
    explicit Rehearsal(SpecimenSnapshot specimen,float orientation=0)
        :world(rehearsalConfig()),simulation(world,rehearsalConfig()) {
        world.cells.clear();world.connections.clear();world.angles.clear();world.creatures.clear();
        world.motes.clear();world.playerCurrents.clear();world.lifeEvents.clear();world.energyLedger={};
        world.nextCellId=world.nextCreatureId=0;world.nextLineageId=1;world.rng.reset(913);
        // Fixed external food, independent of the subject's body and identity.
        for(Vec2 patch:std::vector<Vec2>{{3,1.4f},{-3,-1.4f},{0,3}})
            for(int i=0;i<24;++i){float a=i*2.399963f,r=.9f*std::sqrt((i+.5f)/24);
                world.addMote(patch+Vec2{std::cos(a),std::sin(a)}*r,{},.12);
                world.energyLedger.seeded+=.12;}
        subject=world.addSpecimen(specimen,{},orientation);simulation.notifyTopologyChanged();
        startCenter=lastCenter=center();trail.push_back(startCenter);
    }
    Vec2 center() const {
        Vec2 sum{};unsigned count=0;
        for(auto const& cell:world.cells)if(cell.creatureId==subject){sum+=cell.position;++count;}
        return count ? sum/float(count) : lastCenter;
    }
    void step() {
        if(steps>=durationSteps || subject==kInvalidId)return;
        simulation.step();++steps;lastCenter=center();
        if(steps%12==0)trail.push_back(lastCenter);
        // Ledger observations are cumulative and survive cell loss.
        acquired=world.energyLedger.absorbed;
        double force=0;for(auto const& cell:world.cells)if(cell.creatureId==subject)force+=length(cell.motorThrust);
        forceIntegral+=force*world.config().fixedTimeStep;
        completedChildren=unsigned(simulation.stats().births);
    }
    std::string message() const {
        if(subject==kInvalidId)return "Body cannot be placed in this practice tank";
        auto causal=observedLifeMessage(world,subject);
        auto owner=world.findCreature(subject);
        if(!owner || owner->fragment || causal.find("lost")!=std::string::npos || causal.find("Red cells")!=std::string::npos)return causal;
        unsigned motors=0,deformers=0,sensors=0;Vec2 net{};float total=0;
        for(auto const& cell:world.cells)if(cell.creatureId==subject){
            if(cell.behavior.role==CellRole::Motor){if(cell.behavior.motorMode==MotorMode::Thrust)++motors;else ++deformers;}
            sensors+=cell.behavior.role==CellRole::EnergySensor;
            net+=cell.motorThrust;total+=length(cell.motorThrust);
        }
        if(!motors)return deformers ? "Flexing changes shape; this body has no thrust organs" : "No thrust organs: food must reach this body";
        if(total>.05f && length(net)<total*.2f)return "Thrust mostly cancels; the body may still turn";
        if(steps>240 && forceIntegral<.01)return "No paid thrust observed yet; inspect motors or try Rewire";
        if(!causal.empty())return causal;
        if(acquired>.05)return "Bright pulses mark food absorbed by real cells";
        if(!sensors)return "No food receptor: thrust cannot respond to nearby food";
        return "Watch the force lines and trail; food is finite";
    }
};
}
