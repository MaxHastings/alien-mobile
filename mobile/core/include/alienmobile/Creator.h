#pragma once
#include "alienmobile/Development.h"
#include <algorithm>
namespace alienmobile {
// An edit copy expands bounded development into ordinary genes. The original
// specimen remains exact. Controllers and physical geometry are retained.
inline Genome editableBody(Genome const& source) {
    Genome result; result.mutationRates=source.mutationRates;
    DevelopmentCursor cursor;
    while(auto n=cursor.next(source)) {
        auto node=n->physical;bool separate=node.construction.separateOffspring;node.construction={};node.construction.separateOffspring=separate;
        result.genes[0].nodes.push_back(node);
    }
    if(cursor.status()!=DevelopmentStatus::Complete || result.genes[0].nodes.size()>64) return {};
    return result;
}
inline bool validCreatorBody(Genome const& g) {
    return g.genes.size()==1 && g.entryGene==0 && isValidDevelopmentGenome(g) && measureDevelopment(g).complete();
}
inline BehaviorGene seededOrgan(CellRole role) {
    BehaviorGene b;b.role=role;
    if(role==CellRole::Motor || role==CellRole::Attacker) {
        b.neural=true;b.motorChannel=Activation;b.biases[Activation]=.65f;
        b.weights[Activation][EnergyIntensity]=.3f;
    }
    return b;
}
inline bool changeOrgan(Genome& g,size_t index,CellRole role) {
    if(!validCreatorBody(g))return false;
    auto copy=g;auto& nodes=copy.genes[0].nodes;
    if(index==0 || index>=nodes.size() || nodes[index].constructorCell)return false;
    nodes[index].behavior=seededOrgan(role);
    if(!validCreatorBody(copy))return false;g=copy;return true;
}
inline bool addBodyCell(Genome& g,size_t parent,Vec2 edge) {
    if(!validCreatorBody(g) || parent>=g.genes[0].nodes.size() || g.genes[0].nodes.size()>=32)return false;
    auto copy=g;copy.genes[0].nodes.emplace_back(int(parent),edge,false);
    if(!validCreatorBody(copy))return false;g=copy;return true;
}
// Move a branch through its attachment edge. Descendants keep their local
// geometry and all controller data; only the authored physical edge changes.
inline bool moveBodyCell(Genome& g,size_t index,Vec2 edge) {
    if(!validCreatorBody(g) || index==0 || index>=g.genes[0].nodes.size() || !isFinite(edge))return false;
    auto copy=g;copy.genes[0].nodes[index].relativePosition=edge;
    if(!validCreatorBody(copy))return false;
    std::vector<Vec2> positions;
    for(auto const& n:copy.genes[0].nodes)
        positions.push_back(n.parentNode<0 ? Vec2{} : positions[n.parentNode]+n.relativePosition);
    // A touch edit must not fold cells onto one another. This constraint is
    // editor-only; inherited mutation is still allowed to make bad anatomy.
    std::vector<bool> moved(positions.size());moved[index]=true;
    for(size_t i=index+1;i<moved.size();++i)moved[i]=moved[copy.genes[0].nodes[i].parentNode];
    for(size_t i=0;i<positions.size();++i)for(size_t j=0;j<i;++j)
        if(moved[i]!=moved[j] && length(positions[i]-positions[j])<.65f)return false;
    g=std::move(copy);return true;
}
inline bool removeBodyCell(Genome& g,size_t index) {
    if(!validCreatorBody(g))return false;
    auto copy=g;auto& nodes=copy.genes[0].nodes;
    if(index==0 || index>=nodes.size() || nodes.size()<=2 || nodes[index].constructorCell)return false;
    for(auto const& n:nodes)if(n.parentNode==int(index))return false;
    nodes.erase(nodes.begin()+index);
    for(auto& n:nodes)if(n.parentNode>int(index))--n.parentNode;
    if(!validCreatorBody(copy))return false;g=copy;return true;
}
// Creator-only compilation. No simulation code calls this function. All output
// is ordinary mutable BehaviorGene data, serialized and inherited with the body.
inline Genome compileCreatorBody(Genome const& body) {
    if(!validCreatorBody(body)) return body;
    auto result=body;auto& nodes=result.genes[0].nodes;
    if(nodes.size()<2)return result;
    std::vector<Vec2> positions(nodes.size()),axes(nodes.size());
    std::vector<unsigned> degree(nodes.size());Vec2 center{},forward{};
    bool food=false,life=false,rhythm=false;
    for(size_t i=0;i<nodes.size();++i) {
        auto const& n=nodes[i];
        if(n.parentNode>=0){positions[i]=positions[n.parentNode]+n.relativePosition;++degree[i];++degree[n.parentNode];}
        center+=positions[i];
        auto edge=i ? n.relativePosition : nodes[1].relativePosition;
        float a=std::atan2(edge.y,edge.x)+n.behavior.axisAngle;
        axes[i]={std::cos(a),std::sin(a)};
        if(n.behavior.role==CellRole::Motor && n.behavior.motorMode==MotorMode::Thrust)forward+=axes[i]*n.behavior.motorStrength;
        rhythm|=n.behavior.role==CellRole::Generator;
        food|=n.behavior.role==CellRole::EnergySensor;life|=n.behavior.role==CellRole::CreatureSensor || n.behavior.role==CellRole::ObstacleSensor;
    }
    center=center/float(nodes.size());
    // No change of motor direction or strength. A cancelling body keeps its
    // cancelling thrust; the first motor only supplies a sensory reference.
    if(length(forward)<.01f)for(size_t i=0;i<nodes.size();++i)
        if(nodes[i].behavior.role==CellRole::Motor){forward=axes[i];break;}
    forward=normalizedOr(forward);Vec2 side{-forward.y,forward.x};
    float maxTorque=.1f;
    for(size_t i=0;i<nodes.size();++i)if(nodes[i].behavior.role==CellRole::Motor && nodes[i].behavior.motorMode==MotorMode::Thrust) {
        auto r=positions[i]-center;maxTorque=std::max(maxTorque,std::abs(r.x*axes[i].y-r.y*axes[i].x));
    }
    for(size_t i=0;i<nodes.size();++i) {
        auto& b=nodes[i].behavior;
        b.neural=true;b.weights={};b.biases={};b.selfWeight=0;b.signalWeight=1;
        float relay=.98f/std::max(1u,degree[i]);
        for(size_t k=0;k<SignalChannelCount;++k)b.weights[k][k]=relay;
        // Receptors measure in their own parent-edge frame. Encode the
        // rotation into the shared authored-body frame before relaying it.
        int channel=b.role==CellRole::EnergySensor ? EnergyX :
            (b.role==CellRole::CreatureSensor || b.role==CellRole::ObstacleSensor) ? CreatureX : -1;
        if(channel>=0) {
            float sign=b.role==CellRole::ObstacleSensor ? -1.f : 1.f;
            float x=axes[i].x*sign,y=axes[i].y*sign;
            b.weights[channel]={};b.weights[channel+1]={};b.weights[channel+2]={};
            b.weights[channel][channel]=x;b.weights[channel][channel+1]=-y;
            b.weights[channel+1][channel]=y;b.weights[channel+1][channel+1]=x;
            b.weights[channel+2][channel+2]=1;
        }
        if(b.role==CellRole::Motor) {
            b.motorChannel=Activation;b.weights[Activation]={};
            if(b.motorMode!=MotorMode::Thrust) {b.weights[Activation][Oscillator]=2*relay;continue;}
            auto r=positions[i]-center;float torque=(r.x*axes[i].y-r.y*axes[i].x)/maxTorque;
            Vec2 response=axes[i]*.45f+side*(1.8f*torque);
            // Modest exploration, slowing in a detected patch. Steering is
            // possible only with physical torque authority or oriented thrust.
            b.biases[Activation]=.3f;
            if(rhythm)b.weights[Activation][Oscillator]=.3f*relay;
            float count=float(food)+float(life);
            for(int ch:{int(EnergyX),int(CreatureX)}) {
                if((ch==EnergyX&&!food)||(ch==CreatureX&&!life))continue;
                b.weights[Activation][ch]=response.x*relay/std::max(1.f,count);
                b.weights[Activation][ch+1]=response.y*relay/std::max(1.f,count);
                b.weights[Activation][ch+2]=-.25f*relay/std::max(1.f,count);
            }
        }
        if(b.role==CellRole::Attacker){b.motorChannel=Activation;b.weights[Activation]={};b.biases[Activation]=.65f;}
    }
    return result;
}
// Opening, naming, or undoing an edit must not flatten a saved developmental
// program or replace a successful inherited controller.
inline Genome prepareCreatorRelease(Genome const& edited,Genome const& source) {
    return edited==editableBody(source) ? source : compileCreatorBody(edited);
}

}
