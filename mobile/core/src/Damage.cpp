#include "alienmobile/World.h"
#include <algorithm>
#include <unordered_set>

namespace alienmobile {
bool World::removeCellsAndFragment(std::vector<uint32_t> const& cellIds) {
    std::unordered_set<uint32_t> dead(cellIds.begin(),cellIds.end());
    if(dead.empty())return false;
    std::vector<uint32_t> map(cells.size(),kInvalidId);
    std::vector<Cell> remaining;
    for(std::size_t i=0;i<cells.size();++i) {
        auto const& cell=cells[i];
        if(dead.count(cell.id)) {
            double total=cell.energy+cell.rawEnergy+cell.embodiedEnergy;
            double returned=total*_config.recycleFraction;
            if(_config.physicalResources) {
                addMote(cell.position,cell.velocity*.3f,returned);
                energyLedger.recycled+=returned;energyLedger.dissipated+=total-returned;
            }
        } else {map[i]=uint32_t(remaining.size());remaining.push_back(cell);}
    }
    if(remaining.size()==cells.size())return false;
    std::vector<Connection> edges;
    for(auto edge:connections) {
        if(edge.cellA>=map.size() || edge.cellB>=map.size())continue;
        edge.cellA=map[edge.cellA];edge.cellB=map[edge.cellB];
        if(edge.cellA!=kInvalidId && edge.cellB!=kInvalidId)edges.push_back(edge);
    }
    std::vector<AngularConstraint> joints;
    for(auto joint:angles) {
        if(joint.cellA>=map.size() || joint.center>=map.size() || joint.cellB>=map.size())continue;
        joint.cellA=map[joint.cellA];joint.center=map[joint.center];joint.cellB=map[joint.cellB];
        if(joint.cellA!=kInvalidId && joint.center!=kInvalidId && joint.cellB!=kInvalidId)joints.push_back(joint);
    }
    cells=std::move(remaining);connections=std::move(edges);angles=std::move(joints);
    std::vector<std::vector<uint32_t>> neighbors(cells.size());
    for(auto const& edge:connections)if(cells[edge.cellA].creatureId==cells[edge.cellB].creatureId) {
        neighbors[edge.cellA].push_back(edge.cellB);neighbors[edge.cellB].push_back(edge.cellA);
    }
    std::vector<bool> seen(cells.size());std::vector<Creature> owners;std::vector<uint32_t> orphaned;
    for(auto const& old:creatures) {
        auto root=old.rootCell<map.size() ? map[old.rootCell] : kInvalidId;
        bool keptIdentity=false,rootSurvived=root!=kInvalidId;
        if(!rootSurvived && old.constructor.offspringCreatureId!=kInvalidId && old.constructor.offspringCreatureId!=old.id)
            orphaned.push_back(old.constructor.offspringCreatureId);
        auto starts=cellIndicesForCreature(old.id);
        // A reserved embryo may legitimately have no physical root yet.
        if(starts.empty() && old.rootCell==kInvalidId && !old.fragment) {
            owners.push_back(old);continue;
        }
        if(rootSurvived) {
            auto it=std::find(starts.begin(),starts.end(),root);
            if(it!=starts.end())std::iter_swap(starts.begin(),it);
        }
        for(auto start:starts) {
            if(seen[start])continue;
            std::vector<uint32_t> component{start};seen[start]=true;
            for(std::size_t at=0;at<component.size();++at)for(auto n:neighbors[component[at]])if(!seen[n]) {
                seen[n]=true;component.push_back(n);
            }
            Creature owner=old;
            owner.id=keptIdentity ? nextCreatureId++ : old.id;keptIdentity=true;
            bool rooted=rootSurvived && std::find(component.begin(),component.end(),root)!=component.end();
            owner.rootCell=rooted ? root : start;
            if(!rooted || old.fragment || old.developmentFailed) {
                owner.fragment=true;owner.mature=false;owner.developingFounder=false;owner.developmentFailed=true;
                owner.constructor={};owner.pendingNode.reset();
                if(!old.fragment)owner.fragmentAge=0;
            }
            for(auto n:component) {
                cells[n].creatureId=owner.id;
                if(owner.fragment){cells[n].viability=CellViability::Dead;cells[n].thrust={};cells[n].currentSignals={};cells[n].nextSignals={};}
            }
            owners.push_back(std::move(owner));
        }
    }
    creatures=std::move(owners);
    for(auto id:orphaned)if(auto* child=findCreature(id)) {
        child->fragment=true;child->mature=false;child->developingFounder=false;child->developmentFailed=true;child->constructor={};
        for(auto n:cellIndicesForCreature(id)){cells[n].viability=CellViability::Dead;cells[n].thrust={};}
    }
    for(auto& owner:creatures)if(owner.constructor.offspringCreatureId!=kInvalidId) {
        auto const* child=findCreature(owner.constructor.offspringCreatureId);
        if(!child || child->fragment)owner.constructor={};
    }
    // Debris is detached from any birth tether. Living retained offspring
    // remain physically connected when their inherited release policy says so.
    connections.erase(std::remove_if(connections.begin(),connections.end(),[&](auto const& e){
        auto a=cells[e.cellA].creatureId,b=cells[e.cellB].creatureId;
        return a!=b && (findCreature(a)->fragment || findCreature(b)->fragment);
    }),connections.end());
    angles.erase(std::remove_if(angles.begin(),angles.end(),[&](auto const& a){
        return !hasConnection(a.cellA,a.center) || !hasConnection(a.cellB,a.center);
    }),angles.end());
    return true;
}
} // namespace alienmobile
