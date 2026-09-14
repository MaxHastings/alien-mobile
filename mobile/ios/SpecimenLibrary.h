#pragma once
#import <Foundation/Foundation.h>
#include "alienmobile/GenomeIO.h"
#include <sstream>
namespace alienmobile {
// Current candidate format only. A corrupt document is read-only so a save
// cannot silently overwrite data that failed validation.
inline std::vector<SpecimenSnapshot> readSpecimens(NSData* data,bool& readOnly) {
    std::vector<SpecimenSnapshot> result;readOnly=false;
    if(!data)return result;
    if(data.length>16*1024*1024){readOnly=true;return result;}
    id records=[NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
    if(![records isKindOfClass:NSArray.class]){readOnly=true;return result;}
    for(id record in records){bool accepted=false;
        if(result.size()<100 && [record isKindOfClass:NSDictionary.class]
            && [record[@"dna"] isKindOfClass:NSString.class] && [record[@"name"] isKindOfClass:NSString.class]
            && [record[@"hue"] isKindOfClass:NSNumber.class]) {
            try {
                std::istringstream input([record[@"dna"] UTF8String]);auto genome=genomeio::readGenome(input);
                double hue=[record[@"hue"] doubleValue];
                if(![record[@"initialEnergy"] isKindOfClass:NSNumber.class])throw std::runtime_error("energy");
                double energy=[record[@"initialEnergy"] doubleValue];
                if(!std::isfinite(hue)||hue<0||hue>1||!std::isfinite(energy)||energy<=0)throw std::runtime_error("values");
                NSString* origin=[record[@"origin"] isKindOfClass:NSString.class]?record[@"origin"]:@"Mine";
                result.push_back({[record[@"name"] UTF8String],genome,float(hue),float(energy),[origin UTF8String]});accepted=true;
            }catch(std::exception const&){}
        }
        if(!accepted)readOnly=true;
    }
    return result;
}
inline NSData* writeSpecimens(std::vector<SpecimenSnapshot> const& library,NSUInteger begin) {
    NSMutableArray* records=[NSMutableArray array];
    for(size_t i=begin;i<library.size();++i){auto const& s=library[i];std::ostringstream output;genomeio::writeGenome(output,s.genome);
        [records addObject:@{@"name":[NSString stringWithUTF8String:s.name.c_str()],@"dna":[NSString stringWithUTF8String:output.str().c_str()],@"hue":@(s.lineageHue),@"initialEnergy":@(s.initialEnergy),@"origin":[NSString stringWithUTF8String:s.ecology.c_str()]}];}
    return [NSJSONSerialization dataWithJSONObject:records options:0 error:nil];
}
}
