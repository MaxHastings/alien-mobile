#include "alienmobile/Development.h"
#include <cassert>
#include <iostream>
using namespace alienmobile;
int main() {
    auto g=makePrimitiveGenome();
    g.mutationRates.neural=.5f;g.mutationRates.geometry=.5f;g.mutationRates.property=.5f;g.mutationRates.meta=.01f;
    DeterministicRng rng(1404);unsigned meta=0,ordinary=0;
    for(unsigned n=0;n<20000;++n) {
        auto child=mutateDevelopmentGenome(g,rng);
        assert(isValidDevelopmentGenome(child.genome));meta+=child.metaMutated;
        ordinary+=child.kind!=MutationKind::None && child.kind!=MutationKind::Meta;
    }
    std::cout<<"meta="<<meta<<" ordinary="<<ordinary<<std::endl;
    assert(meta>100 && meta<320 && ordinary>14000);
    auto frozen=g;frozen.mutationRates={0,0,0,0,0,0,0,0,0,0,0,0};
    for(unsigned n=0;n<1000;++n)assert(mutateDevelopmentGenome(frozen,rng).genome==frozen);
    auto zeroStep=g;zeroStep.mutationRates.neuralMagnitude=0;
    for(unsigned n=0;n<100;++n)assert(applyDevelopmentMutation(zeroStep,rng,MutationKind::Behavior).genome==zeroStep);
    // Independent inherited regimes use the same operator, without ecological
    // selection or a novelty objective. This tests tempo, not D's long-run gate.
    unsigned slow=0,fast=0;auto a=frozen,b=frozen;
    a.mutationRates.geometry=.02f;b.mutationRates.geometry=.4f;
    for(unsigned n=0;n<20000;++n) {
        slow+=mutateDevelopmentGenome(a,rng).mutated();fast+=mutateDevelopmentGenome(b,rng).mutated();
    }
    assert(fast>slow*8);
    auto lineage=g;for(unsigned n=0;n<5000;++n) {
        auto child=applyDevelopmentMutation(lineage,rng,MutationKind::Meta);
        assert(validMutationRates(child.genome.mutationRates));lineage=child.genome;
    }
    assert(!(lineage.mutationRates==g.mutationRates));
    std::cout<<"saturated ordinary regime: meta="<<meta<<" ordinary="<<ordinary
        <<" inherited geometry regime changed counts="<<slow<<','<<fast<<" bounded meta steps=5000\n";
}
