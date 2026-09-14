#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "alienmobile/Math.h"
#include "alienmobile/Behavior.h"

namespace alienmobile {

struct ConstructionGene {
    int targetGene = -1; // -1: no developmental invocation.
    uint16_t branches = 1;
    uint16_t repetitions = 1;
    float angle = 0;
    float branchAngle = 1.04719755f;
    float repetitionAngle = 0;
    float intervalScale = 1;
    // Root controls release of its completed offspring. Internal calls grow
    // attached structures; independent fragment development is a later gate.
    bool separateOffspring = true;
};

struct GenomeNode {
    int parentNode = -1;
    Vec2 relativePosition;
    bool constructorCell = false;
    BehaviorGene behavior;
    ConstructionGene construction;
    float stiffness = 1.0f;
    GenomeNode() = default;
    GenomeNode(int parent, Vec2 relative, bool constructor, BehaviorGene gene = {})
        : parentNode(parent), relativePosition(relative), constructorCell(constructor), behavior(gene)
    {
        if(constructor) behavior.role=CellRole::Constructor;
    }
};

struct Gene {
    std::vector<GenomeNode> nodes;
    float orientation = 0;
    float phaseAdvance = 0;
};

struct MutationRates {
    // Most births are recognizable descendants: small controller, geometry,
    // and sensor/motor-property edits are common. Role and node changes are
    // noticeable but occasional; module-scale developmental changes are rare.
    float neural = 0.34f, geometry = 0.24f, property = 0.14f, role = 0.035f;
    float insert = 0.040f, erase = 0.025f, duplicateGene = 0.008f, deleteGene = 0.004f;
    float copySection = 0.010f, moveSection = 0.006f, constructor = 0.035f;
    float meta = 0.002f;
    float neuralMagnitude = 1, geometryMagnitude = 1, propertyMagnitude = 1;
};

struct Genome {
    std::vector<Gene> genes = {Gene{}};
    uint16_t entryGene = 0;
    MutationRates mutationRates;
};

// Portable starting material for the catalog and a future "My Discoveries"
// shelf.  It is data only: World converts it to an ordinary creature and then
// forgets where it came from.
struct SpecimenSnapshot {
    std::string name;
    Genome genome;
    float lineageHue = .54f;
    float initialEnergy = .52f;
    // Player-facing ecological promise. This remains catalog-only metadata:
    // World converts the genome to an ordinary organism and retains no class,
    // protection, or special behavior flag.
    std::string ecology;
};

class DeterministicRng {
public:
    explicit DeterministicRng(uint64_t seed = 0xA11E'0003'2026'0913ULL)
        : _state(seed == 0 ? 0x9E3779B97F4A7C15ULL : seed)
    {}

    void reset(uint64_t seed) { _state = seed == 0 ? 0x9E3779B97F4A7C15ULL : seed; }
    uint64_t state() const { return _state; }
    uint32_t nextUInt();
    float nextUnit();
    std::size_t nextIndex(std::size_t count);

private:
    uint64_t _state;
};

struct GenomeMutationConfig {
    std::size_t minNodes = 2;
    std::size_t maxNodes = 10;
    float geometryProbability = 0.24f;
    float additionProbability = 0.055f;
    float removalProbability = 0.040f;
    float maxPerturbation = 0.28f;
    float minEdgeLength = 0.45f;
    float maxEdgeLength = 2.10f;
    float oscillatorProbability = 0.08f;
    float motorModeProbability = 0.0f;
    float behaviorProbability = 0.0f; // Per birth, at most one small behavioral edit.
    bool openStructuralMutation = false;
};

enum class MutationKind { None, Geometry, AddTerminal, RemoveTerminal, Oscillator, Behavior, Property, Role, InsertNode, DeleteNode, DuplicateGene, DeleteGene, CopySection, MoveSection, Constructor, Meta };

struct GenomeMutationResult {
    Genome genome;
    MutationKind kind = MutationKind::None;

    bool behaviorMutated = false;
    float geneticDistance = 0;
    bool metaMutated = false;

    bool mutated() const { return kind != MutationKind::None; }
};

// Initial seed genome. Each creature owns an inherited, potentially mutated copy.
Genome makeDefaultGenome();
Genome makePrimitiveGenome();
Genome makeLocomotionGenome();
Genome makeFeederGenome();
Genome makeContractileFeederGenome();
Genome makeHunterGenome();
// Historical human-authored search material and regression fixtures.
// The playable catalog below consists of archived simulation descendants.
std::vector<Genome> makeGardenFounders();
std::vector<SpecimenSnapshot> makeCuratedSpecimenCatalog();

bool hasDefaultGenomeSemantics(Genome const& genome);
bool isValidGenome(Genome const& genome, GenomeMutationConfig const& config = {});
GenomeMutationResult mutateGenome(
    Genome const& parent,
    DeterministicRng& rng,
    GenomeMutationConfig const& config = {});

inline bool operator==(ConstructionGene const& a, ConstructionGene const& b)
{
    return a.targetGene == b.targetGene && a.branches == b.branches
        && a.repetitions == b.repetitions && a.angle == b.angle
        && a.branchAngle == b.branchAngle && a.repetitionAngle == b.repetitionAngle
        && a.intervalScale == b.intervalScale && a.separateOffspring == b.separateOffspring;
}
inline bool operator==(Gene const& a, Gene const& b);
inline bool operator==(GenomeNode const& left, GenomeNode const& right)
{
    return left.parentNode == right.parentNode && left.relativePosition == right.relativePosition
        && left.constructorCell == right.constructorCell && left.behavior == right.behavior && left.construction == right.construction && left.stiffness==right.stiffness;
}

inline bool operator==(Gene const& a, Gene const& b)
{
    return a.nodes == b.nodes && a.orientation == b.orientation && a.phaseAdvance==b.phaseAdvance;
}
inline bool operator==(MutationRates const& a, MutationRates const& b)
{
    return a.neural==b.neural && a.geometry==b.geometry && a.property==b.property && a.role==b.role
        && a.insert==b.insert && a.erase==b.erase && a.duplicateGene==b.duplicateGene
        && a.deleteGene==b.deleteGene && a.copySection==b.copySection && a.moveSection==b.moveSection
        && a.constructor==b.constructor && a.meta==b.meta
        && a.neuralMagnitude==b.neuralMagnitude && a.geometryMagnitude==b.geometryMagnitude && a.propertyMagnitude==b.propertyMagnitude;
}
inline bool operator==(Genome const& left, Genome const& right)
{
    return left.genes == right.genes && left.entryGene == right.entryGene && left.mutationRates==right.mutationRates;
}

inline bool operator!=(Genome const& left, Genome const& right)
{
    return !(left == right);
}

inline bool operator==(GenomeMutationResult const& a,GenomeMutationResult const& b) {
    return a.genome==b.genome && a.kind==b.kind && a.behaviorMutated==b.behaviorMutated
        && a.geneticDistance==b.geneticDistance && a.metaMutated==b.metaMutated;
}
} // namespace alienmobile
