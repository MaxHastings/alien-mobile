#include "alienmobile/Simulation.h"
#include "alienmobile/Ecology.h"

#include <algorithm>
#include <chrono>
#include <cassert>
#include <cmath>
#include <vector>

namespace alienmobile {

Simulation::Simulation(World& world, SimulationConfig config)
    : _world(world)
    , _config(config)
    , _cpuPhysicsBackend(config)
    , _physicsBackend(&_cpuPhysicsBackend)
{
    _world.setSimulationConfig(config);
    _physicsBackend->initialize(_world);
}

void Simulation::step()
{
    step(_config.fixedTimeStep);
}

void Simulation::step(float dt)
{
    if (!std::isfinite(dt) || dt <= 0.0f) {
        return;
    }

    // GPU physics owns positions between topology events. Biology needs the
    // current positions for spatial energy and hazard falloff before it can
    // decide who reproduces or dies.
    auto start=std::chrono::steady_clock::now();
    _physicsBackend->syncToCpu(_world);
    auto biologyStart=std::chrono::steady_clock::now();
    _stats.synchronizationMs+=std::chrono::duration<double,std::milli>(biologyStart-start).count();
    auto mark=std::chrono::steady_clock::now();
    auto sample=[&](double& total){auto now=std::chrono::steady_clock::now();total+=std::chrono::duration<double,std::milli>(now-mark).count();mark=now;};
    updateEnergy(dt);sample(_stats.resourcesMs);
    updateSignals(_world, dt);sample(_stats.signalsMs);
    updateTrophicOrgans(_world,dt);sample(_stats.attacksMs);
    double motorCost=updateActuation(_world, dt, _config.motorEnergyCost);
    _stats.motorEnergy+=motorCost;_world.energyLedger.organCost+=motorCost;
    sample(_stats.actuationMs);
    _topologyDirty = removeStarvingCreatures(dt);sample(_stats.damageMs);
    updateConstructors(dt);sample(_stats.developmentMs);
    _world.mutagen.remaining=std::max(0.f,_world.mutagen.remaining-dt);
    if (_topologyDirty) {
        _physicsBackend->topologyChanged(_world);
    }
    sample(_stats.topologyMs);
    _stats.biologyMs+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-biologyStart).count();
    ++_stats.steps;
    _physicsBackend->step(_world, dt);
    debugValidate();
}

void Simulation::resetWithSeed(uint64_t seed) { _config.randomSeed=seed;reset(); }

void Simulation::reset()
{
    _world.reset(_config);
    _stats = SimulationStats{};
    _physicsBackend->topologyChanged(_world);
}

void Simulation::setPhysicsBackend(PhysicsBackend& backend)
{
    if (_physicsBackend == &backend) {
        return;
    }
    _physicsBackend->syncToCpu(_world);
    _physicsBackend = &backend;
    _physicsBackend->initialize(_world);
}

void Simulation::notifyTopologyChanged()
{
    _physicsBackend->topologyChanged(_world);
}

float Simulation::spatialInfluence(Vec2 position, Vec2 sourcePosition, float radius) const
{
    if (!std::isfinite(radius) || radius <= 0.0f) {
        return 0.0f;
    }
    auto const normalizedDistance = length(position - sourcePosition) / radius;
    auto const edge = clamp(1.0f - normalizedDistance, 0.0f, 1.0f);
    // Smoothstep keeps the source boundary visually readable without a hard
    // energy discontinuity at the edge of the influence radius.
    return edge * edge * (3.0f - 2.0f * edge);
}

