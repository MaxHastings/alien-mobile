#pragma once

#include "alienmobile/CpuPhysicsBackend.h"
#include "alienmobile/SimulationConfig.h"
#include "alienmobile/World.h"

namespace alienmobile {

struct SimulationStats {
    double motorEnergy = 0.0;
    double biologyMs=0, synchronizationMs=0;
    double resourcesMs=0, signalsMs=0, attacksMs=0, actuationMs=0, damageMs=0, developmentMs=0, mutationMs=0, topologyMs=0;
    uint64_t steps=0;
    uint64_t births = 0;
    uint64_t deaths = 0;
    uint64_t cellDeaths = 0, fragmentsCreated = 0;
    uint64_t mutations = 0;
    uint64_t behaviorMutations = 0;
    uint32_t maximumGeneration = 0;
    uint32_t maximumMatureGeneration = 0;
    std::array<uint64_t,16> completedMutationBirths{};
    uint64_t completedMetaBirths=0;
    uint64_t starvationLosses=0, damageLosses=0, invalidDevelopmentAttempts=0;
    uint64_t capacityWaitSteps=0; // Sum of constructor steps denied technical space.
    bool populationCapReached = false;
};

class Simulation {
public:
    explicit Simulation(World& world, SimulationConfig config = {});

    void step();
    void step(float dt);
    void reset();
    void resetWithSeed(uint64_t seed);
    void setPhysicsBackend(PhysicsBackend& backend);
    void notifyTopologyChanged();

    World& world() { return _world; }
    World const& world() const { return _world; }
    SimulationConfig const& config() const { return _config; }
    PhysicsBackend& physicsBackend() { return *_physicsBackend; }
    PhysicsBackend const& physicsBackend() const { return *_physicsBackend; }
    SimulationStats const& stats() const { return _stats; }

private:
    void updateEnergy(float dt);
    bool removeStarvingCreatures(float dt);
    void updateConstructors(float dt);
    void constructNextNode(Creature& parent, float dt);
    void finishConstruction(Creature& parent, Creature& child);
    void applySeparationImpulse(uint32_t parentId, uint32_t childId);

    bool canConstruct(Creature const& creature) const;
    uint32_t cellForCreatureNode(uint32_t creatureId, uint32_t nodeIndex) const;
    GenomeMutationConfig mutationConfig() const;
    float spatialInfluence(Vec2 position, Vec2 sourcePosition, float radius) const;
    void debugValidate() const;

    World& _world;
    SimulationConfig _config;
    CpuPhysicsBackend _cpuPhysicsBackend;
    PhysicsBackend* _physicsBackend;
    SimulationStats _stats;
    bool _topologyDirty = false;
};

} // namespace alienmobile
