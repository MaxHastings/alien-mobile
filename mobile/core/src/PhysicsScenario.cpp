#include "alienmobile/PhysicsScenario.h"

#include <array>

namespace alienmobile {

SimulationConfig makePhysicsParityConfig()
{
    auto config = SimulationConfig{};
    config.cellRadius = 0.15f;
    config.springStiffness = 20.0f;
    config.springDamping = 1.8f;
    config.linearDrag = 0.35f;
    config.repulsionDistance = 0.78f;
    config.repulsionStrength = 5.0f;
    config.worldMinX = -2.0f;
    config.worldMaxX = 2.0f;
    config.worldMinY = -1.5f;
    config.worldMaxY = 1.5f;
    config.boundaryBounce = 0.65f;
    return config;
}

World makePhysicsParityWorld()
{
    World world;
    world.cells.clear();
    world.connections.clear();
    world.creatures.clear();
    world.nextCellId = 0;
    world.nextCreatureId = 0;

    auto const creatureIndex = world.addCreature(0, false);
    auto const creatureId = world.creatures[creatureIndex].id;
    constexpr std::array<Vec2, 10> positions = {{
        {-1.72f, 0.00f}, {-1.05f, 0.35f}, {-0.35f, 0.00f}, {0.40f, 0.10f}, {1.00f, 0.70f},
        {-0.10f, -0.45f}, {0.45f, -0.75f}, {1.65f, -0.25f}, {-1.45f, -0.95f}, {1.65f, 1.25f},
    }};
    constexpr std::array<Vec2, 10> velocities = {{
        {1.20f, 0.40f}, {-0.70f, 0.30f}, {0.30f, -0.80f}, {0.90f, 0.20f}, {-0.45f, 0.75f},
        {0.20f, -0.65f}, {-0.80f, -0.10f}, {1.60f, 0.45f}, {-0.50f, -1.20f}, {0.40f, 1.10f},
    }};

    for (std::size_t index = 0; index < positions.size(); ++index) {
        world.addCell(creatureId, positions[index], velocities[index], 1.0f, false);
    }

    constexpr std::array<std::array<uint32_t, 3>, 9> connectionData = {{
        {{0, 1, 0}}, {{1, 2, 0}}, {{2, 3, 0}}, {{3, 4, 0}}, {{2, 5, 0}},
        {{5, 6, 0}}, {{3, 7, 0}}, {{0, 8, 0}}, {{4, 9, 0}},
    }};
    constexpr std::array<float, 9> restLengths = {{0.45f, 0.60f, 0.75f, 0.65f, 0.55f, 0.50f, 1.00f, 0.70f, 0.65f}};
    auto const config = makePhysicsParityConfig();
    for (std::size_t index = 0; index < connectionData.size(); ++index) {
        world.addConnection(connectionData[index][0], connectionData[index][1], restLengths[index], config.springStiffness);
    }
    return world;
}

World makePhysicsStressWorld(std::size_t cellCount)
{
    World world;
    world.cells.clear();
    world.connections.clear();
    world.creatures.clear();
    world.nextCellId = 0;
    world.nextCreatureId = 0;
    auto const creatureIndex = world.addCreature(0, false);
    auto const creatureId = world.creatures[creatureIndex].id;

    auto const side = static_cast<std::size_t>(32);
    for (std::size_t index = 0; index < cellCount; ++index) {
        auto const x = static_cast<float>(index % side);
        auto const y = static_cast<float>(index / side);
        auto const position = Vec2{-11.0f + x * 0.70f, -11.0f + y * 0.70f};
        world.addCell(creatureId, position, {0.10f, -0.05f}, 1.0f, false);
    }
    return world;
}

} // namespace alienmobile
