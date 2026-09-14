#pragma once
#include "alienmobile/Types.h"

namespace alienmobile {
// Two slots per edge, with each cell's incident edges in original vector order.
// Built for one pass so public topology edits cannot leave a stale index.
class ConnectionIndex {
public:
    ConnectionIndex(std::size_t cellCount,std::vector<Connection> const& edges)
        : _first(cellCount,kInvalidId),_next(edges.size()*2,kInvalidId) {
        for(std::size_t n=edges.size();n>0;--n) {
            auto const& edge=edges[n-1];
            if(edge.cellA>=cellCount || edge.cellB>=cellCount) continue;
            auto a=uint32_t(2*(n-1)),b=a+1;
            _next[b]=_first[edge.cellB];_first[edge.cellB]=b;
            _next[a]=_first[edge.cellA];_first[edge.cellA]=a;
        }
    }
    uint32_t first(std::size_t cell) const {return _first[cell];}
    uint32_t next(uint32_t slot) const {return _next[slot];}
private:
    std::vector<uint32_t> _first,_next;
};
}
