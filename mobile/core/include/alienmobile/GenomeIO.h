#pragma once
#include "alienmobile/Genome.h"
#include <fstream>
#include "alienmobile/Development.h"
#include <iomanip>
#include <stdexcept>
#include <type_traits>
namespace alienmobile::genomeio {
using namespace alienmobile;
template<class T> void put(std::ostream& o,T v){if constexpr(std::is_enum_v<T>)o<<int(v)<<' ';else o<<v<<' ';}
template<class T> void get(std::istream& i,T& v){if constexpr(std::is_enum_v<T>){int n=0;i>>n;v=T(n);}else i>>v;}
inline void writeGenome(std::ostream& o,Genome const& g){o<<std::setprecision(9);put(o,g.entryGene);put(o,g.genes.size());
put(o,g.mutationRates.neural);put(o,g.mutationRates.geometry);put(o,g.mutationRates.property);put(o,g.mutationRates.role);put(o,g.mutationRates.insert);put(o,g.mutationRates.erase);put(o,g.mutationRates.duplicateGene);put(o,g.mutationRates.deleteGene);put(o,g.mutationRates.copySection);put(o,g.mutationRates.moveSection);put(o,g.mutationRates.constructor);put(o,g.mutationRates.meta);put(o,g.mutationRates.neuralMagnitude);put(o,g.mutationRates.geometryMagnitude);put(o,g.mutationRates.propertyMagnitude);
for(auto const& gene:g.genes){put(o,gene.orientation);put(o,gene.phaseAdvance);put(o,gene.nodes.size());
for(auto const& n:gene.nodes){put(o,n.parentNode);put(o,n.relativePosition.x);put(o,n.relativePosition.y);put(o,n.constructorCell);put(o,n.stiffness);
auto const& c=n.construction;put(o,c.targetGene);put(o,c.branches);put(o,c.repetitions);put(o,c.angle);put(o,c.branchAngle);put(o,c.repetitionAngle);put(o,c.intervalScale);put(o,c.separateOffspring);auto const& b=n.behavior;
put(o,b.role);put(o,b.waveform);put(o,b.motorMode);put(o,b.period);put(o,b.phase);put(o,b.amplitude);put(o,b.bendingAngle);put(o,b.contraction);put(o,b.motorStrength);put(o,b.axisAngle);put(o,b.signalWeight);put(o,b.motorChannel);put(o,b.sensorRange);put(o,b.sensitivity);put(o,b.extractionRate);put(o,b.digestionRate);put(o,b.storageCapacity);put(o,b.defenseStrength);put(o,b.memoryMode);put(o,b.memoryTime);put(o,b.neural);put(o,b.selfWeight);for(auto const& row:b.weights)for(auto v:row)put(o,v);for(auto v:b.biases)put(o,v);o<<"\n";}}}
inline Genome readGenome(std::istream& i){Genome g;size_t count=0;get(i,g.entryGene);get(i,count);if(count>16)throw std::runtime_error("modules");g.genes.resize(count);
get(i,g.mutationRates.neural);get(i,g.mutationRates.geometry);get(i,g.mutationRates.property);get(i,g.mutationRates.role);get(i,g.mutationRates.insert);get(i,g.mutationRates.erase);get(i,g.mutationRates.duplicateGene);get(i,g.mutationRates.deleteGene);get(i,g.mutationRates.copySection);get(i,g.mutationRates.moveSection);get(i,g.mutationRates.constructor);get(i,g.mutationRates.meta);get(i,g.mutationRates.neuralMagnitude);get(i,g.mutationRates.geometryMagnitude);get(i,g.mutationRates.propertyMagnitude);for(auto& gene:g.genes){get(i,gene.orientation);get(i,gene.phaseAdvance);get(i,count);if(count>64)throw std::runtime_error("nodes");gene.nodes.resize(count);
for(auto& n:gene.nodes){get(i,n.parentNode);get(i,n.relativePosition.x);get(i,n.relativePosition.y);get(i,n.constructorCell);get(i,n.stiffness);
auto& c=n.construction;get(i,c.targetGene);get(i,c.branches);get(i,c.repetitions);get(i,c.angle);get(i,c.branchAngle);get(i,c.repetitionAngle);get(i,c.intervalScale);get(i,c.separateOffspring);auto& b=n.behavior;
get(i,b.role);get(i,b.waveform);get(i,b.motorMode);get(i,b.period);get(i,b.phase);get(i,b.amplitude);get(i,b.bendingAngle);get(i,b.contraction);get(i,b.motorStrength);get(i,b.axisAngle);get(i,b.signalWeight);get(i,b.motorChannel);get(i,b.sensorRange);get(i,b.sensitivity);get(i,b.extractionRate);get(i,b.digestionRate);get(i,b.storageCapacity);get(i,b.defenseStrength);get(i,b.memoryMode);get(i,b.memoryTime);get(i,b.neural);get(i,b.selfWeight);for(auto& row:b.weights)for(auto& v:row)get(i,v);for(auto& v:b.biases)get(i,v);}}if(!i)throw std::runtime_error("truncated genome");if(!isValidDevelopmentGenome(g) || !measureDevelopment(g).complete())throw std::runtime_error("invalid genome");return g;}
}
