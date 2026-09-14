#pragma once

#include <cstddef>
#include <cstdint>

#include "alienmobile/Math.h"

namespace alienmobile {

struct SimulationConfig {
    bool localDamage = false;
    float energyDiffusion = 0.8f;
    float fragmentLifetime = 4.0f;
    bool articulatedPhysics = false;
    float angularStiffness = 4.0f;
    float bendingEnergyCost = 0.04f;
    bool developmentalSeed = false;
    bool primitiveSeed = false;
    bool heterogeneousEnvironment = false;
    bool activeDevelopment = false;
    // Historical fixed-bed fixture. The playable ecology leaves this off:
    // food is emitted as free, finite motes and is never replenished in place.
    bool spatialResources = false;
    bool gardenSeed = false;
    bool catalogSeed = false;
    // Player-controlled worlds may begin with food but no resident organism.
    // When false, the legacy unconfigured fallback still creates its starter.
    bool emptyStart = false;
    bool mixedSeed = false;
    bool heterogeneousBeds = false;
    bool minimalOrigin = false;
    bool plantedFounders = false;
    bool frozenFounderMutation = false;
    float resourceBedSpacing = 14.f;
    float resourceBedRadius = 4.f;
    float resourceLightRadius = 30.f;
    unsigned resourceSitesPerBed = 64;
    double resourceSiteCapacity = .4;
    float backgroundResourceFraction = .25f;
    float currentStrength = .25f;
    float resourceCycleSeconds = 240.f;
    bool openStructuralMutation = false;
    bool physicalResources = false;
    // The playable garden emits finite motes from several world-owned patches.
    // Legacy source/bed fixtures deliberately leave this disabled.
    bool autonomousResources = false;
    bool toroidal = false;
    bool ecosystemSeed = false;
    float emissionRate = 4.0f;
    float moteEnergy = 0.16f;
    float moteLifetime = 150.0f;
    // Every emitted mote keeps this initial momentum after the light moves.
    // These speeds deliberately carry food beyond a stationary source camp.
    float moteDriftSpeedMin = 0.10f;
    float moteDriftSpeedMax = 0.32f;
    std::size_t maxMotes = 3000;
    float recycleFraction = 0.80f;
    unsigned autonomousPatchCount = 6;
    float autonomousPatchRadius = 3.0f;
    float autonomousPatchDrift = 2.2f;
    float autonomousPatchCycleSeconds = 85.0f;
    float playerCurrentStrength = 14.0f;
    float playerCurrentRadius = 3.5f;
    float playerCurrentLifetime = 1.35f;
    float contractileEnergyCost = 0.06f;
    float attackEnergyCost = 0.025f;
    float attackRate = 0.85f;
    float attackRange = 0.90f;
    float digestionRate = 0.90f;
    // Combat already pays per extraction and loses energy in digestion.
    // Idle organ upkeep must leave time to physically encounter a target.
    float attackerMaintenance = 0.04f;
    float digestorMaintenance = 0.025f;
    float digestionEfficiency = 0.85f;
    bool behavioralSeed = false; // Legacy foundation fixtures explicitly retain their seed.
    float fixedTimeStep = 1.0f / 120.0f;

    float motorEnergyCost = 0.035f;
    float sensorEnergyCost = 0.001f; // Per range squared × sensitivity per second.
    float metabolismRate = 0.025f;
    float resourceCapacity = 2.4f; // Maximum energy supplied per second across all cells.
    float energySourceStrength = 0.72f;
    float energySourceRadius = 3.8f;
    Vec2 energySourcePosition = {-2.8f, 0.0f};
    float hazardStrength = 0.48f;
    float hazardRadius = 3.1f;
    Vec2 hazardSourcePosition = {4.6f, 2.0f};
    float maxCellEnergy = 1.20f;
    float starvationEnergyThreshold = 0.002f;
    float starvationGracePeriod = 1.20f;
    float constructionInterval = 1.20f;
    float constructionEnergy = 0.45f;
    float initialCellEnergy = 0.20f;
    float initialRootEnergy = 0.55f;
    Vec2 initialRootPosition = {-4.0f, 0.0f};
    Vec2 initialVelocity = {0.55f, 0.15f};
    float offspringAnchorDistance = 2.60f;

