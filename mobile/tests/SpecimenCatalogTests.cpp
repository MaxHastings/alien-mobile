#include "alienmobile/Simulation.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <set>

using namespace alienmobile;

int main()
{
    auto catalog=makeCuratedSpecimenCatalog();
    assert(catalog.size()==4);
    std::set<std::string> names;
    std::set<float> hues;
    std::set<std::size_t> silhouettes;
    for(auto const& specimen:catalog) {
        assert(names.insert(specimen.name).second && hues.insert(specimen.lineageHue).second);
        assert(!specimen.ecology.empty());
        assert(specimen.initialEnergy>0 && isValidDevelopmentGenome(specimen.genome));
        silhouettes.insert(measureDevelopment(specimen.genome).cells);
    }
    // Four different developed cell counts, plus distinct role layouts below,
    // keep this from quietly regressing into four variants of one chassis.
    assert(silhouettes.size()>=4);
    auto hasRole=[](SpecimenSnapshot const& specimen,CellRole role) {
        for(auto const& gene:specimen.genome.genes)for(auto const& node:gene.nodes)
            if(node.behavior.role==role)return true;
        return false;
    };
    auto hasMotorMode=[](SpecimenSnapshot const& specimen,MotorMode mode) {
        for(auto const& gene:specimen.genome.genes)for(auto const& node:gene.nodes)
            if(node.behavior.role==CellRole::Motor && node.behavior.motorMode==mode)return true;
        return false;
    };
    // The catalog is a tour of real systems, not four recolored feeders.
    assert(hasRole(catalog[0],CellRole::EnergySensor) && hasMotorMode(catalog[0],MotorMode::Thrust));
    assert(measureDevelopment(catalog[1].genome).cells==9 && hasMotorMode(catalog[1],MotorMode::Thrust));
    assert(catalog[2].genome.genes.size()==2 && measureDevelopment(catalog[2].genome).cells==7);
    assert(hasRole(catalog[3],CellRole::Depot));


    auto c=evolutionPlaytestConfig();
    World world(c);Simulation simulation(world,c);
    // The normal opening has a light cast; no automatic catalog replenishment
    // occurs after reset.  Catalog provenance is absent from Creature.
    assert(world.creatures.size()==4);
    for(auto const& creature:world.creatures) {
        assert(creature.genome==catalog[3].genome || creature.genome==catalog[0].genome || creature.genome==catalog[1].genome || creature.genome==catalog[2].genome);
        assert(creature.lineageHue==catalog[3].lineageHue || creature.lineageHue==catalog[0].lineageHue || creature.lineageHue==catalog[1].lineageHue || creature.lineageHue==catalog[2].lineageHue);
    }

    // Rejected input must leave topology and IDs untouched.
    auto originalCells=world.cells.size(),originalCreatures=world.creatures.size();
    auto originalId=world.nextCreatureId;
    auto invalid=catalog[0];invalid.genome.genes.clear();
    assert(world.addSpecimen(invalid,{0,0})==kInvalidId);
    assert(world.addSpecimen(catalog[1],{c.worldMaxX,0})==kInvalidId);
    assert(world.addSpecimen(catalog[0],world.cells.front().position)==kInvalidId);
    auto limited=c;limited.maxCellCount=world.cells.size()+1;world.setSimulationConfig(limited);
    assert(world.addSpecimen(catalog[1],{8,0})==kInvalidId);
    assert(world.cells.size()==originalCells && world.creatures.size()==originalCreatures && world.nextCreatureId==originalId);
    world.setSimulationConfig(c);
    auto initialMotes=world.motes.size();double initialFood=world.energyLedger.seeded;
    assert(world.scatterFood({0,0}));
    assert(world.motes.size()==initialMotes+48);
    assert(std::abs(world.energyLedger.seeded-initialFood-48*c.moteEnergy)<.0001);
    assert(!world.scatterFood({c.worldMaxX,0}));
    auto before=world.creatures.size();
    double organismEnergy=world.energyLedger.organismSeeded;
    auto id=world.addSpecimen(catalog[1],{8,0},.3f);
    assert(id!=kInvalidId && world.creatures.size()==before+1);
    assert(world.energyLedger.organismSeeded>organismEnergy);
    auto const* placed=world.findCreature(id);
    assert(placed && placed->genome==catalog[1].genome && placed->lineageHue==catalog[1].lineageHue);
    for(auto cell:world.cellIndicesForCreature(id))
        assert(world.cells[cell].energy<=catalog[1].initialEnergy+.0001f);
    // It is now indistinguishable from every normal founder in the simulation.
    simulation.notifyTopologyChanged();
    for(unsigned step=0;step<600;++step) simulation.step();
    assert(world.allValuesFinite() && world.allConnectionsValid());
    double retained=world.emissionAccumulator;
    for(auto const& cell:world.cells)retained+=cell.energy+cell.rawEnergy+cell.embodiedEnergy;
    for(auto const& mote:world.motes)retained+=mote.energy;
    auto const& ledger=world.energyLedger;
    double error=ledger.emitted+ledger.seeded+ledger.organismSeeded-retained
        -ledger.expired-ledger.dissipated-ledger.organCost-ledger.digestionLoss;
    assert(std::abs(error)<.001);
    World reservedWorld(c);
    auto reservedConfig=c;reservedConfig.maxCellCount=reservedWorld.cells.size()+8;
    reservedWorld.setSimulationConfig(reservedConfig);
    reservedWorld.addCreature(1,false,catalog[1].genome);
    auto reservedId=reservedWorld.nextCreatureId;
    assert(reservedWorld.addSpecimen(catalog[0],{8,0})==kInvalidId);
    assert(reservedWorld.nextCreatureId==reservedId);
    std::cout<<"four discovered data-only specimens; ordinary placement and light four-organism opening passed\n";
}