void Simulation::updateEnergy(float dt)
{
    if(_config.physicalResources) {
        updateResources(_world,dt);
        for(auto& cell:_world.cells) {
            cell.visualAge+=dt;
            auto const* owner=_world.findCreature(cell.creatureId);
            if(owner && owner->fragment) continue;
            double cost=_config.metabolismRate;
            if(owner && _world.organsActive(*owner)) {
                if(cell.behavior.role==CellRole::EnergySensor || cell.behavior.role==CellRole::CreatureSensor || cell.behavior.role==CellRole::ObstacleSensor)
                    cost+=_config.sensorEnergyCost*cell.behavior.sensorRange*cell.behavior.sensorRange*cell.behavior.sensitivity;
                if(cell.behavior.role==CellRole::Memory) cost+=0.004f;
                if(cell.behavior.role==CellRole::Defender) cost+=0.01f*cell.behavior.defenseStrength;
                if(cell.behavior.role==CellRole::Digestor) cost+=_config.digestorMaintenance*cell.behavior.digestionRate;
                if(cell.behavior.role==CellRole::Attacker) cost+=_config.attackerMaintenance*cell.behavior.extractionRate;
            }
            auto d=_world.displacement(_world.hazardSource.position,cell.position);
            cost+=_world.hazardSource.strength*spatialInfluence(d,{},_world.hazardSource.radius);
            double paid=std::min(cell.energy,cost*dt);cell.energy-=paid;_world.energyLedger.dissipated+=paid;
        }
        if(_config.localDamage) {
            // Diffuse along actual connections. Outgoing budgets use one input
            // snapshot, so high-degree cells cannot send more than they own.
            for(bool raw:{false,true}) {
                struct Flow {uint32_t from,to;double amount;};
                std::vector<Flow> flows;std::vector<double> outgoing(_world.cells.size()),incoming(_world.cells.size()),values(_world.cells.size());
                for(std::size_t i=0;i<values.size();++i)values[i]=raw ? _world.cells[i].rawEnergy : _world.cells[i].energy;
                for(auto const& edge:_world.connections) {
                    auto a=edge.cellA,b=edge.cellB;
                    if(_world.cells[a].creatureId!=_world.cells[b].creatureId)continue;
                    auto owner=_world.findCreature(_world.cells[a].creatureId);if(!owner || owner->fragment)continue;
                    double ca=raw ? 4.f : _world.cellCapacity(_world.cells[a]);
                    double cb=raw ? 4.f : _world.cellCapacity(_world.cells[b]);
                    double delta=values[a]/ca-values[b]/cb;
                    double amount=std::abs(delta)*std::min(ca,cb)*_config.energyDiffusion*dt;
                    if(delta<0)std::swap(a,b);
                    flows.push_back({a,b,amount});outgoing[a]+=amount;incoming[b]+=amount;
                }
                auto next=values;
                for(auto const& f:flows) {
                    double capacity=raw ? 4.f : _world.cellCapacity(_world.cells[f.to]);
                    double supply=outgoing[f.from]>0 ? std::min<double>(1.f,values[f.from]/outgoing[f.from]) : 0;
                    double space=incoming[f.to]>0 ? std::min<double>(1.f,std::max<double>(0.f,capacity-values[f.to])/incoming[f.to]) : 0;
                    double amount=f.amount*std::min(supply,space);
                    next[f.from]-=amount;next[f.to]+=amount;
                }
                for(std::size_t i=0;i<next.size();++i) {
                    if(raw)_world.cells[i].rawEnergy=std::max<double>(0.f,next[i]);else _world.cells[i].energy=std::max<double>(0.f,next[i]);
                }
            }
        } else {
        for(auto const& creature:_world.creatures) {
            auto indices=_world.cellIndicesForCreature(creature.id);double sum=0;
            for(auto i:indices) sum+=_world.cells[i].energy;
            if(!indices.empty()) for(auto i:indices) _world.cells[i].energy=sum/indices.size();
        }
        }
        return;
    }
    // A finite source is shared by exposed cells. Crowding reduces each
    // cell's intake; collecting surface, placement and upkeep all matter.
    float demand = 0.0f;
    for (auto const& cell : _world.cells) {
        if (cell.alive) demand += _world.energySource.strength * spatialInfluence(
            cell.position, _world.energySource.position, _world.energySource.radius);
    }
    float supplyScale = demand > 0.0f ? std::min(1.0f, _config.resourceCapacity / demand) : 0.0f;
    for (auto& cell : _world.cells) {
        if (!cell.alive) {
            continue;
        }
        cell.visualAge += dt;
        auto const sourceInfluence = spatialInfluence(
            cell.position,
            _world.energySource.position,
            _world.energySource.radius);
        auto const hazardInfluence = spatialInfluence(
            cell.position,
            _world.hazardSource.position,
            _world.hazardSource.radius);
        auto const* owner = _world.findCreature(cell.creatureId);
        float sensorCost = cell.behavior.role == CellRole::EnergySensor && owner && owner->mature
            ? _config.sensorEnergyCost * cell.behavior.sensorRange * cell.behavior.sensorRange
                * cell.behavior.sensitivity : 0.0f;
        auto const energyDelta = _world.energySource.strength * sourceInfluence * supplyScale
            - _world.hazardSource.strength * hazardInfluence
            - _config.metabolismRate - sensorCost;
        cell.energy = clamp(
            cell.energy + energyDelta * dt,
            0.0f,
            std::max<double>(0.0f, _config.maxCellEnergy));
    }
    // Connected bodies share their energy. This is an intentional coarse
    // transport model: outer cells can feed construction and cost upkeep.
    for (auto const& creature : _world.creatures) {
        auto indices = _world.cellIndicesForCreature(creature.id);
        float total = 0.0f;
        for (auto i : indices) total += _world.cells[i].energy;
        for (auto i : indices) _world.cells[i].energy = total / indices.size();
    }
}

