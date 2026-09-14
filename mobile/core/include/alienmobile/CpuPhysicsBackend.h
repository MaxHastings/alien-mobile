#pragma once

#include "alienmobile/PhysicsBackend.h"
#include "alienmobile/SimulationConfig.h"

namespace alienmobile {

class CpuPhysicsBackend final : public PhysicsBackend {
public:
    explicit CpuPhysicsBackend(SimulationConfig config = {});

    void initialize(World& world) override;
    void step(World& world, float dt) override;
    void syncToCpu(World& world) override;
    void topologyChanged(World& world) override;

private:
    void applyConnectionForces(World& world, float dt);
    void applyRepulsionForces(World& world, float dt);
    void integrate(World& world, float dt);

    SimulationConfig _config;
};

} // namespace alienmobile
