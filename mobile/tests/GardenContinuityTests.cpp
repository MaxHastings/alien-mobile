#include "alienmobile/Development.h"
#include "alienmobile/Simulation.h"

#include <cassert>
#include <iostream>

using namespace alienmobile;

namespace {
bool isArchitectureMutation(MutationKind kind)
{
    return kind==MutationKind::Role || kind==MutationKind::InsertNode || kind==MutationKind::DeleteNode
        || kind==MutationKind::DuplicateGene || kind==MutationKind::DeleteGene
        || kind==MutationKind::CopySection || kind==MutationKind::MoveSection
        || kind==MutationKind::Constructor;
}
}

int main()
{
    auto founder=makeGardenFounders().front();
    DeterministicRng sampleRng(0xC01171u);
    unsigned unchanged=0,controller=0,geometry=0,architecture=0,meta=0;
    for(unsigned birth=0;birth<100000;++birth) {
        auto child=mutateDevelopmentGenome(founder,sampleRng);
        assert(isValidDevelopmentGenome(child.genome));
        unchanged+=child.kind==MutationKind::None;
        controller+=child.kind==MutationKind::Behavior || child.kind==MutationKind::Property;
        geometry+=child.kind==MutationKind::Geometry;
        architecture+=isArchitectureMutation(child.kind);
        meta+=child.metaMutated;
    }
    // Tempo is intentionally tiered, not uniformly reduced: most mutations
    // are local controller/property edits; architecture survives almost every
    // birth, while rare structural novelty remains observable at this scale.
    assert(controller>geometry*5);
    assert(geometry>architecture*5);
    assert(architecture>10 && meta>0);
    assert(unchanged+controller+geometry>99000);

    // Follow one viable inherited chain without repairing sterile attempts.
    // A valid, functioning architecture should be able to persist for many
    // generations; this is a heredity measurement, not a fitness guarantee.
    DeterministicRng lineageRng(0x1A7E5EEDU);
    auto lineage=founder;
    unsigned viableDescendants=0,sterileAttempts=0;
    for(unsigned birth=0;birth<2500;++birth) {
        auto child=mutateDevelopmentGenome(lineage,lineageRng);
        auto development=measureDevelopment(child.genome);
        if(!development.complete()) { ++sterileAttempts; continue; }
        lineage=child.genome;
        ++viableDescendants;
    }
    assert(viableDescendants>2450);
    // Functional development fixture with abundant field energy. Ecological
    // viability is measured separately in the finite-particle discovery runs.
    // Exercise every current catalog entry, including invoked-module organs.
    auto catalog=makeCuratedSpecimenCatalog();
    for(std::size_t specimenIndex=0;specimenIndex<catalog.size();++specimenIndex) {
        auto c=evolutionPlaytestConfig();
        c.ecosystemSeed=false;c.autonomousResources=false;c.physicalResources=false;
        c.openStructuralMutation=true;c.hazardStrength=0;c.metabolismRate=.006f;
        c.energySourcePosition={0,0};c.energySourceRadius=80;c.energySourceStrength=1.f;
        c.resourceCapacity=100;c.constructionEnergy=.35f;c.constructionInterval=.09f;
        c.cooldownDuration=.35f;c.maxCellCount=90;c.worldMinX=c.worldMinY=-40;c.worldMaxX=c.worldMaxY=40;
        World world(c);world.cells.clear();world.connections.clear();world.angles.clear();world.creatures.clear();world.motes.clear();
        auto const& specimen=catalog[specimenIndex];
        assert(world.addSpecimen(specimen,{0,0})!=kInvalidId);
        Simulation sim(world,c);
        for(unsigned step=0;step<4800;++step) sim.step();
        unsigned recognizable=0;
        for(auto const& creature:world.creatures) if(creature.mature) {
            bool constructor=false,organ=false;
            DevelopmentCursor cursor;
            while(auto developed=cursor.next(creature.genome)) {
                auto role=developed->physical.behavior.role;
                constructor|=role==CellRole::Constructor;
                organ|=role==CellRole::Motor || role==CellRole::Depot;
            }
            recognizable+=constructor && organ;
        }
        assert(sim.stats().maximumMatureGeneration>=3 && recognizable>0);
        std::cout<<" specimen="<<specimen.name<<" generations="<<sim.stats().maximumMatureGeneration
            <<" mature recognizable="<<recognizable<<" births="<<sim.stats().births<<'\n';
    }
    std::cout<<"garden mutation tempo: unchanged="<<unchanged<<" controller="<<controller
        <<" geometry="<<geometry<<" architecture="<<architecture<<" meta="<<meta
        <<" viable lineage generations="<<viableDescendants<<" sterile="<<sterileAttempts<<'\n';
}
