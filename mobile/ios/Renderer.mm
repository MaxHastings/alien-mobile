#import "Renderer.h"
#include "alienmobile/Camera.h"
#include "alienmobile/FrameStepBudget.h"

#import <QuartzCore/QuartzCore.h>
#import <mach/mach.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <unordered_map>

namespace {

struct CameraUniforms {
    float centerX;
    float centerY;
    float zoom;
    float padding;
    float viewportWidth;
    float viewportHeight;
    float worldWidth;
    float worldHeight;
};

struct RenderConnection {
    uint32_t cellA;
    uint32_t cellB;
    float colorR;
    float colorG;
    float colorB;
    float colorA;
};

struct RenderCellMetadata {
    float radius;
    float alpha;
    float colorR;
    float colorG;
    float colorB;
    float padding0;
    float padding1;
    float padding2;
};

static_assert(sizeof(RenderConnection) == 24, "RenderConnection must match Metal layout");
static_assert(sizeof(RenderCellMetadata) == 32, "RenderCellMetadata must match Metal layout");

std::array<float, 3> colorForHue(float hue, float saturation, float value)
{
    hue -= std::floor(hue);
    auto const scaled = hue * 6.0f;
    auto const sector = static_cast<int>(std::floor(scaled));
    auto const fraction = scaled - static_cast<float>(sector);
    auto const p = value * (1.0f - saturation);
    auto const q = value * (1.0f - saturation * fraction);
    auto const t = value * (1.0f - saturation * (1.0f - fraction));
    switch (sector % 6) {
    case 0: return {value, t, p};
    case 1: return {q, value, p};
    case 2: return {p, value, t};
    case 3: return {p, q, value};
    case 4: return {t, p, value};
    default: return {value, p, q};
    }
}

std::array<float, 3> colorForCreature(alienmobile::World const& world, uint32_t creatureId)
{
    auto const* creature = world.findCreature(creatureId);
    float luminance=0.86f+0.14f*float((creatureId*2654435761u)%101)/100.f;
    return colorForHue(creature == nullptr ? 0.54f : creature->lineageHue, 0.68f, luminance);
}

float energyBrightness(float energy)
{
    return alienmobile::clamp(0.38f + energy * 0.46f, 0.16f, 1.0f);
}

} // namespace

@implementation Renderer

