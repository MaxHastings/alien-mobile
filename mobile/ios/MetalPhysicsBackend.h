#import <Metal/Metal.h>

#include <cstddef>
#include <cstdint>
#include <array>
#include <vector>

#include "alienmobile/PhysicsBackend.h"
#include "alienmobile/SimulationConfig.h"

namespace alienmobile {

// These structs contain only the data needed by the continuous physics
// kernel. Scalar fields are used where possible so C++ and Metal offsets are
// explicit and easy to audit.
struct GpuCellState {
    float positionX;
    float positionY;
    float velocityX;
    float velocityY;
};

struct GpuNeighbor {
    uint32_t neighborIndex;
    float restLength;
    float stiffness;
    uint32_t padding;
};

struct GpuAngle {
    uint32_t cellA, center, cellB, slot;
    float targetAngle, stiffness, padding0, padding1;
};
static_assert(sizeof(GpuAngle)==32,"GpuAngle must match Metal layout");

struct GpuAdjacency {
    uint32_t offset;
    uint32_t count;
    uint32_t padding0;
    uint32_t padding1;
};

struct GpuPhysicsParams {
    float dt;
    float drag;
    float connectionDamping;
    float repulsionRadius;
    float repulsionStrength;
    float worldMinX;
    float worldMinY;
    float worldMaxX;
    float worldMaxY;
    float boundaryRestitution;
    float cellRadius;
    uint32_t cellCount;
    uint32_t toroidal;
    uint32_t gridX, gridY;
};

static_assert(sizeof(GpuCellState) == 16, "GpuCellState must match Metal layout");
static_assert(sizeof(GpuNeighbor) == 16, "GpuNeighbor must match Metal layout");
static_assert(sizeof(GpuAdjacency) == 16, "GpuAdjacency must match Metal layout");
static_assert(sizeof(GpuPhysicsParams) == 60, "GpuPhysicsParams must match Metal layout");
static_assert(offsetof(GpuPhysicsParams, cellCount) == 44, "GpuPhysicsParams field offset changed");

class MetalPhysicsBackend final : public PhysicsBackend {
public:
    MetalPhysicsBackend(
        id<MTLDevice> device,
        id<MTLCommandQueue> commandQueue,
        id<MTLLibrary> library,
        SimulationConfig config);

    bool available() const;
    double lastKernelMs() const { return _lastKernelMs; }

    void initialize(World& world) override;
    void step(World& world, float dt) override;
    void syncToCpu(World& world) override;
    void topologyChanged(World& world) override;

    id<MTLBuffer> currentStateBuffer() const { return _readState; }

#ifndef NDEBUG
    void runDebugParity();
    void runDebugStressScenario();
#endif

private:
    double _lastKernelMs=0;
    void waitForGpu();
    void uploadState(World& world);
    void rebuildAdjacency(World& world);
    void updateParams(World const& world, float dt);
    void updateRestTargets(World const& world);
    std::vector<GpuNeighbor> _neighbors;
    std::vector<GpuAngle> _angles;
    std::vector<std::array<uint32_t,2>> _edgeSlots;
    std::vector<std::array<uint32_t,3>> _jointSlots;

    id<MTLDevice> _device;
    id<MTLCommandQueue> _commandQueue;
    id<MTLLibrary> _library;
    id<MTLComputePipelineState> _pipeline, _gridPipeline;

    id<MTLBuffer> _readState;
    id<MTLBuffer> _writeState;
    id<MTLBuffer> _adjacencyBuffer;
    id<MTLBuffer> _neighborBuffer;
    id<MTLBuffer> _angleBuffer;
    id<MTLBuffer> _paramsBuffer;
    id<MTLBuffer> _gridHeads, _gridLinks;
    id<MTLCommandBuffer> _lastCommandBuffer;

    std::size_t _currentCellCount = 0;
    SimulationConfig _config;
};

} // namespace alienmobile
