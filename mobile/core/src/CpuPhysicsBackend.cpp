#include "alienmobile/CpuPhysicsBackend.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include "alienmobile/SpatialGrid.h"
#include "alienmobile/ConnectionIndex.h"

namespace alienmobile {

CpuPhysicsBackend::CpuPhysicsBackend(SimulationConfig config)
    : _config(config)
{}

void CpuPhysicsBackend::initialize(World& world)
{
    (void)world;
}

void CpuPhysicsBackend::syncToCpu(World& world)
{
    (void)world;
}

void CpuPhysicsBackend::topologyChanged(World& world)
{
    (void)world;
}

void CpuPhysicsBackend::step(World& world, float dt)
{
    if (!std::isfinite(dt) || dt <= 0.0f) {
        return;
    }
    applyConnectionForces(world, dt);
    applyRepulsionForces(world, dt);
    integrate(world, dt);

#ifndef NDEBUG
    assert(world.allConnectionsValid());
    assert(world.allValuesFinite());
    for (auto const& cell : world.cells) {
        assert(cell.energy >= 0.0f);
    }
#endif
}

void CpuPhysicsBackend::applyConnectionForces(World& world, float dt)
{
    // Evaluate damping from the same input snapshot as the Metal kernel.
    std::vector<Vec2> inputVelocities;
    inputVelocities.reserve(world.cells.size());
    for (auto const& cell : world.cells) inputVelocities.push_back(cell.velocity);
    for (auto const& connection : world.connections) {
        if (connection.cellA >= world.cells.size() || connection.cellB >= world.cells.size()) {
            continue;
        }
        auto& cellA = world.cells[connection.cellA];
        auto& cellB = world.cells[connection.cellB];
        if (!cellA.alive || !cellB.alive) {
            continue;
        }

        auto const displacement = world.displacement(cellA.position,cellB.position);
        auto const distance = length(displacement);
        auto const direction = normalizedOr(displacement);
        auto const relativeVelocity = inputVelocities[connection.cellB] - inputVelocities[connection.cellA];
        auto const velocityAlongConnection = dot(relativeVelocity, direction);
        auto const springForce = connection.stiffness * (distance - connection.restLength)
            + _config.springDamping * velocityAlongConnection;
        auto const force = direction * springForce;
        cellA.velocity += force * dt;
        cellB.velocity -= force * dt;
    }
    for(auto const& joint:world.angles) {
        auto a=joint.cellA,c=joint.center,b=joint.cellB;
        auto u=world.displacement(world.cells[c].position,world.cells[a].position);
        auto v=world.displacement(world.cells[c].position,world.cells[b].position);
        float uu=lengthSquared(u),vv=lengthSquared(v);
        if(uu<0.04f || vv<0.04f) continue;
        Vec2 pu{-u.y,u.x},pv{-v.y,v.x};
        float theta=std::atan2(u.x*v.y-u.y*v.x,dot(u,v));
        float error=std::atan2(std::sin(theta-joint.targetAngle),std::cos(theta-joint.targetAngle));
        float omega=dot(pv,inputVelocities[b]-inputVelocities[c])/vv-dot(pu,inputVelocities[a]-inputVelocities[c])/uu;
        float torque=clamp(joint.stiffness*error+0.35f*omega,-8.f,8.f);
        Vec2 fa=pu*(torque/uu),fb=pv*(-torque/vv);
        world.cells[a].velocity+=fa*dt;
        world.cells[b].velocity+=fb*dt;
        world.cells[c].velocity-=(fa+fb)*dt;
    }
}

void CpuPhysicsBackend::applyRepulsionForces(World& world, float dt)
{
    SpatialGrid grid(world.cells,_config,_config.repulsionDistance);
    ConnectionIndex connections(world.cells.size(),world.connections);
    std::vector<unsigned> contacts;
    std::vector<uint32_t> connectedTo(world.cells.size(),kInvalidId);
    for (uint32_t cellAIndex = 0; cellAIndex < world.cells.size(); ++cellAIndex) {
        auto& cellA = world.cells[cellAIndex];
        if (!cellA.alive) {
            continue;
        }
        for(auto slot=connections.first(cellAIndex);slot!=kInvalidId;slot=connections.next(slot)) {
            auto const& edge=world.connections[slot/2];
            connectedTo[edge.cellA==cellAIndex ? edge.cellB : edge.cellA]=cellAIndex;
        }
        grid.query(cellA.position,_config.repulsionDistance,contacts);
        for (auto cellBIndex:contacts) {
            if(cellBIndex<=cellAIndex)continue;
            auto& cellB = world.cells[cellBIndex];
            if (!cellB.alive) {
                continue;
            }

            auto const displacement = world.displacement(cellA.position,cellB.position);
            auto const distance = length(displacement);
            if (distance >= _config.repulsionDistance || connectedTo[cellBIndex]==cellAIndex) {
                continue;
            }
            auto const direction = normalizedOr(displacement);
            auto const forceMagnitude = _config.repulsionStrength * (_config.repulsionDistance - distance);
            auto const force = direction * forceMagnitude;
            cellA.velocity -= force * dt;
            cellB.velocity += force * dt;
        }
    }
}

void CpuPhysicsBackend::integrate(World& world, float dt)
{
    auto const drag = clamp(1.0f - _config.linearDrag * dt, 0.0f, 1.0f);
    for (auto& cell : world.cells) {
        if (!cell.alive) {
            continue;
        }
        cell.velocity += cell.thrust * dt;
        cell.velocity *= drag;
        cell.position += cell.velocity * dt;

        if(_config.toroidal) {cell.position=world.wrapped(cell.position);continue;}
        auto const minX = _config.worldMinX + _config.cellRadius;
        auto const maxX = _config.worldMaxX - _config.cellRadius;
        auto const minY = _config.worldMinY + _config.cellRadius;
        auto const maxY = _config.worldMaxY - _config.cellRadius;
        if (cell.position.x < minX) {
            cell.position.x = minX;
            if (cell.velocity.x < 0.0f) {
                cell.velocity.x *= -_config.boundaryBounce;
            }
        } else if (cell.position.x > maxX) {
            cell.position.x = maxX;
            if (cell.velocity.x > 0.0f) {
                cell.velocity.x *= -_config.boundaryBounce;
            }
        }
        if (cell.position.y < minY) {
            cell.position.y = minY;
            if (cell.velocity.y < 0.0f) {
                cell.velocity.y *= -_config.boundaryBounce;
            }
        } else if (cell.position.y > maxY) {
            cell.position.y = maxY;
            if (cell.velocity.y > 0.0f) {
                cell.velocity.y *= -_config.boundaryBounce;
            }
        }
    }
}

} // namespace alienmobile