- (instancetype)initWithView:(MTKView*)view
                         world:(alienmobile::World&)world
                    simulation:(alienmobile::Simulation&)simulation
{
    self = [super init];
    if (self == nil) {
        return nil;
    }

    _world = &world;
    _simulation = &simulation;
    _device = view.device;
    _commandQueue = [_device newCommandQueue];
    _cameraCenter = {0,0};
    _cameraZoom = simulation.config().autonomousResources ? .052f : (simulation.config().spatialResources ? .068f : simulation.config().primitiveSeed ? 0.15f : (simulation.config().developmentalSeed ? 0.11f : 0.18f));
    _simulationAccumulator = 0.0;
    _simulationTimeScale = 1.0;
    _lastFrameTime = 0.0;
    _lastDebugLogTime = 0.0;
    _creatingCurrent = NO;
    _followedCreature = alienmobile::kInvalidId;_followEnded=NO;
    _familyNames=[NSMutableDictionary dictionary];
    _debugLoggingEnabled = NO;
    if (auto const* debug = std::getenv("ALIEN_MOBILE_DEBUG_OVERLAY")) {
        _debugLoggingEnabled = std::strcmp(debug, "1") == 0;
    }
    if(_debugLoggingEnabled)NSLog(@"ALIEN initial seed=%llu",(unsigned long long)simulation.config().randomSeed);

    NSError* libraryError = nil;
    NSURL* libraryURL = [[NSBundle mainBundle] URLForResource:@"AlienMobileShaders" withExtension:@"metallib"];
    id<MTLLibrary> library = libraryURL == nil ? nil : [_device newLibraryWithURL:libraryURL error:&libraryError];
    if (library == nil) {
        library = [_device newDefaultLibrary];
    }
    MTLRenderPipelineDescriptor* lineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    lineDescriptor.vertexFunction = [library newFunctionWithName:@"lineVertex"];
    lineDescriptor.fragmentFunction = [library newFunctionWithName:@"lineFragment"];
    lineDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
    lineDescriptor.colorAttachments[0].blendingEnabled = YES;
    lineDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    lineDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    lineDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    lineDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    _linePipeline = [_device newRenderPipelineStateWithDescriptor:lineDescriptor error:nil];

    MTLRenderPipelineDescriptor* cellDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    cellDescriptor.vertexFunction = [library newFunctionWithName:@"cellVertex"];
    cellDescriptor.fragmentFunction = [library newFunctionWithName:@"cellFragment"];
    cellDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
    cellDescriptor.colorAttachments[0].blendingEnabled = YES;
    cellDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    cellDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    cellDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    cellDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    _cellPipeline = [_device newRenderPipelineStateWithDescriptor:cellDescriptor error:nil];

    if(!simulation.config().toroidal) {
        auto const& c=simulation.config();
        alienmobile::GpuCellState corners[]={{c.worldMinX,c.worldMinY,0,0},{c.worldMaxX,c.worldMinY,0,0},
            {c.worldMaxX,c.worldMaxY,0,0},{c.worldMinX,c.worldMaxY,0,0}};
        RenderConnection edges[]={{0,1,.22f,.32f,.4f,.5f},{1,2,.22f,.32f,.4f,.5f},
            {2,3,.22f,.32f,.4f,.5f},{3,0,.22f,.32f,.4f,.5f}};
        _boundaryStateBuffer=[_device newBufferWithBytes:corners length:sizeof(corners) options:MTLResourceStorageModeShared];
        _boundaryConnectionBuffer=[_device newBufferWithBytes:edges length:sizeof(edges) options:MTLResourceStorageModeShared];
    }
    // Simulator's virtual GPU has a costly CPU/GPU round trip at each biology
    // step. Use the existing parity-tested CPU physics there; devices retain
    // Metal physics. Rendering remains Metal on both platforms.
    bool useCpuPhysics = TARGET_OS_SIMULATOR;
    if (auto const* requestedBackend = std::getenv("ALIEN_MOBILE_PHYSICS")) {
        useCpuPhysics = std::strcmp(requestedBackend, "cpu") == 0;
    }
    if (!useCpuPhysics) {
        auto metalBackend = std::make_unique<alienmobile::MetalPhysicsBackend>(_device, _commandQueue, library, simulation.config());
        if (metalBackend->available()) {
            _metalPhysicsBackend = std::move(metalBackend);
            _simulation->setPhysicsBackend(*_metalPhysicsBackend);
#ifndef NDEBUG
            if (auto const* parity = std::getenv("ALIEN_MOBILE_PARITY"); parity && std::strcmp(parity, "1") == 0)
                _metalPhysicsBackend->runDebugParity();
            if (auto const* stress = std::getenv("ALIEN_MOBILE_STRESS"); stress != nullptr && std::strcmp(stress, "1") == 0) {
                _metalPhysicsBackend->runDebugStressScenario();
            }
#endif
        } else {
            NSLog(@"ALIEN Mobile: Metal physics pipeline unavailable; using CPU reference physics");
        }
    } else {
        NSLog(@"ALIEN Mobile: using CPU reference physics");
    }

    return self;
}

