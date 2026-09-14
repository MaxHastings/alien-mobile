#include "alienmobile/Ecology.h"
#include <algorithm>
#include <cmath>
#include "alienmobile/SpatialGrid.h"
#include "alienmobile/ConnectionIndex.h"

namespace alienmobile {
Vec2 environmentCurrent(World const& world,Vec2 position) {
    Vec2 force{};
    // The gesture is sampled here for every individual cell / mote.  No
    // creature center, orientation, lineage, or controller is consulted.
    for(auto const& current:world.playerCurrents) {
        if(current.age>=current.lifetime || current.radius<=0) continue;
        auto offset=world.displacement(current.position,position);
        float normalizedDistance=length(offset)/current.radius;
        if(normalizedDistance>=1) continue;
        float falloff=1-normalizedDistance;
        falloff*=falloff*(3-2*falloff);
        float fade=1-current.age/current.lifetime;
        force+=current.direction*(current.strength*falloff*fade);
        // A transverse component makes an off-centre force torque flexible
        // bodies without ever rotating a creature as a unit.
        force+=Vec2{-current.direction.y,current.direction.x}
            *(dot(offset,Vec2{-current.direction.y,current.direction.x})/current.radius
                *current.strength*.18f*falloff*fade);
    }
    if(!world.config().autonomousResources && world.config().heterogeneousEnvironment && !world.config().spatialResources) {
        auto offset=world.displacement(world.energySource.position,position);
        float radius=length(offset);
        force+=Vec2{-offset.y,offset.x}*(world.config().currentStrength/(3.f+radius));
    }
    return force;
}
void updateResources(World& world,float dt) {
    auto const& c=world.config();
    if(!c.physicalResources) return;
    world.ecologicalTime+=dt;
    for(auto& current:world.playerCurrents) current.age+=dt;
    world.playerCurrents.erase(std::remove_if(world.playerCurrents.begin(),world.playerCurrents.end(),
        [](auto const& current){return current.age>=current.lifetime;}),world.playerCurrents.end());
    if(c.autonomousResources) {
        // Each patch is an independent, slowly moving substrate emitter.
        // It neither measures nor reacts to population density or hunger.
        for(unsigned patch=0;patch<world.resourcePatches.size();++patch) {
            auto& source=world.resourcePatches[patch];
            float pulse=.70f+.30f*std::sin(float(world.ecologicalTime*.19)+source.phase);
            float supplied=std::max(0.f,c.emissionRate)*pulse*dt
                /std::max<std::size_t>(1,world.resourcePatches.size());
            source.emissionAccumulator+=supplied;
            world.energyLedger.emitted+=supplied;
            while(source.emissionAccumulator>=c.moteEnergy && c.moteEnergy>0) {
                float angle=world.rng.nextUnit()*6.28318530718f;
                Vec2 direction{std::cos(angle),std::sin(angle)};
                float radius=std::sqrt(world.rng.nextUnit())*c.autonomousPatchRadius;
                float drift=c.moteDriftSpeedMin+(c.moteDriftSpeedMax-c.moteDriftSpeedMin)*world.rng.nextUnit();
                world.addMote(world.resourcePatchPosition(patch)+direction*radius,direction*drift,c.moteEnergy);
                source.emissionAccumulator-=c.moteEnergy;

            }
        }
        // Retain the aggregate unmaterialized fraction for ledger observers
        // and historical accounting fixtures.
        world.emissionAccumulator=0;
        for(auto const& patch:world.resourcePatches) world.emissionAccumulator+=patch.emissionAccumulator;
    } else if(!c.spatialResources) {
    double phase=world.ecologicalTime*6.28318530718/std::max(1.f,c.resourceCycleSeconds);
    double intensity=c.heterogeneousEnvironment ? 1+.6*std::sin(phase*.7) : 1;
    double emitted=std::max<double>(0.f,c.emissionRate)*intensity*dt;
    world.energyLedger.emitted+=emitted;world.emissionAccumulator+=emitted;
    while(world.emissionAccumulator>=c.moteEnergy && c.moteEnergy>0) {
        float angle=world.rng.nextUnit()*6.28318530718f;
        Vec2 direction{std::cos(angle),std::sin(angle)};
        float radius=std::sqrt(world.rng.nextUnit())*world.energySource.radius;
        float low=std::max(0.f,std::min(c.moteDriftSpeedMin,c.moteDriftSpeedMax));
        float high=std::max(low,std::max(c.moteDriftSpeedMin,c.moteDriftSpeedMax));
        float drift=low+(high-low)*world.rng.nextUnit();
        Vec2 origin=world.energySource.position;
        if(c.heterogeneousEnvironment && world.rng.nextUnit()>=c.backgroundResourceFraction) {
            // A concentrating patch migrates around the movable gold source.
            // Old particles remain physical and can be intercepted downstream.
            origin+=Vec2{float(std::cos(phase)),float(std::sin(phase))}*(world.energySource.radius*.6f);
            radius*=.3f;
        }
        world.addMote(origin+direction*radius,direction*drift,c.moteEnergy);
        world.emissionAccumulator-=c.moteEnergy;
    }
    }
    world.energyLedger.pendingEmission=world.emissionAccumulator;
    SpatialGrid cells(world.cells,c);
    std::vector<unsigned> contacts;
    for(auto& mote:world.motes) {
        if(mote.resourceBed>=0) {
            // Fixed substrate captures light into a finite local reservoir.
            // Uncaptured light never enters the usable-energy ledger.
            float distance=length(world.displacement(world.energySource.position,mote.position));
            double light=std::max(0.f,1.f-distance/std::max(.001f,c.resourceLightRadius));
            double input=std::min(std::max(0.0,c.resourceSiteCapacity-mote.energy),
                std::max(0.f,c.emissionRate)*light*dt/(4*std::max(1u,c.resourceSitesPerBed)));
            mote.energy+=input;world.energyLedger.emitted+=input;
        } else {
            mote.position=world.wrapped(mote.position+(mote.velocity+environmentCurrent(world,mote.position))*dt);mote.age+=dt;
        }
        // Contact interception, shared across simultaneously exposed cells.
        // This avoids vector order deciding who gets an overlapping particle.
        double demand=0;
        cells.query(mote.position,c.cellRadius+.10f,contacts);
        for(auto index:contacts) {auto const& cell=world.cells[index];
            if(cell.alive && cell.viability!=CellViability::Dead && lengthSquared(world.displacement(cell.position,mote.position))
                <=(c.cellRadius+0.10f)*(c.cellRadius+0.10f))
                demand+=std::max<double>(0.f,world.cellCapacity(cell)-cell.energy);
        }
        double delivered=std::min<double>(mote.energy,demand);
        if(demand>0) for(auto index:contacts) {auto& cell=world.cells[index];
            if(cell.alive && cell.viability!=CellViability::Dead && lengthSquared(world.displacement(cell.position,mote.position))
                <=(c.cellRadius+0.10f)*(c.cellRadius+0.10f))
            {
                double share=delivered*std::max<double>(0.f,world.cellCapacity(cell)-cell.energy)/demand;
                cell.energy+=share;cell.acquiredEnergy+=share;if(share>0)cell.absorptionFlash=1;
            }
        }
        mote.harvested+=delivered;
        mote.energy=std::max<double>(0.f,mote.energy-delivered);world.energyLedger.absorbed+=delivered;
        if(mote.resourceBed<0 && mote.age>=c.moteLifetime) {
            world.energyLedger.expired+=mote.energy;mote.energy=0;
        }
    }
    world.motes.erase(std::remove_if(world.motes.begin(),world.motes.end(),[](auto const& m){return m.energy<=0 && m.resourceBed<0;}),world.motes.end());
}

void updateTrophicOrgans(World& world,float dt) {
    auto const& c=world.config();
    for(auto& cell:world.cells) {
        cell.absorptionFlash=std::max(0.f,cell.absorptionFlash-dt*5);
        cell.attackFlash=std::max<double>(0.f,cell.attackFlash-dt*5);
        cell.digestionFlash=std::max<double>(0.f,cell.digestionFlash-dt*3);
    }
    // Raw resources circulate only inside a physical organism, independently
    // of usable energy. No constructor or motor can spend this pool.
    if(!c.localDamage) for(auto const& creature:world.creatures) {
        auto indices=world.cellIndicesForCreature(creature.id);double raw=0;
        for(auto i:indices) raw+=world.cells[i].rawEnergy;
        if(!indices.empty()) for(auto i:indices) world.cells[i].rawEnergy=raw/indices.size();
    }
    SpatialGrid cells(world.cells,c);
    ConnectionIndex connections(world.cells.size(),world.connections);
    std::vector<unsigned> contacts;
    struct Target { unsigned index; float distance; Vec2 ray; };
    std::vector<Target> targets;
    std::vector<Vec2> offsets;
    for(std::size_t i=0;i<world.cells.size();++i) {
        auto& organ=world.cells[i];auto const* owner=world.findCreature(organ.creatureId);
        if(!organ.alive || !owner || !world.organsActive(*owner)) continue;
        if(organ.behavior.role==CellRole::Digestor) {
            // Draw from the connected body's raw pool; conversion itself is
            // bounded by this physical digestor's throughput and spare capacity.
            auto indices=c.localDamage ? std::vector<uint32_t>{uint32_t(i)} : world.cellIndicesForCreature(organ.creatureId);double raw=0,space=0;
            for(auto j:indices){raw+=world.cells[j].rawEnergy;space+=std::max<double>(0.f,world.cellCapacity(world.cells[j])-world.cells[j].energy);}
            double amount=std::min<double>({raw,c.digestionRate*organ.behavior.digestionRate*dt,space/std::max<double>(0.001f,c.digestionEfficiency)});
            if(amount>0) {
                for(auto j:indices) {
                    auto& cell=world.cells[j];cell.rawEnergy=std::max<double>(0.f,cell.rawEnergy-amount*cell.rawEnergy/raw);
                    if(space>0) cell.energy+=amount*c.digestionEfficiency*std::max<double>(0.f,world.cellCapacity(cell)-cell.energy)/space;
                }
                world.energyLedger.digested+=amount*c.digestionEfficiency;organ.convertedEnergy+=amount*c.digestionEfficiency;
                world.energyLedger.digestionLoss+=amount*(1-c.digestionEfficiency);organ.digestionFlash=1;
            }
        }
        if(organ.behavior.role!=CellRole::Attacker) continue;
        float activation=std::max<double>(0.f,organ.currentSignals[Activation]);
        double cost=activation*c.attackEnergyCost*organ.behavior.extractionRate*dt;
        if(cost<=0 || organ.energy<cost) continue;
        organ.energy-=cost;world.energyLedger.organCost+=cost;
        // Local nearest accessible foreign cell. Selection is private to this
        // contact organ; neither a creature AI nor a persistent pursuit target.
        std::size_t nearest=world.cells.size();float best=c.attackRange;
        cells.query(organ.position,c.attackRange+c.cellRadius,contacts);
        targets.clear();offsets.clear();
        for(auto j:contacts) {
            auto const& victim=world.cells[j];
            Vec2 ray=world.displacement(organ.position,victim.position);
            offsets.push_back(ray);
            if(!victim.alive || victim.creatureId==organ.creatureId
                || victim.creatureId==owner->constructor.offspringCreatureId) continue;
            float distance=length(ray);
            if(distance<best && victim.energy+victim.embodiedEnergy+victim.rawEnergy>0)
                targets.push_back({j,distance,ray});
        }
        // The old scan sought the nearest unblocked target in index order.
        // Testing nearest first gives the identical winner (including ties),
        // without ray-testing farther targets after a winner is known.
        std::sort(targets.begin(),targets.end(),[](auto const& a,auto const& b) {
            return a.distance==b.distance ? a.index<b.index : a.distance<b.distance;
        });
        for(auto const& target:targets) {
            bool blocked=false;
            auto denominator=std::max<double>(1e-8f,lengthSquared(target.ray));
            for(std::size_t n=0;n<contacts.size();++n) {
                auto k=contacts[n];
                if(k==i || k==target.index || !world.cells[k].alive) continue;
                auto offset=offsets[n];
                float t=dot(offset,target.ray)/denominator;
                if(t>0.05f && t<0.95f && length(offset-target.ray*t)<c.cellRadius*0.8f) {blocked=true;break;}
            }
            if(!blocked){nearest=target.index;break;}
        }
        if(nearest==world.cells.size()) continue;
        auto& victim=world.cells[nearest];
        double amount=std::min<double>(activation*c.attackRate*organ.behavior.extractionRate*dt,std::max<double>(0.f,4.f-organ.rawEnergy));
        // A defender protects only itself and directly attached neighbors.
        // Protection consumes its own energy; no organism-level armor exists.
        std::vector<uint32_t> defenders{uint32_t(nearest)};
        for(auto slot=connections.first(nearest);slot!=kInvalidId;slot=connections.next(slot)) {
            auto const& edge=world.connections[slot/2];
            if(edge.cellA==nearest)defenders.push_back(edge.cellB);
            else if(edge.cellB==nearest)defenders.push_back(edge.cellA);
        }
        std::sort(defenders.begin(),defenders.end());
        defenders.erase(std::unique(defenders.begin(),defenders.end()),defenders.end());
        for(auto d:defenders) {
            auto& defender=world.cells[d];
            if(defender.creatureId!=victim.creatureId || defender.behavior.role!=CellRole::Defender
                || defender.viability==CellViability::Dead || !world.organsActive(*world.findCreature(defender.creatureId)))continue;
            double blocked=std::min<double>({amount,defender.behavior.defenseStrength*dt,defender.energy/0.35f});
            defender.energy-=blocked*0.35f;world.energyLedger.organCost+=blocked*0.35f;amount-=blocked;
        }
        double transferred=0;
        for(double* reservoir:{&victim.energy,&victim.rawEnergy,&victim.embodiedEnergy}) {
            double take=std::min<double>(*reservoir,amount-transferred);*reservoir-=take;transferred+=take;
        }
        if(transferred>0) {
            organ.rawEnergy+=transferred;organ.extractedEnergy+=transferred;world.energyLedger.attacked+=transferred;
            if(c.localDamage && victim.energy+victim.rawEnergy+victim.embodiedEnergy<=c.starvationEnergyThreshold)
                victim.deathRequested=true;
            organ.attackFlash=victim.attackFlash=1;organ.transferOrigin=victim.position;
        }
    }
}
}
