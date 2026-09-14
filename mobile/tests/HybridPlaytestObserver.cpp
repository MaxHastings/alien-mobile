// Bounded observation of an authored edit in the unmodified player world.
// Measurements never feed back into selection or resource placement.
#include "alienmobile/Creator.h"
#include "alienmobile/GenomeIO.h"
#include "alienmobile/Simulation.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
using namespace alienmobile;
int main(int argc,char** argv) {
    if(argc!=4 && argc!=5)return 2;
    unsigned seed=std::stoul(argv[1]),seconds=std::stoul(argv[2]);std::string stem=argv[3];
    auto config=evolutionPlaytestConfig();config.randomSeed=seed;
    bool emptyStart=argc==5 && std::string(argv[4])=="empty";
    if(emptyStart){config.emptyStart=true;config.ecosystemSeed=config.catalogSeed=false;}
    World world(config);Simulation simulation(world,config);
    auto specimen=makeCuratedSpecimenCatalog()[0];specimen.name="Fanfin";
    auto source=specimen.genome;specimen.genome=editableBody(source);
    assert(addBodyCell(specimen.genome,0,{-1.15f,0}));
    assert(changeOrgan(specimen.genome,4,CellRole::Motor));
    specimen.genome=prepareCreatorRelease(specimen.genome,source);
    {std::ofstream dna(stem+"-ancestor.dna");genomeio::writeGenome(dna,specimen.genome);}
    auto id=world.addSpecimenNearby(specimen,world.resourcePatchPosition(2));
    assert(id!=kInvalidId);simulation.notifyTopologyChanged();
    Vec2 start=world.cells[world.findCreature(id)->rootCell].position;
    unsigned children=0,changed=0,structural=0,maxGeneration=0;float maxDistance=0;double food=0,thrust=0;
    std::set<unsigned> seen;std::ofstream events(stem+"-births.csv");events<<"seconds,id,parent,generation,kind,cells,changed\n";
    for(unsigned step=0;step<120*seconds;++step) {
        simulation.step();
        if(step%120==0)assert(world.allValuesFinite() && world.allConnectionsValid());
        if(auto parent=world.findCreature(id)) {
            if(parent->rootCell<world.cells.size())maxDistance=std::max(maxDistance,length(world.cells[parent->rootCell].position-start));
            for(auto i:world.cellIndicesForCreature(id)){food=std::max(food,world.cells[i].acquiredEnergy);thrust+=length(world.cells[i].motorThrust)*config.fixedTimeStep;}
        }
        for(auto const& c:world.creatures)if(c.ancestorId==id && c.generation && c.mature && !c.fragment && seen.insert(c.id).second) {
            ++children;changed+=c.genome!=specimen.genome;maxGeneration=std::max(maxGeneration,c.generation);
            structural+=measureDevelopment(c.genome).cells!=measureDevelopment(specimen.genome).cells;
            events<<step/120.0<<','<<c.id<<','<<c.parentId<<','<<c.generation<<','<<unsigned(c.birthMutation)<<','<<c.bodyNodes.size()<<','<<(c.genome!=specimen.genome)<<'\n';
            if(children<=4){std::ofstream dna(stem+"-child-"+std::to_string(c.id)+".dna");genomeio::writeGenome(dna,c.genome);}
        }
    }
    unsigned alive=0;for(auto const& c:world.creatures)alive+=c.ancestorId==id && c.mature && !c.fragment;
    std::cout<<"scenario="<<(emptyStart?"player-empty":"mixed")<<" seed="<<seed<<" seconds="<<seconds<<" authored_cells=5 descendants="<<children<<" different_from_ancestor="<<changed
        <<" different_cell_count="<<structural<<" generation="<<maxGeneration<<" living_family="<<alive
        <<" ancestor_max_displacement="<<maxDistance<<" ancestor_max_cell_food="<<food<<" ancestor_integrated_thrust="<<thrust
        <<" capacity_wait_steps="<<simulation.stats().capacityWaitSteps<<" starvation_losses="<<simulation.stats().starvationLosses<<" damage_losses="<<simulation.stats().damageLosses<<" invalid_development="<<simulation.stats().invalidDevelopmentAttempts
        <<" world_births="<<simulation.stats().births<<" world_deaths="<<simulation.stats().deaths<<'\n';
}