- (void)drawInMTKView:(MTKView*)view
{
    CFTimeInterval now = CACurrentMediaTime();
    if (_lastFrameTime == 0.0) {
        _lastFrameTime = now;
    }
    auto const elapsed = now - _lastFrameTime > 0.25 ? 0.0 : std::max(0.0, now - _lastFrameTime);
    _lastFrameTime = now;
    _simulationAccumulator += elapsed * _simulationTimeScale;

    auto const fixedStep = static_cast<double>(_simulation->config().fixedTimeStep);
    int steps = 0;
    auto const simulationStart=CACurrentMediaTime();
    alienmobile::FrameStepBudget frameBudget(0.5 / std::max<NSInteger>(30,view.preferredFramesPerSecond));
    while (frameBudget.canStep(_simulationAccumulator,fixedStep,CACurrentMediaTime()-simulationStart)) {
#ifndef NDEBUG
        // Integration-test input sequence: move only the environment, not life.
        if(auto check=std::getenv("ALIEN_MOBILE_FOLLOWING_CHECK"); check && std::strcmp(check,"1")==0
            && !_world->cells.empty()) {
            float time=_world->cells[0].visualAge;
            if(time>=18 && time<19) {
                float t=alienmobile::clamp(time-18,0.0f,1.0f);
                _world->energySource.position=alienmobile::Vec2{-0.5f,-1.5f}*(1-t)+alienmobile::Vec2{2,1.5f}*t;
            } else if(time>=19) _world->energySource.position={2,1.5f};
            if(_debugLoggingEnabled && time>=18 && time<18+fixedStep)
                NSLog(@"FOLLOWING CHECK: moving food; organism state is untouched");
        }
#endif
        _simulation->step();
        _simulationAccumulator -= fixedStep;
        ++steps;
        frameBudget.didStep();
    }
    _simulationAccumulator=alienmobile::FrameStepBudget::discardBacklog(_simulationAccumulator,fixedStep);

    ++_debugFrames;_debugSteps+=steps;
    if (_debugLoggingEnabled && now - _lastDebugLogTime >= 1.0) {
        if(_lastDebugLogTime>0) NSLog(@"ALIEN device: fps=%.2f thermal=%ld motorEnergy=%.5f behaviorMutations=%llu",
            _debugFrames/(now-_lastDebugLogTime),(long)NSProcessInfo.processInfo.thermalState,_simulation->stats().motorEnergy,_simulation->stats().behaviorMutations);
#ifndef NDEBUG
        if(std::getenv("ALIEN_MOBILE_FOLLOWING_CHECK") && !_world->cells.empty()) {
            alienmobile::Vec2 center{};
            for(auto const& cell:_world->cells) center+=cell.position;
            center=center/static_cast<float>(_world->cells.size());
            NSLog(@"FOLLOWING t=%.2f com=(%.3f,%.3f) food=(%.3f,%.3f) distance=%.3f",
                _world->cells[0].visualAge,center.x,center.y,_world->energySource.position.x,
                _world->energySource.position.y,alienmobile::length(center-_world->energySource.position));
        }
#endif
        mach_task_basic_info_data_t memory{};mach_msg_type_number_t count=MACH_TASK_BASIC_INFO_COUNT;
        task_info(mach_task_self(),MACH_TASK_BASIC_INFO,(task_info_t)&memory,&count);
        auto const& timings=_simulation->stats();
        if(_lastDebugLogTime>0) NSLog(@"ECOSYSTEM tps=%.1f biologyMs=%.4f syncMs=%.4f metalMs=%.4f renderCpuMs=%.4f renderGpuMs=%.4f memoryMB=%.1f motes=%lu attacked=%.3f digested=%.3f emitted=%.3f recycled=%.3f",
            _debugSteps/(now-_lastDebugLogTime),timings.biologyMs/std::max<uint64_t>(1,timings.steps),
            timings.synchronizationMs/std::max<uint64_t>(1,timings.steps),
            _metalPhysicsBackend ? _metalPhysicsBackend->lastKernelMs() : 0,_renderCpuMs,_renderGpuMs,
            memory.resident_size/1048576.0,(unsigned long)_world->motes.size(),_world->energyLedger.attacked,
            _world->energyLedger.digested,_world->energyLedger.emitted,_world->energyLedger.recycled);
        double denominator=std::max<uint64_t>(1,timings.steps);
        NSLog(@"PROFILE resources=%.4f signals=%.4f attacks=%.4f actuation=%.4f damage=%.4f development=%.4f mutation=%.4f topology=%.4f cellDeaths=%llu fragments=%llu",
            timings.resourcesMs/denominator,timings.signalsMs/denominator,timings.attacksMs/denominator,
            timings.actuationMs/denominator,timings.damageMs/denominator,timings.developmentMs/denominator,
            timings.mutationMs/denominator,timings.topologyMs/denominator,timings.cellDeaths,timings.fragmentsCreated);
        auto living=std::count_if(_world->cells.begin(),_world->cells.end(),[](auto const& cell){return cell.viability!=alienmobile::CellViability::Dead;});
        NSLog(@"MATERIAL simulatedSeconds=%.2f livingCells=%lu debrisCells=%lu livingOrganisms=%lu",
            timings.steps*_simulation->config().fixedTimeStep,(unsigned long)living,(unsigned long)(_world->cells.size()-living),
            (unsigned long)std::count_if(_world->creatures.begin(),_world->creatures.end(),[](auto const& owner){return !owner.fragment;}));
        unsigned huntingBodies=0;unsigned huntingGeneration=0;
        for(auto const& body:_world->creatures) for(auto const& node:body.genome.genes[0].nodes)
            if(node.behavior.role==alienmobile::CellRole::Attacker) {
                ++huntingBodies;huntingGeneration=std::max(huntingGeneration,body.generation);break;
            }
        NSLog(@"ORGANS attackerBodies=%u maximumLivingAttackerGeneration=%u",huntingBodies,huntingGeneration);
        _debugFrames=0;_debugSteps=0;
        auto const& stats = _simulation->stats();
        NSLog(@"ALIEN debug: creatures=%lu cells=%lu generation=%u lineages=%lu births=%llu deaths=%llu mutations=%llu speed=%.1fx",
            static_cast<unsigned long>(_world->creatures.size()),
            static_cast<unsigned long>(_world->aliveCellCount()),
            stats.maximumGeneration,
            static_cast<unsigned long>(_world->lineageCount()),
            stats.births,
            stats.deaths,
            stats.mutations,
            _simulationTimeScale);
        _lastDebugLogTime = now;
    }

    auto renderStart=CACurrentMediaTime();
    if(_lastRenderCommandBuffer.status==MTLCommandBufferStatusCompleted)
        _renderGpuMs=1000*std::max(0.0,_lastRenderCommandBuffer.GPUEndTime-_lastRenderCommandBuffer.GPUStartTime);
    [self rebuildRenderBuffersForView:view pulseTime:now];

    double preparationMs=1000*(CACurrentMediaTime()-renderStart);
    MTLRenderPassDescriptor* pass = view.currentRenderPassDescriptor;
    id<CAMetalDrawable> drawable = view.currentDrawable;
    if (pass == nil || drawable == nil || _commandQueue == nil) {
        return;
    }

    auto encodeStart=CACurrentMediaTime();
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:pass];
    [encoder setVertexBuffer:_cameraBuffer offset:0 atIndex:1];
    if(_boundaryStateBuffer && _linePipeline) {
        [encoder setRenderPipelineState:_linePipeline];
        [encoder setVertexBuffer:_boundaryConnectionBuffer offset:0 atIndex:0];
        [encoder setVertexBuffer:_boundaryStateBuffer offset:0 atIndex:2];
        [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6 instanceCount:4];
    }
    NSUInteger copies=_simulation->config().toroidal ? 9 : 1;
    if(_moteCount && _cellPipeline) {
        [encoder setRenderPipelineState:_cellPipeline];
        [encoder setVertexBuffer:_moteStateBuffer offset:0 atIndex:0];
        [encoder setVertexBuffer:_moteMetadataBuffer offset:0 atIndex:2];
        [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6 instanceCount:_moteCount*copies];
    }

    if (_sourceInstanceCount > 0 && _sourceMetadataBuffer != nil && _sourceStateBuffer != nil && _cellPipeline != nil) {
        [encoder setRenderPipelineState:_cellPipeline];
        // Fields stay behind life. Handles are drawn above it in the last pass.
        for(NSUInteger index=0;index<_sourceInstanceCount;index+=2) {
            [encoder setVertexBuffer:_sourceStateBuffer offset:index*sizeof(alienmobile::GpuCellState) atIndex:0];
            [encoder setVertexBuffer:_sourceMetadataBuffer offset:index*sizeof(RenderCellMetadata) atIndex:2];
            [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6 instanceCount:copies];
        }
    }

    id<MTLBuffer> stateBuffer = _metalPhysicsBackend != nullptr
        ? _metalPhysicsBackend->currentStateBuffer()
        : _cpuStateBuffer;
    if (_connectionCount > 0 && _connectionBuffer != nil && stateBuffer != nil && _linePipeline != nil) {
        [encoder setRenderPipelineState:_linePipeline];
        [encoder setVertexBuffer:_connectionBuffer offset:0 atIndex:0];
        [encoder setVertexBuffer:stateBuffer offset:0 atIndex:2];
        [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6 instanceCount:_connectionCount*copies];
    }

    if (_cellInstanceCount > 0 && _cellMetadataBuffer != nil && stateBuffer != nil && _cellPipeline != nil) {
        [encoder setRenderPipelineState:_cellPipeline];
        [encoder setVertexBuffer:stateBuffer offset:0 atIndex:0];
        [encoder setVertexBuffer:_cellMetadataBuffer offset:0 atIndex:2];
        [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6 instanceCount:_cellInstanceCount*copies];
    }

    if(_sourceInstanceCount>0 && _sourceStateBuffer && _sourceMetadataBuffer && _cellPipeline) {
        [encoder setRenderPipelineState:_cellPipeline];
        for(NSUInteger index=1;index<_sourceInstanceCount;index+=2) {
            [encoder setVertexBuffer:_sourceStateBuffer offset:index*sizeof(alienmobile::GpuCellState) atIndex:0];
            [encoder setVertexBuffer:_sourceMetadataBuffer offset:index*sizeof(RenderCellMetadata) atIndex:2];
            [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6 instanceCount:copies];
        }
    }

    [encoder endEncoding];
    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
    _lastRenderCommandBuffer=commandBuffer;_renderCpuMs=preparationMs+1000*(CACurrentMediaTime()-encodeStart);
}

