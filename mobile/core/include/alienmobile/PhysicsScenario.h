#pragma once

#include <cstddef>

#include "alienmobile/SimulationConfig.h"
#include "alienmobile/World.h"

namespace alienmobile {

// A fixed-topology, no-reproduction world shared by CPU tests and the iOS
// debug parity path.
World makePhysicsParityWorld();
SimulationConfig makePhysicsParityConfig();

// Debug-only stress input. It contains cells but no biology or topology work.
World makePhysicsStressWorld(std::size_t cellCount);

} // namespace alienmobile
