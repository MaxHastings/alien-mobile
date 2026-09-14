#include "alienmobile/Simulation.h"
#include "alienmobile/PhysicsScenario.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <set>
using namespace alienmobile;

void validate(World const& w, SimulationConfig const& config)
{
    assert(w.allValuesFinite() && w.allConnectionsValid());
    assert(w.cells.size() <= config.maxCellCount);
    std::set<uint32_t> ids, creatures;
    for (auto const& c : w.creatures) {
        assert(creatures.insert(c.id).second);
        auto cells = w.cellIndicesForCreature(c.id);
        assert(cells.size() <= c.genome.genes[0].nodes.size());
        if (c.mature) assert(cells.size() == c.genome.genes[0].nodes.size());
        if (!cells.empty()) assert(w.cells[c.rootCell].creatureId == c.id);
        if (c.constructor.offspringCreatureId != kInvalidId)
            assert(w.findCreature(c.constructor.offspringCreatureId));
    }
    for (auto const& c : w.cells) {
        assert(ids.insert(c.id).second && creatures.count(c.creatureId));
        assert(c.energy >= 0 && c.energy <= config.maxCellEnergy);
    }
}

int main()
{
    uint64_t controlBirthsAtTenMinutes = 0;
    for (int experiment = 0; experiment < 3; ++experiment) {
        SimulationConfig config;
        World w(config); Simulation sim(w, config);
        int seconds = experiment == 0 ? 1800 : 600;
        std::size_t peak = 0;
        auto start = std::chrono::steady_clock::now();
        for (int i = 1; i <= seconds * 120; ++i) {
            if (i == 60 * 120 && experiment == 1) w.energySource.position = {7, -6};
            if (i == 60 * 120 && experiment == 2) {
                w.hazardSource.position = w.energySource.position;
            }
            sim.step(); peak = std::max(peak, w.cells.size());
            if (i % 120 == 0) validate(w, config);
            if (experiment == 0 && i == 600 * 120) controlBirthsAtTenMinutes = sim.stats().births;
            if (i == 60 * 120 || i == 600 * 120 || i == seconds * 120) {
                std::cout << "experiment=" << experiment << " seconds=" << i/120
                    << " cells=" << w.cells.size() << " births=" << sim.stats().births
                    << " deaths=" << sim.stats().deaths << " generation=" << sim.stats().maximumGeneration
                    << " mutations=" << sim.stats().mutations << " lineages=" << w.lineageCount() << '\n';
            }
        }
        assert(sim.stats().births > 0 && sim.stats().deaths > 0);
        if (experiment == 0) {
            assert(sim.stats().maximumGeneration >= 3 && sim.stats().births > 100);
            assert(!w.cells.empty());
        }
        if (experiment == 1) assert(w.cells.empty());
        if (experiment == 2) assert(sim.stats().births < controlBirthsAtTenMinutes);
        auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
        std::cout << "peakCells=" << peak << " totalHostMs=" << ms << '\n';
        sim.reset(); validate(w, config); assert(w.cells.size() == 3);
    }
    for (std::size_t n : {60, 120, 240}) {
        auto w = makePhysicsStressWorld(n);
        for (uint32_t i = 0; i + 2 < n; i += 3) {
            w.addConnection(i, i+1, 0.7f, 30);
            w.addConnection(i, i+2, 1.4f, 30);
            w.addConnection(i+1, i+2, 0.7f, 30);
        }
        CpuPhysicsBackend physics;
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < 240; ++i) physics.step(w, 1.0f/120);
        auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count()/240;
        std::cout << "cpuCells=" << n << " averageStepMs=" << ms << '\n';
    }
    // Native M4 settings, including behavioral and structural mutations.
    for(unsigned seed=0;seed<3;++seed) {
        auto config=behavioralPlaytestConfig(); config.randomSeed+=seed;
        World w(config); Simulation sim(w,config); std::size_t peak=0;
        for(int step=0;step<120*600;++step) {
            sim.step();peak=std::max(peak,w.cells.size());
            if(step%120==0) validate(w,config);
        }
        assert(sim.stats().births>0 && sim.stats().behaviorMutations>0);
        std::cout<<"M4 native soak seed="<<seed<<" seconds=600 peakCells="<<peak
            <<" cells="<<w.cells.size()<<" births="<<sim.stats().births
            <<" deaths="<<sim.stats().deaths<<" generation="<<sim.stats().maximumGeneration
            <<" behaviorMutations="<<sim.stats().behaviorMutations<<'\n';
        sim.reset();validate(w,config);
        assert(sim.stats().births==0 && sim.stats().behaviorMutations==0);
        assert(w.creatures[0].genome==makeFeederGenome());
    }
}