    float cellRadius = 0.34f;
    float springStiffness = 30.0f;
    float springDamping = 1.8f;
    float linearDrag = 0.35f;
    float repulsionDistance = 0.78f;
    float repulsionStrength = 5.0f;

    float worldMinX = -12.0f;
    float worldMaxX = 12.0f;
    float worldMinY = -12.0f;
    float worldMaxY = 12.0f;
    float boundaryBounce = 0.65f;

    float cooldownDuration = 4.0f;
    float separationSpeed = 0.32f;

    std::size_t maxCellCount = 240;
    std::size_t maxGenomeCells = 6;
    std::size_t minGenomeCells = 2;
    float behaviorMutationProbability = 0.0f;
    float oscillatorMutationProbability = 0.08f;
    float geometryMutationProbability = 0.24f;
    float nodeAdditionProbability = 0.055f;
    float nodeRemovalProbability = 0.040f;
    float maxGeometryPerturbation = 0.28f;
    float minGenomeEdgeLength = 0.45f;
    float maxGenomeEdgeLength = 2.10f;
    uint64_t randomSeed = 0xA11E'0003'2026'0913ULL;
};

// Shared by the native app and its seeded behavioral soak.
inline SimulationConfig behavioralPlaytestConfig()
{
    SimulationConfig c;
    c.behavioralSeed=true;
    c.behaviorMutationProbability=0.65f;
    c.geometryMutationProbability=0.08f;
    c.nodeAdditionProbability=0.015f;
    c.nodeRemovalProbability=0.01f;
    c.energySourcePosition={-0.5f,-1.5f};
    return c;
}

inline SimulationConfig ecosystemPlaytestConfig()
{
    auto c=behavioralPlaytestConfig();
    c.physicalResources=c.toroidal=c.ecosystemSeed=true;
    c.spatialResources=false; // The game ecology is free resource flow, not beds.
    c.worldMinX=c.worldMinY=-24; c.worldMaxX=c.worldMaxY=24;
    c.maxGenomeCells=10; c.maxCellCount=360;
    c.initialCellEnergy=c.initialRootEnergy=0.65f;
    c.metabolismRate=0.016f; c.energySourceRadius=2.6f;
    return c;
}
inline SimulationConfig depthPlaytestConfig()
{
    auto c=ecosystemPlaytestConfig();
    c.worldMinX=c.worldMinY=-36; c.worldMaxX=c.worldMaxY=36;
    c.maxCellCount=1200;c.maxMotes=5000;
    c.energySourceRadius=10.0f;
    c.emissionRate=28.0f;
    c.localDamage=true;
    c.articulatedPhysics=true;
    c.developmentalSeed=true;
    c.openStructuralMutation=true;
    return c;
}
inline SimulationConfig evolutionPlaytestConfig()
{
    auto c=depthPlaytestConfig();
    // Commercial/default world: a readable, diverse cast of ordinary genomes.
    // Origin remains available as an explicit developer configuration.
    c.developmentalSeed=false;c.primitiveSeed=false;c.gardenSeed=false;c.catalogSeed=true;
    c.heterogeneousEnvironment=true;c.activeDevelopment=true;
    c.autonomousResources=true;
    // A modest source emits packets into the world instead of refilling an
    // anchored patch. At these rates, old packets form visible, finite trails.
    c.emissionRate=9.f;c.moteEnergy=.12f;c.moteLifetime=55;
    c.moteDriftSpeedMin=.10f;c.moteDriftSpeedMax=.32f;
    // Keep a phone-sized garden readable.  This is a finite-material limit,
    // not a diversity target: reproduction still competes for the same local
    // motes and a full world never receives replacement organisms.
    c.maxCellCount=240;c.energySourceRadius=4.4f;c.resourceCycleSeconds=120;
    c.worldMinX=c.worldMinY=-24.f;c.worldMaxX=c.worldMaxY=24.f;
    c.autonomousPatchDrift=1.5f;c.energyDiffusion=1.6f;
    c.toroidal=false; // A single readable landscape, not repeated wrap copies.
    c.hazardStrength=0; // Historical hazard is not part of the player world.
    c.repulsionDistance=.84f;c.repulsionStrength=8.0f;
    c.initialCellEnergy=c.initialRootEnergy=.52f;
    c.constructionEnergy=.55f;c.cooldownDuration=6.0f;
    c.offspringAnchorDistance=2*c.cellRadius+.12f;
    return c;
}
} // namespace alienmobile
