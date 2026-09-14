#include "alienmobile/Genome.h"
#include "alienmobile/Development.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace alienmobile {

uint32_t DeterministicRng::nextUInt()
{
    // xorshift64* is tiny, deterministic, and sufficient for prototype
    // mutation choices. Simulation code never uses platform randomness.
    auto value = _state;
    value ^= value >> 12;
    value ^= value << 25;
    value ^= value >> 27;
    _state = value;
    return static_cast<uint32_t>((value * 0x2545F4914F6CDD1DULL) >> 32);
}

float DeterministicRng::nextUnit()
{
    return static_cast<float>(nextUInt() >> 8) / 16777216.0f;
}

std::size_t DeterministicRng::nextIndex(std::size_t count)
{
    return count == 0 ? 0 : static_cast<std::size_t>(nextUInt() % count);
}

Genome makePrimitiveGenome() {
    Genome g;
    g.genes[0].nodes={{-1,{},true},{0,{.9f,0},false},{1,{.8f,.25f},false}};
    auto& sensor=g.genes[0].nodes[1].behavior;
    sensor.role=CellRole::EnergySensor;sensor.sensorRange=2;sensor.sensitivity=.8f;
    auto& motor=g.genes[0].nodes[2].behavior;
    motor.role=CellRole::Motor;motor.motorChannel=EnergyIntensity;motor.motorStrength=.6f;
    // One unsteered propulsor responds to local food intensity. There is no
    // navigation circuit, hunting strategy, optimized body or special energy.
    return g;
}

Genome makeDefaultGenome()
{
    Genome genome;
    genome.genes[0].nodes = {
        GenomeNode{-1, {0.0f, 0.0f}, true},
        GenomeNode{0, {1.25f, 0.0f}, false},
        GenomeNode{0, {0.0f, 1.25f}, false},
    };
    return genome;
}

bool hasDefaultGenomeSemantics(Genome const& genome)
{
    if(genome.genes.size()!=1) return false;
    auto const expected = makeDefaultGenome();
    if (genome.genes[0].nodes.size() != expected.genes[0].nodes.size()) {
        return false;
    }
    for (std::size_t index = 0; index < genome.genes[0].nodes.size(); ++index) {
        auto const& actual = genome.genes[0].nodes[index];
        auto const& wanted = expected.genes[0].nodes[index];
        if (actual.parentNode != wanted.parentNode || actual.constructorCell != wanted.constructorCell
            || !nearlyEqual(actual.relativePosition.x, wanted.relativePosition.x)
            || !nearlyEqual(actual.relativePosition.y, wanted.relativePosition.y)) {
            return false;
        }
    }
    return true;
}

bool isValidGenome(Genome const& genome, GenomeMutationConfig const& config)
{
    if(genome.genes.empty()) return false;
    bool developmental=genome.genes.size()>1 || genome.entryGene!=0;
    for(auto const& node:genome.genes[0].nodes) developmental |= node.construction.targetGene>=0;
    if(developmental) return isValidDevelopmentGenome(genome);
    if (genome.genes[0].nodes.size() < config.minNodes || genome.genes[0].nodes.size() > config.maxNodes || genome.genes[0].nodes.empty()) {
        return false;
    }
    for(auto const& node:genome.genes[0].nodes) {
        auto const& b=node.behavior;
        if(node.constructorCell != (b.role==CellRole::Constructor)) return false;
        auto bounded=[](float v,float lo,float hi){return std::isfinite(v)&&v>=lo&&v<=hi;};
        if(static_cast<unsigned>(b.role)>static_cast<unsigned>(CellRole::Count)-1
            || static_cast<unsigned>(b.waveform)>static_cast<unsigned>(Waveform::Square)
            || static_cast<unsigned>(b.motorMode)>static_cast<unsigned>(MotorMode::Bending) || !bounded(b.bendingAngle,0,1.2f) || !bounded(b.contraction,0,0.40f) || b.motorChannel>=SignalChannelCount
            || !bounded(b.period,0.1f,20) || !bounded(b.phase,0,1) || !bounded(b.amplitude,0,1)
            || !bounded(b.motorStrength,0,4) || !bounded(b.axisAngle,-3.142f,3.142f)
            || !bounded(b.signalWeight,-2,2) || !bounded(b.sensorRange,0.5f,6)
            || !bounded(b.extractionRate,0.25f,2) || !bounded(b.digestionRate,0.25f,2)
            || !bounded(b.storageCapacity,0.5f,6) || !bounded(b.defenseStrength,0.25f,2)
            || static_cast<unsigned>(b.memoryMode)>static_cast<unsigned>(MemoryMode::Integrate) || !bounded(b.memoryTime,0.02f,2)
            || !bounded(b.sensitivity,0.1f,4) || !bounded(b.selfWeight,-1,1)) return false;
        for(auto const& row:b.weights)for(float weight:row)if(!bounded(weight,-4,4))return false;
        for(float bias:b.biases)if(!bounded(bias,-4,4))return false;
    }
    if (!isFinite(genome.genes[0].nodes[0].relativePosition)) return false;
    if (genome.genes[0].nodes[0].parentNode != -1 || !genome.genes[0].nodes[0].constructorCell
        || length(genome.genes[0].nodes[0].relativePosition) > 0.0001f) {
        return false;
    }

    for (std::size_t index = 1; index < genome.genes[0].nodes.size(); ++index) {
        auto const& node = genome.genes[0].nodes[index];
        if (node.constructorCell || node.parentNode < 0 || static_cast<std::size_t>(node.parentNode) >= index) {
            return false;
        }
        auto const edgeLength = length(node.relativePosition);
        if (!std::isfinite(edgeLength) || edgeLength < config.minEdgeLength || edgeLength > config.maxEdgeLength) {
            return false;
        }

        // Parent indices are constrained to point backwards, so this loop is
        // also a compact cycle/disconnected-node proof for a tree genome.
        auto current = static_cast<int>(index);
        std::size_t hops = 0;
        while (current != 0 && current >= 0 && hops <= genome.genes[0].nodes.size()) {
            current = genome.genes[0].nodes[static_cast<std::size_t>(current)].parentNode;
            ++hops;
        }
        if (current != 0) {
            return false;
        }
    }
    return true;
}

