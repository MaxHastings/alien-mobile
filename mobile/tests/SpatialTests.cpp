#include "alienmobile/World.h"
#include "alienmobile/SpatialGrid.h"
#include <cassert>
#include <algorithm>
#include <iostream>
using namespace alienmobile;
int main() {
    for(bool wrapped:{false,true}) {
        auto c=depthPlaytestConfig();c.toroidal=wrapped;World w(c);
        w.cells.clear();DeterministicRng rng(1234);
        for(int i=0;i<1200;++i){Cell cell;cell.position={rng.nextUnit()*90-45,rng.nextUnit()*90-45};if(wrapped)cell.position=w.wrapped(cell.position);w.cells.push_back(cell);}
        SpatialGrid grid(w.cells,c);
        for(float radius:{.44f,.9f,6.f,40.f,100.f})for(int q=0;q<100;++q) {
            Vec2 p{rng.nextUnit()*90-45,rng.nextUnit()*90-45};auto candidates=grid.query(p,radius);
            assert(std::adjacent_find(candidates.begin(),candidates.end())==candidates.end());
            for(unsigned i=0;i<w.cells.size();++i)if(length(w.displacement(p,w.cells[i].position))<=radius)
                assert(std::binary_search(candidates.begin(),candidates.end(),i));
        }
    }
    std::cout<<"1,000 wrapped/bounded grid queries against 1,200-cell exhaustive reference passed\n";
}
