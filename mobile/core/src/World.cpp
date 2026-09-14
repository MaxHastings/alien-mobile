#include "alienmobile/World.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>
#include <vector>

namespace alienmobile {

World::World(SimulationConfig config)
    : rng(config.randomSeed)
    , _config(config)
{
    reset();
}

void World::setSimulationConfig(SimulationConfig config)
{
    _config = config;
    energySource = EnergySource{config.energySourcePosition, config.energySourceRadius, config.energySourceStrength};
    hazardSource = HazardSource{config.hazardSourcePosition, config.hazardRadius, config.hazardStrength};
}

void World::reset()
{
    reset(_config);
}

void World::reset(SimulationConfig config)
{
    _config = config;
    motes.clear(); resourcePatches.clear(); playerCurrents.clear(); mutagen={};
    nextMoteId=0; emissionAccumulator=0; ecologicalTime=0; energyLedger={};
    cells.clear();
    connections.clear();
    angles.clear();
    creatures.clear();
    genome = config.behavioralSeed ? makeFeederGenome() : makeDefaultGenome();
    energySource = EnergySource{config.energySourcePosition, config.energySourceRadius, config.energySourceStrength};
    hazardSource = HazardSource{config.hazardSourcePosition, config.hazardRadius, config.hazardStrength};
    nextCellId = 0;
    nextCreatureId = 0;
    nextLineageId = 1;
    rng.reset(config.randomSeed);
    if(config.autonomousResources) {
        // These anchors are a property of the world, not of its inhabitants.
        // Keep them deliberately separated so the opening view has geography
        // and negative space before any lineage has had time to reproduce.
        static constexpr Vec2 gardenAnchors[]={{-12,-10},{-11,11},{-1,-15},
                                                {2,3},{12,-9},{12,12}};
        for(unsigned i=0;i<config.autonomousPatchCount;++i) {
            auto anchor=gardenAnchors[i % (sizeof(gardenAnchors)/sizeof(*gardenAnchors))];
            if(i>=sizeof(gardenAnchors)/sizeof(*gardenAnchors)) {
                float a=6.2831853f*float(i)/float(config.autonomousPatchCount);
                anchor={18.f*std::cos(a),18.f*std::sin(a)};
            }
            resourcePatches.push_back({anchor,6.2831853f*rng.nextUnit(),0});
        }
    }
    if(config.ecosystemSeed) {
      if(config.catalogSeed) {
        // A deliberately light opening cast.  They are ordinary catalog
        // snapshots placed once at reset; no catalog material is injected
        // later by the simulation.
        auto catalog=makeCuratedSpecimenCatalog();
        if(catalog.size()>=4) {
            addSpecimen(catalog[3],resourcePatchPosition(0),.18f);
            addSpecimen(catalog[0],resourcePatchPosition(4),-.72f);
            addSpecimen(catalog[1],resourcePatchPosition(1),2.35f);
            addSpecimen(catalog[2],resourcePatchPosition(3),.68f);
        }
      } else if(config.gardenSeed) {
        // The default is intentionally authored initial conditions, not
        // authored outcomes.  Every entry is a normal mature organism with
        // normal construction, energy, damage, mutation and extinction rules.
        auto founders=makeGardenFounders();
        static constexpr Vec2 positions[]={{-19.f,-13.f},{-17.f,13.f},{-5.f,-18.f},
                                            {2.f,4.f},{14.f,-12.f},{18.f,14.f}};
        static constexpr float angles[]={.18f,2.35f,-.72f,3.18f,-2.35f,.68f};
        static constexpr float hues[]={.09f,.34f,.57f,.70f,.84f,.96f};
        for(std::size_t i=0;i<founders.size();++i)
            addFounder(founders[i],positions[i],angles[i],hues[i]);
      } else if(config.spatialResources) {
        for(unsigned i=0;i<8;++i) {
            auto dna=makePrimitiveGenome();
            if(config.minimalOrigin || (config.mixedSeed && i%2==0))dna.genes[0].nodes.resize(1);
            if(config.frozenFounderMutation)dna.mutationRates={0,0,0,0,0,0,0,0,0,0,0,0,1,1,1};
            float a=rng.nextUnit()*6.2831853f;
            auto origin=config.spatialResources ? resourceBedPosition(i/2) : energySource.position;
            auto previous=creatures.size();
            addDevelopingFounder(dna,origin+Vec2{std::cos(a),std::sin(a)},a,.48f+.05f*(i%2));
            if(creatures.size()==previous)continue;
            creatures.back().lineageId=1;
        }
        nextLineageId=2;
      } else if(config.primitiveSeed) {
        auto ancestor=makePrimitiveGenome();
        for(unsigned i=0;i<8;++i) {
            float a=rng.nextUnit()*6.2831853f;
            Vec2 origin=energySource.position;
            if(config.heterogeneousEnvironment)origin.x+=energySource.radius*.6f;
            float radius=config.heterogeneousEnvironment ? energySource.radius*.2f*std::sqrt(rng.nextUnit()) : 1.f+2*rng.nextUnit();
            auto previous=creatures.size();
            addDevelopingFounder(ancestor,origin+Vec2{std::cos(a),std::sin(a)}*radius,a,.54f);
            if(creatures.size()==previous)continue;
            creatures.back().lineageId=1; // Replicates of the same ancestor.
        }
        nextLineageId=2;
      } else if(config.developmentalSeed) {
        auto founders=makeDevelopmentFounders();
        for(std::size_t i=0;i<founders.size();++i) {
            float a=float(i)*1.5707963f;
            addDevelopingFounder(founders[i],energySource.position+Vec2{std::cos(a),std::sin(a)}*1.6f,a,0.32f+0.13f*i);
        }
        addDevelopingFounder(makeFeederGenome(),energySource.position,0,0.54f);
        addDevelopingFounder(makeHunterGenome(),energySource.position+Vec2{-3,1},-.6f,.79f);
        // Founder DNA samples the new primitives; no runtime ecological classes.
        auto reserve=makeFeederGenome();
        reserve.genes[0].nodes.push_back({0,{-0.8f,0},false});
        reserve.genes[0].nodes.back().behavior.role=CellRole::Depot;
        addDevelopingFounder(reserve,energySource.position+Vec2{2,-2},.5f,.40f);
        auto guarded=reserve;
        guarded.genes[0].nodes.push_back({0,{0,.8f},false});
        guarded.genes[0].nodes.back().behavior.role=CellRole::Defender;
        addDevelopingFounder(guarded,energySource.position+Vec2{-2,-2},-.8f,.42f);
        auto temporal=makeContractileFeederGenome();
        temporal.genes[0].nodes[4].behavior.role=CellRole::Memory;
        temporal.genes[0].nodes[4].behavior.memoryMode=MemoryMode::Integrate;
        addDevelopingFounder(temporal,energySource.position+Vec2{2,2},2.f,.62f);
      } else {
        addFounder(makeFeederGenome(),{-3,-1},0,0.54f);
        addFounder(makeFeederGenome(),{2,-2},2.5f,0.56f);
        addFounder(makeFeederGenome(),{-2,3},-1.2f,0.52f);
        addFounder(makeContractileFeederGenome(),{3,3},3.5f,0.47f);
        addFounder(makeHunterGenome(),{-5,3},-0.6f,0.79f);
      }
        energyLedger.organismSeeded=0;
        for(auto const& cell:cells) energyLedger.organismSeeded+=cell.energy+cell.rawEnergy+cell.embodiedEnergy;
        // Finite initial environmental reserve, explicitly accounted separately
        // from the continuous emitter; reset is the only founder injection.
        if(config.spatialResources) {
            for(unsigned bed=0;bed<4;++bed) for(unsigned i=0;i<config.resourceSitesPerBed;++i) {
                float a=rng.nextUnit()*6.2831853f;
                float r=std::sqrt(rng.nextUnit())*(config.heterogeneousBeds ? (1.8f+bed*.9f) : config.resourceBedRadius);
                auto previous=motes.size();
                addMote(resourceBedPosition(bed)+Vec2{std::cos(a),std::sin(a)}*r,{},config.resourceSiteCapacity);
                energyLedger.seeded+=std::max(0.0,config.resourceSiteCapacity);
                if(motes.size()==previous)continue;
                motes.back().resourceBed=int(bed);
                if(config.plantedFounders && i<2) {
                    auto* owner=findCreature(bed*2+i);
                    if(owner)for(auto index:cellIndicesForCreature(owner->id))cells[index].position=motes.back().position;
                }
            }
        } else for(int i=0;i<160;++i) {
            // Reset seeds a short, already-moving resource wake. It is not a
            // reservoir: every packet is free, finite, and expires or is eaten.
            float a=rng.nextUnit()*6.2831853f, r=std::sqrt(rng.nextUnit())
                *(config.autonomousResources ? config.autonomousPatchRadius : config.energySourceRadius);
            Vec2 dir{std::cos(a),std::sin(a)};
            float speed=config.moteDriftSpeedMin+(config.moteDriftSpeedMax-config.moteDriftSpeedMin)*rng.nextUnit();
            Vec2 origin=config.autonomousResources && !resourcePatches.empty()
                ? resourcePatchPosition(unsigned(i)%resourcePatches.size()) : energySource.position;
            addMote(origin+dir*r,dir*std::max(0.f,speed),config.moteEnergy);
            energyLedger.seeded+=config.moteEnergy;
        }
    } else createInitialOrganism();
}

uint32_t World::addCell(uint32_t creatureId, Vec2 position, Vec2 velocity, float energy, bool constructorCell)
{
    auto const index = static_cast<uint32_t>(cells.size());
    auto const* owner=findCreature(creatureId);
    auto node = static_cast<uint32_t>(owner ? owner->bodyNodes.size() : 0);
    cells.push_back(Cell{nextCellId++, creatureId, wrapped(position), velocity, energy, constructorCell, true});
    auto& cell = cells.back();
    cell.genomeNode = node;
    if(_config.physicalResources) cell.embodiedEnergy=std::max(0.f,_config.constructionEnergy-energy);
    auto* creature = findCreature(creatureId);
    if(creature) {
        if(!creature->pendingNode) creature->pendingNode=creature->development.next(creature->genome);
        if(creature->pendingNode) {
            cell.behavior=creature->pendingNode->physical.behavior;
            creature->bodyNodes.push_back(creature->pendingNode->physical);
            creature->pendingNode.reset();
        } else creature->bodyNodes.push_back(GenomeNode{}); // Explicit physics fixtures may add unencoded cells.
    }
    if (constructorCell) cell.behavior.role = CellRole::Constructor;
    return index;
}

uint32_t World::addCreature(uint32_t generation, bool mature, Genome creatureGenome)
{
    if (creatureGenome.genes.empty() || creatureGenome.genes[0].nodes.empty()) {
        creatureGenome = genome.genes[0].nodes.empty() ? makeDefaultGenome() : genome;
    }
    auto const index = static_cast<uint32_t>(creatures.size());
    Creature creature;
    creature.id = nextCreatureId++;
    creature.generation = generation;
    creature.mature = mature;
    creature.genome = std::move(creatureGenome);
    auto summary=measureDevelopment(creature.genome);
    creature.expectedCells=summary.cells;
    creature.developmentFailed=!summary.complete();
    creatures.push_back(std::move(creature));
    return index;
}

void World::addConnection(uint32_t cellA, uint32_t cellB, float restLength, float stiffness)
{
    if (cellA >= cells.size() || cellB >= cells.size() || cellA == cellB) {
        return;
    }
    connections.push_back(Connection{cellA, cellB, restLength, stiffness, restLength});
}

void World::braceGenomeNode(uint32_t creatureId, uint32_t nodeIndex)
{
    auto const* creature = findCreature(creatureId);
    if (!creature || nodeIndex < 2 || nodeIndex >= creature->bodyNodes.size()) return;
    if(_config.articulatedPhysics) {
        auto const& nodes=creature->bodyNodes;
        auto parent=nodes[nodeIndex].parentNode;
        if(parent<0) return;
        int reference=nodes[parent].parentNode;
        if(reference<0) {
            for(uint32_t n=1;n<nodeIndex;++n) if(nodes[n].parentNode==parent
                && findCellForGenomeNode(creatureId,n)!=kInvalidId) {reference=int(n);break;}
        }
        if(reference<0) return;
        std::vector<Vec2> rest(nodes.size());
        for(std::size_t n=1;n<nodes.size();++n) rest[n]=rest[nodes[n].parentNode]+nodes[n].relativePosition;
        auto u=rest[reference]-rest[parent],v=rest[nodeIndex]-rest[parent];
        float theta=std::atan2(u.x*v.y-u.y*v.x,dot(u,v));
        auto a=findCellForGenomeNode(creatureId,reference),center=findCellForGenomeNode(creatureId,uint32_t(parent));
        auto b=findCellForGenomeNode(creatureId,nodeIndex);
        if(a==kInvalidId || center==kInvalidId || b==kInvalidId || !hasConnection(a,center) || !hasConnection(b,center))return;
        angles.push_back({a,center,b,theta,theta,_config.angularStiffness*nodes[nodeIndex].stiffness});
        return;
    }
    // Flexible joints cannot be locked by a second, fixed triangular brace.
    auto const& gene=creature->bodyNodes[nodeIndex].behavior;
    if(gene.role==CellRole::Motor && gene.motorMode==MotorMode::Contractile) return;
    // A second spring triangulates each added node. Genomic angles then
    // survive collisions instead of being cosmetic initial placement only.
    std::vector<Vec2> rest(creature->bodyNodes.size());
    for (std::size_t i = 1; i < rest.size(); ++i) {
        auto const& node = creature->bodyNodes[i];
        rest[i] = rest[node.parentNode] + node.relativePosition;
    }
    uint32_t nearest = kInvalidId;
    float distance = 1e9f;
    for (uint32_t i = 0; i < nodeIndex; ++i) {
        if (i == static_cast<uint32_t>(creature->bodyNodes[nodeIndex].parentNode)) continue;
        float d = length(rest[i] - rest[nodeIndex]);
        if (d > 0.05f && d < distance) { distance = d; nearest = i; }
    }
    if (nearest != kInvalidId) addConnection(findCellForGenomeNode(creatureId, nearest),
        findCellForGenomeNode(creatureId, nodeIndex), distance, _config.springStiffness*creature->bodyNodes[nodeIndex].stiffness);
}

bool World::removeCreatureAndCells(uint32_t creatureId)
{
    auto const target = std::find_if(creatures.begin(), creatures.end(), [creatureId](Creature const& creature) {
        return creature.id == creatureId;
    });
    if (target == creatures.end()) {
        return false;
    }

    // A parent that dies cannot finish its in-flight child. Remove that
    // immature branch with it so no orphan constructor state survives.
    std::unordered_set<uint32_t> removedIds{creatureId};
    bool expanded = true;
    while (expanded) {
        expanded = false;
        for (auto const& creature : creatures) {
            if (removedIds.count(creature.id) == 0 || creature.constructor.offspringCreatureId == kInvalidId) {
                continue;
            }
            if (removedIds.insert(creature.constructor.offspringCreatureId).second) {
                expanded = true;
            }
        }
    }

    std::vector<uint32_t> oldToNew(cells.size(), kInvalidId);
    std::vector<Cell> compactedCells;
    compactedCells.reserve(cells.size());
    for (uint32_t oldIndex = 0; oldIndex < cells.size(); ++oldIndex) {
        auto const& cell = cells[oldIndex];
        if (!cell.alive || removedIds.count(cell.creatureId) != 0) {
            if(_config.physicalResources) {
                double total=cell.energy+cell.rawEnergy+cell.embodiedEnergy;
                double returned=total*_config.recycleFraction;
                addMote(cell.position,cell.velocity*0.3f,returned);
                energyLedger.recycled+=returned;
                energyLedger.dissipated+=total-returned;
            }
            continue;
        }
        oldToNew[oldIndex] = static_cast<uint32_t>(compactedCells.size());
        compactedCells.push_back(cell);
    }

    std::vector<Connection> compactedConnections;
    compactedConnections.reserve(connections.size());
    for (auto const& connection : connections) {
        if (connection.cellA >= oldToNew.size() || connection.cellB >= oldToNew.size()) {
            continue;
        }
        auto const newA = oldToNew[connection.cellA];
        auto const newB = oldToNew[connection.cellB];
        if (newA == kInvalidId || newB == kInvalidId || newA == newB) {
            continue;
        }
        auto copy=connection; copy.cellA=newA;copy.cellB=newB;compactedConnections.push_back(copy);
    }

    std::vector<Creature> remainingCreatures;
    remainingCreatures.reserve(creatures.size());
    for (auto creature : creatures) {
        if (removedIds.count(creature.id) != 0) {
            continue;
        }
        if (creature.rootCell != kInvalidId && creature.rootCell < oldToNew.size()) {
            creature.rootCell = oldToNew[creature.rootCell];
        }
        if (creature.rootCell == kInvalidId) {
            creature.mature = false;
        }
        if (removedIds.count(creature.constructor.offspringCreatureId) != 0) {
            creature.constructor = ConstructorState{};
        }
        remainingCreatures.push_back(std::move(creature));
    }

    std::vector<AngularConstraint> survivingAngles;
    for(auto angle:angles) {
        if(angle.cellA>=oldToNew.size() || angle.center>=oldToNew.size() || angle.cellB>=oldToNew.size()) continue;
        angle.cellA=oldToNew[angle.cellA];angle.center=oldToNew[angle.center];angle.cellB=oldToNew[angle.cellB];
        if(angle.cellA!=kInvalidId && angle.center!=kInvalidId && angle.cellB!=kInvalidId) survivingAngles.push_back(angle);
    }
    angles=std::move(survivingAngles);
    cells = std::move(compactedCells);
    connections = std::move(compactedConnections);
    creatures = std::move(remainingCreatures);
    return true;
}

void World::rebuildCreatureIndices() const
{
    _creatureIndices.clear();
    _creatureIndices.reserve(creatures.size());
    for(std::size_t i=0;i<creatures.size();++i) _creatureIndices.emplace(creatures[i].id,i);
}

Creature* World::findCreature(uint32_t creatureId)
{
    return const_cast<Creature*>(static_cast<World const&>(*this).findCreature(creatureId));
}

Creature const* World::findCreature(uint32_t creatureId) const
{
    if(_creatureIndices.size()!=creatures.size()) rebuildCreatureIndices();
    auto it=_creatureIndices.find(creatureId);
    if(it!=_creatureIndices.end() && it->second<creatures.size()
        && creatures[it->second].id==creatureId) return &creatures[it->second];
    // An absent ID can be a same-size public-vector replacement. Avoid
    // rebuilding/allocating the whole map for ordinary missing-owner probes.
    auto found=std::find_if(creatures.begin(),creatures.end(),[&](auto const& owner) {
        return owner.id==creatureId;
    });
    if(found==creatures.end()) return nullptr;
    rebuildCreatureIndices();
    return &*found;
}

uint32_t World::findCellForGenomeNode(uint32_t creatureId, uint32_t nodeIndex) const
{
    for (uint32_t cellIndex = 0; cellIndex < cells.size(); ++cellIndex) {
        auto const& cell = cells[cellIndex];
        if (cell.alive && cell.creatureId == creatureId) {
            if (cell.genomeNode == nodeIndex) {
                return cellIndex;
            }
        }
    }
    return kInvalidId;
}

std::vector<uint32_t> World::cellIndicesForCreature(uint32_t creatureId) const
{
    std::vector<uint32_t> result;
    for (uint32_t cellIndex = 0; cellIndex < cells.size(); ++cellIndex) {
        if (cells[cellIndex].alive && cells[cellIndex].creatureId == creatureId) {
            result.push_back(cellIndex);
        }
    }
    return result;
}

bool World::hasConnection(uint32_t cellA, uint32_t cellB) const
{
    for (auto const& connection : connections) {
        if ((connection.cellA == cellA && connection.cellB == cellB) || (connection.cellA == cellB && connection.cellB == cellA)) {
            return true;
        }
    }
    return false;
}

bool World::allConnectionsValid() const
{
    for(auto const& angle:angles) {
        if(angle.cellA>=cells.size() || angle.center>=cells.size() || angle.cellB>=cells.size()
            || angle.cellA==angle.cellB || !std::isfinite(angle.targetAngle) || !std::isfinite(angle.baseAngle)
            || !std::isfinite(angle.stiffness) || !hasConnection(angle.cellA,angle.center) || !hasConnection(angle.cellB,angle.center)) return false;
    }
    for (auto const& connection : connections) {
        if (connection.cellA >= cells.size() || connection.cellB >= cells.size() || connection.cellA == connection.cellB
            || !cells[connection.cellA].alive || !cells[connection.cellB].alive
            || !std::isfinite(connection.restLength) || !std::isfinite(connection.stiffness)) {
            return false;
        }
    }
    return true;
}

bool World::allValuesFinite() const
{
    for(auto const& mote:motes) if(!isFinite(mote.position)||!isFinite(mote.velocity)
        ||!std::isfinite(mote.energy)||mote.energy<0||!std::isfinite(mote.age)||!std::isfinite(mote.harvested)) return false;
    for(auto const& cell:cells) if(!std::isfinite(cell.rawEnergy)||cell.rawEnergy<0
        ||!std::isfinite(cell.embodiedEnergy)||cell.embodiedEnergy<0) return false;
    for (auto const& cell : cells) {
        for(float value:cell.memoryState)if(!std::isfinite(value) || std::abs(value)>1.00001f)return false;
        for(auto const& signals:cell.signalHistory)for(float value:signals)if(!std::isfinite(value) || std::abs(value)>1.00001f)return false;
        if (!isFinite(cell.thrust) || !std::isfinite(cell.starvationTimer) || !std::isfinite(cell.signalAge)) return false;
        for(float value:cell.currentSignals) if(!std::isfinite(value) || std::abs(value)>1.00001f) return false;
        for(float value:cell.nextSignals) if(!std::isfinite(value) || std::abs(value)>1.00001f) return false;
        if (!isFinite(cell.position) || !isFinite(cell.velocity) || !std::isfinite(cell.energy) || !std::isfinite(cell.visualAge)) {
            return false;
        }
    }
    for (auto const& connection : connections) {
        if (!std::isfinite(connection.restLength) || !std::isfinite(connection.stiffness)) {
            return false;
        }
    }
    for (auto const& creature : creatures) {
        if (!std::isfinite(creature.lineageHue) || !std::isfinite(creature.starvationTimer)
            || !std::isfinite(creature.visualAge) || !isValidDevelopmentGenome(creature.genome)) {
            return false;
        }
    }
    return true;
}

std::size_t World::matureCreatureCount() const
{
    std::size_t result = 0;
    for (auto const& creature : creatures) {
        if (creature.mature) {
            ++result;
        }
    }
    return result;
}

std::size_t World::aliveCellCount() const
{
    std::size_t result = 0;
    for (auto const& cell : cells) {
        result += cell.alive ? 1 : 0;
    }
    return result;
}

std::size_t World::lineageCount() const
{
    std::unordered_set<uint32_t> lineages;
    for (auto const& creature : creatures) {
        lineages.insert(creature.lineageId);
    }
    return lineages.size();
}

Vec2 World::displacement(Vec2 from, Vec2 to) const {
    auto d=to-from;
    if(_config.toroidal) {
        float w=_config.worldMaxX-_config.worldMinX,h=_config.worldMaxY-_config.worldMinY;
        d.x-=w*std::round(d.x/w);d.y-=h*std::round(d.y/h);
    }
    return d;
}
Vec2 World::wrapped(Vec2 p) const {
    if(_config.toroidal) {
        float w=_config.worldMaxX-_config.worldMinX,h=_config.worldMaxY-_config.worldMinY;
        p.x-=(std::floor((p.x-_config.worldMinX)/w))*w;
        p.y-=(std::floor((p.y-_config.worldMinY)/h))*h;
    }
    return p;
}

Vec2 World::resourcePatchPosition(unsigned patch) const
{
    if(patch>=resourcePatches.size()) return {};
    auto const& p=resourcePatches[patch];
    auto const cycle=std::max(1.f,_config.autonomousPatchCycleSeconds);
    float t=float(ecologicalTime/cycle*6.28318530718)+p.phase;
    // Slow, pre-authored substrate movement: it never observes organisms.
    return wrapped(p.anchor+Vec2{std::cos(t),std::sin(t*.73f)}*_config.autonomousPatchDrift);
}

void World::addPlayerCurrent(Vec2 position, Vec2 direction, float strength)
{
    if(lengthSquared(direction)<.0001f) return;
    auto const& c=_config;
    playerCurrents.push_back({wrapped(position),normalizedOr(direction),0,
        c.playerCurrentLifetime,c.playerCurrentRadius,
        strength>=0 ? strength : c.playerCurrentStrength});
    // Recent segments blend into a gesture trail; bounding this also makes
    // repeated interaction safe under a long held drag.
    if(playerCurrents.size()>48) playerCurrents.erase(playerCurrents.begin(),playerCurrents.begin()+8);
}
bool World::applyMutagen(Vec2 position) {
    if(mutagen.remaining>0 || !std::isfinite(position.x) || !std::isfinite(position.y)
        || position.x<_config.worldMinX+4 || position.x>_config.worldMaxX-4
        || position.y<_config.worldMinY+4 || position.y>_config.worldMaxY-4) return false;
    mutagen={position,20.f,4.f};
    return true;
}
float World::mutationExposure(Vec2 position) const {
    return mutagen.remaining>0 && length(displacement(position,mutagen.position))<=mutagen.radius ? 1.5f : 1.f;
}
bool World::scatterFood(Vec2 position) {
    constexpr unsigned count=48;
    if(!std::isfinite(position.x)||!std::isfinite(position.y)
        || position.x<_config.worldMinX+2 || position.x>_config.worldMaxX-2
        || position.y<_config.worldMinY+2 || position.y>_config.worldMaxY-2
        || motes.size()+count>_config.maxMotes) return false;
    for(unsigned i=0;i<count;++i) {
        float a=rng.nextUnit()*6.2831853f,r=std::sqrt(rng.nextUnit())*1.8f;
        Vec2 d{std::cos(a),std::sin(a)};
        addMote(position+d*r,d*.22f,_config.moteEnergy);
        energyLedger.seeded+=_config.moteEnergy;
    }
    return true;
}

void World::addMote(Vec2 p, Vec2 v, double e) {
    if(e<=0) return;
    if(motes.size()>=_config.maxMotes) {
        // Overflow is an explicit radiation sink. Never relocate new food to
        // a distant existing particle merely to satisfy the allocation cap.
        energyLedger.expired+=e;return;
    }
    motes.push_back({nextMoteId++,wrapped(p),v,e,0});
}
void World::addDevelopingFounder(Genome g, Vec2 p, float angle, float hue) {
    auto summary=measureDevelopment(g);
    if(!summary.complete() || cells.size()>=_config.maxCellCount) return;
    auto ci=addCreature(0,false,std::move(g));auto id=creatures[ci].id;
    auto root=addCell(id,p,{},_config.initialRootEnergy,true);
    auto& owner=creatures[ci];
    owner.ancestorId=owner.id;owner.rootCell=root;owner.birthAngle=angle;owner.lineageHue=hue;owner.lineageId=nextLineageId++;
    owner.developingFounder=true;
    owner.constructor.status=ConstructorState::Constructing;
    owner.constructor.offspringCreatureId=id;owner.constructor.nextNode=1;
}

void World::addFounder(Genome g, Vec2 p, float angle, float hue) {
    auto ci=addCreature(0,true,g);auto id=creatures[ci].id;
    creatures[ci].ancestorId=creatures[ci].id;creatures[ci].lineageId=nextLineageId++;creatures[ci].lineageHue=hue;
    for(uint32_t n=0;n<creatures[ci].expectedCells;++n) {
        auto& owner=creatures[ci];owner.pendingNode=owner.development.next(owner.genome);
        if(!owner.pendingNode) break;
        auto const node=owner.pendingNode->physical;Vec2 position=p;
        if(n) {
            Vec2 edge{std::cos(angle)*node.relativePosition.x-std::sin(angle)*node.relativePosition.y,
                      std::sin(angle)*node.relativePosition.x+std::cos(angle)*node.relativePosition.y};
            position=cells[findCellForGenomeNode(id,node.parentNode)].position+edge;
        }
        auto i=addCell(id,position,{},_config.initialCellEnergy,node.constructorCell);
        if(!n) creatures[ci].rootCell=i;
        else {addConnection(findCellForGenomeNode(id,node.parentNode),i,length(node.relativePosition),_config.springStiffness*node.stiffness);braceGenomeNode(id,n);}
    }
}

uint32_t World::addSpecimen(SpecimenSnapshot const& specimen, Vec2 position, float angle)
{
    if(!isValidDevelopmentGenome(specimen.genome) || !std::isfinite(position.x)
        || !std::isfinite(position.y) || !std::isfinite(angle)
        || !std::isfinite(specimen.initialEnergy) || specimen.initialEnergy<=0) return kInvalidId;
    auto summary=measureDevelopment(specimen.genome);
    std::size_t reserved=cells.size();
    for(auto const& pending:creatures) if(!pending.mature && !pending.fragment && !pending.developmentFailed)
        reserved+=pending.expectedCells>pending.bodyNodes.size() ? pending.expectedCells-pending.bodyNodes.size() : 0;
    if(!summary.complete() || summary.cells>_config.maxCellCount
        || reserved>_config.maxCellCount-summary.cells) return kInvalidId;
    // Preflight the complete developed body. Rejection is atomic: no partial
    // organisms, consumed identities, or cells outside the tank.
    DevelopmentCursor cursor;std::vector<Vec2> positions;
    while(auto node=cursor.next(specimen.genome)) {
        auto edge=node->physical.relativePosition;
        Vec2 p=position;
        if(!positions.empty()) p=positions[node->physical.parentNode]+Vec2{
            std::cos(angle)*edge.x-std::sin(angle)*edge.y,
            std::sin(angle)*edge.x+std::cos(angle)*edge.y};
        if(!_config.toroidal && (p.x<_config.worldMinX+_config.cellRadius
            || p.x>_config.worldMaxX-_config.cellRadius || p.y<_config.worldMinY+_config.cellRadius
            || p.y>_config.worldMaxY-_config.cellRadius)) return kInvalidId;
        for(auto const& cell:cells) if(cell.alive && length(displacement(p,cell.position))<2*_config.cellRadius)
            return kInvalidId;
        positions.push_back(p);
    }
    auto const before=creatures.size();
    addFounder(specimen.genome,position,angle,specimen.lineageHue);
    if(creatures.size()==before) return kInvalidId;
    auto& creature=creatures.back();
    // This is the same finite starting reserve every ordinary founder gets;
    // the catalog merely stores it as data so a discovery can be replayed.
    for(auto index:cellIndicesForCreature(creature.id))
        {
        cells[index].energy=std::min<double>(cellCapacity(cells[index]),specimen.initialEnergy);
        energyLedger.organismSeeded+=cells[index].energy+cells[index].rawEnergy+cells[index].embodiedEnergy;
    }
    return creature.id;
}

uint32_t World::addSpecimenNearby(SpecimenSnapshot const& specimen, Vec2 requested)
{
    if(!isFinite(requested) || !isValidDevelopmentGenome(specimen.genome))return kInvalidId;
    auto summary=measureDevelopment(specimen.genome);
    std::size_t reserved=cells.size();
    for(auto const& pending:creatures)if(!pending.mature && !pending.fragment && !pending.developmentFailed)
        reserved+=pending.expectedCells>pending.bodyNodes.size()?pending.expectedCells-pending.bodyNodes.size():0;
    if(!summary.complete() || reserved+summary.cells>_config.maxCellCount)return kInvalidId;
    std::vector<Vec2> offsets;DevelopmentCursor cursor;
    while(auto node=cursor.next(specimen.genome))offsets.push_back(node->physical.parentNode<0?Vec2{}:offsets[node->physical.parentNode]+node->physical.relativePosition);
    float minX=_config.worldMinX+_config.cellRadius,maxX=_config.worldMaxX-_config.cellRadius;
    float minY=_config.worldMinY+_config.cellRadius,maxY=_config.worldMaxY-_config.cellRadius;
    for(auto p:offsets){minX=std::max(minX,_config.worldMinX+_config.cellRadius-p.x);maxX=std::min(maxX,_config.worldMaxX-_config.cellRadius-p.x);
        minY=std::max(minY,_config.worldMinY+_config.cellRadius-p.y);maxY=std::min(maxY,_config.worldMaxY-_config.cellRadius-p.y);}
    if(minX>maxX || minY>maxY)return kInvalidId;
    requested={clamp(requested.x,minX,maxX),clamp(requested.y,minY,maxY)};
    std::vector<Vec2> candidates{requested};
    float step=std::max(.45f,2*_config.cellRadius);
    for(float x=minX;x<=maxX;x+=step)for(float y=minY;y<=maxY;y+=step)candidates.push_back({x,y});
    std::stable_sort(candidates.begin(),candidates.end(),[&](Vec2 a,Vec2 b){return dot(a-requested,a-requested)<dot(b-requested,b-requested);});
    for(auto origin:candidates){bool clear=true;
        for(auto offset:offsets){for(auto const& cell:cells)if(cell.alive && length(displacement(origin+offset,cell.position))<2*_config.cellRadius){clear=false;break;}if(!clear)break;}
        if(clear)return addSpecimen(specimen,origin);
    }
    return kInvalidId;
}

void World::createInitialOrganism()
{
    auto const creatureIndex = addCreature(0, true, genome);
    auto& creature = creatures[creatureIndex];
    creature.ancestorId=creature.id;creature.lineageId = nextLineageId++;
    creature.lineageHue = 0.54f;

    auto id=creature.id;
    for(uint32_t n=0;n<genome.genes[0].nodes.size();++n) {
        auto const& node=genome.genes[0].nodes[n];
        Vec2 position=_config.initialRootPosition;
        if(n) position=cells[findCellForGenomeNode(id,node.parentNode)].position+node.relativePosition;
        auto index=addCell(id,position,_config.behavioralSeed ? Vec2{} : _config.initialVelocity,
            n ? _config.initialCellEnergy : _config.initialRootEnergy,node.constructorCell);
        if(!n) creature.rootCell=index;
        else {
            addConnection(findCellForGenomeNode(id,node.parentNode),index,length(node.relativePosition),_config.springStiffness*node.stiffness);
            braceGenomeNode(id,n);
        }
    }
}

} // namespace alienmobile

namespace alienmobile {
Vec2 World::resourceBedPosition(unsigned bed) const {
    return wrapped(_config.energySourcePosition+Vec2{(bed%2 ? 1.f:-1.f)*_config.resourceBedSpacing*.5f,
        (bed/2 ? 1.f:-1.f)*_config.resourceBedSpacing*.5f});
}
}
