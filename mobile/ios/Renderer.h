#import <MetalKit/MetalKit.h>

#include <memory>

#import "MetalPhysicsBackend.h"
#include "alienmobile/Simulation.h"
#include "alienmobile/CreatureFocus.h"

@interface Renderer : NSObject <MTKViewDelegate> {
@private
    alienmobile::World* _world;
    alienmobile::Simulation* _simulation;
    id<MTLDevice> _device;
    id<MTLCommandQueue> _commandQueue;
    id<MTLRenderPipelineState> _linePipeline;
    id<MTLRenderPipelineState> _cellPipeline;
    id<MTLBuffer> _cameraBuffer;
    id<MTLBuffer> _boundaryStateBuffer, _boundaryConnectionBuffer;
    id<MTLBuffer> _moteStateBuffer;
    id<MTLBuffer> _moteMetadataBuffer;
    NSUInteger _moteCount;
    id<MTLBuffer> _connectionBuffer;
    id<MTLBuffer> _cellMetadataBuffer;
    id<MTLBuffer> _cpuStateBuffer;
    id<MTLBuffer> _sourceStateBuffer;
    id<MTLBuffer> _sourceMetadataBuffer;
    std::unique_ptr<alienmobile::MetalPhysicsBackend> _metalPhysicsBackend;
    NSUInteger _connectionCount;
    NSUInteger _cellInstanceCount;
    NSUInteger _sourceInstanceCount;
    CFTimeInterval _lastFrameTime;
    double _simulationAccumulator;
    double _simulationTimeScale;
    CFTimeInterval _lastDebugLogTime;
    NSUInteger _debugFrames;
    NSUInteger _debugSteps;
    id<MTLCommandBuffer> _lastRenderCommandBuffer;
    double _renderGpuMs, _renderCpuMs;
    BOOL _debugLoggingEnabled;
    alienmobile::Vec2 _cameraCenter;
    float _cameraZoom;
    alienmobile::Vec2 _lastCurrentPoint;
    BOOL _creatingCurrent;
    uint32_t _followedCreature;
    BOOL _followEnded, _tracking;
    alienmobile::CreatureFocus _focus;
    NSMutableDictionary<NSNumber*,NSString*>* _familyNames;
}

- (instancetype)initWithView:(MTKView*)view
                         world:(alienmobile::World&)world
                    simulation:(alienmobile::Simulation&)simulation;

- (void)panByScreenTranslation:(CGPoint)translation viewportSize:(CGSize)viewportSize;
- (void)zoomByScale:(CGFloat)scale atPoint:(CGPoint)point viewportSize:(CGSize)viewportSize;
- (void)resetCamera;
- (void)beginCurrentAtScreenPoint:(CGPoint)point viewportSize:(CGSize)viewportSize;
- (void)extendCurrentToScreenPoint:(CGPoint)point viewportSize:(CGSize)viewportSize;
- (void)endCurrent;
- (BOOL)placeSpecimen:(alienmobile::SpecimenSnapshot const&)specimen
     atScreenPoint:(CGPoint)point viewportSize:(CGSize)viewportSize;
- (BOOL)applyMutagenAtScreenPoint:(CGPoint)point viewportSize:(CGSize)size;
- (BOOL)scatterFoodAtScreenPoint:(CGPoint)point viewportSize:(CGSize)size;
- (BOOL)followAtScreenPoint:(CGPoint)point viewportSize:(CGSize)size;
- (NSString*)focusDescription;
- (uint32_t)followedCreatureId;
- (BOOL)followChild;
- (BOOL)hasChild;
- (std::optional<alienmobile::SpecimenSnapshot>)focusedSpecimen;
- (BOOL)isTracking;
- (BOOL)refocus;
- (void)showWholeWorld;
- (void)clearFamilyNames;
- (void)setFastMode:(BOOL)fastMode;
- (void)setObservationSpeed:(NSUInteger)speed;

@end
