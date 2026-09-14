#include "CompetitionResearch.h"
#include <chrono>
#include <iostream>
int main(int argc,char** argv) {
    using namespace competition;
    if(argc<5){std::cerr<<"usage: CompetitionResearch seed seconds stem fixed|small|full [sample_seconds]\n";return 2;}
    auto c=referenceConfig(std::stoull(argv[1]));World w(c);initialRegime(w,argv[4]);Simulation sim(w,c);Observer observer;
    double duration=std::stod(argv[2]);std::string stem=argv[3];
    unsigned sampleSeconds=argc>5 ? std::stoul(argv[5]):60;if(!sampleSeconds || duration<=0)return 2;
    std::ofstream pop(stem+"-population.csv"),life(stem+"-lives.csv");if(!pop || !life)return 2;
    pop<<"seconds,ancestry,mature,juvenile,cells,inherited_cells,intact_sensor_motor_bodies,moving_bodies,motors,sensors,energy,created,born,juvenile_deaths,adult_deaths,observed_uptake,max_generation,bed0,bed1,bed2,bed3,energy_error,cap\n";
    auto start=std::chrono::steady_clock::now();observer.observe(w,0);observer.sample(w,sim,pop);
    for(uint64_t step=1;step<=uint64_t(duration/c.fixedTimeStep);++step){sim.step();observer.observe(w,step);
        if(step%(sampleSeconds*120)==0){if(!w.allValuesFinite() || !w.allConnectionsValid())throw std::runtime_error("invalid world");observer.sample(w,sim,pop);pop.flush();}
        if(w.creatures.empty())break;}
    observer.sample(w,sim,pop);observer.writeLives(life);
    std::cout<<"seed="<<c.randomSeed<<" regime="<<argv[4]<<" simulated_seconds="<<sim.stats().steps*double(c.fixedTimeStep)<<" wall_seconds="<<std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count()<<" births="<<sim.stats().births<<" maximum_energy_error="<<observer.maxEnergyError<<" outcome_hash="<<stateHash(w)<<'\n';
    return observer.maxEnergyError>.1 ? 1:0;
}
