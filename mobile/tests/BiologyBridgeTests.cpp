#include "alienmobile/Creator.h"
#include "alienmobile/CreatureFocus.h"
#include "alienmobile/Rehearsal.h"
#include "alienmobile/GenomeIO.h"
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace alienmobile;
void empty(World& w){w.cells.clear();w.creatures.clear();w.connections.clear();w.angles.clear();w.motes.clear();w.lifeEvents.clear();}
void run(Rehearsal& r){while(r.steps<r.durationSteps){r.step();assert(r.world.allValuesFinite());assert(r.world.allConnectionsValid());}}
Vec2 center(World const& w){Vec2 p{};for(auto const& c:w.cells)p+=c.position;return p/float(w.cells.size());}
int main(int argc,char** argv){
    auto catalog=makeCuratedSpecimenCatalog();
    auto starter=makeCreatorStarterGenome();
    assert(validCreatorBody(starter));
    assert(measureDevelopment(starter).cells==1);
    assert(addBodyCell(starter,0,{1.1f,0}));
    assert(validCreatorBody(starter));
    for(auto s:catalog){
        auto body=editableBody(s.genome);auto source=s.genome;
        auto unchanged=prepareCreatorRelease(body,source);assert(unchanged==source);
        Rehearsal a(s);s.genome=unchanged;Rehearsal b(s);run(a);run(b);
        assert(a.world.cells==b.world.cells);assert(a.world.creatures==b.world.creatures);
        // Selecting the same role is a no-op, including all inherited parameters.
        for(size_t i=1;i<body.genes[0].nodes.size();++i)if(!body.genes[0].nodes[i].constructorCell){auto before=body;assert(changeOrgan(body,i,body.genes[0].nodes[i].behavior.role));assert(body==before);}
        size_t tip=body.genes[0].nodes.size()-1;
        assert(moveBodyCell(body,tip,body.genes[0].nodes[tip].relativePosition*1.01f));
        auto prepared=editableBody(prepareCreatorRelease(body,source));
        for(size_t i=0;i<body.genes[0].nodes.size();++i)assert(prepared.genes[0].nodes[i].behavior==body.genes[0].nodes[i].behavior);
        assert(prepareCreatorRelease(editableBody(source),source)==source); // undo
        s.genome=prepareCreatorRelease(body,source);Rehearsal edited(s);run(edited);
        auto organCopy=body;assert(changeOrgan(organCopy,tip,CellRole::Depot));
        s.genome=prepareCreatorRelease(organCopy,source);Rehearsal storageEdit(s);run(storageEdit);
        std::cout<<s.name<<" geometry/storage edits: finite; moved="<<length(edited.lastCenter-edited.startCenter)<<" storage births="<<storageEdit.completedChildren<<'\n';
        std::cout<<s.name<<" identical-copy 12s replay: exact; food="<<a.acquired<<" births="<<a.completedChildren<<'\n';
    }
    // Unique edits retain calls, unused genes, orientation, and construction timing.
    Genome program;program.genes.resize(3);program.genes[0].nodes={{-1,{},true},{0,{0,1},false}};
    program.genes[0].nodes[0].construction.targetGene=1;program.genes[0].nodes[0].construction.intervalScale=2;
    program.genes[1].orientation=.4f;program.genes[1].nodes={{-1,{1,0},false},{0,{1,0},false}};
    program.genes[2].nodes={{-1,{1,0},false}};
    auto body=editableBody(program);assert(changeOrgan(body,2,CellRole::Depot));
    auto retained=prepareCreatorRelease(body,program);auto expected=program;expected.genes[1].nodes[1].behavior=seededOrgan(CellRole::Depot);
    assert(retained==expected);
    auto shared=program;shared.genes[0].nodes[0].construction.branches=3;
    auto independent=editableBody(shared);assert(changeOrgan(independent,2,CellRole::Depot));
    assert(!retainedCreatorProgram(independent,shared));assert(prepareCreatorRelease(independent,shared)==independent);
    assert(prepareCreatorRelease(editableBody(shared),shared)==shared);
    // No external coupling: powered internal deformation changes shape, not COM.
    for(auto mode:{MotorMode::Bending,MotorMode::Contractile})for(float angle:{0.f,1.3f,-2.1f}){
        auto c=rehearsalConfig();World w(c);empty(w);
        Genome g;g.genes[0].nodes={{-1,{},true},{0,{1,0},false},{1,{1,.2f},false},{2,{1,0},false}};
        for(unsigned i=2;i<4;++i){auto& b=g.genes[0].nodes[i].behavior;b=seededOrgan(CellRole::Motor);b.motorMode=mode;b.biases[Activation]=.8f;}
        w.addFounder(g,{},angle,.5f);auto origin=center(w);auto initial=w.cells.back().position;CpuPhysicsBackend physics(c);
        double paid=0;for(int n=0;n<600;++n){updateSignals(w,c.fixedTimeStep);paid+=updateActuation(w,c.fixedTimeStep,c.motorEnergyCost);physics.step(w,c.fixedTimeStep);}
        assert(paid>0);assert(length(center(w)-origin)<.001f);assert(length(w.cells.back().position-initial)>.02f);
        std::cout<<"deformation mode="<<int(mode)<<" rotation="<<angle<<" COM="<<length(center(w)-origin)<<" tip="<<length(w.cells.back().position-initial)<<'\n';
    }
    // Geometry/direction is the only changed input; no controller compensation.
    Genome g;g.genes[0].nodes={{-1,{},true},{0,{1,0},false}};g.genes[0].nodes[1].behavior=seededOrgan(CellRole::Motor);
    auto c=rehearsalConfig();
    auto displacement=[&](Genome dna,float orientation){World w(c);empty(w);w.addFounder(dna,{},orientation,.5f);auto start=center(w);CpuPhysicsBackend physics(c);for(int i=0;i<600;++i){updateSignals(w,c.fixedTimeStep);updateActuation(w,c.fixedTimeStep,c.motorEnergyCost);physics.step(w,c.fixedTimeStep);}return center(w)-start;};
    for(float orientation:{0.f,.83f,-1.9f}){auto initial=displacement(g,orientation);auto turned=g;turned.genes[0].nodes[1].behavior.axisAngle=3.14159265f;auto other=displacement(prepareCreatorRelease(turned,g),orientation);auto disabled=g;disabled.genes[0].nodes[1].behavior.motorStrength=0;auto still=displacement(disabled,orientation);assert(length(initial)>1);assert(dot(initial,other)<-1);assert(length(still)<.0001f);std::cout<<"thrust rotation="<<orientation<<" distance="<<length(initial)<<" turned dot="<<dot(initial,other)<<" disabled="<<length(still)<<'\n';}
    // Event evidence survives loss and distinguishes attacks, depleted cells and capacity.
    {World w(c);empty(w);w.addFounder(g,{},0,.5f);auto id=w.creatures.back().id;Simulation sim(w,c);for(auto& cell:w.cells)cell.energy=0;
     for(int i=0;i<180;++i)sim.step();assert(sim.stats().starvationLosses>0);assert(sim.stats().damageLosses==0);assert(!w.lifeEvents.empty());assert(observedLifeMessage(w,id).find("energy")!=std::string::npos);}
    {auto legacy=c;legacy.localDamage=false;legacy.physicalResources=false;legacy.autonomousResources=false;legacy.energySourceStrength=0;legacy.hazardStrength=0;legacy.metabolismRate=0;World w(legacy);empty(w);w.addFounder(g,{},0,.5f);auto initialCells=w.cells.size();for(auto& cell:w.cells)cell.energy=0;Simulation sim(w,legacy);
     for(int i=0;i<180;++i)sim.step();assert(sim.stats().starvationLosses==initialCells);assert(w.lifeEvents.size()==initialCells);}
    {World w(c);empty(w);w.addFounder(g,{},0,.5f);w.cells.back().deathRequested=true;Simulation sim(w,c);sim.step();assert(sim.stats().damageLosses==1);assert(sim.stats().starvationLosses==0);}
    {auto cap=c;cap.maxCellCount=3;World w(cap);empty(w);w.addFounder(g,{},0,.5f);for(auto& cell:w.cells)cell.energy=1;Simulation sim(w,cap);sim.step();assert(sim.stats().capacityWaitSteps==1);assert(w.creatures[0].constructor.wait==ConstructorState::Capacity);assert(sim.stats().starvationLosses==0);}
    {World w(c);empty(w);auto s=catalog[0];s.genome=g;assert(w.addSpecimen(s,{100,100})==kInvalidId);assert(w.lastPlacementFailure==PlacementFailure::Bounds);assert(w.addSpecimen(s,{})!=kInvalidId);assert(w.addSpecimen(s,{})==kInvalidId);assert(w.lastPlacementFailure==PlacementFailure::Occupied);s.genome.genes.clear();assert(w.addSpecimen(s,{})==kInvalidId);assert(w.lastPlacementFailure==PlacementFailure::InvalidData);}
    // Selection keeps exact DNA through extinction and skips missing generations.
    {World w(c);empty(w);w.addFounder(g,{},0,.5f);auto founder=w.creatures.back().id;
     CreatureFocus focus;assert(focus.select(w,founder,"Test ancestor"));auto dna=focus.specimen->genome;
     w.addFounder(g,{5,0},0,.5f);auto& grandchild=w.creatures.back();grandchild.ancestorId=founder;grandchild.parentId=999;grandchild.generation=2;
     auto next=grandchild.id;assert(focus.nextRelative(w)==next);assert(focus.livingFamily(w)==2);
     w.creatures.erase(w.creatures.begin());assert(focus.nextRelative(w)==next);assert(focus.specimen->genome==dna);
     assert(!focus.select(w,999,"Missing"));assert(focus.id==founder);
     w.creatures.clear();assert(focus.nextRelative(w)==kInvalidId);assert(focus.specimen->genome==dna);
     w.recordEvent(LifeEventKind::StarvationLoss,founder);focus.observe(w);
     for(int i=0;i<140;++i)w.recordEvent(LifeEventKind::DamageLoss,999);
     focus.observe(w);assert(focus.lossMessage().find("energy")!=std::string::npos);
     focus.clear();assert(!focus.specimen && !focus.lastLoss);}
    // Preview startup and complete runs on a held-out shape, without changing DNA.
    std::vector<double> latencies;
    for(int i=0;i<40;++i){auto start=std::chrono::steady_clock::now();Rehearsal r(catalog[i%4]);latencies.push_back(std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count());}
    std::sort(latencies.begin(),latencies.end());std::cout<<"practice startup ms p50="<<latencies[20]<<" p95="<<latencies[38]<<'\n';assert(latencies[38]<50);
    if(argc>1){std::filesystem::create_directories(argv[1]);
        std::ofstream csv(std::string(argv[1])+"/rehearsal.csv");csv<<"example,seconds,x,y,absorbed,births,starvation,damage,capacity_steps\n";
        auto original=catalog[0];auto turn=original;auto edit=editableBody(turn.genome);for(auto& n:edit.genes[0].nodes)if(n.behavior.role==CellRole::Motor && n.behavior.motorMode==MotorMode::Thrust){n.behavior.axisAngle+=.785398f;if(n.behavior.axisAngle>3.141592f)n.behavior.axisAngle-=6.283185f;break;}turn.genome=prepareCreatorRelease(edit,original.genome);
        auto noMotor=original;edit=editableBody(noMotor.genome);for(size_t i=1;i<edit.genes[0].nodes.size();++i)if(edit.genes[0].nodes[i].behavior.role==CellRole::Motor)changeOrgan(edit,i,CellRole::Structural);noMotor.genome=prepareCreatorRelease(edit,original.genome);
        std::vector<SpecimenSnapshot> examples={original,turn,noMotor};const char* names[]={"original","turned","no-motor"};
        for(size_t k=0;k<examples.size();++k){std::ofstream dna(std::string(argv[1])+"/"+names[k]+".genome");genomeio::writeGenome(dna,examples[k].genome);Rehearsal r(examples[k]);while(r.steps<r.durationSteps){r.step();if(r.steps%120==0)csv<<names[k]<<','<<r.steps/120<<','<<r.lastCenter.x<<','<<r.lastCenter.y<<','<<r.acquired<<','<<r.completedChildren<<','<<r.simulation.stats().starvationLosses<<','<<r.simulation.stats().damageLosses<<','<<r.simulation.stats().capacityWaitSteps<<'\n';}}
    }
    auto emptyConfig=alienmobile::evolutionPlaytestConfig();
    emptyConfig.ecosystemSeed=false;
    emptyConfig.catalogSeed=false;
    emptyConfig.emptyStart=true;
    alienmobile::World emptyWorld(emptyConfig);
    assert(emptyWorld.creatures.empty());
    assert(emptyWorld.cells.empty());
    assert(!emptyWorld.motes.empty());
    std::cout<<"biology bridge causal checks passed\n";
}
