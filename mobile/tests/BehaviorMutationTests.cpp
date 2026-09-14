#include "BehaviorFixture.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace alienmobile;
int main() {
    GenomeMutationConfig forced;
    forced.geometryProbability=forced.additionProbability=forced.removalProbability=forced.oscillatorProbability=0;
    forced.behaviorProbability=1;
    auto frozen=forced; frozen.behaviorProbability=0;
    auto ancestor=makeFeederGenome();
    auto current=ancestor;
    DeterministicRng rng(1808), replay(1808);
    bool weight=false,bias=false,connection=false,strength=false,axis=false,range=false,sensitivity=false;
    Genome neuralChild;
    for(int iteration=0;iteration<12000;++iteration) {
        auto next=mutateGenome(current,rng,forced);
        auto identical=mutateGenome(current,replay,forced);
        assert(next.genome==identical.genome);
        assert(isValidGenome(next.genome));
        assert(next.genome.genes[0].nodes.size()==current.genes[0].nodes.size());
        unsigned changed=0;
        auto count=[&](float a,float b,float limit) {
            assert(std::abs(a-b)<=limit+1e-6f); changed+=a!=b;
        };
        for(size_t n=0;n<current.genes[0].nodes.size();++n) {
            auto const& a=current.genes[0].nodes[n];auto const& b=next.genome.genes[0].nodes[n];
            assert(a.parentNode==b.parentNode && a.relativePosition==b.relativePosition);
            assert(a.behavior.role==b.behavior.role && a.constructorCell==b.constructorCell);
            auto const& x=a.behavior;auto const& y=b.behavior;
            for(size_t row=0;row<SignalChannelCount;++row) {
                for(size_t col=0;col<SignalChannelCount;++col) count(x.weights[row][col],y.weights[row][col],0.24f);
                count(x.biases[row],y.biases[row],0.12f);
            }
            count(x.signalWeight,y.signalWeight,0.15f);count(x.motorStrength,y.motorStrength,0.24f);
            count(x.axisAngle,y.axisAngle,0.18f);axis|=x.axisAngle!=y.axisAngle;
            count(x.sensorRange,y.sensorRange,0.36f);count(x.sensitivity,y.sensitivity,0.18f);
            weight|=x.weights!=y.weights;bias|=x.biases!=y.biases;
            connection|=x.signalWeight!=y.signalWeight;strength|=x.motorStrength!=y.motorStrength;
            range|=x.sensorRange!=y.sensorRange;sensitivity|=x.sensitivity!=y.sensitivity;
            if(neuralChild.genes[0].nodes.empty() && x.weights!=y.weights) neuralChild=next.genome;
        }
        assert(changed<=1);assert(next.behaviorMutated==(changed==1));
        auto grandchild=mutateGenome(next.genome,rng,frozen);
        // Frozen mutation still consumes the legacy morphology draw; mirror it.
        mutateGenome(next.genome,replay,frozen);
        assert(grandchild.genome==next.genome && !grandchild.mutated());
        current=next.genome;
    }
    assert(weight&&bias&&connection&&strength&&axis&&range&&sensitivity);
    assert(neuralChild!=ancestor);
    // A real B-to-grandchild construction lineage: mutation is disabled after B.
    SimulationConfig c;c.geometryMutationProbability=c.nodeAdditionProbability=c.nodeRemovalProbability=0;
    c.oscillatorMutationProbability=c.behaviorMutationProbability=0;
    c.energySourceRadius=100;c.resourceCapacity=100;c.hazardStrength=0;c.maxCellCount=80;
    auto world=specimen(neuralChild,c);Simulation sim(world,c);
    for(int n=0;n<7200;++n)sim.step();
    bool matureGrandchild=false;
    for(auto const& creature:world.creatures) {
        assert(creature.genome==neuralChild && creature.genome!=ancestor);
        matureGrandchild|=creature.generation>=2&&creature.mature;
        for(auto i:world.cellIndicesForCreature(creature.id)) {
            auto const& cell=world.cells[i];assert(cell.behavior==neuralChild.genes[0].nodes[cell.genomeNode].behavior);
        }
    }
    assert(matureGrandchild);
    // A mutation in an exercised neural weight changes the physical trajectory.
    {
        DeterministicRng search(8);Genome altered;
        for(int n=0;n<20000;++n) {
            auto candidate=mutateGenome(ancestor,search,forced).genome;
            if(candidate.genes[0].nodes[2].behavior.weights[Activation][EnergyX]
                !=ancestor.genes[0].nodes[2].behavior.weights[Activation][EnergyX]) { altered=candidate;break; }
        }
        assert(!altered.genes[0].nodes.empty());
        SimulationConfig trial;trial.constructionEnergy=10;trial.hazardStrength=0;
        trial.energySourcePosition={4,2};trial.energySourceRadius=2;
        auto a=specimen(ancestor,trial),b=specimen(altered,trial);
        Simulation sa(a,trial),sb(b,trial);
        for(int n=0;n<720;++n) {sa.step();sb.step();}
        float difference=length(center(a)-center(b));
        assert(difference>0.0001f);
        std::cout<<"single inherited weight mutation changes 6s COM trajectory by="<<difference<<'\n';
    }
    // Normal low-rate births retain ancestry, including while morphology mutates.
    c.behavioralSeed=true;c.behaviorMutationProbability=0.12f;
    c.geometryMutationProbability=0.24f;c.nodeAdditionProbability=0.055f;c.nodeRemovalProbability=0.04f;
    World evolving(c);Simulation evolution(evolving,c);
    for(int n=0;n<120*180;++n)evolution.step();
    assert(evolving.allValuesFinite()&&evolving.allConnectionsValid());
    assert(evolution.stats().behaviorMutations>0);
    std::cout<<"12000 bounded/replayed mutation trials: weights, biases, connections, motors, sensors covered; B inherited through mature grandchildren\n"
        <<"evolving session births="<<evolution.stats().births<<" behavioral mutations="<<evolution.stats().behaviorMutations
        <<" generation="<<evolution.stats().maximumGeneration<<'\n';
}
