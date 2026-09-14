#pragma once

#include <cstdint>
#include <limits>
#include <vector>

#include "alienmobile/Genome.h"
#include "alienmobile/Development.h"
#include "alienmobile/Math.h"

namespace alienmobile {

inline constexpr uint32_t kInvalidId = std::numeric_limits<uint32_t>::max();

enum class CellViability { Healthy, Starving, Dead };

struct Cell {
    uint32_t id = kInvalidId;
    uint32_t creatureId = kInvalidId;
    Vec2 position;
    Vec2 velocity;
    double energy = 0.0;
    bool constructorCell = false;
    bool alive = true;
    float visualAge = 0.0f;
    float starvationTimer = 0;
    bool deathRequested = false;
    CellViability viability = CellViability::Healthy;
    uint32_t genomeNode = 0;
    BehaviorGene behavior;
    Signals currentSignals{}, nextSignals{};
    Signals memoryState{};
    std::vector<Signals> signalHistory; // Allocated only by memory cells.
    uint16_t historyCursor = 0;
    double signalAge = 0.0;
    Vec2 thrust;
    Vec2 motorThrust; // Paid propulsion only; excludes environmental flow.
    float absorptionFlash = 0;
    double rawEnergy = 0;
    double embodiedEnergy = 0;
    double acquiredEnergy = 0, extractedEnergy = 0, convertedEnergy = 0; // Observation only.
    float attackFlash = 0;
    float digestionFlash = 0;
    Vec2 transferOrigin;

};

struct Connection {
    // Indices into World::cells; remapped together on death compaction.
    uint32_t cellA = kInvalidId;
    uint32_t cellB = kInvalidId;
    float restLength = 0.0f;
    float stiffness = 0.0f;
    float baseRestLength = 0.0f;
};

struct AngularConstraint {
    uint32_t cellA = kInvalidId, center = kInvalidId, cellB = kInvalidId;
    float targetAngle = 0, baseAngle = 0, stiffness = 1;
};

struct ConstructorState {
    enum Status { Idle, Constructing, Cooldown };
    enum Wait { None, Energy, Capacity, Development };

    Status status = Idle;
    Wait wait = None; // Observation only; never consulted by biology.
    uint32_t nextNode = 0;
    uint32_t offspringCreatureId = kInvalidId;
    float timer = 0.0f;
};

struct Creature {
    uint32_t id = kInvalidId;
    uint32_t generation = 0;
    uint32_t parentId = kInvalidId, parentLineageId = kInvalidId;
    uint32_t ancestorId = kInvalidId; // Observation only; stable through branch divergence.
    MutationKind birthMutation = MutationKind::None;
    bool birthMetaMutation = false;
    bool mature = false;
    uint32_t rootCell = kInvalidId;
    ConstructorState constructor;
    Genome genome;
    std::optional<GenomeMutationResult> proposedOffspring; // Retained while technical capacity is unavailable.
    std::optional<DevelopmentSummary> proposedOffspringSummary; // Cached with the immutable proposal.
    DevelopmentCursor development;
    std::optional<DevelopedNode> pendingNode;
    std::vector<GenomeNode> bodyNodes; // Realized topology, not inherited DNA.
    uint32_t expectedCells = 0;
    bool developmentFailed = false;
    bool developingFounder = false;
    bool fragment = false;
    float fragmentAge = 0;
    uint32_t lineageId = kInvalidId;
    float lineageHue = 0.54f;
    float lineageDistance = 0;
    float starvationTimer = 0.0f;
    float visualAge = 0.0f;
    float birthAngle = 0.0f;
};

inline bool operator==(Cell const& left, Cell const& right)
{
    return left.id == right.id && left.creatureId == right.creatureId && left.position == right.position
        && left.velocity == right.velocity && left.energy == right.energy && left.constructorCell == right.constructorCell
        && left.alive == right.alive && left.visualAge == right.visualAge
        && left.starvationTimer==right.starvationTimer && left.deathRequested==right.deathRequested && left.viability==right.viability
        && left.genomeNode == right.genomeNode && left.behavior == right.behavior
        && left.currentSignals == right.currentSignals && left.nextSignals == right.nextSignals
        && left.memoryState==right.memoryState && left.signalHistory==right.signalHistory && left.historyCursor==right.historyCursor
        && left.signalAge == right.signalAge && left.thrust == right.thrust
        && left.motorThrust==right.motorThrust && left.absorptionFlash==right.absorptionFlash
        && left.rawEnergy==right.rawEnergy && left.embodiedEnergy==right.embodiedEnergy
        && left.attackFlash==right.attackFlash && left.digestionFlash==right.digestionFlash
        && left.transferOrigin==right.transferOrigin && left.acquiredEnergy==right.acquiredEnergy
        && left.extractedEnergy==right.extractedEnergy && left.convertedEnergy==right.convertedEnergy;
}

inline bool operator==(Connection const& left, Connection const& right)
{
    return left.cellA == right.cellA && left.cellB == right.cellB && left.restLength == right.restLength
        && left.stiffness == right.stiffness && left.baseRestLength==right.baseRestLength;
}

inline bool operator==(ConstructorState const& left, ConstructorState const& right)
{
    return left.status == right.status && left.wait == right.wait && left.nextNode == right.nextNode && left.offspringCreatureId == right.offspringCreatureId
        && left.timer == right.timer;
}

inline bool operator==(Creature const& left, Creature const& right)
{
    return left.id == right.id && left.generation == right.generation && left.mature == right.mature && left.rootCell == right.rootCell
        && left.constructor == right.constructor && left.proposedOffspring==right.proposedOffspring
        && left.parentId==right.parentId && left.parentLineageId==right.parentLineageId && left.genome == right.genome && left.lineageId == right.lineageId
        && left.fragment==right.fragment && left.fragmentAge==right.fragmentAge && left.lineageDistance==right.lineageDistance && left.lineageHue == right.lineageHue && left.starvationTimer == right.starvationTimer && left.visualAge == right.visualAge && left.birthAngle == right.birthAngle;
}

} // namespace alienmobile
