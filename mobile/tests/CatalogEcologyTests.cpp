#include "alienmobile/Simulation.h"

#include <array>
#include <cassert>
#include <iostream>
#include <set>

using namespace alienmobile;

namespace {

constexpr unsigned kFounderCount=4;
constexpr unsigned kSeconds=180;

void freeze(Genome& genome)
{
    // Competition measures the fixed genomes, not a lucky early mutation.
    genome.mutationRates={0,0,0,0,0,0,0,0,0,0,0,0,1,1,1};
}

struct Trial {
    std::array<unsigned,kFounderCount> finalMature{};
    std::array<unsigned,kFounderCount> finalCells{};
    std::array<unsigned,kFounderCount> matureBirths{};
    int winner=-1;
    bool monopoly=false;
    double attacked=0, digested=0;
};

SimulationConfig competitionConfig(unsigned profile,uint64_t seed)
{
    auto c=evolutionPlaytestConfig();
    c.randomSeed=seed;c.ecosystemSeed=false;c.catalogSeed=false;c.openStructuralMutation=true;
    c.worldMinX=c.worldMinY=-24;c.worldMaxX=c.worldMaxY=24;
    c.maxCellCount=240;c.maxMotes=1800;c.hazardStrength=0;
    c.autonomousPatchCount=1;c.autonomousPatchRadius=3.0f;
    if(profile==0) { // a moving, concentrated food cloud
        c.autonomousResources=true;c.emissionRate=10;c.autonomousPatchDrift=1.5f;
    } else if(profile==1) { // food is broad and nearly still
        c.autonomousResources=false;c.heterogeneousEnvironment=false;c.energySourcePosition={0,0};
        c.energySourceRadius=7;c.emissionRate=11;c.moteDriftSpeedMin=c.moteDriftSpeedMax=.04f;
    } else { // food trails are sparse, fast, and short-lived
        c.autonomousResources=true;c.emissionRate=6;c.moteLifetime=32;c.autonomousPatchRadius=2.2f;
        c.autonomousPatchDrift=2.1f;c.moteDriftSpeedMin=.22f;c.moteDriftSpeedMax=.42f;
    }
    return c;
}

Trial runCompetition(unsigned profile,uint64_t seed)
{
    auto c=competitionConfig(profile,seed);
    World world(c);
    // Drop only the constructor's unrelated default body. Keep the seeded,
    // finite motes so every competitor faces the same opening food field.
    world.cells.clear();world.connections.clear();world.angles.clear();world.creatures.clear();
    world.nextCellId=world.nextCreatureId=0;world.nextLineageId=1;
    auto catalog=makeCuratedSpecimenCatalog();
    Vec2 center=profile==1 ? c.energySourcePosition : world.resourcePatchPosition(0);
    // Rotate physical start slots. A name never owns the privileged up-current
    // or central position, while all four begin in the same food patch.
    unsigned rotation=unsigned(seed%kFounderCount);
    for(unsigned founder=0;founder<kFounderCount;++founder) {
        freeze(catalog[founder].genome);
        float slotAngle=float((founder+rotation)%kFounderCount)*1.57079633f;
        Vec2 slot{5*std::cos(slotAngle),5*std::sin(slotAngle)};
        assert(world.addSpecimen(catalog[founder],center+slot,float((founder*2+rotation)%6)*1.04719755f)!=kInvalidId);
    }
    Simulation simulation(world,c);
    std::array<std::set<uint32_t>,kFounderCount> matureSeen;
    for(unsigned step=0;step<120*kSeconds;++step) {
        simulation.step();
        assert(world.allValuesFinite() && world.allConnectionsValid());
        for(auto const& creature:world.creatures) if(creature.mature && creature.lineageId>=1 && creature.lineageId<=kFounderCount) {
            auto founder=creature.lineageId-1;
            if(creature.parentId!=kInvalidId) matureSeen[founder].insert(creature.id);
        }
    }
    Trial result;
    for(auto const& creature:world.creatures) if(creature.mature && creature.lineageId>=1 && creature.lineageId<=kFounderCount) {
        auto founder=creature.lineageId-1;
        ++result.finalMature[founder];result.finalCells[founder]+=unsigned(world.cellIndicesForCreature(creature.id).size());
    }
    for(unsigned founder=0;founder<kFounderCount;++founder) result.matureBirths[founder]=unsigned(matureSeen[founder].size());
    unsigned total=0,best=0;int leader=-1;
    for(unsigned founder=0;founder<kFounderCount;++founder) {
        total+=result.finalCells[founder];
        if(result.finalCells[founder]>best) {best=result.finalCells[founder];leader=int(founder);}
    }
    result.winner=leader;result.monopoly=total>0 && best*100>=total*70;
    result.attacked=world.energyLedger.attacked;result.digested=world.energyLedger.digested;
    return result;
}

} // namespace

int main()
{
    // Matched, rotated starts in each landscape. The thresholds below are
    // intentionally about opportunity and monopoly, not identical performance:
    // no catalog entry may be shut out everywhere.
    std::array<unsigned,kFounderCount> survived{},reproduced{},wins{};
    std::array<unsigned,3> monopolies{};
    constexpr std::array<uint64_t,6> seeds={{17,43,71,101,131,167}};
    for(unsigned profile=0;profile<3;++profile) for(auto seed:seeds) {
        auto result=runCompetition(profile,seed);
        std::cout<<"profile="<<profile<<" seed="<<seed<<" mature=";
        for(auto n:result.finalMature)std::cout<<n<<' ';
        std::cout<<" cells=";for(auto n:result.finalCells)std::cout<<n<<' ';
        std::cout<<" births=";for(auto n:result.matureBirths)std::cout<<n<<' ';
        std::cout<<" winner="<<result.winner<<" monopoly="<<result.monopoly
            <<" attacked="<<result.attacked<<" digested="<<result.digested<<'\n';
        for(unsigned founder=0;founder<kFounderCount;++founder) {
            survived[founder]+=result.finalMature[founder]>0;
            reproduced[founder]+=result.matureBirths[founder]>0;
        }
        if(result.winner>=0)++wins[result.winner];
        monopolies[profile]+=result.monopoly;
    }
    // Calibration thresholds are added after recording this first matched run.
    for(auto count:survived) assert(count>0);
    std::cout<<"survival=";for(auto n:survived)std::cout<<n<<' ';
    std::cout<<" reproduction=";for(auto n:reproduced)std::cout<<n<<' ';
    std::cout<<" wins=";for(auto n:wins)std::cout<<n<<' ';
    std::cout<<" monopolies=";for(auto n:monopolies)std::cout<<n<<' ';
    std::cout<<'\n';
}