- (void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size
{
    (void)view;
    (void)size;
}

- (void)rebuildRenderBuffersForView:(MTKView*)view pulseTime:(CFTimeInterval)pulseTime
{
    if(_followedCreature!=alienmobile::kInvalidId) {
        auto indices=_world->cellIndicesForCreature(_followedCreature);
        if(indices.empty()) {_followEnded=YES;}
        else { alienmobile::Vec2 center{};
            for(auto i:indices)center+=_world->cells[i].position;
            center=center/float(indices.size());
            _cameraCenter+= (center-_cameraCenter)*.18f;
        }
    }
    std::unordered_map<uint32_t,std::array<float,3>> colors;
    colors.reserve(_world->creatures.size());
    for(auto const& creature:_world->creatures)
        colors.emplace(creature.id,colorForCreature(*_world,creature.id));
    auto bodyColor=[&](uint32_t id) {
        auto it=colors.find(id);
        return it!=colors.end() ? it->second : colorForCreature(*_world,id);
    };
    std::vector<RenderConnection> connections;
    connections.reserve(_world->connections.size());
    for (auto const& connection : _world->connections) {
        if (connection.cellA >= _world->cells.size() || connection.cellB >= _world->cells.size()) {
            continue;
        }
        auto const& cellA = _world->cells[connection.cellA];
        auto const& cellB = _world->cells[connection.cellB];
        if (!cellA.alive || !cellB.alive) {
            continue;
        }
        auto const color = bodyColor(cellA.creatureId);
        auto const* creature = _world->findCreature(cellA.creatureId);
        auto const alpha = (creature != nullptr && !creature->mature ? 0.36f : 0.72f)
            * (0.55f + 0.45f * energyBrightness((cellA.energy + cellB.energy) * 0.5f));
        connections.push_back(RenderConnection{connection.cellA, connection.cellB, color[0], color[1], color[2], alpha});
    }

    std::vector<RenderCellMetadata> metadata;
    std::vector<alienmobile::GpuCellState> cpuState;
    metadata.reserve(_world->cells.size());
    cpuState.reserve(_world->cells.size());
    for (auto const& cell : _world->cells) {
        if (!cell.alive) {
            continue;
        }
        auto const color = bodyColor(cell.creatureId);
        auto const brightness = energyBrightness(cell.energy);
        auto const* creature = _world->findCreature(cell.creatureId);
        auto const age = creature == nullptr ? 1.0f : alienmobile::clamp(cell.visualAge * 1.5f, 0.0f, 1.0f);
        auto const alpha = creature != nullptr && !creature->mature
            ? 0.30f + age * 0.55f
            : 0.70f + age * 0.30f;
        auto const isRoot = creature != nullptr && creature->rootCell < _world->cells.size()
            && &_world->cells[creature->rootCell] == &cell;
        auto const newbornScale = creature == nullptr
            ? 1.0f
            : alienmobile::clamp(0.78f + cell.visualAge * 0.45f, 0.78f, 1.0f);
        auto const radius = _simulation->config().cellRadius * (isRoot ? 1.08f : 1.0f) * newbornScale;
        float activityGlow=1+0.22f*alienmobile::length(cell.motorThrust)+.18f*cell.absorptionFlash+0.45f*cell.attackFlash+0.25f*cell.digestionFlash;
        if(cell.behavior.role==alienmobile::CellRole::Generator)
            activityGlow+=0.15f*std::max(0.0f,cell.currentSignals[alienmobile::Oscillator]);
        if(cell.behavior.role==alienmobile::CellRole::EnergySensor)
            activityGlow+=.22f*cell.currentSignals[alienmobile::EnergyIntensity];
        if(cell.behavior.role==alienmobile::CellRole::CreatureSensor)
            activityGlow+=.22f*cell.currentSignals[alienmobile::CreatureIntensity];
        if(isRoot && creature->constructor.offspringCreatureId!=alienmobile::kInvalidId)
            activityGlow+=.12f*(1+std::sin(cell.visualAge*5));
        float reserve=cell.behavior.role==alienmobile::CellRole::Depot
            ? float(cell.energy/_world->cellCapacity(cell)) : 0;
        metadata.push_back(RenderCellMetadata{
            radius * 2.1f,
            alpha * (creature == nullptr ? 1.0f : 1.0f - 0.85f * alienmobile::clamp(cell.starvationTimer / _simulation->config().starvationGracePeriod, 0.0f, 1.0f)),
            color[0] * brightness * activityGlow * (creature && creature->fragment ? 0.35f : 1.f),
            color[1] * brightness * activityGlow * (creature && creature->fragment ? 0.35f : 1.f),
            color[2] * brightness * activityGlow * (creature && creature->fragment ? 0.35f : 1.f),
            (creature && creature->fragment) || _cameraZoom<0.11f ? 0.f : -static_cast<float>(cell.behavior.role),
            cell.behavior.role==alienmobile::CellRole::Depot ? reserve : cell.motorThrust.x,
            cell.motorThrust.y,
        });
        cpuState.push_back(alienmobile::GpuCellState{
            cell.position.x,
            cell.position.y,
            cell.velocity.x,
            cell.velocity.y,
        });
    }

    std::vector<RenderCellMetadata> sourceMetadata;
    std::vector<alienmobile::GpuCellState> sourceState;
    auto const pulse = 1.0f + 0.06f * std::sin(static_cast<float>(pulseTime) * 3.0f);
    auto const energyColor = colorForHue(0.12f, 0.76f, 1.0f);
    auto const hazardColor = colorForHue(0.94f, 0.78f, 0.92f);
    auto addSource = [&](alienmobile::Vec2 position, float radius, std::array<float, 3> color, bool hazard) {
        sourceState.push_back(alienmobile::GpuCellState{position.x, position.y, 0.0f, 0.0f});
        sourceMetadata.push_back(RenderCellMetadata{
            radius,
            hazard ? 0.24f : 0.20f,
            color[0], color[1], color[2], 1.0f, 0.0f, 0.0f,
        });
        sourceState.push_back(alienmobile::GpuCellState{position.x, position.y, 0.0f, 0.0f});
        sourceMetadata.push_back(RenderCellMetadata{
            std::max(_simulation->config().cellRadius * 1.7f, 0.52f) * pulse,
            hazard ? 0.92f : 0.96f,
            color[0], color[1], color[2], hazard ? 3.0f : 2.0f, 0.0f, 0.0f,
        });
    };
    // Gold source and magenta hazard remain renderable for developer fixtures,
    // but are absent from the autonomous player experience.
    if(!_simulation->config().autonomousResources) {
        addSource(_world->energySource.position, _simulation->config().spatialResources ? _simulation->config().resourceLightRadius : _world->energySource.radius, energyColor, false);
        addSource(_world->hazardSource.position, _world->hazardSource.radius, hazardColor, true);
    }

    std::vector<RenderCellMetadata> moteMetadata;
    std::vector<alienmobile::GpuCellState> moteState;
    auto addParticle=[&](alienmobile::Vec2 p,float radius,float alpha,float red,float green,float blue) {
        moteState.push_back({p.x,p.y,0,0});moteMetadata.push_back({radius,alpha,red,green,blue,4,0,0});
    };
    // Dotted boundary depicts the actual exposure radius, in world coordinates.
    if(_world->mutagen.remaining>0) {
        auto const& field=_world->mutagen;
        for(unsigned n=0;n<64;++n) {
            float a=n*6.2831853f/64;
            addParticle(field.position+alienmobile::Vec2{std::cos(a),std::sin(a)}*field.radius,
                .10f,.35f+.5f*std::min(1.f,field.remaining/3.f),.85f,.48f,1.f);
        }
    }
    for(auto const& mote:_world->motes) {
        if(mote.resourceBed>=0) {
            float fullness=alienmobile::clamp(float(mote.energy/std::max(.001,_simulation->config().resourceSiteCapacity)),0.f,1.f);
            addParticle(mote.position,.08f+.13f*std::sqrt(fullness),.08f+.72f*fullness,1,.74f,.22f);
        } else addParticle(mote.position,0.14f+0.06f*std::sqrt(mote.energy),0.8f,1,0.74f,0.22f);
    }
    // Current wisps are not UI: they are a direct picture of the same fields
    // sampled by cells and motes in Ecology.cpp.
    for(auto const& current:_world->playerCurrents) {
        float fade=alienmobile::clamp(1-current.age/current.lifetime,0.f,1.f);
        auto side=alienmobile::Vec2{-current.direction.y,current.direction.x};
        for(int n=-2;n<=2;++n) {
            float spacing=float(n)*.34f*current.radius;
            auto p=current.position+side*spacing-current.direction*(.30f*current.radius*(1-fade));
            addParticle(p,.075f+.045f*fade,.18f+.55f*fade,.26f,.84f,1.f);
        }
    }
    for(auto const& cell:_world->cells) if(cell.behavior.role==alienmobile::CellRole::Attacker && cell.attackFlash>0) {
        auto d=_world->displacement(cell.position,cell.transferOrigin);
        for(int n=1;n<6;++n) addParticle(cell.position+d*(n/6.f),0.12f,cell.attackFlash,0.95f,0.8f,1.f);
    }
    _moteCount=moteState.size();
    _moteStateBuffer=moteState.empty()?nil:[_device newBufferWithBytes:moteState.data() length:moteState.size()*sizeof(alienmobile::GpuCellState) options:MTLResourceStorageModeShared];
    _moteMetadataBuffer=moteMetadata.empty()?nil:[_device newBufferWithBytes:moteMetadata.data() length:moteMetadata.size()*sizeof(RenderCellMetadata) options:MTLResourceStorageModeShared];
    CameraUniforms camera{
        _cameraCenter.x,
        _cameraCenter.y,
        _cameraZoom,
        0.0f,
        static_cast<float>(view.drawableSize.width),
        static_cast<float>(view.drawableSize.height),
        _simulation->config().toroidal ? _simulation->config().worldMaxX-_simulation->config().worldMinX : 0,
        _simulation->config().toroidal ? _simulation->config().worldMaxY-_simulation->config().worldMinY : 0,
    };
    _cameraBuffer = [_device newBufferWithBytes:&camera length:sizeof(camera) options:MTLResourceStorageModeShared];
    _connectionBuffer = connections.empty()
        ? nil
        : [_device newBufferWithBytes:connections.data() length:connections.size() * sizeof(RenderConnection) options:MTLResourceStorageModeShared];
    _cellMetadataBuffer = metadata.empty()
        ? nil
        : [_device newBufferWithBytes:metadata.data() length:metadata.size() * sizeof(RenderCellMetadata) options:MTLResourceStorageModeShared];
    _cpuStateBuffer = cpuState.empty()
        ? nil
        : [_device newBufferWithBytes:cpuState.data() length:cpuState.size() * sizeof(alienmobile::GpuCellState) options:MTLResourceStorageModeShared];
    _sourceMetadataBuffer = sourceMetadata.empty()
        ? nil
        : [_device newBufferWithBytes:sourceMetadata.data() length:sourceMetadata.size() * sizeof(RenderCellMetadata) options:MTLResourceStorageModeShared];
    _sourceStateBuffer = sourceState.empty()
        ? nil
        : [_device newBufferWithBytes:sourceState.data() length:sourceState.size() * sizeof(alienmobile::GpuCellState) options:MTLResourceStorageModeShared];
    _connectionCount = connections.size();
    _cellInstanceCount = metadata.size();
    _sourceInstanceCount = sourceMetadata.size();
}

- (alienmobile::Vec2)worldPointForScreenPoint:(CGPoint)point viewportSize:(CGSize)viewportSize
{
    return alienmobile::worldFromScreen({static_cast<float>(point.x), static_cast<float>(point.y)},
        {static_cast<float>(viewportSize.width), static_cast<float>(viewportSize.height)}, _cameraCenter, _cameraZoom);
}

- (void)panByScreenTranslation:(CGPoint)translation viewportSize:(CGSize)viewportSize
{
    _followedCreature = alienmobile::kInvalidId;_followEnded=NO;
    auto const minimumDimension = static_cast<float>(std::max<CGFloat>(1.0, std::min(viewportSize.width, viewportSize.height)));
    auto const worldUnitsPerPoint = 2.0f / (minimumDimension * _cameraZoom);
    _cameraCenter.x -= static_cast<float>(translation.x) * worldUnitsPerPoint;
    _cameraCenter.y += static_cast<float>(translation.y) * worldUnitsPerPoint;
    auto const& config = _simulation->config();
    _cameraCenter.x = alienmobile::clamp(_cameraCenter.x, config.worldMinX, config.worldMaxX);
    _cameraCenter.y = alienmobile::clamp(_cameraCenter.y, config.worldMinY, config.worldMaxY);
}

- (void)beginCurrentAtScreenPoint:(CGPoint)point viewportSize:(CGSize)viewportSize
{
    _lastCurrentPoint=[self worldPointForScreenPoint:point viewportSize:viewportSize];
    _creatingCurrent=YES;
}

- (void)extendCurrentToScreenPoint:(CGPoint)point viewportSize:(CGSize)viewportSize
{
    if(!_creatingCurrent) return;
    auto next=[self worldPointForScreenPoint:point viewportSize:viewportSize];
    auto delta=_world->displacement(_lastCurrentPoint,next);
    // Touch events can be dense; emit evenly-spaced physical segments so a
    // slow drag and a fast drag leave a comparably continuous current.
    float distance=alienmobile::length(delta);
    if(distance<.08f) return;
    int segments=std::min(5,std::max(1,int(std::ceil(distance/.55f))));
    for(int i=1;i<=segments;++i)
        _world->addPlayerCurrent(_lastCurrentPoint+delta*(float(i)/segments-.5f/segments),delta);
    _lastCurrentPoint=next;
}

- (void)endCurrent
{
    _creatingCurrent=NO;
}

- (BOOL)placeSpecimen:(alienmobile::SpecimenSnapshot const&)specimen
     atScreenPoint:(CGPoint)point viewportSize:(CGSize)viewportSize
{
    auto position=[self worldPointForScreenPoint:point viewportSize:viewportSize];
    auto const& c=_simulation->config();
    position.x=alienmobile::clamp(position.x,c.worldMinX+c.cellRadius,c.worldMaxX-c.cellRadius);
    position.y=alienmobile::clamp(position.y,c.worldMinY+c.cellRadius,c.worldMaxY-c.cellRadius);
    // Placement calls the exact same founder construction path used by reset.
    // It does not tag, shield, or otherwise retain a catalog relationship.
    auto placed=_world->addSpecimenNearby(specimen,position);
    if(placed==alienmobile::kInvalidId) return NO;
    _familyNames[@(placed)]=[NSString stringWithUTF8String:specimen.name.c_str()];
    _followedCreature=placed;_followEnded=NO;_cameraZoom=std::max(_cameraZoom,.22f);
    _simulation->notifyTopologyChanged();
    return YES;
}

- (BOOL)applyMutagenAtScreenPoint:(CGPoint)point viewportSize:(CGSize)size
{
    return _world->applyMutagen([self worldPointForScreenPoint:point viewportSize:size]);
}
- (BOOL)scatterFoodAtScreenPoint:(CGPoint)point viewportSize:(CGSize)size
{
    return _world->scatterFood([self worldPointForScreenPoint:point viewportSize:size]);
}
- (BOOL)followAtScreenPoint:(CGPoint)point viewportSize:(CGSize)size
{
    auto p=[self worldPointForScreenPoint:point viewportSize:size];
    float closest=24.f*2.f/(std::min(size.width,size.height)*_cameraZoom);
    uint32_t chosen=alienmobile::kInvalidId;
    for(auto const& cell:_world->cells) if(cell.alive) {
        auto owner=_world->findCreature(cell.creatureId);
        if(!owner || owner->fragment)continue;
        float distance=alienmobile::length(cell.position-p);
        if(distance<closest){closest=distance;chosen=cell.creatureId;}
    }
    _followedCreature=chosen;_followEnded=NO;
    if(chosen==alienmobile::kInvalidId)return NO;
    _cameraZoom=std::max(_cameraZoom,.22f);
    return YES;
}
- (void)clearFamilyNames {[_familyNames removeAllObjects];}
- (BOOL)hasChild {if(_followedCreature==alienmobile::kInvalidId)return NO;for(auto const& c:_world->creatures)if(c.parentId==_followedCreature && c.mature && !c.fragment)return YES;return NO;}
- (BOOL)followChild {if(_followedCreature==alienmobile::kInvalidId)return NO;for(auto const& c:_world->creatures)if(c.parentId==_followedCreature && c.mature && !c.fragment){_followedCreature=c.id;_followEnded=NO;return YES;}return NO;}
- (uint32_t)followedCreatureId { return _followedCreature; }
- (NSString*)focusDescription
{
    auto owner=_world->findCreature(_followedCreature);
    if(!owner)return _followEnded ? @"The organism you followed is gone · its descendants may remain" : @"";
    NSString* event=owner->constructor.offspringCreatureId!=alienmobile::kInvalidId ? @" · building offspring" : @"";
    if(owner->rootCell<_world->cells.size() && _world->mutationExposure(_world->cells[owner->rootCell].position)>1)
        event=@" · temporary exposure";
    NSString* change=@"";
    if(owner->generation>0){switch(owner->birthMutation){
        case alienmobile::MutationKind::Geometry:change=@" · shape shifted";break;
        case alienmobile::MutationKind::InsertNode:case alienmobile::MutationKind::DeleteNode:change=@" · body changed";break;
        case alienmobile::MutationKind::Role:change=@" · organ changed";break;
        case alienmobile::MutationKind::Behavior:case alienmobile::MutationKind::Property:change=@" · function varied";break;
        default:break;}}
    NSString* family=_familyNames[@(owner->ancestorId)]?:[NSString stringWithFormat:@"Ancestor %u",owner->ancestorId+1];
    return [NSString stringWithFormat:@"%@ · generation %u%@%@",family,owner->generation,change,event];
}

- (void)setFastMode:(BOOL)fastMode
{
    _simulationTimeScale = fastMode ? 2.0 : 1.0;
}

- (void)setObservationSpeed:(NSUInteger)speed
{
    if(speed==0 || speed==1 || speed==2 || speed==4 || speed==8) _simulationTimeScale=double(speed);
}

- (void)zoomByScale:(CGFloat)scale atPoint:(CGPoint)point viewportSize:(CGSize)viewportSize
{
    if (!std::isfinite(scale) || scale <= 0.0) {
        return;
    }
    _followedCreature=alienmobile::kInvalidId;_followEnded=NO;
    auto before = [self worldPointForScreenPoint:point viewportSize:viewportSize];
    _cameraZoom = alienmobile::clamp(_cameraZoom * static_cast<float>(scale), 0.025f, 1.5f);
    auto after = [self worldPointForScreenPoint:point viewportSize:viewportSize];
    _cameraCenter += before - after;
}

- (void)resetCamera
{
    _cameraCenter = {0,0};
    _cameraZoom = _simulation->config().autonomousResources ? .052f : (_simulation->config().spatialResources ? .068f : _simulation->config().primitiveSeed ? 0.15f : (_simulation->config().developmentalSeed ? 0.11f : 0.18f));
    _simulationAccumulator = 0.0;
    _lastFrameTime = 0.0;
    _creatingCurrent = NO;
    _followedCreature = alienmobile::kInvalidId;_followEnded=NO;
}

@end
