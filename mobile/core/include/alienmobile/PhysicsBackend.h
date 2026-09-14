#pragma once

#include "alienmobile/World.h"

namespace alienmobile {

// This is intentionally the smallest boundary between discrete biology and
// continuous motion. It is not a general engine abstraction.
class PhysicsBackend {
public:
    virtual ~PhysicsBackend() = default;

    virtual void initialize(World& world) = 0;
    virtual void step(World& world, float dt) = 0;
    virtual void syncToCpu(World& world) = 0;
    virtual void topologyChanged(World& world) = 0;
};

} // namespace alienmobile
