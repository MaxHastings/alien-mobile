#import <Metal/Metal.h>
#include "MetalPhysicsBackend.h"
#include "alienmobile/CpuPhysicsBackend.h"
#include "alienmobile/PhysicsScenario.h"
#include "alienmobile/Simulation.h"
#include "BehaviorFixture.h"
#include <cassert>
#include <iostream>
#include <chrono>
using namespace alienmobile;
int main(int argc, char** argv) {
    @autoreleasepool {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (!device) { std::cerr << "No Metal device\n"; return 77; }
        NSError* error = nil;
        id<MTLLibrary> library = [device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[1])] error:&error];
        assert(library);
        auto queue = [device newCommandQueue];
        auto config = makePhysicsParityConfig();
        auto a = makePhysicsParityWorld(), b = a;
        CpuPhysicsBackend cpu(config); MetalPhysicsBackend gpu(device, queue, library, config);
        assert(gpu.available()); cpu.initialize(a); gpu.initialize(b);
        float maxP = 0, maxV = 0;
        for (int i = 0; i < 1000; ++i) {
            cpu.step(a, config.fixedTimeStep); gpu.step(b, config.fixedTimeStep);
            if (i % 100 == 99 || i == 0) {
                gpu.syncToCpu(b);
                for (size_t j = 0; j < a.cells.size(); ++j) {
                    maxP = std::max(maxP, length(a.cells[j].position - b.cells[j].position));
                    maxV = std::max(maxV, length(a.cells[j].velocity - b.cells[j].velocity));
                }
            }
        }
        assert(maxP < 0.0001f && maxV < 0.0001f);
        std::cout << "Metal parity 1000 steps: position=" << maxP << " velocity=" << maxV << '\n';
        // Articulated, actuated body: dynamic angular targets on the GPU,
        // including a wrapped edge and inherited stiffness extremes.
        for(float stiffness:{0.1f,2.f}) {
            auto c=depthPlaytestConfig();c.ecosystemSeed=false;c.emissionRate=0;
            c.constructionEnergy=10;c.hazardStrength=c.metabolismRate=0;
            Genome g;g.genes[0].nodes={{-1,{},true},{0,{1,0},false},{1,{1,.2f},false},{2,{1,.1f},false}};
            g.genes[0].nodes[1].behavior.role=CellRole::Generator;
            g.genes[0].nodes[1].behavior.waveform=Waveform::Sine;
            for(int n=2;n<4;++n) {auto& node=g.genes[0].nodes[n];node.stiffness=stiffness;
                node.behavior.role=CellRole::Motor;node.behavior.motorMode=MotorMode::Bending;}
            World body(c);body.cells.clear();body.connections.clear();body.angles.clear();body.creatures.clear();body.motes.clear();
            body.addFounder(g,{23.5f,0},0,.5f);auto reference=body;
            Simulation actual(body,c),expected(reference,c);
            MetalPhysicsBackend articulatedGpu(device,queue,library,c);actual.setPhysicsBackend(articulatedGpu);
            for(int step=0;step<1200;++step) { @autoreleasepool {
                for(auto& cell:body.cells)cell.energy=1.2f;
                for(auto& cell:reference.cells)cell.energy=1.2f;
                actual.step();expected.step();
            } }
            articulatedGpu.syncToCpu(body);assert(body.cells.size()==4 && reference.cells.size()==4);float error=0;
            for(std::size_t n=0;n<body.cells.size();++n)error=std::max(error,length(body.displacement(body.cells[n].position,reference.cells[n].position)));
            std::cout<<"Metal angular + bending stiffness="<<stiffness<<" parity="<<error<<'\n';
            assert(error<.01f && actual.stats().motorEnergy>0);
            assert(body.allConnectionsValid() && body.allValuesFinite());
        }
        // Damage compaction must invalidate all GPU indices and preserve debris.
        {
            auto c=depthPlaytestConfig();c.ecosystemSeed=false;c.emissionRate=0;
            c.metabolismRate=c.hazardStrength=0;c.constructionEnergy=10;
            auto w=specimen(makeHunterGenome(),c);Simulation sim(w,c);
            MetalPhysicsBackend damageGpu(device,queue,library,c);sim.setPhysicsBackend(damageGpu);
            for(int cycle=0;cycle<40;++cycle) { @autoreleasepool {
                if(w.cells.empty()) {w.addFounder(makeHunterGenome(),{},0,.4f);sim.notifyTopologyChanged();}
                w.cells[w.cells.size()/2].deathRequested=true;sim.step();
                damageGpu.syncToCpu(w);assert(w.allValuesFinite() && w.allConnectionsValid());
                for(auto const& owner:w.creatures)assert(owner.rootCell<w.cells.size());
            } }
            std::cout<<"Metal 40 local damage/topology cycles passed\n";
        }
        // Phase 4: the same oscillator body moves through actual Metal forces.
        {
            SimulationConfig c; c.energySourceStrength=0; c.hazardStrength=0;
            c.metabolismRate=0; c.constructionEnergy=10;
            auto body=specimen(makeLocomotionGenome(),c), reference=body;
            auto initial=center(body);
            Simulation actual(body,c), expected(reference,c);
            MetalPhysicsBackend motorGpu(device,queue,library,c); actual.setPhysicsBackend(motorGpu);
            for(int n=0;n<720;++n) { @autoreleasepool { actual.step(); expected.step(); } }
            motorGpu.syncToCpu(body);
            auto displacement=length(center(body)-initial);
            std::cout<<"Metal oscillator displacement="<<displacement<<" parity="<<length(center(body)-center(reference))<<'\n';
            assert(displacement>0.5f && length(center(body)-center(reference))<0.001f);
            assert(actual.stats().motorEnergy>0.01);
        }
        // Dynamic spring targets and wrapped physical edges use the same GPU path.
        {
            auto c=ecosystemPlaytestConfig();c.ecosystemSeed=false;c.emissionRate=0;
            c.constructionEnergy=10;c.hazardStrength=c.metabolismRate=c.sensorEnergyCost=0;
            auto body=specimen(makeContractileFeederGenome(),c);
            for(auto& cell:body.cells) cell.position=body.wrapped(cell.position+Vec2{23.5f,0});
            auto reference=body;Simulation actual(body,c),expected(reference,c);
            MetalPhysicsBackend dynamicGpu(device,queue,library,c);actual.setPhysicsBackend(dynamicGpu);
            for(int n=0;n<720;++n) { @autoreleasepool { actual.step();expected.step(); } }
            dynamicGpu.syncToCpu(body);float error=0;
            for(std::size_t i=0;i<body.cells.size();++i)
                error=std::max(error,length(body.displacement(body.cells[i].position,reference.cells[i].position)));
            assert(error<0.003f);assert(actual.stats().motorEnergy>0);
            std::cout<<"Metal contractile + torus parity error="<<error<<'\n';
        }
        {
            auto c=ecosystemPlaytestConfig();World w(c);Simulation simulation(w,c);
            MetalPhysicsBackend ecologyGpu(device,queue,library,c);simulation.setPhysicsBackend(ecologyGpu);
            for(int cycle=0;cycle<2;++cycle) {
                for(int step=0;step<120*120;++step) { @autoreleasepool {
                    simulation.step();
                    if(step%120==0) assert(w.allValuesFinite()&&w.allConnectionsValid());
                } }
                ecologyGpu.syncToCpu(w);
                assert(simulation.stats().births>0 && w.energyLedger.attacked>0 && w.energyLedger.digested>0);
                std::cout<<"Metal ecology cycle="<<cycle<<" births="<<simulation.stats().births
                    <<" attacked="<<w.energyLedger.attacked<<" digested="<<w.energyLedger.digested<<'\n';
                simulation.reset();assert(w.creatures.size()==5 && w.motes.size()==160);
            }
        }
        // Exactly overlapping unconnected cells must separate symmetrically.
        a = makePhysicsStressWorld(2); a.cells[1].position = a.cells[0].position; b = a;
        cpu.initialize(a); gpu.initialize(b);
        cpu.step(a, config.fixedTimeStep); gpu.step(b, config.fixedTimeStep); gpu.syncToCpu(b);
        for (size_t i = 0; i < 2; ++i) assert(length(a.cells[i].velocity - b.cells[i].velocity) < 0.0001f);
        config = SimulationConfig{};
        config.energySourceRadius = 100; config.resourceCapacity = 100; config.hazardStrength = 0;
        World world(config); Simulation sim(world, config);
        MetalPhysicsBackend biologyGpu(device, queue, library, config); sim.setPhysicsBackend(biologyGpu);
        for (int cycle = 0; cycle < 3; ++cycle) {
            for (int i = 0; i < 120*30; ++i) { @autoreleasepool { sim.step(); } }
            biologyGpu.syncToCpu(world);
            assert(world.allValuesFinite() && world.allConnectionsValid());
            assert(sim.stats().births > 10 && sim.stats().maximumGeneration >= 2);
            std::cout << "Metal cycle=" << cycle << " cells=" << world.cells.size() << " births=" << sim.stats().births << '\n';
            world.energySource.strength = 0; world.hazardSource.radius = 100; world.hazardSource.strength = 10;
            for (int i = 0; i < 120*5; ++i) { @autoreleasepool { sim.step(); } }
            assert(world.cells.empty()); sim.reset(); assert(world.cells.size() == 3);
        }
        for (size_t n : {60, 120, 240}) {
            auto w = makePhysicsStressWorld(n);
            for (uint32_t i = 0; i+2 < n; i+=3) {
                w.addConnection(i,i+1,0.7f,30); w.addConnection(i,i+2,1.4f,30); w.addConnection(i+1,i+2,0.7f,30);
            }
            MetalPhysicsBackend perf(device,queue,library,config); perf.initialize(w);
            auto start=std::chrono::steady_clock::now();
            for(int i=0;i<240;++i) { @autoreleasepool { perf.step(w,config.fixedTimeStep); perf.syncToCpu(w); } }
            auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/240;
            std::cout<<"metalCells="<<n<<" averageStepAndSyncMs="<<ms<<'\n';
        }
    }
}
