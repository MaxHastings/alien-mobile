// Developer-only observer. It never selects offspring or changes fitness.
#include "alienmobile/Simulation.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
using namespace alienmobile;
struct History { unsigned parent=0; double first=0,last=0; unsigned samples=0,maxGeneration=0,maxCells=0; };
int main(int argc,char** argv) {
    if(argc<5) {std::cerr<<"usage: EvolutionResearch seed seconds stem [authored|primitive] [default|static|moving|scarce|heterogeneous|heterogeneous-phone] [cell-cap] [sample-seconds]\n";return 2;}
    double duration=std::stod(argv[2]);std::string stem=argv[3],founder=argv[4],environment=argc>5?argv[5]:"static";
    auto c=(environment=="default" || environment=="spatial" || environment=="beds" || environment=="planted" || environment=="connected" || environment=="productive") ? evolutionPlaytestConfig() : depthPlaytestConfig();
    c.randomSeed=std::strtoull(argv[1],nullptr,10);
    if(argc>6)c.maxCellCount=std::stoul(argv[6]);
    c.catalogSeed=founder=="catalog";
    c.primitiveSeed=founder=="primitive" || founder=="origin";
    c.mixedSeed=founder=="mixed";
    c.gardenSeed=founder=="authored" || founder=="mixed" || founder=="garden" || founder=="garden-static" || founder=="garden-passive" || founder=="garden-blind";
    c.frozenFounderMutation=founder=="garden-static" || founder=="garden-passive" || founder=="garden-blind";
    c.minimalOrigin=founder=="origin";
    if(founder!="catalog" && founder!="authored" && founder!="primitive" && founder!="garden" && founder!="origin" && founder!="garden-static" && founder!="garden-passive" && founder!="garden-blind" && founder!="mixed") {std::cerr<<"unsupported founder\n";return 2;}
    if(environment=="heterogeneous" || environment=="heterogeneous-phone") {
        c.heterogeneousEnvironment=true;c.activeDevelopment=true;c.moteLifetime=30;
        c.offspringAnchorDistance=2*c.cellRadius+.12f;
        c.resourceCycleSeconds=environment=="heterogeneous" ? 120:180;
        c.emissionRate=environment=="heterogeneous" ? 4:12;
        c.energySourceRadius=environment=="heterogeneous" ? 6:10;
    }
    else if(environment=="spatial" || environment=="beds" || environment=="planted" || environment=="connected" || environment=="productive") {c.spatialResources=true;c.heterogeneousBeds=environment!="spatial";c.plantedFounders=environment=="planted" || environment=="connected" || environment=="productive";if(environment=="productive")c.emissionRate=8;if(environment=="connected" || environment=="productive"){c.worldMinX=c.worldMinY=-16;c.worldMaxX=c.worldMaxY=16;}}
    else if(environment=="scarce") {c.emissionRate=8;c.energySourceRadius=8;}
    else if(environment!="static" && environment!="moving" && environment!="default")return 2;
    World w(c);
    if(founder=="garden-passive" || founder=="garden-blind") {
        auto ablate=[&](BehaviorGene& b) {
            if(founder=="garden-passive" && b.role==CellRole::Motor)b.motorStrength=0;
            if(founder=="garden-blind" && b.role==CellRole::EnergySensor)b.role=CellRole::Structural;
        };
        for(auto& owner:w.creatures){for(auto& gene:owner.genome.genes)for(auto& node:gene.nodes)ablate(node.behavior);for(auto& node:owner.bodyNodes)ablate(node.behavior);}
        for(auto& cell:w.cells)ablate(cell.behavior);
    }
    Simulation sim(w,c);
    std::ofstream population(stem+"-population.csv"),phenotypes(stem+"-phenotypes.csv"),body(stem+"-bodies.csv"),lineages(stem+"-lineages.csv");
    if(!population||!phenotypes||!body||!lineages)return 2;
    population<<"seconds,mature,cells,births,deaths,cell_deaths,generation,lineages,motes,absorbed,attacked,digested,energy_error,cap,mature_generation,mutated_births,structural_births,meta_births\n";
    phenotypes<<"seconds,id,lineage,generation,cells,genome_nodes,modules,branches,width,height,speed,heading,motors,sensors,storage,attackers,neural_rate,geometry_rate,property_rate,role_rate,insert_rate,erase_rate,duplication_rate,deletion_rate,copy_rate,move_rate,constructor_rate,meta_rate,neural_magnitude,geometry_magnitude,property_magnitude,acquired_energy,extracted_energy,converted_energy,age,parent,parent_lineage,defenders,memory,senders,receivers,paid_thrust\n";
    body<<"seconds,id,lineage,generation,cell,parent,x,y,role,vx,vy,hue,mature\n";
    std::ofstream regions(stem+"-regions.csv");
    regions<<"seconds,region,resource_energy,harvested,mature,cells,mean_body,lineages,motors,sensors\n";
    std::map<unsigned,History> history;
    auto start=std::chrono::steady_clock::now();
    unsigned peak=0;double maxError=0,maxRelativeError=0;
    auto sample=[&](double t) {
        if(!w.allValuesFinite()||!w.allConnectionsValid())throw std::runtime_error("invalid world");
        for(auto const& owner:w.creatures)if(!owner.fragment)for(auto index:w.cellIndicesForCreature(owner.id)) {
            auto const& cell=w.cells[index];
            body<<t<<','<<owner.id<<','<<owner.lineageId<<','<<owner.generation<<','<<cell.genomeNode<<','<<owner.bodyNodes[cell.genomeNode].parentNode<<','<<cell.position.x<<','<<cell.position.y<<','<<unsigned(cell.behavior.role)<<','<<cell.velocity.x<<','<<cell.velocity.y<<','<<owner.lineageHue<<','<<owner.mature<<'\n';
        }
        for(unsigned region=0;region<4;++region) {
            double stock=0,harvested=0;unsigned count=0,cells=0,motors=0,sensors=0;std::set<unsigned> families;
            for(auto const& mote:w.motes)if(mote.resourceBed==int(region)){stock+=mote.energy;harvested+=mote.harvested;}
            for(auto const& owner:w.creatures)if(owner.mature && !owner.fragment) {
                auto indices=w.cellIndicesForCreature(owner.id);if(indices.empty())continue;
                auto pos=w.cells[indices[0]].position;unsigned nearest=0;float distance=1e9;
                for(unsigned b=0;b<4;++b){float d=length(w.displacement(pos,w.resourceBedPosition(b)));if(d<distance){distance=d;nearest=b;}}
                if(nearest!=region)continue;
                ++count;cells+=indices.size();families.insert(owner.lineageId);
                for(auto i:indices){motors+=w.cells[i].behavior.role==CellRole::Motor;sensors+=w.cells[i].behavior.role==CellRole::EnergySensor;}
            }
            regions<<t<<','<<region<<','<<stock<<','<<harvested<<','<<count<<','<<cells<<','<<(count ? double(cells)/count:0)<<','<<families.size()<<','<<motors<<','<<sensors<<'\n';
        }
        regions.flush();
        std::set<unsigned> extant;
        for(auto const& owner:w.creatures)if(owner.mature && !owner.fragment) {
            auto indices=w.cellIndicesForCreature(owner.id);if(indices.empty())continue;
            auto& h=history[owner.lineageId];if(!h.samples){h.first=t;h.parent=owner.parentLineageId;}h.last=t;++h.samples;
            h.maxGeneration=std::max(h.maxGeneration,owner.generation);h.maxCells=std::max(h.maxCells,unsigned(indices.size()));extant.insert(owner.lineageId);
            unsigned nodes=0,branches=0;for(auto const& gene:owner.genome.genes)nodes+=gene.nodes.size();
            std::vector<unsigned> degree(owner.bodyNodes.size());
            for(auto i:indices){auto p=owner.bodyNodes[w.cells[i].genomeNode].parentNode;if(p>=0)++degree[p];}
            for(auto d:degree)branches+=d>1;
            auto origin=w.cells[indices[0]].position;Vec2 lo{},hi{},velocity{};
            std::array<unsigned,unsigned(CellRole::Count)> roles{};float capacity=0;double acquired=0,extracted=0,converted=0,paidThrust=0;
            for(auto i:indices) {
                auto const& cell=w.cells[i];auto p=w.displacement(origin,cell.position);
                lo.x=std::min(lo.x,p.x);lo.y=std::min(lo.y,p.y);hi.x=std::max(hi.x,p.x);hi.y=std::max(hi.y,p.y);
                velocity+=cell.velocity;roles[unsigned(cell.behavior.role)]++;capacity+=w.cellCapacity(cell);acquired+=cell.acquiredEnergy;extracted+=cell.extractedEnergy;converted+=cell.convertedEnergy;paidThrust+=length(cell.motorThrust);

            }
            velocity=velocity/float(indices.size());auto axis=cellAxis(w,indices[0]);
            auto const& r=owner.genome.mutationRates;
            phenotypes<<t<<','<<owner.id<<','<<owner.lineageId<<','<<owner.generation<<','<<indices.size()<<','<<nodes<<','<<owner.genome.genes.size()<<','<<branches<<','<<hi.x-lo.x<<','<<hi.y-lo.y<<','<<length(velocity)<<','<<std::atan2(axis.y,axis.x)<<','<<roles[unsigned(CellRole::Motor)]<<','<<roles[unsigned(CellRole::EnergySensor)]+roles[unsigned(CellRole::CreatureSensor)]<<','<<capacity<<','<<roles[unsigned(CellRole::Attacker)]<<','<<r.neural<<','<<r.geometry<<','<<r.property<<','<<r.role<<','<<r.insert<<','<<r.erase<<','<<r.duplicateGene<<','<<r.deleteGene<<','<<r.copySection<<','<<r.moveSection<<','<<r.constructor<<','<<r.meta<<','<<r.neuralMagnitude<<','<<r.geometryMagnitude<<','<<r.propertyMagnitude<<','<<acquired<<','<<extracted<<','<<converted<<','<<owner.visualAge<<','<<owner.parentId<<','<<owner.parentLineageId<<','<<roles[unsigned(CellRole::Defender)]<<','<<roles[unsigned(CellRole::Memory)]<<','<<roles[unsigned(CellRole::Sender)]<<','<<roles[unsigned(CellRole::Receiver)]<<','<<paidThrust<<'\n';
        }
        double retained=w.emissionAccumulator;for(auto const& cell:w.cells)retained+=cell.energy+cell.rawEnergy+cell.embodiedEnergy;for(auto const& mote:w.motes)retained+=mote.energy;
        auto const& l=w.energyLedger;double error=l.emitted+l.seeded+l.organismSeeded-retained-l.expired-l.dissipated-l.organCost-l.digestionLoss;
        maxError=std::max(maxError,std::abs(error));maxRelativeError=std::max(maxRelativeError,std::abs(error)/std::max(1.0,l.emitted+l.seeded+l.organismSeeded));peak=std::max(peak,unsigned(w.cells.size()));
        auto const& s=sim.stats();population<<t<<','<<w.matureCreatureCount()<<','<<w.cells.size()<<','<<s.births<<','<<s.deaths<<','<<s.cellDeaths<<','<<s.maximumGeneration<<','<<extant.size()<<','<<w.motes.size()<<','<<l.absorbed<<','<<l.attacked<<','<<l.digested<<','<<error<<','<<s.populationCapReached<<','<<s.maximumMatureGeneration<<','<<s.births-s.completedMutationBirths[0]-s.completedMutationBirths[unsigned(MutationKind::Meta)]<<','
            <<s.completedMutationBirths[unsigned(MutationKind::InsertNode)]+s.completedMutationBirths[unsigned(MutationKind::DeleteNode)]+s.completedMutationBirths[unsigned(MutationKind::DuplicateGene)]+s.completedMutationBirths[unsigned(MutationKind::DeleteGene)]+s.completedMutationBirths[unsigned(MutationKind::CopySection)]+s.completedMutationBirths[unsigned(MutationKind::MoveSection)]<<','<<s.completedMetaBirths<<'\n';
        population.flush();phenotypes.flush();body.flush();
    };
    sample(0);
    for(uint64_t step=1;step<=uint64_t(duration/c.fixedTimeStep);++step) {
        double t=step*double(c.fixedTimeStep);
        if(environment=="moving")w.energySource.position=c.energySourcePosition+Vec2{float(5*std::sin(t/90)),float(4*std::sin(t/143))};
        sim.step();if(step%(argc>7 ? std::stoull(argv[7])*120 : 1200)==0)sample(t);
        if(w.creatures.empty())break;
    }
    sample(sim.stats().steps*double(c.fixedTimeStep));
    lineages<<"lineage,parent_lineage,first_mature_observation,last_mature_observation,observations,max_generation,max_cells\n";
    unsigned persistent=0;for(auto const& [id,h]:history){lineages<<id<<','<<h.parent<<','<<h.first<<','<<h.last<<','<<h.samples<<','<<h.maxGeneration<<','<<h.maxCells<<'\n';persistent+=h.last-h.first>=120;}
    std::ofstream dna(stem+"-candidate-genomes.txt");
    std::set<unsigned> sizes;unsigned candidates=0;
    for(auto const& owner:w.creatures)if(owner.mature && !owner.fragment && owner.generation>=2) {
        auto count=w.cellIndicesForCreature(owner.id).size();
        if(count<4 || !sizes.insert(unsigned(count)).second)continue;
        dna<<"creature "<<owner.id<<" generation "<<owner.generation<<" lineage "<<owner.lineageId<<" physical_cells "<<count<<'\n';
        for(unsigned gi=0;gi<owner.genome.genes.size();++gi) {
            auto const& gene=owner.genome.genes[gi];dna<<"module "<<gi<<" orientation "<<gene.orientation<<" phase "<<gene.phaseAdvance<<'\n';
            for(unsigned ni=0;ni<gene.nodes.size();++ni) {
                auto const& n=gene.nodes[ni];auto const& b=n.behavior;
                dna<<"node "<<ni<<" parent "<<n.parentNode<<" edge "<<n.relativePosition.x<<','<<n.relativePosition.y<<" role "<<unsigned(b.role)<<" mode "<<unsigned(b.motorMode)<<" strength "<<b.motorStrength<<" channel "<<b.motorChannel<<" neural "<<b.neural<<" call "<<n.construction.targetGene<<" repeats "<<n.construction.repetitions<<" branches "<<n.construction.branches<<'\n';
                dna<<"biases ";for(float v:b.biases)dna<<v<<',';dna<<"\nweights ";for(auto const& row:b.weights)for(float v:row)dna<<v<<',';dna<<'\n';
            }
        }
        if(++candidates==4)break;
    }
    uint64_t hash=1469598103934665603ULL;
    auto addHash=[&](auto const& value){auto p=reinterpret_cast<unsigned char const*>(&value);for(unsigned i=0;i<sizeof(value);++i){hash^=p[i];hash*=1099511628211ULL;}};
    addHash(w.rng.state());
    for(auto const& cell:w.cells){addHash(cell.id);addHash(cell.creatureId);addHash(cell.position);addHash(cell.velocity);addHash(cell.energy);addHash(cell.rawEnergy);addHash(cell.currentSignals);}
    double elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
    std::cout<<"seed="<<c.randomSeed<<" simulated_seconds="<<sim.stats().steps*c.fixedTimeStep<<" wall_seconds="<<elapsed<<" peak_cells="<<peak<<" births="<<sim.stats().births<<" generation="<<sim.stats().maximumGeneration<<" mature_generation="<<sim.stats().maximumMatureGeneration<<" mature="<<w.matureCreatureCount()<<" observed_lineages="<<history.size()<<" persistent_120s="<<persistent<<" maximum_energy_error="<<maxError<<" outcome_hash="<<hash<<" relative_error="<<maxRelativeError<<'\n';
    // Double reservoirs keep cumulative conservation error below 0.1 energy
    // units even in long runs. Neither error metric changes physical energy.
    return maxError>.1 ? 1:0;
}
