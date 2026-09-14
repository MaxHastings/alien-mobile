#include "alienmobile/Development.h"
#include "alienmobile/Simulation.h"
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <set>
#include <array>
#include <algorithm>
using namespace alienmobile;
void dumpDescendant(Genome const& g,unsigned seed,unsigned generation) {
    auto dir=std::getenv("ALIEN_DEVELOPMENT_DUMP");if(!dir)return;
    std::string stem=std::string(dir)+"/evolved-"+std::to_string(seed);
    std::ofstream dna(stem+".txt");dna<<std::setprecision(9);dna<<"seed="<<seed<<" generation="<<generation<<" entry="<<g.entryGene<<'\n';
    for(std::size_t gi=0;gi<g.genes.size();++gi) {
        auto const& gene=g.genes[gi];dna<<"gene "<<gi<<" orientation="<<gene.orientation<<" phaseAdvance="<<gene.phaseAdvance<<'\n';
        for(std::size_t ni=0;ni<gene.nodes.size();++ni) {
            auto const& n=gene.nodes[ni];auto const& c=n.construction;
            dna<<" node "<<ni<<" parent="<<n.parentNode<<" edge="<<n.relativePosition.x<<','<<n.relativePosition.y
                <<" role="<<int(n.behavior.role)<<" stiffness="<<n.stiffness<<" target="<<c.targetGene<<" branches="<<c.branches
                <<" repeats="<<c.repetitions<<" angle="<<c.angle<<" branchAngle="<<c.branchAngle<<" repeatAngle="<<c.repetitionAngle<<" interval="<<c.intervalScale<<" separation="<<c.separateOffspring<<'\n';
            auto const& b=n.behavior;
            dna<<"  organ mode="<<int(b.motorMode)<<" waveform="<<int(b.waveform)<<" period="<<b.period<<" phase="<<b.phase
                <<" amplitude="<<b.amplitude<<" contraction="<<b.contraction<<" bending="<<b.bendingAngle<<" strength="<<b.motorStrength
                <<" axis="<<b.axisAngle<<" signalWeight="<<b.signalWeight<<" channel="<<b.motorChannel<<" range="<<b.sensorRange
                <<" sensitivity="<<b.sensitivity<<" extraction="<<b.extractionRate<<" digestion="<<b.digestionRate<<" neural="<<b.neural<<" self="<<b.selfWeight<<'\n';
            for(auto const& row:b.weights){dna<<"  weights";for(float v:row)dna<<' '<<v;dna<<'\n';}
            dna<<"  biases";for(float v:b.biases)dna<<' '<<v;dna<<'\n';
        }
    }
    auto c=depthPlaytestConfig();c.ecosystemSeed=false;c.developmentalSeed=false;c.emissionRate=0;
    c.hazardStrength=0;c.metabolismRate=0;c.constructionInterval=.02f;c.cooldownDuration=10000;
    c.worldMinX=c.worldMinY=-100;c.worldMaxX=c.worldMaxY=100;c.toroidal=false;
    World w(c);w.cells.clear();w.connections.clear();w.angles.clear();w.creatures.clear();w.motes.clear();
    w.addDevelopingFounder(g,{},0,.5f);Simulation sim(w,c);
    for(int step=0;step<40000 && !w.creatures[0].mature;++step) {
        for(auto& cell:w.cells)cell.energy=1.2f;sim.step();
    }
    assert(w.creatures[0].mature);
    for(int step=0;step<600;++step)sim.step();
    std::ofstream out(stem+".csv");out<<"cell,x,y,parent,role\n";
    for(std::size_t n=0;n<w.cells.size();++n)out<<n<<','<<w.cells[n].position.x<<','<<w.cells[n].position.y<<','<<w.creatures[0].bodyNodes[n].parentNode<<','<<int(w.cells[n].behavior.role)<<'\n';
}
int main() {
    constexpr std::array<MutationKind,12> kinds={MutationKind::Behavior,MutationKind::Geometry,MutationKind::Property,
        MutationKind::Role,MutationKind::InsertNode,MutationKind::DeleteNode,MutationKind::DuplicateGene,MutationKind::DeleteGene,
        MutationKind::CopySection,MutationKind::MoveSection,MutationKind::Constructor,MutationKind::Meta};
    auto founders=makeDevelopmentFounders();
    DeterministicRng rng(1948),repeat(1948);std::array<unsigned,12> changed{};
    std::set<unsigned> counts,genes,roles;unsigned failures=0;
    for(unsigned i=0;i<24000;++i) {
        auto const& parent=founders[(i/12)%founders.size()];auto k=i%kinds.size();
        auto a=applyDevelopmentMutation(parent,rng,kinds[k]);
        auto b=applyDevelopmentMutation(parent,repeat,kinds[k]);
        assert(a.genome==b.genome && a.kind==b.kind);
        if(!isValidDevelopmentGenome(a.genome)) {
            std::cerr<<"invalid mutation operation="<<k<<" iteration="<<i<<'\n';
            for(std::size_t gi=0;gi<a.genome.genes.size();++gi) {
                auto const& gene=a.genome.genes[gi];std::cerr<<"gene "<<gi<<" count "<<gene.nodes.size()<<'\n';
                for(std::size_t ni=0;ni<gene.nodes.size();++ni) {auto const& n=gene.nodes[ni];std::cerr<<ni<<":"<<n.parentNode<<" length "<<length(n.relativePosition)<<" role "<<int(n.behavior.role)<<" ctor "<<n.constructorCell<<" ref "<<n.construction.targetGene<<'\n';}
            } return 1;
        }
        assert(validMutationRates(a.genome.mutationRates));
        auto summary=measureDevelopment(a.genome);
        assert(summary.cells<=128);failures+=!summary.complete();
        changed[k]+=a.mutated();
        if(a.mutated()) founders[(i/12)%founders.size()]=a.genome;
    }
    for(unsigned k=0;k<kinds.size();++k) {std::cout<<"operator "<<int(kinds[k])<<" accepted="<<changed[k]<<'\n';assert(changed[k]>0);}
    std::cout<<"forced deterministic operations=24000 bounded development failures="<<failures<<'\n';
    // A mutation-only lineage experiment. Developmental viability is the only
    // filter; it is not an ecological survival or fitness test.
    unsigned sterile=0,mutations=0;
    for(uint64_t seed=1;seed<=8;++seed) {
        auto g=makeDevelopmentFounders()[(seed-1)%4];DeterministicRng line(seed);
        bool dumped=false;
        for(unsigned generation=0;generation<4000;++generation) {
            auto child=mutateDevelopmentGenome(g,line);assert(isValidDevelopmentGenome(child.genome));
            auto summary=measureDevelopment(child.genome);
            if(!summary.complete()) {++sterile;continue;}
            g=child.genome;mutations+=child.mutated();
            if(!dumped && seed<=4 && generation>150 && summary.cells>=20 && summary.cells<=100) {
                dumpDescendant(g,unsigned(seed),generation);dumped=true;
            }
            counts.insert(summary.cells);genes.insert(unsigned(g.genes.size()));
            for(auto const& gene:g.genes) for(auto const& node:gene.nodes) roles.insert(unsigned(node.behavior.role));
        }
    }
    std::cout<<"lineage attempts=32000 changed="<<mutations<<" sterile="<<sterile
        <<" cell range="<<*counts.begin()<<".."<<*counts.rbegin()<<" distinct counts="<<counts.size()
        <<" gene range="<<*genes.begin()<<".."<<*genes.rbegin()<<" roles="<<roles.size()<<'\n';
    assert(counts.size()>40 && *counts.rbegin()>80 && *counts.begin()<10);
    assert(genes.size()>5 && roles.size()==static_cast<unsigned>(CellRole::Count));
    std::cout<<"structural mutation: all assertions passed\n";
}
