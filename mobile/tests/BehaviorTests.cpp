#include "alienmobile/Simulation.h"
#include <cassert>
#include <algorithm>
#include <cmath>
#include <iostream>
using namespace alienmobile;

#include "BehaviorFixture.h"
int main() {
    auto g=makeLocomotionGenome(); auto a=specimen(g), b=a;
    assert(generatorOutput(g.genes[0].nodes[1].behavior,0)==1);
    assert(generatorOutput(g.genes[0].nodes[1].behavior,1)==-1);
    assert(generatorOutput(g.genes[0].nodes[1].behavior,2)==1);
    auto phase=g.genes[0].nodes[1].behavior;phase.phase=0.5f;assert(generatorOutput(phase,0)==-1);
    phase.waveform=Waveform::Sine;phase.phase=0;assert(std::abs(generatorOutput(phase,0.5)-1)<1e-6);
    updateSignals(a,1.f/120);assert(a.cells[2].currentSignals[Oscillator]==0);
    updateSignals(a,1.f/120);assert(a.cells[2].currentSignals[Oscillator]==1);
    updateSignals(b,1.f/120);updateSignals(b,1.f/120);assert(a.cells==b.cells);
    // Reversing traversal cannot leak next signals into current signals.
    std::reverse(b.connections.begin(),b.connections.end());updateSignals(a,1.f/120);updateSignals(b,1.f/120);assert(a.cells==b.cells);
    SimulationConfig cfg;cfg.energySourceStrength=0;cfg.hazardStrength=0;cfg.metabolismRate=0;
    cfg.constructionEnergy=10;cfg.initialVelocity={};
    a=specimen(g,cfg);b=a;for(auto& c:b.cells)c.behavior.motorStrength=0;
    Simulation sa(a,cfg),sb(b,cfg);auto start=center(a);
    for(int i=0;i<120*6;++i){sa.step();sb.step();}
    std::cout<<"oscillator locomotion displacement="<<length(center(a)-start)<<" inactive="<<length(center(b)-start)<<" paid="<<sa.stats().motorEnergy<<'\n';
    assert(length(center(a)-start)>0.5f);assert(length(center(b)-start)<1e-4f);
    assert(sa.stats().motorEnergy>0.01);assert(sb.stats().motorEnergy==0);
    for(auto& c:a.cells){c.energy=0;c.currentSignals[Oscillator]=1;}
    updateActuation(a,0.1,0.035);for(auto const& c:a.cells)assert(length(c.thrust)==0);
    auto rotated=specimen(g);for(auto& c:rotated.cells)c.position=c.position*-1;
    assert(cellAxis(rotated,2).x<-0.99);
    std::cout<<"signals, oscillator, physical actuation gates passed\n";

    GenomeMutationConfig mutation;mutation.geometryProbability=mutation.additionProbability=mutation.removalProbability=0;
    mutation.oscillatorProbability=1;DeterministicRng rng(77);
    auto descendant=g;bool periodChanged=false,phaseChanged=false,amplitudeChanged=false;
    for(int trial=0;trial<100;++trial) {
        auto child=mutateGenome(descendant,rng,mutation);
        periodChanged |= child.genome.genes[0].nodes[1].behavior.period!=descendant.genes[0].nodes[1].behavior.period;
        phaseChanged |= child.genome.genes[0].nodes[1].behavior.phase!=descendant.genes[0].nodes[1].behavior.phase;
        amplitudeChanged |= child.genome.genes[0].nodes[1].behavior.amplitude!=descendant.genes[0].nodes[1].behavior.amplitude;
        auto noMutation=mutation;noMutation.oscillatorProbability=0;
        assert(mutateGenome(child.genome,rng,noMutation).genome==child.genome);
        assert(isValidGenome(child.genome));descendant=child.genome;
    }
    assert(periodChanged&&phaseChanged&&amplitudeChanged);

    auto feeder=makeFeederGenome();
    for(Vec2 source : {Vec2{3,0},Vec2{-3,0},Vec2{0,3},Vec2{0,-3}}) {
        auto w=specimen(feeder);w.energySource.position=w.cells[1].position+source;w.energySource.radius=2;
        auto out=energySensor(w,1);
        assert(out[EnergyX]*source.x+out[EnergyY]*source.y>0.1f);
        for(auto& c:w.cells)c.position=c.position*-1;
        w.energySource.position=w.energySource.position*-1;
        auto rotatedOut=energySensor(w,1);
        assert(std::abs(out[EnergyX]-rotatedOut[EnergyX])<1e-5f);
        assert(std::abs(out[EnergyY]-rotatedOut[EnergyY])<1e-5f);
        w.energySource.position={100,100};assert(energySensor(w,1)[EnergyIntensity]==0);
    }
    // Crude founder's policy is contained entirely in the copied matrix.
    for(Vec2 food : {Vec2{5,0},Vec2{0,5},Vec2{0,-5},Vec2{-4,0}}) {
        auto w=specimen(feeder,cfg); w.energySource.position=food;w.energySource.strength=0.72f;w.energySource.radius=2;
        Simulation sim(w,cfg); w.energySource.position=food;w.energySource.strength=0.72f;w.energySource.radius=2;
        float initial=length(center(w)-food),best=initial;
        for(int n=0;n<120*40;++n) {sim.step();best=std::min(best,length(center(w)-food));}
        std::cout<<"feeder food="<<food.x<<","<<food.y<<" initial="<<initial<<" closest="<<best<<'\n';
        assert(best<initial-1.0f);
    }

    // Actual construction through grandchildren retains the modified genome,
    // including its controller, rather than restoring the initial founder.
    {
        auto inherited=feeder;
        inherited.genes[0].nodes[2].behavior.weights[Activation][EnergyX]=0.91f;
        inherited.genes[0].nodes[3].behavior.motorStrength=1.1f;
        SimulationConfig c;c.geometryMutationProbability=c.nodeAdditionProbability=c.nodeRemovalProbability=0;
        c.oscillatorMutationProbability=0;c.hazardStrength=0;c.energySourceRadius=100;
        c.resourceCapacity=100;c.maxCellCount=80;
        auto w=specimen(inherited,c);Simulation sim(w,c);
        for(int n=0;n<120*60;++n)sim.step();
        assert(sim.stats().maximumGeneration>=2);
        bool grandchild=false;
        for(auto const& body:w.creatures) {
            assert(body.genome==inherited);
            grandchild|=body.generation>=2&&body.mature;
            for(auto i:w.cellIndicesForCreature(body.id)) {
                auto const& cell=w.cells[i];
                assert(cell.behavior==inherited.genes[0].nodes[cell.genomeNode].behavior);
            }
        }
        assert(grandchild);
        sim.reset();assert(w.cells.size()==3);
    }
    // Changing motor geometry changes the resulting motion, not a speed trait.
    {
        auto altered=g;altered.genes[0].nodes[2].relativePosition={0,1};
        auto straight=specimen(g,cfg), bent=specimen(altered,cfg);
        Simulation first(straight,cfg),second(bent,cfg);
        auto x=center(straight),y=center(bent);
        for(int n=0;n<480;++n){first.step();second.step();}
        assert(length((center(straight)-x)-(center(bent)-y))>0.1f);
    }
    // Broken/missing physical references disable force even with full activation.
    {
        auto w=specimen(g);w.cells[2].position=w.cells[1].position;
        w.cells[2].currentSignals[Oscillator]=1;updateActuation(w,0.1f,0.035f);
        assert(length(w.cells[2].thrust)==0);
        w.creatures[0].mature=false;updateSignals(w,0.1f);
        for(auto const& cell:w.cells)for(float v:cell.currentSignals)assert(v==0);
        auto invalid=g;invalid.genes[0].nodes[1].behavior.period=0;assert(!isValidGenome(invalid));
        invalid=g;invalid.genes[0].nodes[2].behavior.motorChannel=999;assert(!isValidGenome(invalid));
    }

    {
        auto w=specimen(feeder);w.energySource.position={4,0};w.energySource.radius=2;
        auto before=energySensor(w,1);auto pivot=w.cells[1].position;
        for(auto& c:w.cells)c.position=pivot+(pivot-c.position);
        auto after=energySensor(w,1);
        assert(before[EnergyX]>0 && after[EnergyX]<0);
        assert(std::abs(before[EnergyIntensity]-after[EnergyIntensity])<1e-5f);
    }
    {
        auto w=specimen(g);w.cells[2].behavior.signalWeight=0;
        updateSignals(w,0.1);updateSignals(w,0.1);
        assert(w.cells[2].currentSignals[Oscillator]==0);
        w=specimen(g);auto& motor=w.cells[2];motor.behavior.neural=true;
        motor.behavior.weights[Activation][Oscillator]=0.5f;motor.behavior.biases[Activation]=0.25f;
        w.cells[1].currentSignals[Oscillator]=1;updateSignals(w,0.1);
        assert(std::abs(motor.currentSignals[Activation]-std::tanh(0.75f))<1e-6f);
    }
    {
        SimulationConfig c;c.behavioralSeed=true;World w(c);Simulation sim(w,c);
        auto initial=w;std::size_t peak=w.cells.size();
        for(int n=0;n<120*300;++n) {
            if(n==120*60)w.energySource.position+={1.5f,-1.5f};
            sim.step();peak=std::max(peak,w.cells.size());
            if(n%120==0)assert(w.allValuesFinite()&&w.allConnectionsValid());
        }
        std::cout<<"feeder 300s moderate food move: cells="<<w.cells.size()<<" peak="<<peak
            <<" births="<<sim.stats().births<<" deaths="<<sim.stats().deaths<<" generation="<<sim.stats().maximumGeneration<<'\n';
        assert(sim.stats().births>0);
        sim.reset();assert(w.cells==initial.cells && w.connections==initial.connections);
    }
}
