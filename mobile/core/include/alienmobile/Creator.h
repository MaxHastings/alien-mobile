#pragma once
#include "alienmobile/Development.h"
#include <algorithm>
namespace alienmobile {
// An edit copy expands bounded development into ordinary genes. The original
// specimen remains exact. Controllers and physical geometry are retained.
inline Genome editableBody(Genome const& source) {
    Genome result; result.mutationRates=source.mutationRates;
    DevelopmentCursor cursor;
    while(auto n=cursor.next(source)) {
        auto node=n->physical;bool separate=node.construction.separateOffspring;node.construction={};node.construction.separateOffspring=separate;
        result.genes[0].nodes.push_back(node);
    }
    if(cursor.status()!=DevelopmentStatus::Complete || result.genes[0].nodes.size()>64) return {};
    return result;
}
inline bool validCreatorBody(Genome const& g) {
    return g.genes.size()==1 && g.entryGene==0 && isValidDevelopmentGenome(g) && measureDevelopment(g).complete();
}
inline BehaviorGene seededOrgan(CellRole role) {
    BehaviorGene b;b.role=role;
    if(role==CellRole::Motor || role==CellRole::Attacker) {
        b.neural=true;b.motorChannel=Activation;b.biases[Activation]=.65f;
        b.weights[Activation][EnergyIntensity]=.3f;
    }
    return b;
}
inline bool changeOrgan(Genome& g,size_t index,CellRole role) {
    if(!validCreatorBody(g))return false;
    auto copy=g;auto& nodes=copy.genes[0].nodes;
    if(index==0 || index>=nodes.size() || nodes[index].constructorCell)return false;
    nodes[index].behavior=seededOrgan(role);
    if(!validCreatorBody(copy))return false;g=copy;return true;
}
inline bool addBodyCell(Genome& g,size_t parent,Vec2 edge) {
    if(!validCreatorBody(g) || parent>=g.genes[0].nodes.size() || g.genes[0].nodes.size()>=32)return false;
    auto copy=g;copy.genes[0].nodes.emplace_back(int(parent),edge,false);
    if(!validCreatorBody(copy))return false;g=copy;return true;
}
// Move a branch through its attachment edge. Descendants keep their local
// geometry and all controller data; only the authored physical edge changes.
inline bool moveBodyCell(Genome& g,size_t index,Vec2 edge) {
    if(!validCreatorBody(g) || index==0 || index>=g.genes[0].nodes.size() || !isFinite(edge))return false;
    auto copy=g;copy.genes[0].nodes[index].relativePosition=edge;
    if(!validCreatorBody(copy))return false;
    std::vector<Vec2> positions;
    for(auto const& n:copy.genes[0].nodes)
        positions.push_back(n.parentNode<0 ? Vec2{} : positions[n.parentNode]+n.relativePosition);
    // A touch edit must not fold cells onto one another. This constraint is
    // editor-only; inherited mutation is still allowed to make bad anatomy.
    std::vector<bool> moved(positions.size());moved[index]=true;
    for(size_t i=index+1;i<moved.size();++i)moved[i]=moved[copy.genes[0].nodes[i].parentNode];
    for(size_t i=0;i<positions.size();++i)for(size_t j=0;j<i;++j)
        if(moved[i]!=moved[j] && length(positions[i]-positions[j])<.65f)return false;
    g=std::move(copy);return true;
}
inline bool removeBodyCell(Genome& g,size_t index) {
    if(!validCreatorBody(g))return false;
    auto copy=g;auto& nodes=copy.genes[0].nodes;
    if(index==0 || index>=nodes.size() || nodes.size()<=2 || nodes[index].constructorCell)return false;
    for(auto const& n:nodes)if(n.parentNode==int(index))return false;
    nodes.erase(nodes.begin()+index);
    for(auto& n:nodes)if(n.parentNode>int(index))--n.parentNode;
    if(!validCreatorBody(copy))return false;g=copy;return true;
}
}