namespace {

float randomSigned(DeterministicRng& rng, float magnitude)
{
    return (rng.nextUnit() * 2.0f - 1.0f) * magnitude;
}

Vec2 boundedEdge(Vec2 value, GenomeMutationConfig const& config)
{
    auto const edgeLength = length(value);
    if (edgeLength < 0.000001f || !std::isfinite(edgeLength)) {
        return {config.minEdgeLength, 0.0f};
    }
    auto const boundedLength = clamp(edgeLength, config.minEdgeLength, config.maxEdgeLength);
    return normalizedOr(value) * boundedLength;
}

std::vector<std::size_t> terminalNodes(Genome const& genome)
{
    std::vector<bool> hasChild(genome.genes[0].nodes.size(), false);
    for (std::size_t index = 1; index < genome.genes[0].nodes.size(); ++index) {
        auto const parent = genome.genes[0].nodes[index].parentNode;
        if (parent >= 0 && static_cast<std::size_t>(parent) < hasChild.size()) {
            hasChild[static_cast<std::size_t>(parent)] = true;
        }
    }

    std::vector<std::size_t> result;
    for (std::size_t index = 1; index < genome.genes[0].nodes.size(); ++index) {
        if (!hasChild[index]) {
            result.push_back(index);
        }
    }
    return result;
}

// One localized perturbation per accepted birth event. Do not reset a network.
bool mutateBehavior(Genome& genome, DeterministicRng& rng, float probability)
{
    if(probability<=0 || rng.nextUnit()>=probability) return false;
    enum Parameter { Weight, Bias, Connection, Strength, Axis, Contraction, Range, Sensitivity };
    struct Candidate { std::size_t node; Parameter parameter; };
    std::vector<Candidate> candidates;
    for(std::size_t n=0;n<genome.genes[0].nodes.size();++n) {
        auto const& node=genome.genes[0].nodes[n]; auto const& gene=node.behavior;
        if(gene.neural) { candidates.push_back({n,Weight}); candidates.push_back({n,Bias}); }
        if(n>0) candidates.push_back({n,Connection});
        if(gene.role==CellRole::Motor) {
            candidates.push_back({n,Strength}); candidates.push_back({n,Axis});
            if(gene.motorMode==MotorMode::Contractile) candidates.push_back({n,Contraction});
        }
        if(gene.role==CellRole::EnergySensor || gene.role==CellRole::CreatureSensor) {
            candidates.push_back({n,Range}); candidates.push_back({n,Sensitivity});
        }
    }
    if(candidates.empty()) return false;
    auto selected=candidates[rng.nextIndex(candidates.size())];
    auto& gene=genome.genes[0].nodes[selected.node].behavior;
    // Box–Muller, truncated at three sigma to bound each generational step.
    float gaussian=std::sqrt(-2.0f*std::log(std::max(rng.nextUnit(),0.000001f)))
        *std::cos(6.28318530718f*rng.nextUnit());
    gaussian=clamp(gaussian,-3.0f,3.0f);
    auto perturb=[&](float& value,float sigma,float low,float high) {
        float before=value; value=clamp(value+gaussian*sigma,low,high); return value!=before;
    };
    auto neuralRow=[&]() {
        // Prefer rows already expressed in the inherited network, independent
        // of role or behavior. Retain access to every row for new pathways.
        std::vector<std::size_t> expressed;
        for(std::size_t row=0;row<SignalChannelCount;++row) {
            bool active=gene.biases[row]!=0;
            for(float weight:gene.weights[row]) active|=weight!=0;
            if(active) expressed.push_back(row);
        }
        return !expressed.empty() && rng.nextUnit()<0.8f
            ? expressed[rng.nextIndex(expressed.size())] : rng.nextIndex(SignalChannelCount);
    };
    switch(selected.parameter) {
    case Weight: {
        auto row=neuralRow(); auto column=rng.nextIndex(SignalChannelCount);
        return perturb(gene.weights[row][column],0.08f,-4,4);
    }
    case Bias: return perturb(gene.biases[neuralRow()],0.04f,-4,4);
    case Connection: return perturb(gene.signalWeight,0.05f,-2,2);
    case Strength: return perturb(gene.motorStrength,0.08f,0,4);
    case Axis: return perturb(gene.axisAngle,0.06f,-3.142f,3.142f);
    case Contraction: return perturb(gene.contraction,0.025f,0,0.4f);
    case Range: return perturb(gene.sensorRange,0.12f,0.5f,6);
    case Sensitivity: return perturb(gene.sensitivity,0.06f,0.1f,4);
    }
    return false;
}

} // namespace