bool Simulation::removeStarvingCreatures(float dt)
{
    if(_config.localDamage) {
        std::vector<uint32_t> removed;
        std::size_t beforeLiving=0,beforeFragments=0;
        for(auto& owner:_world.creatures) {
            owner.visualAge+=dt;
            if(owner.fragment){owner.fragmentAge+=dt;++beforeFragments;}else ++beforeLiving;
        }
        for(auto& cell:_world.cells) {
            auto const* owner=_world.findCreature(cell.creatureId);
            if(!owner)continue;
            if(owner->fragment) {
                if(owner->fragmentAge>=_config.fragmentLifetime)removed.push_back(cell.id);
                continue;
            }
            if(cell.energy<=_config.starvationEnergyThreshold)cell.starvationTimer+=dt;
            else cell.starvationTimer=std::max<double>(0.f,cell.starvationTimer-dt*2);
            cell.viability=cell.starvationTimer>0 ? CellViability::Starving : CellViability::Healthy;
            if(cell.deathRequested || cell.starvationTimer>=_config.starvationGracePeriod) {
                removed.push_back(cell.id);
                auto cause=cell.deathRequested ? LifeEventKind::DamageLoss : LifeEventKind::StarvationLoss;
                if(cell.deathRequested)++_stats.damageLosses;else ++_stats.starvationLosses;
                _world.recordEvent(cause,cell.creatureId,cell.id,cell.genomeNode,cell.position);
            }
        }
        // Empty orphan reservations contain no material, but must not leak owners.
        _world.creatures.erase(std::remove_if(_world.creatures.begin(),_world.creatures.end(),[&](auto const& owner){
            return owner.fragment && _world.cellIndicesForCreature(owner.id).empty();
        }),_world.creatures.end());
        if(removed.empty())return false;
        _stats.cellDeaths+=removed.size();
        bool changed=_world.removeCellsAndFragment(removed);
        std::size_t living=0,fragments=0;
        for(auto const& owner:_world.creatures) {if(owner.fragment)++fragments;else ++living;}
        if(beforeLiving>living)_stats.deaths+=beforeLiving-living;
        if(fragments>beforeFragments)_stats.fragmentsCreated+=fragments-beforeFragments;
        return changed;
    }
    std::vector<uint32_t> starvingIds;
    for (auto& creature : _world.creatures) {
        creature.visualAge += dt;
        auto const indices = _world.cellIndicesForCreature(creature.id);
        double totalEnergy = 0.0f;
        for (auto const cellIndex : indices) {
            totalEnergy += _world.cells[cellIndex].energy;
        }

        if (indices.empty()) continue;
        if (totalEnergy <= _config.starvationEnergyThreshold) {
            creature.starvationTimer += dt;
        } else {
            creature.starvationTimer = std::max<double>(0.0f, creature.starvationTimer - dt * 2.0f);
        }
        if (creature.starvationTimer >= _config.starvationGracePeriod) {
            starvingIds.push_back(creature.id);
        }
    }

    bool topologyChanged = false;
    for (auto const creatureId : starvingIds) {
        auto const previousCount = _world.creatures.size();
        auto const indices = _world.cellIndicesForCreature(creatureId);
        for(auto index:indices) {
            auto const& cell=_world.cells[index];
            ++_stats.starvationLosses;
            _world.recordEvent(LifeEventKind::StarvationLoss,creatureId,cell.id,cell.genomeNode,cell.position);
        }
        if (_world.removeCreatureAndCells(creatureId)) {
            _stats.deaths += previousCount - _world.creatures.size();
            topologyChanged = true;
        }
    }
    return topologyChanged;
}

