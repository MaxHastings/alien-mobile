#import <Metal/Metal.h>
#include "MetalPhysicsBackend.h"
#include "alienmobile/CpuPhysicsBackend.h"
#include <cassert>
#include <iostream>
using namespace alienmobile;
int main(int argc,char**argv) {@autoreleasepool {
 auto device=MTLCreateSystemDefaultDevice();assert(device);NSError*error=nil;
 auto lib=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[1])] error:&error];assert(lib);
 for(int count:{2,1200})for(bool wrapped:{false,true}) {
 auto c=depthPlaytestConfig();c.toroidal=wrapped;if(count==2){c.worldMinX=c.worldMinY=-.5;c.worldMaxX=c.worldMaxY=.5;}
 World a(c);a.cells.clear();a.connections.clear();a.angles.clear();a.creatures.clear();
 auto owner=a.addCreature(0,true);auto id=a.creatures[owner].id;a.creatures[owner].rootCell=0;
 DeterministicRng rng(11);
 for(int i=0;i<count;++i){float x=count==2?0:rng.nextUnit()*2-1;float y=count==2?0:rng.nextUnit()*2-1;a.addCell(id,{x,y},{},1,false);}
 auto b=a;CpuPhysicsBackend cpu(c);MetalPhysicsBackend gpu(device,[device newCommandQueue],lib,c);gpu.initialize(b);
 cpu.step(a,c.fixedTimeStep);gpu.step(b,c.fixedTimeStep);gpu.syncToCpu(b);
 float maxError=0;for(int i=0;i<count;++i)maxError=std::max(maxError,length(a.cells[i].velocity-b.cells[i].velocity));
 std::cout<<"cells="<<count<<" wrapped="<<wrapped<<" dense grid velocity error="<<maxError<<'\n';assert(maxError<.0001);
 }
}}
