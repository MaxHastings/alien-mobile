#include "BehaviorFixture.h"
#include "alienmobile/Ecology.h"
#include <cassert>
#include <iostream>
#include <set>
using namespace alienmobile;

void validate(World const& w) {
    assert(w.allValuesFinite()&&w.allConnectionsValid());
    std::set<uint32_t> ids;
    for(auto const& cell:w.cells) {
        assert(ids.insert(cell.id).second);assert(w.findCreature(cell.creatureId));
        assert(cell.energy>=0 && cell.rawEnergy>=0 && cell.embodiedEnergy>=0);
    }
    std::set<uint64_t> motes;
    for(auto const& m:w.motes) assert(motes.insert(m.id).second);
    assert(w.cells.size()<=w.config().maxCellCount && w.motes.size()<=w.config().maxMotes);
}
int main() {
    std::cout<<std::unitbuf;
    auto c=ecosystemPlaytestConfig();c.ecosystemSeed=false;c.emissionRate=0;c.hazardStrength=0;
    World w(c);w.cells.clear();w.creatures.clear();w.connections.clear();w.angles.clear();
    assert(w.displacement({23.8f,0},{-23.8f,0}).x>0 && w.displacement({23.8f,0},{-23.8f,0}).x<0.41f);
    w.addMote({23.99f,0},{1,0},1);updateResources(w,0.1f);assert(w.motes[0].position.x< -23.8f);
    auto copy=w;updateResources(w,0.1f);updateResources(copy,0.1f);
    assert(w.motes[0].position==copy.motes[0].position);
    w.motes.clear();auto emitting=c;emitting.emissionRate=2;w.setSimulationConfig(emitting);
    for(int i=0;i<1200;++i)updateResources(w,1.f/120);
    double sum=w.emissionAccumulator;for(auto const& m:w.motes)sum+=m.energy;
    assert(std::abs(sum-w.energyLedger.emitted)<1e-4 && std::abs(sum-20)<1e-3);
    w.setSimulationConfig(c);w.motes.clear();w.energyLedger={};w.emissionAccumulator=0;
    auto ci=w.addCreature(0,true,makeFeederGenome());auto id=w.creatures[ci].id;
    auto first=w.addCell(id,{}, {},0,true);w.creatures[ci].rootCell=first;
    w.addMote({2,0},{},0.6f);updateResources(w,0.01f);assert(w.cells[0].energy==0);
    w.motes[0].position={};w.addCell(id,{}, {},0,false);updateResources(w,0.01f);
    assert(std::abs(w.cells[0].energy-0.3f)<1e-6f&&std::abs(w.cells[1].energy-0.3f)<1e-6f);
    assert(w.motes.empty());
    double recycled=(w.cells[0].energy+w.cells[0].embodiedEnergy+w.cells[1].energy+w.cells[1].embodiedEnergy)*c.recycleFraction;
    w.removeCreatureAndCells(id);double returned=0;for(auto const& m:w.motes)returned+=m.energy;
    assert(std::abs(returned-recycled)<1e-6);

    auto feeder=specimen(makeFeederGenome(),c);feeder.motes.clear();
    feeder.addMote(feeder.cells[1].position+Vec2{1,0},{},1);
    assert(energySensor(feeder,1)[EnergyX]>0);
    feeder.energySource.position={-10,10};assert(energySensor(feeder,1)[EnergyX]>0);
    feeder.motes.clear();assert(energySensor(feeder,1)[EnergyIntensity]==0);

    auto hybrid=specimen(makeContractileFeederGenome(),c);auto baseline=hybrid.connections.back().baseRestLength;
    hybrid.cells[5].currentSignals[Oscillator]=-1;
    float spent=updateActuation(hybrid,0.1f,0.035f);
    assert(spent>0 && hybrid.connections.back().restLength<baseline);
    hybrid.cells[5].currentSignals[Oscillator]=0;updateActuation(hybrid,0.1f,0.035f);
    assert(hybrid.connections.back().restLength==baseline);
    hybrid.cells[5].energy=0;hybrid.cells[5].currentSignals[Oscillator]=1;
    updateActuation(hybrid,0.1f,0.035f);assert(hybrid.connections.back().restLength==baseline);

    auto hunter=specimen(makeHunterGenome(),c);hunter.motes.clear();
    auto foreign=hunter.addCreature(0,true,makeFeederGenome());auto foreignId=hunter.creatures[foreign].id;
    auto victim=hunter.addCell(foreignId,hunter.cells[4].position+Vec2{0.75f,0},{},1,true);
    hunter.creatures[foreign].rootCell=victim;
    auto out=creatureSensor(hunter,1);assert(out[CreatureX]>0 && out[CreatureIntensity]>0);
    double before=hunter.cells[victim].energy;
    updateTrophicOrgans(hunter,0.1f);assert(hunter.cells[victim].energy==before);
    hunter.cells[4].currentSignals[Activation]=1;
    hunter.cells[5].behavior.role=CellRole::Structural;
    auto usable=hunter.cells[4].energy;updateTrophicOrgans(hunter,0.1f);
    assert(hunter.cells[victim].energy<before && hunter.energyLedger.attacked>0);
    assert(hunter.cells[4].energy<usable);assert(hunter.energyLedger.digested==0);
    assert(hunter.cells[4].rawEnergy>0);
    hunter.cells[5].behavior.role=CellRole::Digestor;updateTrophicOrgans(hunter,0.1f);
    assert(hunter.energyLedger.digested>0);
    // Removing digestion leaves captured raw resource unavailable to motors.
    hunter.cells[5].behavior.role=CellRole::Structural;hunter.cells[4].currentSignals[Activation]=1;
    auto digested=hunter.energyLedger.digested;updateTrophicOrgans(hunter,0.1f);
    assert(hunter.energyLedger.digested==digested);
    hunter.cells[victim].position={15,15};before=hunter.energyLedger.attacked;
    updateTrophicOrgans(hunter,0.1f);assert(hunter.energyLedger.attacked==before);
    assert(creatureSensor(hunter,1)[CreatureIntensity]==0);
    std::cout<<"finite emission, contact competition, recycling, mote sensing, wrapping, contractile energy, attack/digestion gates passed\n";

    // Three integrated ten-minute runs. Extinction is permitted, never repaired.
    for(int seed=0;seed<3;++seed) {
        auto config=ecosystemPlaytestConfig();config.randomSeed+=seed;
        World world(config);Simulation sim(world,config);std::size_t peakCells=0,peakMotes=0;std::set<uint32_t> bornHunters;uint32_t hunterGeneration=0;
        for(int n=0;n<120*600;++n) {
            if(seed==1 && n==120*120) world.energySource.position={8,4};
            if(seed==2 && n==120*180) world.hazardSource.position=world.energySource.position;
            sim.step();peakCells=std::max(peakCells,world.cells.size());peakMotes=std::max(peakMotes,world.motes.size());
            if(n%120==0) {
                validate(world);
                for(auto const& creature:world.creatures) if(creature.mature && creature.generation>0)
                    for(auto const& node:creature.genome.genes[0].nodes) if(node.behavior.role==CellRole::Attacker) {
                        bornHunters.insert(creature.id);hunterGeneration=std::max(hunterGeneration,creature.generation);break;
                    }
            }
        }
        std::size_t huntingBodies=0;
        for(auto const& creature:world.creatures)for(auto const& node:creature.genome.genes[0].nodes)
            if(node.behavior.role==CellRole::Attacker){++huntingBodies;break;}
        auto const& e=world.energyLedger;double environment=world.emissionAccumulator;
        for(auto const& mote:world.motes)environment+=mote.energy;
        assert(std::abs(environment-(e.seeded+e.emitted+e.recycled-e.absorbed-e.expired))<0.1);
        double living=0;for(auto const& cell:world.cells)living+=cell.energy+cell.rawEnergy+cell.embodiedEnergy;
        double accounted=environment+living+e.expired+e.dissipated+e.organCost+e.digestionLoss;
        double supplied=e.seeded+e.organismSeeded+e.emitted;
        assert(std::abs(accounted-supplied)<0.15);
        std::cout<<"energy accounting residual="<<accounted-supplied<<'\n';
        std::cout<<"ecosystem seed="<<seed<<" seconds=600 cells="<<world.cells.size()<<" population="<<world.creatures.size()
            <<" hunters="<<huntingBodies<<" hunterBirths="<<bornHunters.size()<<" hunterGeneration="<<hunterGeneration<<" births="<<sim.stats().births<<" deaths="<<sim.stats().deaths
            <<" generation="<<sim.stats().maximumGeneration<<" attacksEnergy="<<e.attacked<<" digested="<<e.digested
            <<" peakCells="<<peakCells<<" peakMotes="<<peakMotes<<" biologyMs="<<sim.stats().biologyMs/sim.stats().steps<<'\n';
        sim.reset();validate(world);assert(world.creatures.size()==5 && world.motes.size()==160);
        assert(world.energyLedger.emitted==0 && world.energyLedger.attacked==0);
    }
}
