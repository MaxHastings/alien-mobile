#include "alienmobile/Development.h"
#include <algorithm>
#include <cmath>

namespace alienmobile {
namespace {
Vec2 rotated(Vec2 p, float a) {
    return {p.x*std::cos(a)-p.y*std::sin(a),p.x*std::sin(a)+p.y*std::cos(a)};
}
bool bounded(float v,float lo,float hi) { return std::isfinite(v) && v>=lo && v<=hi; }
}

bool isValidDevelopmentGenome(Genome const& genome) {
    if(!validMutationRates(genome.mutationRates)) return false;
    if(genome.genes.empty() || genome.genes.size()>16 || genome.entryGene>=genome.genes.size()) return false;
    std::size_t total=0;
    for(std::size_t g=0;g<genome.genes.size();++g) {
        auto const& gene=genome.genes[g];
        if(gene.nodes.empty() || gene.nodes.size()>64 || !bounded(gene.orientation,-6.284f,6.284f) || !bounded(gene.phaseAdvance,-0.5f,0.5f)) return false;
        total+=gene.nodes.size();
        if(total>256) return false;
        for(std::size_t n=0;n<gene.nodes.size();++n) {
            auto const& node=gene.nodes[n]; auto const& c=node.construction;
            if(!isFinite(node.relativePosition) || (n==0 ? node.parentNode!=-1 : node.parentNode<0 || std::size_t(node.parentNode)>=n)) return false;
            if(!bounded(node.stiffness,0.1f,2.f)) return false;
            float edge=length(node.relativePosition);
            if(n==0 && g==genome.entryGene) {
                if(edge>0.0001f || !node.constructorCell) return false;
            } else if(!bounded(edge,0.45f,2.10f)) return false;
            if(c.targetGene < -1 || c.targetGene>=int(genome.genes.size()) || (c.targetGene>=0 && !node.constructorCell)
                || c.branches<1 || c.branches>8 || c.repetitions<1 || c.repetitions>16
                || !bounded(c.angle,-6.284f,6.284f) || !bounded(c.branchAngle,-6.284f,6.284f)
                || !bounded(c.repetitionAngle,-3.142f,3.142f) || !bounded(c.intervalScale,0.1f,10.f)) return false;
            // Reuse the existing complete organ/controller validator on a
            // minimal legacy tree. Developmental role placement is unrestricted.
            Genome probe=makeDefaultGenome();
            probe.genes[0].nodes[1].behavior=node.behavior;
            probe.genes[0].nodes[1].constructorCell=false;
            if(node.constructorCell != (node.behavior.role==CellRole::Constructor)) return false;
            if(node.constructorCell) probe.genes[0].nodes[1].behavior.role=CellRole::Structural;
            if(!isValidGenome(probe)) return false;
        }
    }
    return true;
}

std::optional<DevelopedNode> DevelopmentCursor::next(Genome const& genome) {
    if(_status!=DevelopmentStatus::Growing) return {};
    if(!_started) {
        _started=true;
        if(!isValidDevelopmentGenome(genome) || _limits.maxDepth>32 || _limits.maxCells>4096 || !_limits.maxCells) {
            _status=DevelopmentStatus::InvalidGenome;return {};
        }
        Frame root;root.gene=genome.entryGene;
        _stack.push_back(root);
    }
    while(!_stack.empty()) {
        auto& f=_stack.back();
        if(f.gene>=genome.genes.size()) {_status=DevelopmentStatus::InvalidGenome;return {};}
        auto const& gene=genome.genes[f.gene];
        if(gene.nodes.empty() || f.node>gene.nodes.size()) {_status=DevelopmentStatus::InvalidGenome;return {};}
        if(f.node==gene.nodes.size()) {
            if(++f.repetition<f.repetitions) {
                f.anchor=f.physicalNodes.back();f.node=0;f.physicalNodes.clear();
            } else if(++f.branch<f.branches) {
                f.repetition=0;f.anchor=f.origin;f.node=0;f.physicalNodes.clear();
            } else { _stack.pop_back();continue; }
        }
        if(f.depth>_limits.maxDepth) {_status=DevelopmentStatus::DepthLimit;return {};}
        if(_emitted>=_limits.maxCells) {_status=DevelopmentStatus::CellLimit;return {};}
        auto const& source=gene.nodes[f.node];
        if(f.node && (source.parentNode<0 || std::size_t(source.parentNode)>=f.physicalNodes.size())) {
            _status=DevelopmentStatus::InvalidGenome;return {};
        }
        if(source.construction.targetGene>=int(genome.genes.size())) {
            _status=DevelopmentStatus::InvalidGenome;return {};
        }
        DevelopedNode out;
        out.physical=source;out.gene=f.gene;out.node=f.node;out.depth=f.depth;
        out.branch=f.branch;out.repetition=f.repetition;out.intervalScale=f.intervalScale;
        float angle=f.angle+gene.orientation+f.branch*f.branchAngle+f.repetition*f.repetitionAngle;
        out.physical.parentNode=f.node==0 ? f.anchor : f.physicalNodes[source.parentNode];
        out.physical.relativePosition=rotated(source.relativePosition,angle);
        out.physical.behavior.phase+=f.repetition*gene.phaseAdvance;
        out.physical.behavior.phase-=std::floor(out.physical.behavior.phase);
        // Root references can recurse too: a root's zero offset becomes a
        // finite attachment edge when that gene is called as a subprogram.
        if(out.physical.parentNode>=0 && length(out.physical.relativePosition)<0.0001f)
            out.physical.relativePosition=rotated({1,0},angle);
        int index=int(_emitted++);
        f.physicalNodes.push_back(index);++f.node;
        if(source.construction.targetGene>=0) {
            auto const& c=source.construction;
            Frame invocation;
            invocation.gene=uint32_t(c.targetGene);invocation.depth=f.depth+1;
            invocation.origin=invocation.anchor=index;
            invocation.angle=angle+c.angle;
            invocation.branchAngle=c.branchAngle;invocation.repetitionAngle=c.repetitionAngle;
            invocation.branches=c.branches;invocation.repetitions=c.repetitions;
            invocation.intervalScale=c.intervalScale;
            _stack.push_back(std::move(invocation));
        }
        return out;
    }
    _status=DevelopmentStatus::Complete;return {};
}

DevelopmentSummary measureDevelopment(Genome const& genome, DevelopmentLimits limits) {
    DevelopmentCursor cursor(limits);
    while(cursor.next(genome)) {}
    return {cursor.emitted(),cursor.status()};
}

std::vector<Genome> makeDevelopmentFounders() {
    // All four are data in the same language, with no runtime body classes.
    Genome chain;
    chain.genes.resize(2);
    chain.genes[0].nodes={{-1,{},true},{0,{0,1},false}};
    chain.genes[0].nodes[0].construction.targetGene=1;
    chain.genes[0].nodes[0].construction.repetitions=8;
    chain.genes[1].nodes={{-1,{1,0},false},{0,{1,0},false},{1,{1,0},false}};
    chain.genes[1].nodes[1].behavior.role=CellRole::Generator;
    chain.genes[1].nodes[2].behavior.role=CellRole::Motor;
    chain.genes[1].nodes[2].behavior.motorMode=MotorMode::Contractile;
    chain.genes[1].nodes[2].behavior.motorChannel=Oscillator;
    chain.genes[1].nodes[2].behavior.motorStrength=0.6f;
    Genome radial=chain;
    radial.genes[0].nodes[0].construction.repetitions=2;
    radial.genes[0].nodes[0].construction.branches=6;
    radial.genes[0].nodes[0].construction.branchAngle=6.2831853f/6;
    radial.genes[0].nodes[1].relativePosition={0.7f,0.7f};
    Genome branching=chain;
    branching.genes.resize(3);
    branching.genes[0].nodes[0].construction.repetitions=1;
    branching.genes[0].nodes[0].construction.branches=3;
    branching.genes[0].nodes[0].construction.branchAngle=2.0943951f;
    branching.genes[1].nodes={{-1,{1,0},false},{0,{1,0},false},{1,{1,0},true}};
    branching.genes[1].nodes[2].construction.targetGene=2;
    branching.genes[1].nodes[2].construction.branches=2;
    branching.genes[1].nodes[2].construction.angle=-0.55f;
    branching.genes[1].nodes[2].construction.branchAngle=1.1f;
    branching.genes[2].nodes={{-1,{1,0},false},{0,{1,0},false},{1,{1,0},false}};
    Genome appendages=chain;
    appendages.genes.resize(3);
    appendages.genes[0].nodes[0].construction.repetitions=6;
    appendages.genes[1].nodes={{-1,{1,0},true},{0,{1,0},false}};
    appendages.genes[1].nodes[0].construction.targetGene=2;
    appendages.genes[1].nodes[0].construction.branches=2;
    appendages.genes[1].nodes[0].construction.angle=1.5707963f;
    appendages.genes[1].nodes[0].construction.branchAngle=3.1415926f;
    appendages.genes[2].nodes={{-1,{1,0},false},{0,{1,0},false},{1,{1,0},false}};
    chain.genes[1].phaseAdvance=.13f;
    chain.genes[1].nodes[1].behavior.waveform=Waveform::Sine;
    chain.genes[1].nodes[2].behavior.motorMode=MotorMode::Bending;
    chain.genes[0].nodes[1].behavior.role=CellRole::Motor;
    chain.genes[0].nodes[1].behavior.motorChannel=Activation;
    chain.genes[0].nodes[1].behavior.neural=true;
    chain.genes[0].nodes[1].behavior.biases[Activation]=.6f;
    chain.genes[0].nodes[1].behavior.axisAngle=-1.5707963f;
    radial.genes[1].nodes[1].behavior.waveform=Waveform::Sine;
    branching.genes[2].nodes[0].behavior.role=CellRole::Generator;
    branching.genes[2].nodes[0].behavior.waveform=Waveform::Sine;
    branching.genes[2].nodes[1].behavior.role=CellRole::Motor;
    branching.genes[2].nodes[1].behavior.motorMode=MotorMode::Bending;
    appendages.genes[2].nodes[0].behavior.role=CellRole::Generator;
    appendages.genes[2].nodes[0].behavior.waveform=Waveform::Sine;
    appendages.genes[2].nodes[1].behavior.role=CellRole::Motor;
    appendages.genes[2].nodes[1].behavior.motorMode=MotorMode::Bending;
    appendages.genes[2].nodes[2].behavior.role=CellRole::Motor;
    appendages.genes[2].nodes[2].behavior.motorChannel=Activation;
    appendages.genes[2].nodes[2].behavior.neural=true;
    appendages.genes[2].nodes[2].behavior.biases[Activation]=.35f;
    return {chain,radial,branching,appendages};
}
} // namespace alienmobile
