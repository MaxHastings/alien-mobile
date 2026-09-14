#pragma once
#include "alienmobile/World.h"
namespace alienmobile {
Vec2 environmentCurrent(World const& world,Vec2 position);
void updateResources(World& world,float dt);
void updateTrophicOrgans(World& world,float dt);
}
