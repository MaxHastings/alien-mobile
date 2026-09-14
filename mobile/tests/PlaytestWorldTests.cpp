#include "alienmobile/Simulation.h"
#include <cassert>
#include <iostream>
using namespace alienmobile;
struct Outcome { double absorbed, dissipated; uint64_t deaths; size_t cells; };
static Outcome intervention(int kind) {
    auto c=evolutionPlaytestConfig();c.randomSeed=42;
    World w(c);Simulation s(w,c);
    for(int i=0;i<7200;++i)s.step();
    auto absorbed=w.energyLedger.absorbed,dissipated=w.energyLedger.dissipated;
    auto deaths=s.stats().cellDeaths;
    auto position=w.cells.front().position;
    if(kind==1)w.addPlayerCurrent(position,{1,0});
    if(kind==2)w.addPlayerCurrent(position,{0,1});
    // A current is a field input only; it never teleports life.
    assert(w.cells.front().position.x==position.x && w.cells.front().position.y==position.y);
    for(int i=0;i<7200;++i)s.step();
    assert(w.allValuesFinite() && w.allConnectionsValid());
    return {w.energyLedger.absorbed-absorbed,w.energyLedger.dissipated-dissipated,
            s.stats().cellDeaths-deaths,w.cells.size()};
}
int main() {
    auto control=intervention(0),resource=intervention(1),hazard=intervention(2);
    std::cout<<"control absorbed="<<control.absorbed<<" deaths="<<control.deaths<<" cells="<<control.cells
             <<"; east current absorbed="<<resource.absorbed<<" deaths="<<resource.deaths<<" cells="<<resource.cells
             <<"; north current absorbed="<<hazard.absorbed<<" deaths="<<hazard.deaths<<" cells="<<hazard.cells<<'\n';
    // Directional fields are physical interventions, not a scripted outcome.
    (void)hazard;
    auto c=evolutionPlaytestConfig();World w(c);Simulation s(w,c);
    for(unsigned seed=1;seed<=20;++seed) {
        s.resetWithSeed(seed);
        assert(s.stats().steps==0 && s.stats().births==0 && w.creatures.size()==4);
        assert(w.allValuesFinite() && w.allConnectionsValid());
        auto catalog=makeCuratedSpecimenCatalog();
        assert(w.creatures[0].genome==catalog[3].genome && w.creatures[1].genome==catalog[0].genome);
        bool hasDepot=false;
        for(auto const& owner:w.creatures) {
            assert(owner.mature && owner.lineageId==owner.id+1);
            for(auto const& node:owner.genome.genes[0].nodes) {
                hasDepot|=node.behavior.role==CellRole::Depot;
            }
        }
        assert(hasDepot);
        for(int i=0;i<600;++i)s.step();
        assert(w.allValuesFinite() && w.allConnectionsValid());
    }
    std::cout<<"20 catalog resets and paired current interventions passed\n";
}
