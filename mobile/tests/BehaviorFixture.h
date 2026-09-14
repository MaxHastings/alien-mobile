#pragma once
#include "alienmobile/Simulation.h"
using namespace alienmobile;
World specimen(Genome g, SimulationConfig config = {}) {
    World w(config); w.cells.clear(); w.connections.clear();w.angles.clear(); w.creatures.clear();
    auto ci=w.addCreature(0,true,g); auto id=w.creatures[ci].id;
    for (uint32_t n=0;n<g.genes[0].nodes.size();++n) {
        auto const& node=g.genes[0].nodes[n]; Vec2 p{};
        if(n) p=w.cells[node.parentNode].position+node.relativePosition;
        auto i=w.addCell(id,p,{},1,node.constructorCell);
        if(!n) w.creatures[ci].rootCell=i;
        else { w.addConnection(node.parentNode,i,length(node.relativePosition),config.springStiffness); w.braceGenomeNode(id,n); }
    }
    return w;
}
Vec2 center(World const& w) { Vec2 c{};for(auto const& cell:w.cells)c+=cell.position;return c/static_cast<float>(w.cells.size()); }