GenomeMutationConfig Simulation::mutationConfig() const
{
    return GenomeMutationConfig{
        _config.minGenomeCells,
        _config.maxGenomeCells,
        _config.geometryMutationProbability,
        _config.nodeAdditionProbability,
        _config.nodeRemovalProbability,
        _config.maxGeometryPerturbation,
        _config.minGenomeEdgeLength,
        _config.maxGenomeEdgeLength,
        _config.oscillatorMutationProbability,
        _config.ecosystemSeed ? 0.008f : 0.0f,
        _config.behaviorMutationProbability,
        _config.openStructuralMutation,
    };
}

bool Simulation::canConstruct(Creature const& creature) const
{
    if (creature.developmentFailed || !creature.mature || creature.rootCell == kInvalidId || creature.rootCell >= _world.cells.size()) {
        return false;
    }
    auto const& root = _world.cells[creature.rootCell];
    return root.alive && root.creatureId == creature.id && root.behavior.role == CellRole::Constructor
        && root.energy >= _config.constructionEnergy;
}

void Simulation::updateConstructors(float dt)
{
    // Creature IDs are stable, so collecting them first avoids invalidating a
    // reference if a new child is appended during this pass.
    std::vector<uint32_t> creatureIds;
    creatureIds.reserve(_world.creatures.size());
    for (auto const& creature : _world.creatures) {
        creatureIds.push_back(creature.id);
    }

    for (auto const creatureId : creatureIds) {
        auto* parent = _world.findCreature(creatureId);
        if(parent)parent->constructor.wait=ConstructorState::None;
        if(parent && parent->developingFounder && !parent->developmentFailed) {
            constructNextNode(*parent,dt);
            continue;
        }
        if (parent == nullptr || !parent->mature) {
            continue;
        }

        if (parent->constructor.status == ConstructorState::Cooldown) {
            parent->constructor.timer -= dt;
            if (parent->constructor.timer <= 0.0f) {
                parent->constructor.status = ConstructorState::Idle;
                parent->constructor.timer = 0.0f;
            }
            continue;
        }

        if (parent->constructor.status == ConstructorState::Idle) {
            if (parent->constructor.offspringCreatureId != kInvalidId || !canConstruct(*parent)) {
                if(parent->rootCell<_world.cells.size() && _world.cells[parent->rootCell].energy<_config.constructionEnergy)
                    parent->constructor.wait=ConstructorState::Energy;
                continue;
            }
            if (_world.cells.size() >= _config.maxCellCount) {
                _stats.populationCapReached = true;
                parent->constructor.wait=ConstructorState::Capacity;++_stats.capacityWaitSteps;
                continue;
            }

            auto const parentLineageId = parent->lineageId;
            auto const parentLineageHue = parent->lineageHue;
            auto const parentGeneration = parent->generation;
            auto const accumulatedDistance=parent->lineageDistance;
            // Draw DNA once per reproductive attempt. Re-rolling on every
            // capacity wait used the allocator as an unintended size filter.
            if(!parent->proposedOffspring) {
                auto mutationStart=std::chrono::steady_clock::now();
                parent->proposedOffspring=_config.openStructuralMutation
                    ? mutateDevelopmentGenome(parent->genome,_world.rng,
                        _world.mutationExposure(_world.cells[parent->rootCell].position))
                    : mutateGenome(parent->genome, _world.rng, mutationConfig());
                parent->proposedOffspringSummary.reset();
                _stats.mutationMs+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-mutationStart).count();
            }
            if(!parent->proposedOffspringSummary)
                parent->proposedOffspringSummary=measureDevelopment(parent->proposedOffspring->genome);
            auto developmentSummary=*parent->proposedOffspringSummary;
            if(!developmentSummary.complete()) {
                ++_stats.invalidDevelopmentAttempts;
                _world.recordEvent(LifeEventKind::InvalidDevelopment,parent->id);
                // A sterile developmental attempt consumes a reproductive
                // cycle. Never search repeatedly for viable DNA in one frame.
                parent->proposedOffspring.reset();
                parent->proposedOffspringSummary.reset();
                parent->constructor.status=ConstructorState::Cooldown;
                parent->constructor.timer=_config.cooldownDuration;
                continue;
            }
            std::size_t reserved = _world.cells.size();
            for (auto const& pending : _world.creatures) {
                if (!pending.mature && !pending.fragment && !pending.developmentFailed)
                    reserved += pending.expectedCells>pending.bodyNodes.size() ? pending.expectedCells-pending.bodyNodes.size() : 0;
            }
            if (reserved + developmentSummary.cells > _config.maxCellCount) {
                _stats.populationCapReached = true;
                parent->constructor.wait=ConstructorState::Capacity;++_stats.capacityWaitSteps;
                continue;
            }
            auto mutation=std::move(*parent->proposedOffspring);
            bool structural = mutation.genome.genes[0].nodes.size() != parent->genome.genes[0].nodes.size();
            if(!structural) for(std::size_t n=0;n<mutation.genome.genes[0].nodes.size();++n)
                structural |= !(mutation.genome.genes[0].nodes[n].relativePosition == parent->genome.genes[0].nodes[n].relativePosition);
            auto ancestorId=parent->ancestorId==kInvalidId ? parent->id : parent->ancestorId;
            auto const childIndex = _world.addCreature(parentGeneration + 1, false, mutation.genome);
            auto const childId = _world.creatures[childIndex].id;
            auto& child = _world.creatures[childIndex];
            child.ancestorId=ancestorId;child.parentId=creatureId;child.parentLineageId=parentLineageId;
            child.birthMutation=mutation.kind;child.birthMetaMutation=mutation.metaMutated;
            child.birthAngle = _world.rng.nextUnit() * 6.28318530718f;
            child.lineageDistance=accumulatedDistance+mutation.geneticDistance;
            bool diverged=_config.openStructuralMutation ? child.lineageDistance>=2.5f : mutation.mutated();
            child.lineageId = diverged ? _world.nextLineageId++ : parentLineageId;
            if(diverged) child.lineageDistance=0;
            child.lineageHue = parentLineageHue;
            if (mutation.mutated()) {
                auto const hueShift = _config.openStructuralMutation ? (diverged ? 0.025f : 0.001f) : structural ? 0.025f + _world.rng.nextUnit() * 0.035f
                    : 0.003f + _world.rng.nextUnit() * 0.009f;
                child.lineageHue += (_world.rng.nextUnit() < 0.5f ? -hueShift : hueShift);
                child.lineageHue -= std::floor(child.lineageHue);
                ++_stats.mutations;
                if(mutation.behaviorMutated) ++_stats.behaviorMutations;
            }

            parent = _world.findCreature(creatureId);
            parent->proposedOffspring.reset();
            parent->proposedOffspringSummary.reset();
            parent->constructor.status = ConstructorState::Constructing;
            parent->constructor.nextNode = 0;
            parent->constructor.offspringCreatureId = childId;
            parent->constructor.timer = 0.0f;
            _stats.maximumGeneration = std::max(_stats.maximumGeneration, parent->generation + 1);
        }

        if (parent->constructor.status == ConstructorState::Constructing) {
            constructNextNode(*parent, dt);
        }
    }
}

