#include "alienmobile/Simulation.h"
#include "DiscoveryGenomeIO.h"
#include <cassert>
#include <fstream>
#include <iostream>
using namespace alienmobile;
int main(){
 auto catalog=makeCuratedSpecimenCatalog();assert(catalog.size()==4);
 for(auto const& s:catalog){std::ifstream input(std::string(ALIEN_DISCOVERY_ARCHIVE)+"/"+s.name+".dna");auto archived=discovery::readGenome(input);
 assert(archived==s.genome);assert(s.initialEnergy==1);assert(isValidDevelopmentGenome(archived));
 auto c=evolutionPlaytestConfig();c.catalogSeed=false;c.ecosystemSeed=false;World w(c);w.cells.clear();w.connections.clear();w.angles.clear();w.creatures.clear();
 auto id=w.addSpecimen(s,{0,0});assert(id!=kInvalidId);assert(w.findCreature(id)->genome==archived);
 }
 std::cout<<"Every playable genome exactly equals its archived simulation descendant; uniform placement reserve.\n";
}
