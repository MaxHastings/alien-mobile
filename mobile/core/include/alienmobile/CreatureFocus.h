#pragma once
#include "alienmobile/World.h"
#include <optional>

namespace alienmobile {
// Player observation state only. Never consulted by biology or physics.
class CreatureFocus {
public:
    uint32_t id=kInvalidId,ancestor=kInvalidId,generation=0;
    std::optional<SpecimenSnapshot> specimen;
    std::optional<LifeEventKind> lastLoss;
    // Keep evidence for the selection after the bounded world event queue rolls over.
    // This is observation state only, sampled after simulation steps.
    void observe(World const& world) {
        if(!specimen)return;
        for(auto it=world.lifeEvents.rbegin();it!=world.lifeEvents.rend();++it)
            if(it->creature==id && it->kind!=LifeEventKind::InvalidDevelopment){lastLoss=it->kind;break;}
    }
    std::string lossMessage() const {
        if(!lastLoss)return "No living body remains in this observation";
        return *lastLoss==LifeEventKind::StarvationLoss
            ? "Body gone · last cell loss: usable energy ran out"
            : "Body gone · last cell loss: physical attack";
    }
    void clear(){id=ancestor=kInvalidId;generation=0;specimen.reset();lastLoss.reset();}
    bool select(World const& world,uint32_t candidate,std::string name) {
        auto c=world.findCreature(candidate);if(!c || c->fragment)return false;
        id=candidate;ancestor=c->ancestorId;generation=c->generation;lastLoss.reset();
        specimen=SpecimenSnapshot{std::move(name),c->genome,c->lineageHue,world.config().initialCellEnergy,
            "Discovered · generation "+std::to_string(generation)};
        observe(world);return true;
    }
    uint32_t nextRelative(World const& world) const {
        if(!specimen)return kInvalidId;
        uint32_t child=kInvalidId,next=kInvalidId,first=kInvalidId;
        for(auto const& c:world.creatures) {
            if(c.id==id || c.ancestorId!=ancestor || !c.mature || c.fragment)continue;
            if(c.parentId==id)child=std::min(child,c.id);
            if(c.id>id)next=std::min(next,c.id);
            first=std::min(first,c.id);
        }
        return child!=kInvalidId ? child : next!=kInvalidId ? next : first;
    }
    unsigned livingFamily(World const& world) const {
        unsigned count=0;for(auto const& c:world.creatures)count+=c.ancestorId==ancestor && c.mature && !c.fragment;
        return count;
    }
};
}
