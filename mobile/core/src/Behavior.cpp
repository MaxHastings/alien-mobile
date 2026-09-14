#include "alienmobile/Behavior.h"
#include "alienmobile/World.h"
#include "alienmobile/Ecology.h"
#include <cmath>
#include <algorithm>
#include "alienmobile/SpatialGrid.h"
#include "alienmobile/ConnectionIndex.h"

namespace alienmobile {
Vec2 cellAxis(World const& world, std::size_t index) {
    auto const& cell = world.cells[index];
    auto const* creature = world.findCreature(cell.creatureId);
    if (!creature || cell.genomeNode >= creature->bodyNodes.size()) return {};
    auto const parentNode = creature->bodyNodes[cell.genomeNode].parentNode;
    auto reference = parentNode >= 0 ? world.findCellForGenomeNode(cell.creatureId, parentNode)
                                     : world.findCellForGenomeNode(cell.creatureId, 1);
    if (reference == kInvalidId || !world.hasConnection(static_cast<uint32_t>(index),reference)) return {}; // No physical reference: no thrust.
    auto edge = parentNode >= 0 ? world.displacement(world.cells[reference].position,cell.position)
                               : world.displacement(cell.position,world.cells[reference].position);
    if (length(edge) < 0.00001f) return {};
    auto axis = normalizedOr(edge);
    float c = std::cos(cell.behavior.axisAngle), s = std::sin(cell.behavior.axisAngle);
    return {axis.x*c-axis.y*s, axis.x*s+axis.y*c};
}
Signals energySensorQuery(World const& world, std::size_t index, SpatialGrid const& grid, std::vector<unsigned>& contacts) {
    auto const& cell=world.cells[index];auto axis=cellAxis(world,index);
    Vec2 side{-axis.y,axis.x}; Vec2 gradient{}; float intensity=0;
    if(world.config().physicalResources) {
        grid.query(cell.position,cell.behavior.sensorRange,contacts);
        for(auto m:contacts) {auto const& mote=world.motes[m];
            auto d=world.displacement(cell.position,mote.position);float distance=length(d);
            if(distance>=cell.behavior.sensorRange) continue;
            float weight=mote.energy*(1-distance/cell.behavior.sensorRange);
            intensity+=weight; if(distance>0.0001f) gradient+=d*(weight/distance);
        }
        float gain=cell.behavior.sensitivity;
        // Keep gradient magnitude: a uniform patch should not invent a direction.
        Signals out{};
        out[EnergyX]=clamp(dot(gradient,axis)*gain,-1.f,1.f);
        out[EnergyY]=clamp(dot(gradient,side)*gain,-1.f,1.f);
        out[EnergyIntensity]=clamp(intensity*gain,0.f,1.f);
        return out;
    }
    // Sample the resource field at finite local receptors. No source coordinate
    // or target identity is returned to a controller.
    for(int ring=1;ring<=2;++ring) for(int n=0;n<16;++n) {
        float angle=n*6.28318530718f/16;
        Vec2 direction{std::cos(angle),std::sin(angle)};
        Vec2 sample=cell.position+direction*(cell.behavior.sensorRange*ring/2);
        float edge=clamp(1-length(sample-world.energySource.position)/std::max<double>(0.001f,world.energySource.radius),0.0f,1.0f);
        float value=edge*edge*(3-2*edge)*world.energySource.strength;
        gradient+=direction*value;intensity+=value;
    }
    intensity=clamp(intensity*cell.behavior.sensitivity/4,0.0f,1.0f);
    Vec2 direction=length(gradient)>0.00001f ? normalizedOr(gradient)*intensity : Vec2{};
    Signals output{};output[EnergyX]=dot(direction,axis);output[EnergyY]=dot(direction,side);
    output[EnergyIntensity]=intensity;return output;
}
Signals creatureSensorQuery(World const& world, std::size_t index, SpatialGrid const& grid, std::vector<unsigned>& contacts) {
    auto const& sensor=world.cells[index];auto axis=cellAxis(world,index);Vec2 side{-axis.y,axis.x};
    float best=sensor.behavior.sensorRange;Vec2 direction{};bool found=false;
    uint32_t bestId=kInvalidId;
    // Nearest foreign physical cell; stable ID resolves exact-distance ties.
    // No target identity or species classification is sent to the network.
    auto const* owner=world.findCreature(sensor.creatureId);
    grid.query(sensor.position,sensor.behavior.sensorRange,contacts);
    for(auto n:contacts) {auto const& cell=world.cells[n];
        if(&cell==&sensor || !cell.alive || (sensor.behavior.role!=CellRole::ObstacleSensor && cell.creatureId==sensor.creatureId)
            || (owner && cell.creatureId==owner->constructor.offspringCreatureId)) continue;
        auto d=world.displacement(sensor.position,cell.position);float distance=length(d);
        if(distance<best || (distance==best && cell.id<bestId)) {
            best=distance;bestId=cell.id;direction=normalizedOr(d);found=true;
        }
    }
    Signals out{};
    if(found) {
        float intensity=clamp((1-best/sensor.behavior.sensorRange)*sensor.behavior.sensitivity,0.f,1.f);
        out[CreatureX]=dot(direction,axis)*intensity;out[CreatureY]=dot(direction,side)*intensity;
        out[CreatureIntensity]=intensity;
    }
    return out;
}
Signals energySensor(World const& world,std::size_t index) {std::vector<unsigned> scratch;return energySensorQuery(world,index,SpatialGrid(world.motes,world.config()),scratch);}
Signals creatureSensor(World const& world,std::size_t index) {std::vector<unsigned> scratch;return creatureSensorQuery(world,index,SpatialGrid(world.cells,world.config()),scratch);}
float generatorOutput(BehaviorGene const& gene, double age) {
    double cycle = age / gene.period + gene.phase;
    cycle -= std::floor(cycle);
    return gene.amplitude * (gene.waveform == Waveform::Sine
        ? static_cast<float>(std::sin(cycle * 6.283185307179586)) : (cycle < 0.5 ? 1.0f : -1.0f));
}
void updateSignals(World& world, float dt) {
    SpatialGrid cellGrid(world.cells,world.config()),moteGrid(world.motes,world.config());
    std::vector<unsigned> contacts;
    for (auto& cell : world.cells) cell.nextSignals.fill(0);
    // Broadcast the previous committed signal state. Payment and reception use
    // separate passes so vector order cannot create same-tick feedback.
    std::vector<Signals> broadcasts(world.cells.size());
    for(std::size_t i=0;i<world.cells.size();++i) {
        auto& sender=world.cells[i];auto owner=world.findCreature(sender.creatureId);
        if(sender.behavior.role!=CellRole::Sender || !owner || !world.organsActive(*owner))continue;
        float magnitude=0;for(float value:sender.currentSignals)magnitude+=std::abs(value)/SignalChannelCount;
        double cost=(.002f+.001f*sender.behavior.sensorRange*sender.behavior.sensorRange*magnitude)*dt;
        double paid=std::min<double>(sender.energy,cost);sender.energy-=paid;world.energyLedger.organCost+=paid;
        for(std::size_t k=0;k<SignalChannelCount;++k)broadcasts[i][k]=sender.currentSignals[k]*(cost>0 ? paid/cost : 0);
    }
    // Propagate only genomic edges. Mechanical braces and birth tethers carry
    // no signals. Each child gene owns the bidirectional connection weight.
    for (auto const& connection : world.connections) {
        auto& a = world.cells[connection.cellA]; auto& b = world.cells[connection.cellB];
        if (!a.alive || !b.alive || a.creatureId != b.creatureId) continue;
        auto const* creature = world.findCreature(a.creatureId);
        if (!creature || !world.organsActive(*creature)) continue;
        auto const& nodes = creature->bodyNodes;
        if (a.genomeNode >= nodes.size() || b.genomeNode >= nodes.size()) continue;
        float weight = 0;
        if (nodes[a.genomeNode].parentNode == static_cast<int>(b.genomeNode)) weight = a.behavior.signalWeight;
        else if (nodes[b.genomeNode].parentNode == static_cast<int>(a.genomeNode)) weight = b.behavior.signalWeight;
        for (std::size_t k=0; k<SignalChannelCount; ++k) {
            a.nextSignals[k] += b.currentSignals[k]*weight;
            b.nextSignals[k] += a.currentSignals[k]*weight;
        }
    }
    for (std::size_t index=0; index<world.cells.size(); ++index) {
        auto& cell=world.cells[index];
        auto const* creature = world.findCreature(cell.creatureId);
        if (!cell.alive || !creature || !world.organsActive(*creature)) continue;
        auto const& gene=cell.behavior;
        Signals input=cell.nextSignals;
        for(std::size_t k=0;k<SignalChannelCount;++k) input[k]+=cell.currentSignals[k]*gene.selfWeight;
        if(gene.role==CellRole::EnergySensor) {
            auto local=energySensorQuery(world,index,moteGrid,contacts);
            for(std::size_t k=EnergyX;k<=EnergyIntensity;++k) input[k]=local[k];
        }
        if(gene.role==CellRole::CreatureSensor || gene.role==CellRole::ObstacleSensor) {
            auto local=creatureSensorQuery(world,index,cellGrid,contacts);
            for(std::size_t k=CreatureX;k<=CreatureIntensity;++k) input[k]=local[k];
        }
        if(gene.role==CellRole::Receiver) {
            double cost=.002f*dt;double paid=std::min<double>(cell.energy,cost);cell.energy-=paid;world.energyLedger.organCost+=paid;
            cellGrid.query(cell.position,gene.sensorRange,contacts);
            for(auto s:contacts) {
                auto const& sender=world.cells[s];
                if(sender.creatureId==cell.creatureId || sender.behavior.role!=CellRole::Sender)continue;
                float range=std::min<double>(gene.sensorRange,sender.behavior.sensorRange);
                float attenuation=std::max<double>(0.f,1-length(world.displacement(cell.position,sender.position))/range);
                for(std::size_t k=0;k<SignalChannelCount;++k)input[k]+=broadcasts[s][k]*attenuation*(cost>0 ? paid/cost : 0);
            }
        }
        if(gene.role==CellRole::Memory) {
            if(gene.memoryMode==MemoryMode::Integrate) {
                float blend=1-std::exp(-dt/gene.memoryTime);
                for(std::size_t k=0;k<SignalChannelCount;++k) {
                    cell.memoryState[k]+=blend*(clamp(input[k],-1.f,1.f)-cell.memoryState[k]);
                    input[k]=cell.memoryState[k];
                }
            } else {
                if(cell.signalHistory.size()!=256)cell.signalHistory.assign(256,Signals{});
                auto delay=std::size_t(clamp(std::round(gene.memoryTime/dt),1.f,255.f));
                auto previous=cell.signalHistory[(cell.historyCursor+256-delay)%256];
                for(auto& value:input)value=clamp(value,-1.f,1.f);
                cell.signalHistory[cell.historyCursor]=input;
                cell.historyCursor=(cell.historyCursor+1)%256;input=previous;
            }
        }
        if(gene.neural) {
            for(std::size_t row=0;row<SignalChannelCount;++row) {
                float value=gene.biases[row];
                for(std::size_t col=0;col<SignalChannelCount;++col) value+=gene.weights[row][col]*input[col];
                cell.nextSignals[row]=std::tanh(value);
            }
        } else {
            // Residual controller: zero DNA is the original relay, and a
            // single weight/bias edit changes it continuously. New organs need
            // not survive several coordinated mutations before any signal can
            // reach their actuator. Replacement-mode networks remain available.
            for(std::size_t row=0;row<SignalChannelCount;++row) {
                float correction=gene.biases[row];
                for(std::size_t col=0;col<SignalChannelCount;++col)correction+=gene.weights[row][col]*input[col];
                cell.nextSignals[row]=clamp(input[row]+std::tanh(correction),-1.f,1.f);
            }
        }
        if (cell.behavior.role == CellRole::Generator)
            cell.nextSignals[Oscillator] = generatorOutput(cell.behavior, cell.signalAge);
        cell.signalAge += dt;
    }
    for (auto& cell : world.cells) cell.currentSignals = cell.nextSignals;
}
double updateActuation(World& world, float dt, float energyPerForceSecond) {
    double spent = 0;
    ConnectionIndex edges(world.cells.size(),world.connections);
    std::vector<uint32_t> firstJoint(world.cells.size(),kInvalidId),nextJoint(world.angles.size(),kInvalidId);
    for(auto& connection:world.connections) connection.restLength=connection.baseRestLength;
    for(std::size_t n=world.angles.size();n>0;--n) {
        auto& joint=world.angles[n-1];joint.targetAngle=joint.baseAngle;
        nextJoint[n-1]=firstJoint[joint.cellB];firstJoint[joint.cellB]=uint32_t(n-1);
    }
    for (std::size_t i=0; i<world.cells.size(); ++i) {
        auto& cell = world.cells[i]; cell.thrust = environmentCurrent(world,cell.position)*world.config().linearDrag;cell.motorThrust={};
        if (!cell.alive || cell.behavior.role != CellRole::Motor) continue;
        auto const* creature = world.findCreature(cell.creatureId);
        if (!creature || !world.organsActive(*creature)) continue;
        if(cell.behavior.motorMode==MotorMode::Bending) {
            for(auto n=firstJoint[i];n!=kInvalidId;n=nextJoint[n]) {
                auto& joint=world.angles[n];
                float activation=cell.currentSignals[cell.behavior.motorChannel];
                float amount=cell.behavior.bendingAngle*clamp(cell.behavior.motorStrength,0.f,1.f);
                double cost=std::abs(activation)*amount*joint.stiffness*world.config().bendingEnergyCost*dt;
                double paid=std::min<double>(cell.energy,cost);double fraction=cost>0 ? paid/cost : 0;
                joint.targetAngle=joint.baseAngle+activation*amount*fraction;
                cell.energy-=paid;spent+=paid;
            }
            continue;
        }
        if(cell.behavior.motorMode==MotorMode::Contractile) {
            auto node=creature->bodyNodes[cell.genomeNode].parentNode;
            if(node<0) continue;
            auto parent=world.findCellForGenomeNode(cell.creatureId,node);
            for(auto slot=edges.first(i);slot!=kInvalidId;slot=edges.next(slot)) {
                auto& connection=world.connections[slot/2];
                if(!((connection.cellA==i && connection.cellB==parent)||(connection.cellB==i && connection.cellA==parent))) continue;
                float activation=cell.currentSignals[cell.behavior.motorChannel];
                float amount=cell.behavior.contraction*clamp(cell.behavior.motorStrength,0.f,1.f);
                double cost=std::abs(activation)*amount*world.config().contractileEnergyCost*dt;
                double fraction=cost>0 ? std::min<double>(1.f,cell.energy/cost) : 0;
                connection.restLength=connection.baseRestLength*(1+activation*amount*fraction);
                cell.energy=std::max<double>(0.f,cell.energy-cost*fraction);spent+=cost*fraction;
            }
            continue;
        }
        // Unidirectional propulsor: negative output closes the motor.
        float force = std::max<double>(0.0f, cell.currentSignals[cell.behavior.motorChannel]) * cell.behavior.motorStrength;
        auto axis = cellAxis(world, i);
        if (length(axis) < 0.5f) continue;
        double cost = force * energyPerForceSecond * dt;
        double fraction = cost > 0 ? std::min<double>(1.0f, cell.energy/cost) : 0.0f;
        cell.motorThrust=axis * (force*fraction);cell.thrust += cell.motorThrust;
        cell.energy = std::max<double>(0.0f, cell.energy-cost*fraction);
        spent += cost*fraction;
    }
    return spent;
}
Genome makeLocomotionGenome() {
    Genome g;
    g.genes[0].nodes = {{-1,{},true},{0,{1,0},false},{1,{1,0},false},{0,{0,1},false}};
    g.genes[0].nodes[0].behavior.role = CellRole::Constructor;
    g.genes[0].nodes[1].behavior.role = CellRole::Generator;
    g.genes[0].nodes[2].behavior.role = CellRole::Motor;
    return g;
}
Genome makeFeederGenome() {
    Genome g;
    g.genes[0].nodes={{-1,{},true},{0,{1,0},false},{1,{-1,0.85f},false},{1,{-1,-0.85f},false}};
    g.genes[0].nodes[0].behavior.role=CellRole::Constructor;
    g.genes[0].nodes[1].behavior.role=CellRole::EnergySensor;
    g.genes[0].nodes[1].behavior.sensitivity=2.0f;
    for(int n=2;n<=3;++n) {
        auto& gene=g.genes[0].nodes[n].behavior;gene.role=CellRole::Motor;
        gene.axisAngle=-std::atan2(g.genes[0].nodes[n].relativePosition.y,g.genes[0].nodes[n].relativePosition.x);
        gene.motorStrength=1.3f;gene.motorChannel=Activation;gene.neural=true;
        gene.weights[Activation][EnergyX]=0.7f;
        gene.weights[Activation][EnergyY]=n==2 ? -2.2f : 2.2f;
        gene.biases[Activation]=n==2 ? 0.10f : 0.20f;
    }
    return g;
}

Genome makeContractileFeederGenome() {
    auto g=makeFeederGenome();
    // A flexible tail changes the loading and orientation of a thrust swimmer.
    // Internal springs alone cannot propel COM in this isotropic-drag medium.
    g.genes[0].nodes.push_back({0,{-1,0},false});g.genes[0].nodes[4].behavior.role=CellRole::Generator;
    g.genes[0].nodes[4].behavior.period=2.8f;g.genes[0].nodes[4].behavior.waveform=Waveform::Sine;
    g.genes[0].nodes.push_back({4,{-1,0.25f},false});auto& b=g.genes[0].nodes[5].behavior;
    b.role=CellRole::Motor;b.motorMode=MotorMode::Contractile;b.motorChannel=Oscillator;
    b.contraction=0.35f;
    return g;
}
Genome makeHunterGenome() {
    auto g=makeFeederGenome();
    g.genes[0].nodes[1].behavior.role=CellRole::CreatureSensor;g.genes[0].nodes[1].behavior.sensorRange=6;
    g.genes[0].nodes[1].behavior.sensitivity=1.5f;
    for(int n=2;n<=3;++n) {
        auto& b=g.genes[0].nodes[n].behavior;b.weights={};
        b.weights[Activation][CreatureX]=0.9f;
        b.weights[Activation][CreatureY]=n==2 ? -2.6f : 2.6f;
        b.motorStrength=2.6f;
    }
    g.genes[0].nodes.push_back({1,{0.75f,0},false});auto& attacker=g.genes[0].nodes.back().behavior;
    attacker.role=CellRole::Attacker;attacker.neural=true;attacker.biases[Activation]=2;
    g.genes[0].nodes.push_back({0,{-0.8f,0},false});g.genes[0].nodes.back().behavior.role=CellRole::Digestor;
    return g;
}

std::vector<Genome> makeGardenFounders() {
    // Catalog genomes are ordinary initial conditions. Geometry is functional:
    // contact area, drag, torque, diffusion paths and attack exposure all count.
    auto dart=makeFeederGenome();
    for(int n=2;n<=3;++n) {
        auto& b=dart.genes[0].nodes[n].behavior;
        b.weights[Activation][EnergyX]=0;
        b.weights[Activation][EnergyIntensity]=-.15f;
        b.biases[Activation]=.35f;
    }

    // A serial body: a driven nose pulls a long, compliant collector.
    // Length intercepts food, but diffusion and drag penalize distant cells.
    auto ribbon=makeFeederGenome();
    for(int n=2;n<=3;++n) ribbon.genes[0].nodes[n].behavior.motorStrength=2.1f;
    ribbon.genes[0].nodes.push_back({0,{-1.25f,0},false});
    for(int n=5;n<9;++n) {
        ribbon.genes[0].nodes.push_back({n-1,{-1.25f,n%2 ? .28f : -.28f},false});
        ribbon.genes[0].nodes.back().stiffness=.35f;
    }

    // Three repeated arms, each with an exposed tangential propulsor. Their
    // torques add while their translation cancels: a rotating interception fan.
    Genome crown;crown.genes.resize(2);
    crown.genes[0].nodes={{-1,{},true}};
    auto& call=crown.genes[0].nodes[0].construction;
    call.targetGene=1;call.branches=3;call.branchAngle=2.0943951f;
    crown.genes[1].nodes={{-1,{1.1f,0},false},{0,{1.1f,0},false}};
    crown.genes[1].nodes[0].behavior.role=CellRole::Depot;
    crown.genes[1].nodes[0].behavior.storageCapacity=.8f;
    auto& spinner=crown.genes[1].nodes[1].behavior;
    spinner.role=CellRole::Motor;spinner.neural=true;spinner.motorChannel=Activation;
    spinner.biases[Activation]=.65f;spinner.axisAngle=1.5707963f;spinner.motorStrength=.8f;

    // A stationary reservoir: reserve cells directly touch the constructor.
    // No receptor or propulsion tax, but no ability to chase a departing patch.
    Genome vault;vault.genes[0].nodes={{-1,{},true},{0,{.82f,0},false},
        {0,{-.41f,.71f},false},{0,{-.41f,-.71f},false},{1,{.75f,0},false}};
    for(int n=1;n<=3;++n) {
        auto& b=vault.genes[0].nodes[n].behavior;b.role=CellRole::Depot;b.storageCapacity=2.4f;
    }
    vault.genes[0].nodes[4].behavior.role=CellRole::Defender;
    vault.genes[0].nodes[4].behavior.defenseStrength=1.2f;

    auto lancer=makeHunterGenome();
    // Keep mouth ahead of the sensor, fins behind it, and digestion immediately
    // beside the mouth so raw material crosses one edge before conversion.
    lancer.genes[0].nodes[5].parentNode=4;
    lancer.genes[0].nodes[5].relativePosition={.7f,.45f};
    lancer.genes[0].nodes[4].behavior.extractionRate=1.0f;
    for(int n=2;n<=3;++n) {
        auto& m=lancer.genes[0].nodes[n].behavior;m.motorStrength=2.2f;
        m.weights[Activation][CreatureX]=.9f;
        m.biases[Activation]=n==2 ? .05f : .09f;
    }
    MutationRates gardenTempo;
    gardenTempo.neural=.22f; gardenTempo.property=.12f; gardenTempo.geometry=.045f;
    gardenTempo.role=.002f; gardenTempo.insert=.0015f; gardenTempo.erase=.001f;
    gardenTempo.duplicateGene=.00025f; gardenTempo.deleteGene=.00015f;
    gardenTempo.copySection=.00025f; gardenTempo.moveSection=.00025f;
    gardenTempo.constructor=.0005f; gardenTempo.meta=.00005f;
    // Small changes remain small even when they accumulate over a lineage.
    gardenTempo.neuralMagnitude=.75f; gardenTempo.geometryMagnitude=.65f;
    gardenTempo.propertyMagnitude=.70f;
    std::vector<Genome> founders={dart,ribbon,crown,vault,lancer};
    for(auto& founder:founders) founder.mutationRates=gardenTempo;
    return founders;
}

} // namespace alienmobile
