#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <unordered_map>

#include "alienmobile/Genome.h"
#include "alienmobile/SimulationConfig.h"
#include "alienmobile/Types.h"

namespace alienmobile {

struct EnergySource {
    Vec2 position;
    float radius = 0.0f;
    float strength = 0.0f;
};

struct HazardSource {
    Vec2 position;
    float radius = 0.0f;
    float strength = 0.0f;
};

struct EnergyMote {
    uint64_t id;
    Vec2 position, velocity;
    double energy; float age;
    int resourceBed = -1; // Anchored renewable substrate; -1 is a free particle.
    double harvested = 0;
};
struct ResourcePatch {
    Vec2 anchor;
    float phase = 0;
    float emissionAccumulator = 0;
};
// A drag creates a short lived force field, sampled independently by every
// physical particle and every cell. It has no creature-level state.
struct PlayerCurrent {
    Vec2 position;
    Vec2 direction;
    float age = 0;
    float lifetime = 0;
    float radius = 0;
    float strength = 0;
};
struct EnergyLedger {
    double emitted=0, absorbed=0, recycled=0, expired=0, dissipated=0;
    double attacked=0, digested=0, digestionLoss=0, organCost=0;
    double seeded=0, organismSeeded=0, pendingEmission=0;
};

class World {
public:
    explicit World(SimulationConfig config = {});

    void reset();
    void reset(SimulationConfig config);
    void setSimulationConfig(SimulationConfig config);

    // IDs are monotonic between resets. Structural references use cell vector
    // indices; object identity exposed to the simulation uses these IDs.
    uint32_t addCell(uint32_t creatureId, Vec2 position, Vec2 velocity, float energy, bool constructorCell);
    uint32_t addCreature(uint32_t generation, bool mature, Genome genome = {});
    void addConnection(uint32_t cellA, uint32_t cellB, float restLength, float stiffness);
    void braceGenomeNode(uint32_t creatureId, uint32_t nodeIndex);
    bool removeCellsAndFragment(std::vector<uint32_t> const& cellIds);
    bool removeCreatureAndCells(uint32_t creatureId);

    Creature* findCreature(uint32_t creatureId);
    Creature const* findCreature(uint32_t creatureId) const;
    uint32_t findCellForGenomeNode(uint32_t creatureId, uint32_t nodeIndex) const;
    std::vector<uint32_t> cellIndicesForCreature(uint32_t creatureId) const;
    bool hasConnection(uint32_t cellA, uint32_t cellB) const;
    bool allConnectionsValid() const;
    bool allValuesFinite() const;
    std::size_t matureCreatureCount() const;
    std::size_t aliveCellCount() const;
    std::size_t lineageCount() const;

    Vec2 displacement(Vec2 from, Vec2 to) const;
    Vec2 wrapped(Vec2 position) const;
    SimulationConfig const& config() const { return _config; }
    bool organsActive(Creature const& owner) const {
        return !owner.fragment && !owner.developmentFailed && (owner.mature || _config.activeDevelopment);
    }
    float cellCapacity(Cell const& cell) const {
        return _config.maxCellEnergy + (cell.behavior.role==CellRole::Depot ? cell.behavior.storageCapacity : 0.f);
    }
    // One visible, non-stacking field. Exposure is sampled at the constructor
    // when offspring DNA is first drawn; already committed DNA is unchanged.
    struct MutagenField { Vec2 position; float remaining=0; float radius=4; };
    MutagenField mutagen;
    bool applyMutagen(Vec2 position);
    float mutationExposure(Vec2 position) const;
    bool scatterFood(Vec2 position);
    void addMote(Vec2 position, Vec2 velocity, double energy);
    Vec2 resourceBedPosition(unsigned bed) const;
    Vec2 resourcePatchPosition(unsigned patch) const;
    void addPlayerCurrent(Vec2 position, Vec2 direction, float strength = -1.f);
    void addDevelopingFounder(Genome genome, Vec2 position, float angle, float hue);
    void addFounder(Genome genome, Vec2 position, float angle, float hue);
    uint32_t addSpecimen(SpecimenSnapshot const& specimen, Vec2 position, float angle = 0);
    uint32_t addSpecimenNearby(SpecimenSnapshot const& specimen, Vec2 requested);
    std::vector<EnergyMote> motes;
    std::vector<ResourcePatch> resourcePatches;
    std::vector<PlayerCurrent> playerCurrents;
    EnergyLedger energyLedger;
    uint64_t nextMoteId=0;
    double emissionAccumulator=0;
    double ecologicalTime=0;
    std::vector<Cell> cells;
    std::vector<Connection> connections;
    std::vector<AngularConstraint> angles;
    std::vector<Creature> creatures;
    Genome genome;
    EnergySource energySource;
    HazardSource hazardSource;
    uint32_t nextCellId = 0;
    uint32_t nextCreatureId = 0;
    uint32_t nextLineageId = 1;
    DeterministicRng rng;

private:
    // Public vectors can be compacted/reordered by topology operations. Cache
    // indices (never pointers), validate hits, and rebuild after such edits.
    mutable std::unordered_map<uint32_t,std::size_t> _creatureIndices;
    void rebuildCreatureIndices() const;
    SimulationConfig _config;
    void createInitialOrganism();
};

} // namespace alienmobile
