#include "alienmobile/Development.h"
#include "alienmobile/Simulation.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <fstream>
#include <cstdlib>

using namespace alienmobile;
namespace {
SimulationConfig fixture() {
    SimulationConfig c;
    c.physicalResources=true;c.emissionRate=0;c.metabolismRate=0;c.hazardStrength=0;
    c.motorEnergyCost=0;c.contractileEnergyCost=0;c.initialRootEnergy=1.2f;
    c.constructionInterval=0.05f;c.maxCellCount=512;
    c.worldMinX=c.worldMinY=-100;c.worldMaxX=c.worldMaxY=100;
    c.geometryMutationProbability=c.nodeAdditionProbability=c.nodeRemovalProbability=0;
    c.oscillatorMutationProbability=c.behaviorMutationProbability=0;
    c.cooldownDuration=10000;
    return c;
}
void clear(World& w) { w.cells.clear();w.connections.clear();w.angles.clear();w.creatures.clear();w.motes.clear(); }
void examplesAndBounds() {
    auto founders=makeDevelopmentFounders();
    uint32_t counts[]={26,38,29,50};
    for(std::size_t i=0;i<founders.size();++i) {
        auto const& g=founders[i];assert(isValidGenome(g));
        auto summary=measureDevelopment(g);assert(summary.complete());assert(summary.cells==counts[i]);
        std::size_t encoded=0;for(auto const& gene:g.genes) encoded+=gene.nodes.size();
        assert(encoded>=5 && encoded<=15);
        DevelopmentCursor a,b;uint32_t n=0;
        while(auto x=a.next(g)) {
            auto y=b.next(g);assert(y && x->physical==y->physical);
            assert(x->gene==y->gene && x->node==y->node);
            assert(x->physical.parentNode<int(n));
            assert(n==0 || x->physical.parentNode>=0);++n;
        }
        assert(!b.next(g));assert(a.status()==DevelopmentStatus::Complete);
        assert(g==Genome(g));
        auto shortRun=measureDevelopment(g,{8,counts[i]-1});
        assert(shortRun.status==DevelopmentStatus::CellLimit && shortRun.cells==counts[i]-1);
        assert(measureDevelopment(g,{8,counts[i]}).complete());
        std::cout<<"reference "<<i<<": genes="<<g.genes.size()<<" nodes="<<encoded<<" cells="<<counts[i]<<'\n';
    }
    auto nested=founders[2];
    assert(measureDevelopment(nested,{1,128}).status==DevelopmentStatus::DepthLimit);
    assert(measureDevelopment(nested,{2,128}).complete());
    auto recursive=founders[0];recursive.genes[1].nodes[0].constructorCell=true;
    recursive.genes[1].nodes[0].behavior.role=CellRole::Constructor;
    recursive.genes[1].nodes[0].construction.targetGene=1;
    assert(isValidGenome(recursive));
    auto cycle=measureDevelopment(recursive);assert(cycle.status==DevelopmentStatus::DepthLimit);
    auto invalid=founders[0];invalid.genes[0].nodes[0].construction.targetGene=32;
    assert(!isValidGenome(invalid));assert(measureDevelopment(invalid).cells==0);
    invalid=founders[0];invalid.genes[1].nodes[1].parentNode=2;assert(!isValidGenome(invalid));
    invalid=founders[0];invalid.genes[1].orientation=std::numeric_limits<float>::quiet_NaN();assert(!isValidGenome(invalid));
    invalid=founders[0];invalid.genes[0].nodes[0].construction.branches=0;assert(!isValidGenome(invalid));
    invalid=founders[0];invalid.genes.clear();assert(!isValidGenome(invalid));
    // Single-gene recursion is real and bounded too.
    auto self=makeDefaultGenome();self.genes[0].nodes[0].construction.targetGene=0;
    assert(measureDevelopment(self,{4,128}).status==DevelopmentStatus::DepthLimit);
}
void physicalGrowth() {
    unsigned example=0;
    for(auto g:makeDevelopmentFounders()) {
        auto c=fixture();World w(c);clear(w);w.addDevelopingFounder(g,{},0,0.5f);
        assert(w.cells.size()==1 && !w.creatures[0].mature);
        auto id=w.creatures[0].id;Simulation sim(w,c);
        std::size_t previous=1;unsigned builds=0;
        for(int step=0;step<3000 && !w.creatures[0].mature;++step) {
            // An explicit test energy source, not a gameplay growth subsidy.
            for(auto& cell:w.cells) cell.energy=1.2f;
            float before=0;for(auto const& cell:w.cells) before+=cell.energy+cell.embodiedEnergy;
            sim.step();
            assert(w.cells.size()>=previous && w.cells.size()<=previous+1);
            if(w.cells.size()>previous) {
                float after=0;for(auto const& cell:w.cells) after+=cell.energy+cell.embodiedEnergy;
                assert(std::abs(before-after)<0.0001f);
                ++builds;assert(w.cells.back().genomeNode==previous);
            }
            previous=w.cells.size();assert(w.allConnectionsValid() && w.allValuesFinite());
        }
        assert(w.creatures[0].mature && !w.creatures[0].developingFounder);
        assert(w.creatures[0].genome==g);
        assert(builds+1==measureDevelopment(g).cells);
        assert(w.creatures[0].development.status()==DevelopmentStatus::Complete);
        assert(w.cellIndicesForCreature(id).size()==builds+1);
        if(auto dir=std::getenv("ALIEN_DEVELOPMENT_DUMP")) {
            // Actual positions after paid growth and ten seconds of physics.
            for(int s=0;s<1200;++s) sim.step();
            std::ofstream out(std::string(dir)+"/phenotype-"+std::to_string(example)+".csv");
            out<<"cell,x,y,parent,role\n";
            for(std::size_t n=0;n<w.cells.size();++n) out<<n<<','<<w.cells[n].position.x<<','<<w.cells[n].position.y<<','<<w.creatures[0].bodyNodes[n].parentNode<<','<<int(w.cells[n].behavior.role)<<'\n';
        }
        ++example;
        // Every expressed topology edge connects the correct invocation instance.
        for(uint32_t n=1;n<w.cells.size();++n)
            assert(w.hasConnection(n,uint32_t(w.creatures[0].bodyNodes[n].parentNode)));
    }
}
void pauseAndTiming() {
    auto c=fixture();auto g=makeDevelopmentFounders()[0];
    g.genes[0].nodes[0].construction.intervalScale=3;
    World w(c);clear(w);w.addDevelopingFounder(g,{},0,0.5f);Simulation sim(w,c);
    for(int s=0;s<10;++s) sim.step();assert(w.cells.size()==1);
    for(int s=0;s<10;++s) sim.step();assert(w.cells.size()==2);
    for(auto& cell:w.cells) cell.energy=0;
    auto before=w.cells.size();for(int s=0;s<40;++s) sim.step();assert(w.cells.size()==before);
    for(auto& cell:w.cells) cell.energy=1.2f;
    sim.step();assert(w.cells.size()==before+1);
}
void heredityAndRelease() {
    for(bool separate:{false,true}) {
        auto c=fixture();auto g=makeDevelopmentFounders()[2];
        g.genes[0].nodes[0].construction.separateOffspring=separate;
        World w(c);clear(w);w.addFounder(g,{},0,0.5f);auto parentId=w.creatures[0].id;
        Simulation sim(w,c);
        for(int s=0;s<3000 && !sim.stats().births;++s) {
            for(auto& cell:w.cells) cell.energy=1.2f;
            sim.step();assert(w.allConnectionsValid());
        }
        std::cerr<<"release "<<separate<<" births="<<sim.stats().births<<" creatures="<<w.creatures.size()<<" cells="<<w.cells.size()<<" expected="<<w.creatures[0].expectedCells<<" failed="<<w.creatures[0].developmentFailed<<" next="<<w.creatures[0].constructor.nextNode<<"\n";
        assert(sim.stats().births==1 && w.matureCreatureCount()==2);
        auto const& child=w.creatures[1];assert(child.genome==g && child.mature);
        assert(w.hasConnection(w.findCreature(parentId)->rootCell,child.rootCell)==!separate);
    }
}
}
int main() {
    examplesAndBounds();physicalGrowth();pauseAndTiming();heredityAndRelease();
    std::cout<<"development: all assertions passed\n";
}
