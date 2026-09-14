#include "alienmobile/Development.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace alienmobile {
namespace {
using RateMember=float MutationRates::*;
constexpr std::array<RateMember,12> rates={&MutationRates::neural,&MutationRates::geometry,&MutationRates::property,
    &MutationRates::role,&MutationRates::insert,&MutationRates::erase,&MutationRates::duplicateGene,
    &MutationRates::deleteGene,&MutationRates::copySection,&MutationRates::moveSection,&MutationRates::constructor,&MutationRates::meta};
constexpr std::array<MutationKind,11> kinds={MutationKind::Behavior,MutationKind::Geometry,MutationKind::Property,
    MutationKind::Role,MutationKind::InsertNode,MutationKind::DeleteNode,MutationKind::DuplicateGene,
    MutationKind::DeleteGene,MutationKind::CopySection,MutationKind::MoveSection,MutationKind::Constructor};
float noise(DeterministicRng& rng,float amplitude) {return (rng.nextUnit()*2-1)*amplitude;}
Vec2 edgeBound(Vec2 p) {return normalizedOr(p)*clamp(length(p),0.45001f,2.09999f);}
std::size_t nodeCount(Genome const& g) {std::size_t n=0;for(auto const& gene:g.genes)n+=gene.nodes.size();return n;}
void deleteSection(Gene& gene,std::size_t begin,std::size_t count) {
    auto old=gene.nodes;
    gene.nodes.erase(gene.nodes.begin()+begin,gene.nodes.begin()+begin+count);
    for(auto& node:gene.nodes) {
        int parent=node.parentNode;
        // Reattach descendants to the nearest retained ancestor. This is index
        // maintenance, not a viability repair; lost organs are not replaced.
        while(parent>=int(begin) && parent<int(begin+count)) parent=old[parent].parentNode;
        if(parent>=int(begin+count)) parent-=int(count);
        node.parentNode=parent;
    }
}
void appendSection(Gene& destination,std::vector<GenomeNode> const& source,std::size_t begin,std::size_t count,DeterministicRng& rng) {
    int offset=int(destination.nodes.size());
    int anchor=int(rng.nextIndex(destination.nodes.size()));
    for(std::size_t i=0;i<count;++i) {
        auto node=source[begin+i];
        node.parentNode=node.parentNode>=int(begin) && node.parentNode<int(begin+i)
            ? offset+node.parentNode-int(begin) : anchor;
        node.relativePosition=edgeBound(node.relativePosition);
        destination.nodes.push_back(node);
    }
}
float distance(Genome const& a,Genome const& b) {
    float d=2*std::abs(float(a.genes.size())-float(b.genes.size()));
    for(std::size_t g=0;g<std::min(a.genes.size(),b.genes.size());++g) {
        auto const& x=a.genes[g].nodes;auto const& y=b.genes[g].nodes;
        d+=std::abs(float(x.size())-float(y.size()));
        for(std::size_t n=0;n<std::min(x.size(),y.size());++n) {
            d+=length(x[n].relativePosition-y[n].relativePosition)*0.3f;
            d+=x[n].behavior.role!=y[n].behavior.role ? 0.6f : 0;
            d+=x[n].construction==y[n].construction ? 0 : 0.5f;
            d+=x[n].behavior==y[n].behavior ? 0 : 0.04f;
        }
    }
    return d;
}
}
bool validMutationRates(MutationRates const& r) {
    for(std::size_t i=0;i<rates.size();++i) {
        float v=r.*rates[i];
        if(!std::isfinite(v) || v<0 || v>(i==11 ? 0.01f : 0.5f)) return false;
    }
    for(float value:{r.neuralMagnitude,r.geometryMagnitude,r.propertyMagnitude})
        if(!std::isfinite(value) || value<0 || value>4)return false;
    return true;
}
GenomeMutationResult applyDevelopmentMutation(Genome const& parent,DeterministicRng& rng,MutationKind kind) {
    GenomeMutationResult result{parent,MutationKind::None};
    if(!isValidDevelopmentGenome(parent)) return result;
    auto& g=result.genome;
    std::size_t gi=rng.nextIndex(g.genes.size()), ni=rng.nextIndex(g.genes[gi].nodes.size());
    auto& gene=g.genes[gi];auto& node=gene.nodes[ni];auto& b=node.behavior;
    switch(kind) {
    case MutationKind::Behavior:
        // A small edit must not replace a working pass-through relay with a
        // zero neural matrix. Controller mode changes belong to Property.
        switch(rng.nextIndex(4)) {
        case 0: {auto& w=b.weights[rng.nextIndex(SignalChannelCount)][rng.nextIndex(SignalChannelCount)];w=clamp(w+noise(rng,0.15f*parent.mutationRates.neuralMagnitude),-4.f,4.f);break;}
        case 1: {auto& bias=b.biases[rng.nextIndex(SignalChannelCount)];bias=clamp(bias+noise(rng,0.1f*parent.mutationRates.neuralMagnitude),-4.f,4.f);break;}
        case 2:b.signalWeight=clamp(b.signalWeight+noise(rng,0.15f*parent.mutationRates.neuralMagnitude),-2.f,2.f);break;
        default:b.selfWeight=clamp(b.selfWeight+noise(rng,0.1f*parent.mutationRates.neuralMagnitude),-1.f,1.f);break;
        }
        result.behaviorMutated=true;break;
    case MutationKind::Geometry:
        switch(rng.nextIndex(10)) {
        case 0: gene.orientation=clamp(gene.orientation+noise(rng,.18f*parent.mutationRates.geometryMagnitude),-6.284f,6.284f);break;
        case 1: gene.phaseAdvance=clamp(gene.phaseAdvance+noise(rng,.03f*parent.mutationRates.geometryMagnitude),-.5f,.5f);break;
        case 2: node.stiffness=clamp(node.stiffness+noise(rng,.2f*parent.mutationRates.geometryMagnitude),.1f,2.f);break;
        default:
            if(ni || gi!=g.entryGene) node.relativePosition=edgeBound(node.relativePosition+Vec2{noise(rng,.12f*parent.mutationRates.geometryMagnitude),noise(rng,.12f*parent.mutationRates.geometryMagnitude)});
            break;
        }
        break;
    case MutationKind::Property:
        // Most property changes are continuous adjustments.  Mode/channel
        // rewrites are functional architecture changes in a tiny body, so
        // they draw from a much smaller branch of this already modest rate.
        switch(rng.nextUnit()<.05f ? std::array<unsigned,4>{9,14,17,18}[rng.nextIndex(4)]
                                   : std::array<unsigned,15>{0,1,2,3,4,5,6,7,8,10,11,12,13,15,16}[rng.nextIndex(15)]) {
        case 15:b.waveform=Waveform(rng.nextIndex(2));break;
        case 16:b.amplitude=clamp(b.amplitude+noise(rng,.08f*parent.mutationRates.propertyMagnitude),0.f,1.f);break;
        case 17:b.motorChannel=rng.nextIndex(SignalChannelCount);break;
        case 18:b.neural=!b.neural;break;
        case 11:b.storageCapacity=clamp(b.storageCapacity+noise(rng,.4f*parent.mutationRates.propertyMagnitude),.5f,6.f);break;
        case 12:b.defenseStrength=clamp(b.defenseStrength+noise(rng,.2f*parent.mutationRates.propertyMagnitude),.25f,2.f);break;
        case 13:b.memoryTime=clamp(b.memoryTime+noise(rng,.15f*parent.mutationRates.propertyMagnitude),.02f,2.f);break;
        case 14:b.memoryMode=MemoryMode(rng.nextIndex(2));break;
        case 9:b.motorMode=MotorMode(rng.nextIndex(3));break;
        case 10:b.bendingAngle=clamp(b.bendingAngle+noise(rng,.15f*parent.mutationRates.propertyMagnitude),0.f,1.2f);break;
        case 7:b.extractionRate=clamp(b.extractionRate+noise(rng,.2f*parent.mutationRates.propertyMagnitude),.25f,2.f);break;
        case 8:b.digestionRate=clamp(b.digestionRate+noise(rng,.2f*parent.mutationRates.propertyMagnitude),.25f,2.f);break;
        case 0:b.sensorRange=clamp(b.sensorRange+noise(rng,.4f*parent.mutationRates.propertyMagnitude),.5f,6.f);break;
        case 1:b.motorStrength=clamp(b.motorStrength+noise(rng,.3f*parent.mutationRates.propertyMagnitude),0.f,4.f);break;
        case 2:b.period=clamp(b.period+noise(rng,.4f*parent.mutationRates.propertyMagnitude),.1f,20.f);break;
        case 3:b.phase=clamp(b.phase+noise(rng,.1f*parent.mutationRates.propertyMagnitude),0.f,1.f);break;
        case 4:b.contraction=clamp(b.contraction+noise(rng,.05f*parent.mutationRates.propertyMagnitude),0.f,.4f);break;
        case 5:b.sensitivity=clamp(b.sensitivity+noise(rng,.2f*parent.mutationRates.propertyMagnitude),.1f,4.f);break;
        case 6:b.axisAngle=clamp(b.axisAngle+noise(rng,.2f*parent.mutationRates.propertyMagnitude),-3.142f,3.142f);break;
        }
        break;
    case MutationKind::Role:
        if(gi==g.entryGene && ni==0) break; // Only the reproductive root is retained.
        b.role=CellRole(rng.nextIndex(static_cast<unsigned>(CellRole::Count)));node.constructorCell=b.role==CellRole::Constructor;
        if(!node.constructorCell) node.construction.targetGene=-1;
        break;
    case MutationKind::InsertNode: {
        if(gene.nodes.size()>=64 || nodeCount(g)>=256) break;
        std::size_t at=1+rng.nextIndex(gene.nodes.size());
        GenomeNode added=node;added.parentNode=int(rng.nextIndex(at));
        added.relativePosition=edgeBound({noise(rng,1.5f),noise(rng,1.5f)});
        for(auto& n:gene.nodes) if(n.parentNode>=int(at)) ++n.parentNode;
        gene.nodes.insert(gene.nodes.begin()+at,added);break;
    }
    case MutationKind::DeleteNode:
        if(gene.nodes.size()>1) deleteSection(gene,1+rng.nextIndex(gene.nodes.size()-1),1);
        break;
    case MutationKind::DuplicateGene: {
        if(g.genes.size()>=16 || gene.nodes.size()>=64 || nodeCount(g)+gene.nodes.size()+1>256) break;
        Gene copy=gene;int appended=int(g.genes.size());
        if(gi==g.entryGene) copy.nodes[0].relativePosition={1,0};
        // Retain the working section and attach its copy through a new call.
        // Merely redirecting the only old call created a silent DNA duplicate,
        // not redundant physical machinery that could diverge subsequently.
        GenomeNode call=node;
        call.parentNode=int(ni);call.relativePosition=edgeBound({1,noise(rng,.5f)});
        call.constructorCell=true;call.behavior.role=CellRole::Constructor;
        call.construction={};call.construction.targetGene=appended;
        gene.nodes.push_back(call);
        g.genes.push_back(std::move(copy));
        break;
    }
    case MutationKind::DeleteGene:
        if(g.genes.size()>1 && gi!=g.entryGene) {
            g.genes.erase(g.genes.begin()+gi);
            if(g.entryGene>gi) --g.entryGene;
            for(auto& sub:g.genes) for(auto& n:sub.nodes) {
                auto& ref=n.construction.targetGene;
                if(ref==int(gi)) ref=-1;else if(ref>int(gi)) --ref;
            }
        }
        break;
    case MutationKind::CopySection:
    case MutationKind::MoveSection: {
        if(gene.nodes.size()<2) break;
        auto source=gene.nodes;
        std::size_t begin=1+rng.nextIndex(source.size()-1);
        std::size_t count=1+rng.nextIndex(std::min<std::size_t>(3,source.size()-begin));
        std::size_t target=rng.nextIndex(g.genes.size());
        if(g.genes[target].nodes.size()+count>64 || (kind==MutationKind::CopySection && nodeCount(g)+count>256)) break;
        if(kind==MutationKind::MoveSection) deleteSection(gene,begin,count);
        appendSection(g.genes[target],source,begin,count,rng);break;
    }
    case MutationKind::Constructor: {
        // Eligible nodes need not already own a call: constructors can acquire
        // references through mutation instead of relying on authored modules.
        if(!node.constructorCell) break;
        auto& c=node.construction;
        switch(rng.nextIndex(7)) {
        case 0:c.targetGene=int(rng.nextIndex(g.genes.size()+1))-1;break;
        case 1:c.branches=uint16_t(clamp(float(c.branches)+(rng.nextUnit()<.5f ? -1.f : 1.f),1,8));break;
        case 2:c.repetitions=uint16_t(clamp(float(c.repetitions)+(rng.nextUnit()<.5f ? -1.f : 1.f),1,16));break;
        case 3:c.angle=clamp(c.angle+noise(rng,.35f),-6.284f,6.284f);break;
        case 4:c.branchAngle=clamp(c.branchAngle+noise(rng,.3f),-6.284f,6.284f);break;
        case 5:c.intervalScale=clamp(c.intervalScale+noise(rng,.2f),.1f,10.f);break;
        case 6:if(gi==g.entryGene && ni==0)c.separateOffspring=!c.separateOffspring;else c.repetitionAngle=clamp(c.repetitionAngle+noise(rng,.2f),-3.142f,3.142f);break;
        }
        break;
    }
    case MutationKind::Meta: {
        auto i=rng.nextIndex(rates.size()+3);
        if(i<rates.size()) {
            auto& r=g.mutationRates.*rates[i];
            r=clamp(r*(1+noise(rng,.04f))+noise(rng,.0001f),0.f,i==11 ? .01f : .5f);
        } else {
            float* values[]={&g.mutationRates.neuralMagnitude,&g.mutationRates.geometryMagnitude,&g.mutationRates.propertyMagnitude};
            auto& v=*values[i-rates.size()];v=clamp(v*(1+noise(rng,.04f))+noise(rng,.0001f),0.f,4.f);
        }
        result.metaMutated=g!=parent;
        break;
    }
    default:break;
    }
    if(g!=parent) {result.kind=kind;result.geneticDistance=distance(parent,g);}
    // Development failure is heritable. Only a memory/topology validity error
    // would be an implementation bug; do not replace sterile DNA with its parent.
    return result;
}
GenomeMutationResult mutateDevelopmentGenome(Genome const& parent,DeterministicRng& rng,float exposureMultiplier) {
    float sum=0;for(std::size_t i=0;i<kinds.size();++i) sum+=parent.mutationRates.*rates[i];
    // Exposure changes only the chance of one ordinary event, preserving its
    // relative kind weights and magnitudes. Never write exposure into DNA.
    // Keep the unexposed path bit-for-bit identical to archived experiments.
    float scale=1.f;
    if(std::isfinite(exposureMultiplier) && exposureMultiplier>1.f && sum>0 && sum<.95f)
        scale=std::min(exposureMultiplier,.95f/sum);
    float roll=rng.nextUnit()*std::max(1.f,sum*scale)/scale;
    GenomeMutationResult result{parent,MutationKind::None};
    for(std::size_t i=0;i<kinds.size();++i) {
        roll-=parent.mutationRates.*rates[i];
        if(roll<0) {result=applyDevelopmentMutation(parent,rng,kinds[i]);break;}
    }
    // Independent and much slower than phenotype edits. Previously total
    // ordinary probability >=1 accidentally made meta-mutation impossible.
    // A birth gets one ordinary mutation.  Meta changes are deliberately not
    // stacked onto structural events, avoiding compound disruptive births.
    bool structural=result.kind==MutationKind::Role || result.kind==MutationKind::InsertNode
        || result.kind==MutationKind::DeleteNode || result.kind==MutationKind::DuplicateGene
        || result.kind==MutationKind::DeleteGene || result.kind==MutationKind::CopySection
        || result.kind==MutationKind::MoveSection || result.kind==MutationKind::Constructor;
    if(!structural && rng.nextUnit()<parent.mutationRates.meta) {
        auto meta=applyDevelopmentMutation(result.genome,rng,MutationKind::Meta);
        if(meta.mutated()) {
            result.genome=std::move(meta.genome);result.metaMutated=true;
            if(result.kind==MutationKind::None)result.kind=MutationKind::Meta;
        }
    }
    return result;
}
} // namespace alienmobile
