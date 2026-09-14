#include "SpecimenLibrary.h"
#include <cassert>
#include <iostream>
using namespace alienmobile;
int main(){@autoreleasepool {
    auto specimens=makeCuratedSpecimenCatalog();
    bool readOnly=false;
    NSData* data=writeSpecimens(specimens,0);auto restored=readSpecimens(data,readOnly);
    assert(!readOnly && restored.size()==specimens.size());
    for(size_t i=0;i<restored.size();++i){assert(restored[i].genome==specimens[i].genome);assert(restored[i].initialEnergy==specimens[i].initialEnergy);}
    NSMutableArray* records=[NSJSONSerialization JSONObjectWithData:data options:NSJSONReadingMutableContainers error:nil];
    [records addObject:@{@"name":@"Corrupt test",@"dna":@"corrupt",@"hue":@.5}];
    data=[NSJSONSerialization dataWithJSONObject:records options:0 error:nil];
    restored=readSpecimens(data,readOnly);assert(readOnly);
    readSpecimens([@"broken entire document" dataUsingEncoding:NSUTF8StringEncoding],readOnly);assert(readOnly);
    readSpecimens(nil,readOnly);assert(!readOnly);
    std::cout<<"Current native library: exact DNA and starting energy; corrupt document cannot be overwritten\n";
}}