uint32_t Simulation::cellForCreatureNode(uint32_t creatureId, uint32_t nodeIndex) const
{
    return _world.findCellForGenomeNode(creatureId, nodeIndex);
}

void Simulation::constructNextNode(Creature& parent, float dt)
{
    parent.constructor.timer += dt;
    auto* child = _world.findCreature(parent.constructor.offspringCreatureId);
    if (child == nullptr) {
        parent.constructor = ConstructorState{};
        return;
    }
    if(child->developmentFailed){parent.constructor.wait=ConstructorState::Development;return;}
    if (parent.constructor.nextNode >= child->expectedCells) {
        finishConstruction(parent, *child);
        return;
    }

    if(!child->pendingNode) child->pendingNode=child->development.next(child->genome);
    if(!child->pendingNode) { child->developmentFailed=true;parent.constructor.wait=ConstructorState::Development;
        ++_stats.invalidDevelopmentAttempts;_world.recordEvent(LifeEventKind::InvalidDevelopment,child->id);return; }
    float interval=_config.constructionInterval*child->pendingNode->intervalScale;
    if(parent.constructor.timer<interval) return;

    // A capped population pauses construction at the pending node; it never
    // kills a creature merely to make room.
    if (_world.cells.size() >= _config.maxCellCount) {
        _stats.populationCapReached = true;
        parent.constructor.wait=ConstructorState::Capacity;++_stats.capacityWaitSteps;
        parent.constructor.timer = interval;
        return;
    }

    // Lack of usable energy leaves the current node pending and visibly
    // pauses construction.
    if (parent.rootCell == kInvalidId || parent.rootCell >= _world.cells.size()
        || _world.cells[parent.rootCell].energy < _config.constructionEnergy) {
        parent.constructor.wait=ConstructorState::Energy;
        parent.constructor.timer = interval;
        return;
    }

    // Positions were synchronized once at the start of the biological step.
    // Never read back here: an earlier birth may already have changed CPU
    // velocities or indices. Upload all events together after this pass.
    auto& root = _world.cells[parent.rootCell];

    auto const nodeIndex = parent.constructor.nextNode;
    auto const node = child->pendingNode->physical;
    Vec2 position;
    Vec2 velocity = root.velocity;
    if (nodeIndex == 0) {
        position = root.position + Vec2{std::cos(child->birthAngle), std::sin(child->birthAngle)}
            * _config.offspringAnchorDistance;
    } else {
        auto const parentCellIndex = node.parentNode < 0
            ? kInvalidId
            : cellForCreatureNode(child->id, static_cast<uint32_t>(node.parentNode));
        if (parentCellIndex == kInvalidId || parentCellIndex >= _world.cells.size()) {
            ++_stats.invalidDevelopmentAttempts;_world.recordEvent(LifeEventKind::InvalidDevelopment,child->id);
            child->developmentFailed=true;
            if(_config.localDamage) {child->fragment=true;child->mature=false;child->developingFounder=false;}
            parent.constructor=ConstructorState{};
            return;
        }
        auto const c = std::cos(child->birthAngle);
        auto const s = std::sin(child->birthAngle);
        Vec2 edge{c * node.relativePosition.x - s * node.relativePosition.y,
                  s * node.relativePosition.x + c * node.relativePosition.y};
        position = _world.cells[parentCellIndex].position + edge;
        velocity = _world.cells[parentCellIndex].velocity;
    }

    auto const parentCells = _world.cellIndicesForCreature(parent.id);
    double available=0;
    for(auto i:parentCells) available+=_world.cells[i].energy;
    if(available<_config.constructionEnergy) return;
    if(_config.localDamage) root.energy-=_config.constructionEnergy;
    else for(auto i:parentCells) _world.cells[i].energy*=1-_config.constructionEnergy/available;
    auto const newCellIndex = _world.addCell(child->id, position, velocity, std::min(_config.initialCellEnergy, _config.constructionEnergy), node.constructorCell);
    if (nodeIndex == 0) {
        child->rootCell = newCellIndex;
        _world.addConnection(parent.rootCell, newCellIndex, _config.offspringAnchorDistance, _config.springStiffness);
    } else {
        auto const parentCellIndex = cellForCreatureNode(child->id, static_cast<uint32_t>(node.parentNode));
        auto const restLength = length(node.relativePosition);
        _world.addConnection(parentCellIndex, newCellIndex, restLength, _config.springStiffness*node.stiffness);
        _world.braceGenomeNode(child->id, nodeIndex);
    }

    ++parent.constructor.nextNode;
    parent.constructor.timer -= interval;
    if (parent.constructor.nextNode >= child->expectedCells) {
        finishConstruction(parent, *child);
    }
    _topologyDirty = true;
}