GenomeMutationResult mutateGenome(Genome const& parent, DeterministicRng& rng, GenomeMutationConfig const& config)
{
    if(config.openStructuralMutation) return mutateDevelopmentGenome(parent,rng);
    GenomeMutationResult result{parent, MutationKind::None};
    if (parent.genes.size()!=1 || !isValidGenome(parent, config)) {
        return result;
    }

    for(auto& node:result.genome.genes[0].nodes) {
        if(node.behavior.role!=CellRole::Generator || config.oscillatorProbability<=0) continue;
        if(rng.nextUnit()>=config.oscillatorProbability) continue;
        switch(rng.nextIndex(3)) {
        case 0: node.behavior.period=clamp(node.behavior.period+randomSigned(rng,0.15f),0.1f,20.0f); break;
        case 1: node.behavior.phase=clamp(node.behavior.phase+randomSigned(rng,0.06f),0.0f,1.0f); break;
        case 2: node.behavior.amplitude=clamp(node.behavior.amplitude+randomSigned(rng,0.05f),0.0f,1.0f); break;
        }
        result.kind=MutationKind::Oscillator;
    }

    auto const roll = rng.nextUnit();
    auto const geometryLimit = config.geometryProbability;
    auto const additionLimit = geometryLimit + config.additionProbability;
    auto const removalLimit = additionLimit + config.removalProbability;

    if (roll < geometryLimit && result.genome.genes[0].nodes.size() > 1) {
        auto const nodeIndex = 1 + rng.nextIndex(result.genome.genes[0].nodes.size() - 1);
        auto& node = result.genome.genes[0].nodes[nodeIndex];
        node.relativePosition += Vec2{
            randomSigned(rng, config.maxPerturbation),
            randomSigned(rng, config.maxPerturbation),
        };
        node.relativePosition = boundedEdge(node.relativePosition, config);
        result.kind = MutationKind::Geometry;
    } else if (roll < additionLimit) {
        if (result.genome.genes[0].nodes.size() < config.maxNodes) {
        auto const parentIndex = rng.nextIndex(result.genome.genes[0].nodes.size());
        auto const angle = rng.nextUnit() * 6.28318530718f;
        auto const edgeLength = config.minEdgeLength
            + rng.nextUnit() * (config.maxEdgeLength - config.minEdgeLength);
        result.genome.genes[0].nodes.push_back(GenomeNode{
            static_cast<int>(parentIndex),
            {std::cos(angle) * edgeLength, std::sin(angle) * edgeLength},
            false,
        });
        result.kind = MutationKind::AddTerminal;
        }
    } else if (roll < removalLimit && result.genome.genes[0].nodes.size() > config.minNodes) {
        auto leaves = terminalNodes(result.genome);
        if (!leaves.empty()) {
            auto const removedIndex = leaves[rng.nextIndex(leaves.size())];
            result.genome.genes[0].nodes.erase(result.genome.genes[0].nodes.begin() + static_cast<std::ptrdiff_t>(removedIndex));
            for (auto& node : result.genome.genes[0].nodes) {
                if (node.parentNode > static_cast<int>(removedIndex)) {
                    --node.parentNode;
                }
            }
            result.kind = MutationKind::RemoveTerminal;
        }
    }

    if(config.motorModeProbability>0 && rng.nextUnit()<config.motorModeProbability) {
        auto n=rng.nextIndex(result.genome.genes[0].nodes.size());auto& b=result.genome.genes[0].nodes[n].behavior;
        if(b.role==CellRole::Motor) {
            b.motorMode=b.motorMode==MotorMode::Thrust ? MotorMode::Contractile : MotorMode::Thrust;
            result.kind=MutationKind::Behavior;
        }
    }
    result.behaviorMutated=mutateBehavior(result.genome,rng,config.behaviorProbability);
    if(result.behaviorMutated) result.kind=MutationKind::Behavior;

    if (!isValidGenome(result.genome, config) || result.genome == parent) {
        result.genome = parent;
        result.kind = MutationKind::None;
        result.behaviorMutated = false;
    }
    return result;
}

} // namespace alienmobile
