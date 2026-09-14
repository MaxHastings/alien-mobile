#include "BehaviorFixture.h"
#include <cassert>
#include <iostream>
using namespace alienmobile;
int main() {
    auto c=ecosystemPlaytestConfig();c.ecosystemSeed=false;c.emissionRate=0;
    c.constructionEnergy=10;c.hazardStrength=c.metabolismRate=0;
    // Catalog Thread has thrust-driven locomotion. This test overrides thrust
    // activation and changes tail geometry; it does not prove flex-only swimming.
    auto genome=makeCuratedSpecimenCatalog()[1].genome;
    unsigned thrust=0,bend=0,contract=0,rhythm=0;
    for(auto const& n:genome.genes[0].nodes){auto const& b=n.behavior;rhythm+=b.role==CellRole::Generator;if(b.role==CellRole::Motor){thrust+=b.motorMode==MotorMode::Thrust;bend+=b.motorMode==MotorMode::Bending;contract+=b.motorMode==MotorMode::Contractile;}}
    std::cout<<"Actual Thread organs: thrust="<<thrust<<" bend="<<bend<<" contract="<<contract<<" rhythm="<<rhythm<<'\n';assert(thrust>0);
    // Identical inherited signals, different physical tail geometry.
    for(auto& node:genome.genes[0].nodes)if(node.behavior.role==CellRole::Motor && node.behavior.motorMode==MotorMode::Thrust) {
        node.behavior.weights={};node.behavior.biases[Activation]=0.5f;
    }
    auto a=specimen(genome,c);genome.genes[0].nodes[5].relativePosition={-0.6f,1.5f};auto b=specimen(genome,c);
    auto ca=center(a),cb=center(b);Simulation sa(a,c),sb(b,c);
    for(int n=0;n<120*8;++n){sa.step();sb.step();}
    float first=length(center(a)-ca),second=length(center(b)-cb);
    float difference=length((center(a)-ca)-(center(b)-cb));
    assert(first>1 && second>1 && difference>0.1f);
    assert(a.allValuesFinite()&&b.allValuesFinite());
    std::cout<<"same signals, altered tail: displacements="<<first<<","<<second<<" trajectory difference="<<difference<<'\n';
    GenomeMutationConfig mutations;mutations.maxNodes=10;
    mutations.behaviorProbability=1;mutations.oscillatorProbability=0.3f;mutations.motorModeProbability=0.03f;
    DeterministicRng rng(1907), replay(1907);bool mode=false,amount=false,amplitude=false;
    auto inherited=makeContractileFeederGenome();
    for(int n=0;n<5000;++n) {
        auto next=mutateGenome(inherited,rng,mutations),same=mutateGenome(inherited,replay,mutations);
        assert(next.genome==same.genome && isValidGenome(next.genome));
        for(std::size_t i=0;i<std::min(inherited.genes[0].nodes.size(),next.genome.genes[0].nodes.size());++i) {
            mode|=inherited.genes[0].nodes[i].behavior.motorMode!=next.genome.genes[0].nodes[i].behavior.motorMode;
            amount|=inherited.genes[0].nodes[i].behavior.contraction!=next.genome.genes[0].nodes[i].behavior.contraction;
            amplitude|=inherited.genes[0].nodes[i].behavior.amplitude!=next.genome.genes[0].nodes[i].behavior.amplitude;
        }
        auto frozen=mutations;frozen.behaviorProbability=frozen.oscillatorProbability=frozen.motorModeProbability=0;
        frozen.geometryProbability=frozen.additionProbability=frozen.removalProbability=0;
        assert(mutateGenome(next.genome,rng,frozen).genome==next.genome);
        mutateGenome(next.genome,replay,frozen);inherited=next.genome;
    }
    assert(mode&&amount&&amplitude);
    for(bool flexible:{false,true}) {
        auto cfg=ecosystemPlaytestConfig();cfg.ecosystemSeed=false;
        cfg.behaviorMutationProbability=cfg.geometryMutationProbability=cfg.nodeAdditionProbability=cfg.nodeRemovalProbability=cfg.oscillatorMutationProbability=0;
        World world(cfg);world.cells.clear();world.connections.clear();world.angles.clear();world.creatures.clear();world.motes.clear();
        world.addFounder(flexible?makeCuratedSpecimenCatalog()[1].genome:makeCuratedSpecimenCatalog()[0].genome,{-3,-1},0,0.5f);
        Simulation sim(world,cfg);for(int n=0;n<120*120;++n)sim.step();
        assert(sim.stats().births>0 && world.allValuesFinite());
        std::cout<<"physical resource foraging: thread="<<flexible<<" births="<<sim.stats().births
            <<" population="<<world.creatures.size()<<'\n';
    }
    std::cout<<"5000 combined replayed mutations: motor modes, contraction, oscillator amplitude, morphology inherited and valid\n";
}
