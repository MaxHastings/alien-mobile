#import "MetalPhysicsBackend.h"

#import <QuartzCore/QuartzCore.h>

#include "alienmobile/CpuPhysicsBackend.h"
#include "alienmobile/PhysicsScenario.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <vector>

namespace alienmobile {

namespace {

template <typename T>
id<MTLBuffer> makeSharedBuffer(id<MTLDevice> device, std::vector<T> const& values)
{
    auto const byteLength = std::max<std::size_t>(sizeof(T), values.size() * sizeof(T));
    if (values.empty()) {
        return [device newBufferWithLength:byteLength options:MTLResourceStorageModeShared];
    }
    return [device newBufferWithBytes:values.data() length:byteLength options:MTLResourceStorageModeShared];
}

} // namespace

MetalPhysicsBackend::MetalPhysicsBackend(
    id<MTLDevice> device,
    id<MTLCommandQueue> commandQueue,
    id<MTLLibrary> library,
    SimulationConfig config)
    : _device(device)
    , _commandQueue(commandQueue)
    , _library(library)
    , _config(config)
{
    NSError* error = nil;
    if (_device != nil && _library != nil) {
        auto const function = [_library newFunctionWithName:@"physicsStep"];
        _pipeline = [_device newComputePipelineStateWithFunction:function error:&error];
        _gridPipeline=[_device newComputePipelineStateWithFunction:[_library newFunctionWithName:@"buildSpatialGrid"] error:&error];
    }
    if (_pipeline == nil && error != nil) {
        NSLog(@"ALIEN Mobile Metal physics unavailable: %@", error);
    }
}

bool MetalPhysicsBackend::available() const
{
    return _device != nil && _commandQueue != nil && _pipeline != nil && _gridPipeline != nil;
}

void MetalPhysicsBackend::initialize(World& world)
{
    topologyChanged(world);
}

void MetalPhysicsBackend::waitForGpu()
{
    if (_lastCommandBuffer != nil) {
        [_lastCommandBuffer waitUntilCompleted];
        _lastKernelMs=1000*std::max(0.0,_lastCommandBuffer.GPUEndTime-_lastCommandBuffer.GPUStartTime);
        _lastCommandBuffer = nil;
    }
}

void MetalPhysicsBackend::uploadState(World& world)
{
    std::vector<GpuCellState> initialState;
    initialState.reserve(world.cells.size());
    for (auto const& cell : world.cells) {
        initialState.push_back(GpuCellState{
            cell.position.x,
            cell.position.y,
            cell.velocity.x,
            cell.velocity.y,
        });
    }

    _readState = makeSharedBuffer(_device, initialState);
    _writeState = [_device newBufferWithLength:std::max<std::size_t>(sizeof(GpuCellState), initialState.size() * sizeof(GpuCellState))
                                      options:MTLResourceStorageModeShared];
    if (!initialState.empty()) {
        std::memcpy([_writeState contents], initialState.data(), initialState.size() * sizeof(GpuCellState));
    }
    _currentCellCount = world.cells.size();
}

void MetalPhysicsBackend::rebuildAdjacency(World& world)
{
    std::vector<GpuAdjacency> adjacency(world.cells.size());
    for (auto const& connection : world.connections) {
        if (connection.cellA >= world.cells.size() || connection.cellB >= world.cells.size()) {
            continue;
        }
        ++adjacency[connection.cellA].count;
        ++adjacency[connection.cellB].count;
    }

    uint32_t nextOffset = 0;
    for (auto& entry : adjacency) {
        entry.offset = nextOffset;
        nextOffset += entry.count;
        entry.padding0 = 0;
        entry.padding1 = 0;
    }

    _neighbors.resize(nextOffset);
    auto& neighbors=_neighbors;
    _edgeSlots.clear();_edgeSlots.reserve(world.connections.size());
    std::vector<uint32_t> nextIndex(adjacency.size());
    for (std::size_t index = 0; index < adjacency.size(); ++index) {
        nextIndex[index] = adjacency[index].offset;
    }
    for (auto const& connection : world.connections) {
        if (connection.cellA >= world.cells.size() || connection.cellB >= world.cells.size()) {
            continue;
        }
        _edgeSlots.push_back({nextIndex[connection.cellA],nextIndex[connection.cellB]});
        neighbors[nextIndex[connection.cellA]++] = GpuNeighbor{
            connection.cellB,
            connection.restLength,
            connection.stiffness,
            0,
        };
        neighbors[nextIndex[connection.cellB]++] = GpuNeighbor{
            connection.cellA,
            connection.restLength,
            connection.stiffness,
            0,
        };
    }

    // Each joint is stored for its three participating cells. Threads still
    // write only their own state; no floating-point atomics or cross writes.
    for(auto const& joint:world.angles) {
        ++adjacency[joint.cellA].padding1;++adjacency[joint.center].padding1;++adjacency[joint.cellB].padding1;
    }
    uint32_t angleOffset=0;
    for(auto& entry:adjacency) {entry.padding0=angleOffset;angleOffset+=entry.padding1;}
    _angles.resize(angleOffset);
    auto& gpuAngles=_angles;
    _jointSlots.clear();_jointSlots.reserve(world.angles.size());
    for(std::size_t i=0;i<adjacency.size();++i) nextIndex[i]=adjacency[i].padding0;
    for(auto const& joint:world.angles) {
        uint32_t ids[]={joint.cellA,joint.center,joint.cellB};
        _jointSlots.push_back({nextIndex[ids[0]],nextIndex[ids[1]],nextIndex[ids[2]]});
        for(uint32_t slot=0;slot<3;++slot) gpuAngles[nextIndex[ids[slot]]++]={joint.cellA,joint.center,joint.cellB,slot,joint.targetAngle,joint.stiffness,0,0};
    }
    _angleBuffer=makeSharedBuffer(_device,gpuAngles);
    _adjacencyBuffer = makeSharedBuffer(_device, adjacency);
    _neighborBuffer = makeSharedBuffer(_device, neighbors);
}

void MetalPhysicsBackend::updateRestTargets(World const& world)
{
    // Connectivity does not change when a muscle changes its target. Refresh
    // only dynamic values; keep adjacency and slot maps until topology changes.
    bool neighborsChanged=false,anglesChanged=false;
    std::size_t n=0;
    for(auto const& edge:world.connections) {
        if(edge.cellA>=world.cells.size() || edge.cellB>=world.cells.size()) continue;
        for(auto slot:_edgeSlots[n++]) {
            auto& neighbor=_neighbors[slot];
            neighborsChanged|=neighbor.restLength!=edge.restLength || neighbor.stiffness!=edge.stiffness;
            neighbor.restLength=edge.restLength;neighbor.stiffness=edge.stiffness;
        }
    }
    for(std::size_t i=0;i<world.angles.size();++i) {
        auto const& joint=world.angles[i];
        for(auto slot:_jointSlots[i]) {
            auto& angle=_angles[slot];
            anglesChanged|=angle.targetAngle!=joint.targetAngle || angle.stiffness!=joint.stiffness;
            angle.targetAngle=joint.targetAngle;angle.stiffness=joint.stiffness;
        }
    }
    // Immutable snapshots: earlier queued commands may still read old targets.
    // Inactive muscles can keep their existing snapshots without allocations.
    if(neighborsChanged) _neighborBuffer=makeSharedBuffer(_device,_neighbors);
    if(anglesChanged) _angleBuffer=makeSharedBuffer(_device,_angles);
}

void MetalPhysicsBackend::topologyChanged(World& world)
{
    waitForGpu();
    if (_device == nil) {
        return;
    }
    rebuildAdjacency(world);
    uploadState(world);
}

void MetalPhysicsBackend::updateParams(World const& world, float dt)
{
    GpuPhysicsParams params{
        dt,
        _config.linearDrag,
        _config.springDamping,
        _config.repulsionDistance,
        _config.repulsionStrength,
        _config.worldMinX,
        _config.worldMinY,
        _config.worldMaxX,
        _config.worldMaxY,
        _config.boundaryBounce,
        _config.cellRadius,
        static_cast<uint32_t>(world.cells.size()),
        _config.toroidal ? 1u : 0u,
        uint32_t(std::clamp(int((_config.worldMaxX-_config.worldMinX)/_config.repulsionDistance),1,512)),
        uint32_t(std::clamp(int((_config.worldMaxY-_config.worldMinY)/_config.repulsionDistance),1,512)),
    };
    _paramsBuffer = [_device newBufferWithBytes:&params length:sizeof(params) options:MTLResourceStorageModeShared];
}

void MetalPhysicsBackend::step(World& world, float dt)
{
    if (!available() || !std::isfinite(dt) || dt <= 0.0f) {
        return;
    }
    if (_currentCellCount != world.cells.size() || _readState == nil || _writeState == nil) {
        topologyChanged(world);
    }
    if (_currentCellCount == 0) {
        return;
    }

    // Rest targets are signal-driven runtime state, not immutable topology.
    // A new immutable buffer snapshot is retained by each submitted command.
    if(std::any_of(world.cells.begin(),world.cells.end(),[](auto const& cell) {
        return cell.behavior.role==CellRole::Motor && cell.behavior.motorMode!=MotorMode::Thrust;
    })) updateRestTargets(world);
    updateParams(world, dt);
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    auto const* parameters=static_cast<GpuPhysicsParams const*>([_paramsBuffer contents]);
    auto headBytes=parameters->gridX*parameters->gridY*sizeof(uint32_t);
    auto linkBytes=_currentCellCount*sizeof(uint32_t);
    if(_gridHeads==nil || _gridHeads.length<headBytes)
        _gridHeads=[_device newBufferWithLength:headBytes options:MTLResourceStorageModePrivate];
    if(_gridLinks==nil || _gridLinks.length<linkBytes)
        _gridLinks=[_device newBufferWithLength:linkBytes options:MTLResourceStorageModePrivate];
    // GPU-only writes on the same serial queue are safe across submissions.
    auto heads=_gridHeads,links=_gridLinks;
    auto clear=[commandBuffer blitCommandEncoder];[clear fillBuffer:heads range:NSMakeRange(0,heads.length) value:255];[clear endEncoding];
    auto build=[commandBuffer computeCommandEncoder];[build setComputePipelineState:_gridPipeline];
    [build setBuffer:_readState offset:0 atIndex:0];[build setBuffer:heads offset:0 atIndex:1];
    [build setBuffer:links offset:0 atIndex:2];[build setBuffer:_paramsBuffer offset:0 atIndex:3];
    [build dispatchThreads:MTLSizeMake(_currentCellCount,1,1) threadsPerThreadgroup:MTLSizeMake(_gridPipeline.threadExecutionWidth,1,1)];
    [build endEncoding];
    id<MTLComputeCommandEncoder> encoder = [commandBuffer computeCommandEncoder];
    [encoder setComputePipelineState:_pipeline];
    [encoder setBuffer:_readState offset:0 atIndex:0];
    [encoder setBuffer:_writeState offset:0 atIndex:1];
    [encoder setBuffer:_adjacencyBuffer offset:0 atIndex:2];
    [encoder setBuffer:_neighborBuffer offset:0 atIndex:3];
    [encoder setBuffer:_paramsBuffer offset:0 atIndex:4];
    // Immutable submission snapshot: no CPU overwrite while GPU is consuming it.
    std::vector<Vec2> actuation;
    actuation.reserve(world.cells.size());
    for (auto const& cell : world.cells) actuation.push_back(cell.alive ? cell.thrust : Vec2{});
    auto actuationBuffer = makeSharedBuffer(_device, actuation);
    [encoder setBuffer:actuationBuffer offset:0 atIndex:5];
    [encoder setBuffer:_angleBuffer offset:0 atIndex:6];
    [encoder setBuffer:heads offset:0 atIndex:7];[encoder setBuffer:links offset:0 atIndex:8];

    auto const width = std::max<NSUInteger>(1, _pipeline.threadExecutionWidth);
    auto const maxThreads = std::max<NSUInteger>(1, _pipeline.maxTotalThreadsPerThreadgroup);
    auto const threadsPerGroup = std::min(maxThreads, width * 4);
    MTLSize grid = MTLSizeMake(_currentCellCount, 1, 1);
    MTLSize group = MTLSizeMake(threadsPerGroup, 1, 1);
    [encoder dispatchThreads:grid threadsPerThreadgroup:group];
    [encoder endEncoding];
    [commandBuffer commit];

    _lastCommandBuffer = commandBuffer;
    std::swap(_readState, _writeState);
}

void MetalPhysicsBackend::syncToCpu(World& world)
{
    waitForGpu();
    if (_readState == nil || world.cells.size() != _currentCellCount) {
        return;
    }

    auto const* states = static_cast<GpuCellState const*>([_readState contents]);
    for (std::size_t index = 0; index < world.cells.size(); ++index) {
        world.cells[index].position = {states[index].positionX, states[index].positionY};
        world.cells[index].velocity = {states[index].velocityX, states[index].velocityY};
    }
}

#ifndef NDEBUG

void MetalPhysicsBackend::runDebugParity()
{
    if (!available()) {
        return;
    }

    auto const config = makePhysicsParityConfig();
    auto cpuWorld = makePhysicsParityWorld();
    auto metalWorld = makePhysicsParityWorld();
    CpuPhysicsBackend cpu(config);
    MetalPhysicsBackend metal(_device, _commandQueue, _library, config);
    cpu.initialize(cpuWorld);
    metal.initialize(metalWorld);

    constexpr std::array<int, 4> checkpoints = {{1, 10, 100, 1000}};
    std::size_t checkpointIndex = 0;
    for (int stepIndex = 1; stepIndex <= checkpoints.back(); ++stepIndex) {
        cpu.step(cpuWorld, config.fixedTimeStep);
        metal.step(metalWorld, config.fixedTimeStep);
        if (stepIndex != checkpoints[checkpointIndex]) {
            continue;
        }
        metal.syncToCpu(metalWorld);
        float maxPositionError = 0.0f;
        float maxVelocityError = 0.0f;
        for (std::size_t index = 0; index < cpuWorld.cells.size(); ++index) {
            maxPositionError = std::max(maxPositionError, length(cpuWorld.cells[index].position - metalWorld.cells[index].position));
            maxVelocityError = std::max(maxVelocityError, length(cpuWorld.cells[index].velocity - metalWorld.cells[index].velocity));
        }
        NSLog(@"ALIEN Mobile Metal parity: steps=%d maxPositionError=%.7g maxVelocityError=%.7g",
            stepIndex,
            maxPositionError,
            maxVelocityError);
        ++checkpointIndex;
        if (checkpointIndex == checkpoints.size()) {
            break;
        }
    }
}

void MetalPhysicsBackend::runDebugStressScenario()
{
    if (!available()) {
        return;
    }

    auto const config = SimulationConfig{};
    for (auto const cellCount : {std::size_t(10), std::size_t(100), std::size_t(500), std::size_t(1000)}) {
        auto stressWorld = makePhysicsStressWorld(cellCount);
        MetalPhysicsBackend stress(_device, _commandQueue, _library, config);
        stress.initialize(stressWorld);
        auto const start = CACurrentMediaTime();
        stress.step(stressWorld, config.fixedTimeStep);
        stress.syncToCpu(stressWorld);
        auto const elapsedMilliseconds = (CACurrentMediaTime() - start) * 1000.0;
        NSLog(@"ALIEN Mobile Metal stress: cells=%lu stepMilliseconds=%.3f finite=%s",
            static_cast<unsigned long>(cellCount),
            elapsedMilliseconds,
            stressWorld.allValuesFinite() ? "yes" : "no");
    }
}

#endif

} // namespace alienmobile
