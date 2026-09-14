#include "alienmobile/Simulation.h"
#include <cassert>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <set>
#include <iomanip>
using namespace alienmobile;
int main(int argc,char** argv) {
    auto c=depthPlaytestConfig();if(argc>2){c.energySourceRadius=std::atof(argv[1]);c.emissionRate=std::atof(argv[2]);}
    if(argc>3)c.randomSeed=std::strtoull(argv[3],nullptr,10);
    int seconds=argc>4 ? std::atoi(argv[4]) : 180;
    World w(c);Simulation sim(w,c);
    std::cout<<std::unitbuf;
    unsigned largest=0;
    for(int n=0;n<120*seconds;++n) {
        sim.step();
        if(n%1200==0) {
            unsigned current=0;
            for(auto const& owner:w.creatures) current=std::max(current,unsigned(w.cellIndicesForCreature(owner.id).size()));
            largest=std::max(largest,current);
            std::cout<<"seconds="<<n/120<<" organisms="<<w.creatures.size()<<" mature="<<w.matureCreatureCount()
                <<" cells="<<w.cells.size()<<" largest="<<current<<" motes="<<w.motes.size()<<" births="<<sim.stats().births<<'\n';
            std::array<unsigned,static_cast<unsigned>(CellRole::Count)> organs{},offspringOrgans{};
            unsigned matureMax=0;float spread=0;
            for(auto const& owner:w.creatures)if(owner.mature && !owner.fragment) {
                auto indices=w.cellIndicesForCreature(owner.id);matureMax=std::max(matureMax,unsigned(indices.size()));
                for(auto i:indices) {++organs[unsigned(w.cells[i].behavior.role)];if(owner.generation)++offspringOrgans[unsigned(w.cells[i].behavior.role)];
                    spread=std::max(spread,length(w.displacement(w.energySource.position,w.cells[i].position)));}
            }
            std::cout<<"matureMax="<<matureMax<<" spread="<<spread<<" liveRoles=";
            for(auto value:organs)std::cout<<value<<',';
            std::cout<<" offspringRoles=";for(auto value:offspringOrgans)std::cout<<value<<',';
            std::cout<<" cellDeaths="<<sim.stats().cellDeaths<<" fragments="<<sim.stats().fragmentsCreated<<'\n';
            assert(w.allValuesFinite() && w.allConnectionsValid());
        }
    }
    double retained=w.emissionAccumulator;
    for(auto const& cell:w.cells)retained+=cell.energy+cell.rawEnergy+cell.embodiedEnergy;
    for(auto const& mote:w.motes)retained+=mote.energy;
    auto const& ledger=w.energyLedger;
    double supplied=ledger.emitted+ledger.seeded+ledger.organismSeeded;
    double accounted=retained+ledger.expired+ledger.dissipated+ledger.organCost+ledger.digestionLoss;
    std::cout<<"energy supplied="<<supplied<<" accounted="<<accounted<<" error="<<supplied-accounted<<'\n';
    assert(std::abs(supplied-accounted)<.1);
    if(argc>5) {
        std::vector<Creature const*> candidates;
        for(auto const& owner:w.creatures)if(owner.mature && owner.generation)candidates.push_back(&owner);
        std::sort(candidates.begin(),candidates.end(),[&](auto a,auto b){return w.cellIndicesForCreature(a->id).size()>w.cellIndicesForCreature(b->id).size();});
        std::set<std::size_t> sizes;int exported=0;
        for(auto owner:candidates) {
            auto indices=w.cellIndicesForCreature(owner->id);if(!sizes.insert(indices.size()).second)continue;
            auto stem=std::string(argv[5])+"-"+std::to_string(exported++);
            std::ofstream dna(stem+".txt"),body(stem+".csv");
            dna<<"seed="<<c.randomSeed<<" seconds="<<seconds<<" creature="<<owner->id<<" generation="<<owner->generation<<" lineage="<<owner->lineageId<<" physicalCells="<<indices.size()<<"\n";
            for(unsigned gi=0;gi<owner->genome.genes.size();++gi) {
                auto const& gene=owner->genome.genes[gi];dna<<"gene "<<gi<<" orientation "<<gene.orientation<<" phaseAdvance "<<gene.phaseAdvance<<'\n';
                for(unsigned n=0;n<gene.nodes.size();++n) {auto const& node=gene.nodes[n];auto const& b=node.behavior;auto const& d=node.construction;
                    dna<<"node "<<n<<" parent "<<node.parentNode<<" edge "<<node.relativePosition.x<<','<<node.relativePosition.y<<" role "<<unsigned(b.role)<<" stiffness "<<node.stiffness
                        <<" target "<<d.targetGene<<" branches "<<d.branches<<" repeats "<<d.repetitions<<" angle "<<d.angle<<" branchAngle "<<d.branchAngle<<" period "<<b.period
                        <<" motor "<<b.motorStrength<<" mode "<<unsigned(b.motorMode)<<" storage "<<b.storageCapacity<<" defense "<<b.defenseStrength<<" memory "<<b.memoryTime<<'\n';
                }
            }
            body<<"cell,x,y,parent,role\n";
            for(unsigned j=0;j<indices.size();++j){auto const& cell=w.cells[indices[j]];int parent=-1;
                auto parentNode=owner->bodyNodes[cell.genomeNode].parentNode;
                for(unsigned k=0;k<indices.size();++k)if(int(w.cells[indices[k]].genomeNode)==parentNode)parent=int(k);
                body<<j<<','<<cell.position.x<<','<<cell.position.y<<','<<parent<<','<<unsigned(cell.behavior.role)<<'\n';}
            if(exported==4)break;
        }
        std::ofstream state(std::string(argv[5])+"-state.txt");state<<std::setprecision(17);
        for(auto const& cell:w.cells)state<<cell.id<<' '<<cell.creatureId<<' '<<cell.position.x<<' '<<cell.position.y<<' '<<cell.energy<<' '<<cell.rawEnergy<<'\n';
    }
    std::cout<<"largest observed="<<largest<<'\n';
}
