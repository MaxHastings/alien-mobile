#pragma once

#include <optional>
#include "alienmobile/Genome.h"

namespace alienmobile {

struct DevelopmentLimits {
    uint32_t maxDepth = 8;
    uint32_t maxCells = 128;
};

enum class DevelopmentStatus { Growing, Complete, InvalidGenome, DepthLimit, CellLimit };

struct DevelopedNode {
    GenomeNode physical;
    uint32_t gene = 0, node = 0, depth = 0, branch = 0, repetition = 0;
    float intervalScale = 1;
};

// A resumable interpreter. It stores invocation frames, never a flattened
// genome. The caller materializes at most one returned cell per paid event.
class DevelopmentCursor {
public:
    explicit DevelopmentCursor(DevelopmentLimits limits = {}) : _limits(limits) {}
    std::optional<DevelopedNode> next(Genome const& genome);
    DevelopmentStatus status() const { return _status; }
    uint32_t emitted() const { return _emitted; }
private:
    struct Frame {
        uint32_t gene = 0, depth = 0, node = 0, branch = 0, repetition = 0;
        uint32_t branches = 1, repetitions = 1;
        int origin = -1, anchor = -1;
        float angle = 0, branchAngle = 0, repetitionAngle = 0, intervalScale = 1;
        std::vector<int> physicalNodes;
    };
    DevelopmentLimits _limits;
    DevelopmentStatus _status = DevelopmentStatus::Growing;
    bool _started = false;
    uint32_t _emitted = 0;
    std::vector<Frame> _stack;
};

bool isValidDevelopmentGenome(Genome const& genome);
struct DevelopmentSummary {
    uint32_t cells = 0;
    DevelopmentStatus status = DevelopmentStatus::InvalidGenome;
    bool complete() const { return status == DevelopmentStatus::Complete; }
};
DevelopmentSummary measureDevelopment(Genome const& genome, DevelopmentLimits limits = {});
std::vector<Genome> makeDevelopmentFounders();
GenomeMutationResult mutateDevelopmentGenome(Genome const&, DeterministicRng&, float exposureMultiplier = 1.f);
GenomeMutationResult applyDevelopmentMutation(Genome const&, DeterministicRng&, MutationKind);
bool validMutationRates(MutationRates const&);

} // namespace alienmobile
