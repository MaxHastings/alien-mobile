#pragma once
// Research-only setup and passive observer. No observer state enters World/Simulation.
#include "alienmobile/Simulation.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
namespace competition {
using namespace alienmobile;
inline SimulationConfig referenceConfig(uint64_t seed) {
    auto c=evolutionPlaytestConfig();
    c.randomSeed=seed;c.primitiveSeed=false;
    c.spatialResources=c.heterogeneousBeds=c.plantedFounders=c.gardenSeed=c.mixedSeed=true;
    c.emissionRate=8;c.worldMinX=c.worldMinY=-16;c.worldMaxX=c.worldMaxY=16;
    return c;
}
inline void initialRegime(World& w,std::string const& regime) {
    if(regime!="fixed" && regime!="small" && regime!="full")throw std::runtime_error("unknown mutation regime");
    for(auto& owner:w.creatures) {
        auto& r=owner.genome.mutationRates;
        if(regime=="full")continue;
        r={0,0,0,0,0,0,0,0,0,0,0,0,1,1,1};
        if(regime=="small"){r.neural=.28f;r.geometry=.18f;}
    }
}
struct Life {
    bool known=false,alive=false;
    uint32_t parent=kInvalidId,ancestry=0,generation=0,expected=0,encoded=0;
    uint32_t geneticMotors=0,geneticSensors=0,mutation=0;
    uint32_t liveCells=0,liveMotors=0,liveSensors=0,children=0,matureChildren=0;
    uint64_t seenAt=0;
    double created=0,matureAt=-1,ended=-1,uptake=0,extracted=0,converted=0;
    double cellSeconds=0,starvingCellSeconds=0,thrustSeconds=0,hazardCellSeconds=0;
    double energy=0,thrust=0;
};
struct Totals {uint64_t created=0,born=0,juvenileDeaths=0,adultDeaths=0;double uptake=0;};
class Observer {
public:
    std::vector<Life> lives;
    std::array<Totals,2> totals{};
    double maxEnergyError=0;
private:
    std::vector<uint32_t> previous;
    std::vector<double> lastUptake,lastExtracted,lastConverted;
public:
    void observe(World const& w,uint64_t step) {
        double t=step*double(w.config().fixedTimeStep),dt=step ? w.config().fixedTimeStep:0;
        if(lives.size()<w.nextCreatureId)lives.resize(w.nextCreatureId);
        std::vector<uint32_t> current;current.reserve(w.creatures.size());
        for(auto const& owner:w.creatures)if(!owner.fragment) {
            auto& life=lives[owner.id];
            if(!life.known) {
                life.known=true;life.alive=true;life.parent=owner.parentId;life.created=t;
                life.generation=owner.generation;life.expected=owner.expectedCells;
                life.mutation=unsigned(owner.birthMutation);
                if(owner.parentId==kInvalidId)life.ancestry=owner.expectedCells>1;
                else {
                    if(owner.parentId>=lives.size() || !lives[owner.parentId].known)throw std::runtime_error("missing observed parent");
                    life.ancestry=lives[owner.parentId].ancestry;++lives[owner.parentId].children;
                    ++totals[life.ancestry].created;
                }
                for(auto const& gene:owner.genome.genes)life.encoded+=gene.nodes.size();
                DevelopmentCursor cursor;
                while(auto node=cursor.next(owner.genome)) {
                    life.geneticMotors+=node->physical.behavior.role==CellRole::Motor;
                    life.geneticSensors+=node->physical.behavior.role==CellRole::EnergySensor;
                }
            }
            if(owner.mature && life.matureAt<0) {
                life.matureAt=t;
                if(life.parent!=kInvalidId){++totals[life.ancestry].born;++lives[life.parent].matureChildren;}
            }
            life.seenAt=step;life.liveCells=life.liveMotors=life.liveSensors=0;life.energy=life.thrust=0;
            current.push_back(owner.id);
        }
        for(auto id:previous)if(lives[id].seenAt!=step) {
            auto& life=lives[id];life.alive=false;life.ended=t;
            if(life.matureAt<0)++totals[life.ancestry].juvenileDeaths;
            else ++totals[life.ancestry].adultDeaths;
        }
        previous=std::move(current);
        if(lastUptake.size()<w.nextCellId){lastUptake.resize(w.nextCellId);lastExtracted.resize(w.nextCellId);lastConverted.resize(w.nextCellId);}
        for(auto const& cell:w.cells) {
            auto& life=lives[cell.creatureId];
            if(!life.known || !life.alive)continue;
            double acquired=std::max(0.0,cell.acquiredEnergy-lastUptake[cell.id]);
            life.uptake+=acquired;totals[life.ancestry].uptake+=acquired;lastUptake[cell.id]=cell.acquiredEnergy;
            life.extracted+=std::max(0.0,cell.extractedEnergy-lastExtracted[cell.id]);lastExtracted[cell.id]=cell.extractedEnergy;
            life.converted+=std::max(0.0,cell.convertedEnergy-lastConverted[cell.id]);lastConverted[cell.id]=cell.convertedEnergy;
            ++life.liveCells;life.liveMotors+=cell.behavior.role==CellRole::Motor;life.liveSensors+=cell.behavior.role==CellRole::EnergySensor;
            life.cellSeconds+=dt;life.starvingCellSeconds+=(cell.starvationTimer>0 ? dt:0);
            life.hazardCellSeconds+=(length(w.displacement(w.hazardSource.position,cell.position))<w.hazardSource.radius ? dt:0);
            double thrust=length(cell.motorThrust);life.thrustSeconds+=thrust*dt;life.thrust+=thrust;life.energy+=cell.energy;
        }
    }
    void sample(World const& w,Simulation const& sim,std::ostream& out) {
        double t=sim.stats().steps*double(w.config().fixedTimeStep);
        double retained=w.emissionAccumulator;
        for(auto const& cell:w.cells)retained+=cell.energy+cell.rawEnergy+cell.embodiedEnergy;
        for(auto const& mote:w.motes)retained+=mote.energy;
        auto const& e=w.energyLedger;double error=e.emitted+e.seeded+e.organismSeeded-retained-e.expired-e.dissipated-e.organCost-e.digestionLoss;
        maxEnergyError=std::max(maxEnergyError,std::abs(error));
        for(unsigned a=0;a<2;++a) {
            unsigned mature=0,juvenile=0,intact=0,moving=0,cells=0,expected=0,motors=0,sensors=0,maxGeneration=0;
            double energy=0;std::array<unsigned,4> regions{};
            for(auto const& owner:w.creatures)if(!owner.fragment) {
                auto const& life=lives[owner.id];if(life.ancestry!=a)continue;
                if(!owner.mature){++juvenile;continue;}
                ++mature;cells+=life.liveCells;expected+=life.expected;motors+=life.liveMotors;sensors+=life.liveSensors;
                intact+=life.liveCells==life.expected && life.liveMotors>0 && life.liveSensors>0;
                moving+=life.thrust>1e-6;energy+=life.energy;maxGeneration=std::max(maxGeneration,owner.generation);
                if(owner.rootCell!=kInvalidId){unsigned b=0;float best=1e9;for(unsigned i=0;i<4;++i){float d=length(w.displacement(w.cells[owner.rootCell].position,w.resourceBedPosition(i)));if(d<best){best=d;b=i;}}++regions[b];}
            }
            auto const& total=totals[a];
            out<<t<<','<<a<<','<<mature<<','<<juvenile<<','<<cells<<','<<expected<<','<<intact<<','<<moving<<','<<motors<<','<<sensors<<','<<energy<<','<<total.created<<','<<total.born<<','<<total.juvenileDeaths<<','<<total.adultDeaths<<','<<total.uptake<<','<<maxGeneration;
            for(auto n:regions)out<<','<<n;
            out<<','<<error<<','<<sim.stats().populationCapReached<<'\n';
        }
    }
    void writeLives(std::ostream& out) const {
        out<<"id,parent,ancestry,generation,created,mature_at,ended,alive,expected_cells,encoded_nodes,genetic_motors,genetic_sensors,mutation,children,mature_children,observed_uptake,extracted,converted,cell_seconds,starving_cell_seconds,thrust_integral,hazard_cell_seconds\n";
        for(unsigned id=0;id<lives.size();++id){auto const& l=lives[id];if(!l.known)continue;
            out<<id<<','<<l.parent<<','<<l.ancestry<<','<<l.generation<<','<<l.created<<','<<l.matureAt<<','<<l.ended<<','<<l.alive<<','<<l.expected<<','<<l.encoded<<','<<l.geneticMotors<<','<<l.geneticSensors<<','<<l.mutation<<','<<l.children<<','<<l.matureChildren<<','<<l.uptake<<','<<l.extracted<<','<<l.converted<<','<<l.cellSeconds<<','<<l.starvingCellSeconds<<','<<l.thrustSeconds<<','<<l.hazardCellSeconds<<'\n';}
    }
};
inline uint64_t stateHash(World const& w) {
    uint64_t hash=1469598103934665603ULL;
    auto add=[&](auto const& v){auto p=reinterpret_cast<unsigned char const*>(&v);for(unsigned i=0;i<sizeof(v);++i){hash^=p[i];hash*=1099511628211ULL;}};
    add(w.rng.state());add(w.ecologicalTime);
    for(auto const& c:w.cells){add(c.id);add(c.creatureId);add(c.position);add(c.velocity);add(c.energy);add(c.rawEnergy);add(c.currentSignals);}
    for(auto const& m:w.motes){add(m.id);add(m.position);add(m.energy);add(m.age);}
    return hash;
}
}
