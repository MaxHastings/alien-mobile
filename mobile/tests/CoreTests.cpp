#include "alienmobile/Simulation.h"
#include "alienmobile/Camera.h"
#include "alienmobile/CpuPhysicsBackend.h"
#include "alienmobile/PhysicsScenario.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <set>
#include <utility>

namespace {

using namespace alienmobile;

void runFor(Simulation& simulation, float seconds)
{
    auto const steps = static_cast<int>(std::ceil(seconds / simulation.config().fixedTimeStep));
    for (int step = 0; step < steps; ++step) {
        simulation.step();
    }
}

float totalEnergy(World const& world, uint32_t creatureId)
{
    float result = 0.0f;
    for (auto const cellIndex : world.cellIndicesForCreature(creatureId)) {
        result += world.cells[cellIndex].energy;
    }
    return result;
}

void testCameraMapping()
{
    for (Vec2 viewport : {Vec2{393, 852}, Vec2{852,393}, Vec2{1179,2556}}) {
        Vec2 center{1,-2}, point{-2.8f,0};
        auto screen = screenFromWorld(point,viewport,center,0.125f);
        auto restored = worldFromScreen(screen,viewport,center,0.125f);
        assert(length(restored-point) < 0.00001f);
        auto origin = screenFromWorld(center,viewport,center,0.125f);
        auto x = screenFromWorld(center+Vec2{1,0},viewport,center,0.125f);
        auto y = screenFromWorld(center+Vec2{0,1},viewport,center,0.125f);
        assert(nearlyEqual(x.x-origin.x,origin.y-y.y));
    }
    auto point = screenFromWorld({-2.8f,0},{393,852},{},0.125f);
    auto pixel = screenFromWorld({-2.8f,0},{1179,2556},{},0.125f);
    assert(length(pixel-point*3) < 0.001f);
}

void testInitialWorld()
{
    World world;
    assert(world.creatures.size() == 1);
    assert(world.matureCreatureCount() == 1);
    assert(world.cells.size() == 3);
    assert(world.connections.size() == 3);
    assert(world.hasConnection(0, 1));
    assert(world.hasConnection(0, 2));
    assert(world.cells[0].constructorCell);
    assert(world.creatures[0].rootCell == 0);
    assert(world.creatures[0].lineageId == 1);
    assert(hasDefaultGenomeSemantics(world.genome));
    assert(world.creatures[0].genome == world.genome);
    assert(world.energySource.radius > 0.0f);
    assert(world.hazardSource.radius > 0.0f);
}

void testCpuPhysicsScenarioIsDeterministic()
{
    auto const config = makePhysicsParityConfig();
    auto first = makePhysicsParityWorld();
    auto second = makePhysicsParityWorld();
    auto const initialCells = first.cells;
    auto const initialConnections = first.connections;

    CpuPhysicsBackend firstPhysics(config);
    CpuPhysicsBackend secondPhysics(config);
    firstPhysics.initialize(first);
    secondPhysics.initialize(second);
    for (int step = 0; step < 240; ++step) {
        firstPhysics.step(first, config.fixedTimeStep);
        secondPhysics.step(second, config.fixedTimeStep);
    }

    assert(first.connections == initialConnections);
    assert(second.connections == initialConnections);
    assert(first.cells.size() == initialCells.size());
    assert(second.cells == first.cells);
    assert(first.allConnectionsValid());
    assert(first.allValuesFinite());

    bool positionChanged = false;
    bool velocityChanged = false;
    for (std::size_t index = 0; index < first.cells.size(); ++index) {
        positionChanged = positionChanged || first.cells[index].position != initialCells[index].position;
        velocityChanged = velocityChanged || first.cells[index].velocity != initialCells[index].velocity;
        auto const& position = first.cells[index].position;
        assert(position.x >= config.worldMinX + config.cellRadius - 0.0001f);
        assert(position.x <= config.worldMaxX - config.cellRadius + 0.0001f);
        assert(position.y >= config.worldMinY + config.cellRadius - 0.0001f);
        assert(position.y <= config.worldMaxY - config.cellRadius + 0.0001f);
    }
    assert(positionChanged);
    assert(velocityChanged);
}

void testGenomeHeredityAndValidity()
{
    GenomeMutationConfig config;
    config.geometryProbability = 0.0f;
    config.additionProbability = 0.0f;
    config.removalProbability = 0.0f;
    DeterministicRng rng(17);
    auto const parent = makeDefaultGenome();
    auto const unchanged = mutateGenome(parent, rng, config);
    assert(!unchanged.mutated());
    assert(unchanged.genome == parent);

    config.geometryProbability = 1.0f;
    config.maxPerturbation = 0;
    assert(!mutateGenome(parent, rng, config).mutated());
    config.maxPerturbation = 0.2f;
    auto const mutated = mutateGenome(parent, rng, config);
    assert(mutated.mutated());
    assert(mutated.genome != parent);
    assert(isValidGenome(mutated.genome, config));

    auto const grandchild = mutateGenome(mutated.genome, rng, GenomeMutationConfig{});
    assert(isValidGenome(grandchild.genome));

    GenomeMutationConfig broad;
    broad.minNodes = 2;
    broad.maxNodes = 6;
    DeterministicRng fuzzRng(0x1234);
    Genome current = parent;
    for (int iteration = 0; iteration < 5000; ++iteration) {
        auto const result = mutateGenome(current, fuzzRng, broad);
        assert(isValidGenome(result.genome, broad));
        current = result.genome;
    }
    assert(current.genes[0].nodes.size() >= broad.minNodes);
    assert(current.genes[0].nodes.size() <= broad.maxNodes);
    assert(current.genes[0].nodes[0].parentNode == -1);
    assert(current.genes[0].nodes[0].constructorCell);
    GenomeMutationConfig bounded;
    bounded.maxNodes = parent.genes[0].nodes.size();
    bounded.geometryProbability = 0;
    bounded.additionProbability = 1;
    assert(!mutateGenome(parent, rng, bounded).mutated());
}

void testReproductionMutationAndLineage()
{
    auto config = SimulationConfig{};
    config.energySourcePosition = config.initialRootPosition;
    config.hazardSourcePosition = {20.0f, 20.0f};
    config.geometryMutationProbability = 1.0f;
    config.nodeAdditionProbability = 0.0f;
    config.nodeRemovalProbability = 0.0f;
    config.maxGeometryPerturbation = 0.12f;
    World world(config);
    Simulation simulation(world, config);
    runFor(simulation, 12.0f);

    assert(simulation.stats().births > 0);
    assert(simulation.stats().mutations > 0);
    bool sawChangedGenome = false;
    bool sawRelatedHue = false;
    for (auto const& creature : world.creatures) {
        if (creature.generation == 0) {
            continue;
        }
        sawChangedGenome = sawChangedGenome || creature.genome != world.genome;
        sawRelatedHue = sawRelatedHue || std::fabs(creature.lineageHue - 0.54f) < 0.15f
            || std::fabs(std::fabs(creature.lineageHue - 0.54f) - 1.0f) < 0.15f;
    }
    assert(sawChangedGenome);
    assert(sawRelatedHue);
    assert(world.lineageCount() > 1);
}

void testSpatialEnergyAndHazard()
{
    auto config = SimulationConfig{};
    config.initialRootPosition = {0.0f, 0.0f};
    config.initialVelocity = {0.0f, 0.0f};
    config.initialRootEnergy = 0.4f;
    config.initialCellEnergy = 0.4f;
    config.energySourcePosition = {0.0f, 0.0f};
    config.energySourceRadius = 2.0f;
    config.energySourceStrength = 1.0f;
    config.hazardSourcePosition = {20.0f, 20.0f};
    config.hazardStrength = 0.0f;
    config.metabolismRate = 0.01f;
    auto nearWorld = World(config);
    Simulation nearSimulation(nearWorld, config);
    auto const nearId = nearWorld.creatures.front().id;
    runFor(nearSimulation, 0.5f);
    assert(totalEnergy(nearWorld, nearId) > 1.20f);

    config.energySourcePosition = {20.0f, 20.0f};
    auto farWorld = World(config);
    Simulation farSimulation(farWorld, config);
    auto const farId = farWorld.creatures.front().id;
    runFor(farSimulation, 0.5f);
    assert(totalEnergy(farWorld, farId) < totalEnergy(nearWorld, nearId));

    config.energySourceStrength = 0.0f;
    config.hazardSourcePosition = {0.0f, 0.0f};
    config.hazardRadius = 2.0f;
    config.hazardStrength = 1.0f;
    auto hazardWorld = World(config);
    Simulation hazardSimulation(hazardWorld, config);
    auto const hazardId = hazardWorld.creatures.front().id;
    runFor(hazardSimulation, 0.1f);
    assert(totalEnergy(hazardWorld, hazardId) < 1.15f);
}

void testMetabolismAndConstructionPause()
{
    auto config = SimulationConfig{};
    config.initialRootPosition = {0.0f, 0.0f};
    config.initialVelocity = {0.0f, 0.0f};
    config.energySourcePosition = {20.0f, 20.0f};
    config.hazardSourcePosition = {20.0f, 20.0f};
    config.energySourceStrength = 0.0f;
    config.hazardStrength = 0.0f;
    config.metabolismRate = 0.10f;
    config.initialRootEnergy = 0.5f;
    config.initialCellEnergy = 0.5f;
    config.constructionEnergy = 10.0f;
    config.starvationGracePeriod = 10.0f;
    auto small = World(config);
    auto large = World(config);
    auto const smallId = small.creatures.front().id;
    auto const largeId = large.creatures.front().id;
    auto const initialSmallEnergy = totalEnergy(small, smallId);
    for (int i = 0; i < 3; ++i) {
        auto const root = large.creatures.front().rootCell;
        (void)root;
        large.addCell(largeId, {static_cast<float>(i + 1), 0.0f}, {}, 0.5f, false);
    }
    auto const initialLargeEnergy = totalEnergy(large, largeId);
    Simulation smallSimulation(small, config);
    Simulation largeSimulation(large, config);
    runFor(smallSimulation, 1.0f);
    runFor(largeSimulation, 1.0f);
    auto const smallEnergyLoss = initialSmallEnergy - totalEnergy(small, smallId);
    auto const largeEnergyLoss = initialLargeEnergy - totalEnergy(large, largeId);
    assert(largeEnergyLoss > smallEnergyLoss * 1.5f);

    config.initialRootEnergy = 0.4f;
    config.initialCellEnergy = 0.4f;
    config.constructionEnergy = 0.3f;
    config.energySourcePosition = {20.0f, 20.0f};
    config.metabolismRate = 0.01f;
    auto pausedWorld = World(config);
    Simulation pausedSimulation(pausedWorld, config);
    runFor(pausedSimulation, 2.0f);
    assert(pausedWorld.creatures.size() >= 2);
    auto const& child = pausedWorld.creatures[1];
    assert(!child.mature);
    assert(pausedWorld.cellIndicesForCreature(child.id).size() < child.genome.genes[0].nodes.size());
}

void testDeathAndTopologyCleanup()
{
    auto config = SimulationConfig{};
    config.initialRootPosition = {0.0f, 0.0f};
    config.initialVelocity = {0.0f, 0.0f};
    config.energySourcePosition = {20.0f, 20.0f};
    config.hazardSourcePosition = {0.0f, 0.0f};
    config.energySourceStrength = 0.0f;
    config.hazardStrength = 0.0f;
    config.metabolismRate = 0.5f;
    config.initialRootEnergy = 0.01f;
    config.initialCellEnergy = 0.01f;
    config.starvationGracePeriod = 0.1f;
    auto world = World(config);
    Simulation simulation(world, config);
    runFor(simulation, 1.0f);
    assert(world.creatures.empty());
    assert(world.cells.empty());
    assert(world.connections.empty());
    assert(world.allConnectionsValid());
    assert(world.allValuesFinite());
    assert(simulation.stats().deaths == 1);
}

// Isolate ecology from motion so the selection pathway is measurable.
class FrozenPhysics : public PhysicsBackend {
public:
    void initialize(World&) override {}
    void step(World&, float) override {}
    void syncToCpu(World&) override {}
    void topologyChanged(World&) override {}
};

void testSelectionHasFitnessConsequences()
{
    auto config = SimulationConfig{};
    config.energySourcePosition = {0, 0};
    config.energySourceRadius = 0.7f;
    config.energySourceStrength = 0.18f;
    config.hazardStrength = 0;
    config.metabolismRate = 0.05f;
    config.initialRootPosition = {0, 0};
    config.initialRootEnergy = config.initialCellEnergy = 0.4f;
    config.constructionEnergy = 0.18f;
    config.constructionInterval = 0.15f;
    config.cooldownDuration = 0.5f;
    config.geometryMutationProbability = config.nodeAdditionProbability = config.nodeRemovalProbability = 0;
    auto run = [&](int count) {
        World world(config);
        auto id = world.creatures.front().id;
        world.cells.clear(); world.connections.clear();world.angles.clear();
        auto& creature = world.creatures.front();
        creature.genome.genes[0].nodes.resize(count);
        creature.genome.genes[0].nodes[0] = {-1, {}, true};
        creature.rootCell = world.addCell(id, {}, {}, 0.4f, true);
        for (int i = 1; i < count; ++i) {
            float angle = i * 6.2831853f / (count - 1);
            Vec2 position{std::cos(angle), std::sin(angle)};
            creature.genome.genes[0].nodes[i] = {0, position, false};
            auto cell = world.addCell(id, position, {}, 0.4f, false);
            world.addConnection(creature.rootCell, cell, 1, config.springStiffness);
        }
        Simulation sim(world, config); FrozenPhysics physics; sim.setPhysicsBackend(physics);
        runFor(sim, 20);
        return sim.stats();
    };
    auto small = run(2), large = run(6);
    assert(small.births > large.births);
}

void testFiniteFoodAndPeripheralEnergy()
{
    auto config = SimulationConfig{};
    config.initialRootPosition = {0, 0};
    config.energySourcePosition = {0, 0};
    config.energySourceRadius = 20;
    config.resourceCapacity = 0.3f;
    config.energySourceStrength = 10;
    config.hazardStrength = config.metabolismRate = 0;
    config.constructionEnergy = 100;
    config.initialRootEnergy = config.initialCellEnergy = 0;
    World world(config); Simulation sim(world, config); FrozenPhysics physics;
    sim.setPhysicsBackend(physics);
    runFor(sim, 0.5f);
    assert(nearlyEqual(totalEnergy(world, 0), 0.15f, 0.0001f));
    assert(nearlyEqual(world.cells[0].energy, world.cells[1].energy));
    // A cell at the edge can feed a root outside the light.
    world.energySource.position = world.cells[1].position;
    world.energySource.radius = 0.3f;
    for (auto& cell : world.cells) cell.energy = 0;
    runFor(sim, 0.5f);
    assert(world.cells[0].energy > 0.04f);
    // Double the consumers: same total supply, reduced share.
    auto before = totalEnergy(world, 0);
    auto ci = world.addCreature(0, true);
    auto id = world.creatures[ci].id;
    world.creatures[ci].rootCell = world.addCell(id, world.energySource.position, {}, 0, true);
    sim.notifyTopologyChanged();
    runFor(sim, 0.5f);
    assert(totalEnergy(world, 0) - before < 0.14f);
    assert(nearlyEqual(totalEnergy(world, 0) + totalEnergy(world, id) - before, 0.15f, 0.001f));
}

void testBirthAttachmentAndHeritableGrandchildren()
{
    auto config = SimulationConfig{};
    config.energySourceRadius = 100;
    config.energySourcePosition = {};
    config.resourceCapacity = 100;
    config.hazardStrength = 0;
    config.geometryMutationProbability = config.nodeAdditionProbability = config.nodeRemovalProbability = 0;
    World world(config);
    // Begin with an actual mutated genome; all later generations must keep it.
    auto mutation = GenomeMutationConfig{}; mutation.geometryProbability = 1;
    auto inherited = mutateGenome(world.genome, world.rng, mutation).genome;
    world.creatures[0].genome = inherited;
    Simulation sim(world, config); FrozenPhysics physics; sim.setPhysicsBackend(physics);
    bool attached = false, detached = false, grandchild = false;
    for (int i = 0; i < 120 * 45; ++i) {
        sim.step();
        for (auto const& c : world.connections) {
            auto a = world.cells[c.cellA].creatureId, b = world.cells[c.cellB].creatureId;
            if (a != b) { attached = true; assert(!world.findCreature(b)->mature); }
        }
        for (auto const& c : world.creatures) {
            assert(c.genome == inherited);
            if (c.generation >= 2 && c.mature) grandchild = true;
            if (c.generation == 1 && c.mature) detached = true;
        }
    }
    assert(attached && detached && grandchild);
}

void testPopulationCapacityDoesNotStrandConstruction()
{
    auto config = SimulationConfig{};
    config.maxCellCount = 8;
    config.resourceCapacity = 100; config.energySourceRadius = 100; config.hazardStrength = 0;
    config.geometryMutationProbability = config.nodeAdditionProbability = config.nodeRemovalProbability = 0;
    World w(config); Simulation sim(w,config); FrozenPhysics physics; sim.setPhysicsBackend(physics);
    runFor(sim,30);
    assert(w.cells.size() == 6 && w.creatures.size() == 2);
    assert(w.matureCreatureCount() == 2 && sim.stats().populationCapReached);
    assert(w.connections.size() == 6);
}

void testCompactionAndOrphanCleanup()
{
    World world;
    auto parent = world.creatures[0].id;
    auto childIndex = world.addCreature(1, false);
    auto childId = world.creatures[childIndex].id;
    world.creatures[0].constructor.offspringCreatureId = childId;
    world.creatures[childIndex].rootCell = world.addCell(childId, {}, {}, 0.2f, true);
    auto survivorIndex = world.addCreature(2, true);
    auto survivorId = world.creatures[survivorIndex].id;
    auto root = world.addCell(survivorId, {2, 2}, {}, 1, true);
    auto cell = world.addCell(survivorId, {3, 2}, {}, 1, false);
    world.creatures[survivorIndex].rootCell = root;
    world.addConnection(root, cell, 1, 30);
    auto stableId = world.cells[root].id;
    assert(world.removeCreatureAndCells(parent));
    assert(world.findCreature(childId) == nullptr);
    assert(world.creatures.size() == 1 && world.cells.size() == 2);
    assert(world.creatures[0].rootCell == 0 && world.cells[0].id == stableId);
    assert(world.connections.size() == 1 && world.hasConnection(0, 1));
    assert(world.allConnectionsValid());
}

class SnapshotPhysics : public PhysicsBackend {
public:
    std::vector<Cell> snapshot;
    int syncs = 0, uploads = 0;
    void initialize(World& w) override { snapshot = w.cells; }
    void step(World& w, float) override { assert(w.cells.size() == snapshot.size()); }
    void syncToCpu(World& w) override {
        ++syncs; assert(w.cells.size() == snapshot.size());
        for (size_t i = 0; i < snapshot.size(); ++i) {
            w.cells[i].position = snapshot[i].position;
            w.cells[i].velocity = snapshot[i].velocity;
        }
    }
    void topologyChanged(World& w) override { ++uploads; snapshot = w.cells; }
};

void testGpuOwnershipAcrossBirthDeathReset()
{
    auto config = SimulationConfig{};
    config.energySourceRadius = 100; config.resourceCapacity = 100; config.hazardStrength = 0;
    World w(config); Simulation sim(w, config); SnapshotPhysics backend; sim.setPhysicsBackend(backend);
    for (int i = 0; i < 120 * 30; ++i) {
        backend.syncs = backend.uploads = 0; sim.step();
        assert(backend.syncs == 1 && backend.uploads <= 1);
    }
    assert(sim.stats().births > 5 && sim.stats().maximumGeneration >= 2);
    w.energySource.strength = 0; w.hazardSource.position = {}; w.hazardSource.radius = 100; w.hazardSource.strength = 10;
    runFor(sim, 5);
    assert(w.cells.empty() && backend.snapshot.empty());
    sim.reset(); assert(w.cells.size() == 3 && backend.snapshot == w.cells);
}

void testResetIsExact()
{
    auto config = SimulationConfig{};
    config.energySourcePosition = config.initialRootPosition;
    config.hazardSourcePosition = {2.0f, 2.0f};
    World world(config);
    Simulation simulation(world, config);
    auto const initialCells = world.cells;
    auto const initialConnections = world.connections;
    auto const initialCreatures = world.creatures;
    auto const initialGenome = world.genome;
    auto const initialEnergySource = world.energySource;
    auto const initialHazardSource = world.hazardSource;
    runFor(simulation, 10.0f);
    simulation.reset();

    assert(world.cells == initialCells);
    assert(world.connections == initialConnections);
    assert(world.creatures == initialCreatures);
    assert(world.genome == initialGenome);
    assert(world.energySource.position == initialEnergySource.position);
    assert(world.energySource.radius == initialEnergySource.radius);
    assert(world.energySource.strength == initialEnergySource.strength);
    assert(world.hazardSource.position == initialHazardSource.position);
    assert(world.hazardSource.radius == initialHazardSource.radius);
    assert(world.hazardSource.strength == initialHazardSource.strength);
    assert(world.nextCellId == 3);
    assert(world.nextCreatureId == 1);
    assert(world.nextLineageId == 2);
    assert(world.rng.state() == DeterministicRng(config.randomSeed).state());
}

} // namespace

int main()
{
    testCameraMapping();
    testInitialWorld();
    testCpuPhysicsScenarioIsDeterministic();
    testGenomeHeredityAndValidity();
    testReproductionMutationAndLineage();
    testSpatialEnergyAndHazard();
    testMetabolismAndConstructionPause();
    testDeathAndTopologyCleanup();
    testSelectionHasFitnessConsequences();
    testFiniteFoodAndPeripheralEnergy();
    testBirthAttachmentAndHeritableGrandchildren();
    testPopulationCapacityDoesNotStrandConstruction();
    testCompactionAndOrphanCleanup();
    testGpuOwnershipAcrossBirthDeathReset();
    testResetIsExact();
    std::cout << "AlienMobileCoreTests passed\n";
    return 0;
}
