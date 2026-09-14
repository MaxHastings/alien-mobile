#pragma once
#include "alienmobile/SimulationConfig.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>
namespace alienmobile {
// Compact counting grid. Query results keep original index order, preserving
// tie-breaking and floating-point accumulation across brute-force/grid paths.
class SpatialGrid {
    SimulationConfig c;
    int nx,ny;
    float sx,sy;
    std::vector<unsigned> offsets,indices;
    int coordinate(float v,float lo,float step,int count) const {
        int i=int(std::floor((v-lo)/step));
        return c.toroidal ? (i%count+count)%count : std::clamp(i,0,count-1);
    }
    unsigned bin(Vec2 p) const {return coordinate(p.y,c.worldMinY,sy,ny)*nx+coordinate(p.x,c.worldMinX,sx,nx);}
public:
    template<class T> SpatialGrid(std::vector<T> const& items,SimulationConfig config,float size=2.f):c(config) {
        nx=std::max(1,int((c.worldMaxX-c.worldMinX)/size));ny=std::max(1,int((c.worldMaxY-c.worldMinY)/size));
        sx=(c.worldMaxX-c.worldMinX)/nx;sy=(c.worldMaxY-c.worldMinY)/ny;
        offsets.assign(nx*ny+1,0);indices.resize(items.size());
        for(auto const& item:items)++offsets[bin(item.position)+1];
        std::partial_sum(offsets.begin(),offsets.end(),offsets.begin());auto next=offsets;
        for(unsigned i=0;i<items.size();++i)indices[next[bin(items[i].position)]++]=i;
    }
    // Caller-owned scratch avoids allocating three vectors for every receptor,
    // food particle and collision query. Results retain stable index order.
    void query(Vec2 p,float radius,std::vector<unsigned>& result) const {
        struct Axis { int first,count; };
        auto axis=[&](float v,float lo,float step,int count) {
            int first=int(std::floor((v-radius-lo)/step)),last=int(std::floor((v+radius-lo)/step));
            if(!c.toroidal){first=std::clamp(first,0,count-1);last=std::clamp(last,0,count-1);}
            if(last-first+1>=count) return Axis{0,count};
            return Axis{first,std::max(0,last-first+1)};
        };
        auto xs=axis(p.x,c.worldMinX,sx,nx),ys=axis(p.y,c.worldMinY,sy,ny);
        result.clear();
        for(int yi=0;yi<ys.count;++yi) for(int xi=0;xi<xs.count;++xi) {
            int x=xs.first+xi,y=ys.first+yi;
            if(c.toroidal){x=(x%nx+nx)%nx;y=(y%ny+ny)%ny;}
            unsigned b=y*nx+x;
            result.insert(result.end(),indices.begin()+offsets[b],indices.begin()+offsets[b+1]);
        }
        std::sort(result.begin(),result.end());
    }
    std::vector<unsigned> query(Vec2 p,float radius) const {
        std::vector<unsigned> result;
        query(p,radius,result);
        return result;
    }
};
}
