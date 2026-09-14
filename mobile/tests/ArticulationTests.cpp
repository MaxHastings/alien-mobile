#include "alienmobile/Simulation.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace alienmobile;
Genome articulatedBody(MotorMode mode,float stiffness) {
    Genome g;g.genes[0].nodes={{-1,{},true},{0,{1,0},false},{1,{1,0.2f},false},{2,{1,0},false}};
    g.genes[0].nodes[1].behavior.role=CellRole::Generator;
    g.genes[0].nodes[1].behavior.waveform=Waveform::Sine;
    for(std::size_t n=2;n<4;++n) {
        auto& node=g.genes[0].nodes[n];node.stiffness=stiffness;node.behavior.role=CellRole::Motor;
        node.behavior.motorMode=mode;node.behavior.motorStrength=0.8f;
    }
    return g;
}
float angle(World const& w,AngularConstraint const& a) {
    auto u=w.displacement(w.cells[a.center].position,w.cells[a.cellA].position);
    auto v=w.displacement(w.cells[a.center].position,w.cells[a.cellB].position);
    return std::atan2(u.x*v.y-u.y*v.x,dot(u,v));
}
Vec2 motion(Genome g) {
    auto c=depthPlaytestConfig();c.ecosystemSeed=false;c.emissionRate=0;c.hazardStrength=c.metabolismRate=0;c.constructionEnergy=10;
    World w(c);w.cells.clear();w.connections.clear();w.angles.clear();w.creatures.clear();w.motes.clear();
    w.addFounder(g,{},0,.5f);Simulation sim(w,c);
    auto center=[&](){Vec2 sum{};for(auto const& cell:w.cells)sum+=cell.position;return sum/float(w.cells.size());};
    auto initial=center();
    for(int s=0;s<600;++s) {for(auto& cell:w.cells)cell.energy=1.2f;sim.step();}
    return center()-initial;
}
int main() {
    Genome a;a.genes[0].nodes={{-1,{},true},{0,{1,0},false},{1,{1,0},false},{2,{0,1},false}};
    BehaviorGene motor;motor.role=CellRole::Motor;motor.motorMode=MotorMode::Thrust;
    motor.neural=true;motor.motorChannel=Activation;motor.biases[Activation]=1;
    a.genes[0].nodes[3].behavior=motor;
    auto b=a;b.genes[0].nodes[3].relativePosition={1,0};
    auto shifted=a;shifted.genes[0].nodes[3].behavior={};shifted.genes[0].nodes[1].behavior=motor;
    auto ma=motion(a),mb=motion(b),mc=motion(shifted);
    assert(length(ma-mb)>.2f && length(ma-mc)>.2f);
    std::cout<<"same controller morphology motion difference="<<length(ma-mb)<<" motor placement difference="<<length(ma-mc)<<'\n';
    for(float stiffness:{0.1f,1.f,2.f}) for(auto mode:{MotorMode::Bending,MotorMode::Contractile,MotorMode::Thrust}) {
        auto c=depthPlaytestConfig();c.ecosystemSeed=false;c.developmentalSeed=false;c.emissionRate=0;
        c.hazardStrength=c.metabolismRate=0;c.constructionEnergy=10;c.linearDrag=.35f;
        World w(c);w.cells.clear();w.connections.clear();w.angles.clear();w.creatures.clear();w.motes.clear();
        w.addFounder(articulatedBody(mode,stiffness),{},0,.5f);assert(w.angles.size()==2);
        auto initial=w.cells.back().position;Simulation sim(w,c);float maxError=0,maxSpeed=0;
        for(int s=0;s<2400;++s) {
            for(auto& cell:w.cells)cell.energy=1.2f;
            sim.step();assert(w.allConnectionsValid() && w.allValuesFinite());
            for(auto const& cell:w.cells)maxSpeed=std::max(maxSpeed,length(cell.velocity));
            auto const& a=w.angles[0];float d=angle(w,a)-a.baseAngle;
            maxError=std::max(maxError,std::abs(std::atan2(std::sin(d),std::cos(d))));
        }
        assert(maxSpeed<15);assert(sim.stats().motorEnergy>0);
        if(mode==MotorMode::Bending)assert(maxError>.1f);
        std::cout<<"mode="<<int(mode)<<" stiffness="<<stiffness<<" angle excursion="<<maxError
            <<" tip displacement="<<length(w.displacement(initial,w.cells.back().position))<<" maximum speed="<<maxSpeed<<" paid="<<sim.stats().motorEnergy<<'\n';
        for(auto& cell:w.cells)cell.energy=0;
        float spent=updateActuation(w,c.fixedTimeStep,c.motorEnergyCost);assert(spent==0);
        for(auto const& joint:w.angles)assert(joint.targetAngle==joint.baseAngle);
        auto id=w.creatures[0].id;assert(w.removeCreatureAndCells(id));assert(w.angles.empty());
    }
    // Angular forces are internal: zero external force conserves COM momentum.
    auto c=depthPlaytestConfig();c.ecosystemSeed=false;c.linearDrag=0;c.repulsionStrength=0;
    World w(c);w.cells.clear();w.connections.clear();w.angles.clear();w.creatures.clear();
    w.addFounder(articulatedBody(MotorMode::Bending,2),{},0,.5f);
    w.cells.back().position.y+=.5f;CpuPhysicsBackend physics(c);
    for(int s=0;s<1200;++s)physics.step(w,c.fixedTimeStep);
    Vec2 momentum{};for(auto const& cell:w.cells)momentum+=cell.velocity;
    assert(length(momentum)<.001f);
    std::cout<<"articulation assertions passed; residual momentum="<<length(momentum)<<'\n';
}