void Simulation::finishConstruction(Creature& parent, Creature& child)
{
    _topologyDirty = true;
    // Seed embryos develop through the same paid physical constructor.
    if(parent.id==child.id) {
        child.mature=true;child.developingFounder=false;
        child.pendingNode=child.development.next(child.genome);
        child.constructor=ConstructorState{};
        child.constructor.status=ConstructorState::Cooldown;
        child.constructor.timer=_config.cooldownDuration;
        return;
    }
    child.pendingNode=child.development.next(child.genome);
    bool separate=parent.genome.genes[parent.genome.entryGene].nodes[0].construction.separateOffspring;
    if(separate) _world.connections.erase(std::remove_if(_world.connections.begin(), _world.connections.end(),
        [&](Connection const& connection) {
            auto a = _world.cells[connection.cellA].creatureId;
            auto b = _world.cells[connection.cellB].creatureId;
            return (a == parent.id && b == child.id) || (b == parent.id && a == child.id);
        }), _world.connections.end());
    child.mature = true;
    child.constructor = ConstructorState{};

    auto const childId = child.id;
    parent.constructor.status = ConstructorState::Cooldown;
    parent.constructor.nextNode = 0;
    parent.constructor.offspringCreatureId = kInvalidId;
    parent.constructor.timer = _config.cooldownDuration;

    ++_stats.births;
    ++_stats.completedMutationBirths[unsigned(child.birthMutation)];
    _stats.completedMetaBirths+=child.birthMetaMutation;
    _stats.maximumMatureGeneration=std::max(_stats.maximumMatureGeneration,child.generation);
    _stats.maximumGeneration = std::max(_stats.maximumGeneration, child.generation);
    if(separate) applySeparationImpulse(parent.id, childId);
}

void Simulation::applySeparationImpulse(uint32_t parentId, uint32_t childId)
{
    auto const* parent = _world.findCreature(parentId);
    auto const* child = _world.findCreature(childId);
    if (parent == nullptr || child == nullptr || parent->rootCell == kInvalidId || child->rootCell == kInvalidId) {
        return;
    }
    // This is construction recoil applied to every real cell, not dispersal
    // steering. It gives a newly detached body the nearby physical space its
    // geometry already occupies.
    auto const direction = normalizedOr(_world.displacement(_world.cells[parent->rootCell].position,
        _world.cells[child->rootCell].position));
    for (auto& cell : _world.cells) {
        if (!cell.alive) {
            continue;
        }
        if (cell.creatureId == parentId) {
            cell.velocity -= direction * _config.separationSpeed;
        } else if (cell.creatureId == childId) {
            cell.velocity += direction * _config.separationSpeed;
        }
    }
}

void Simulation::debugValidate() const
{
#ifndef NDEBUG
    assert(_world.allConnectionsValid());
    assert(_world.allValuesFinite());
    for (auto const& cell : _world.cells) {
        assert(cell.energy >= 0.0f);
    }
#endif
}

} // namespace alienmobile
