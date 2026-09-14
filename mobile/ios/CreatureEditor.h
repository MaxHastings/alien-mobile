#import <UIKit/UIKit.h>
#include "alienmobile/Genome.h"
@interface CreatureEditor : UIViewController
@property(nonatomic,copy) void (^onCancel)(void);
- (instancetype)initWithSpecimen:(alienmobile::SpecimenSnapshot)specimen completion:(void (^)(alienmobile::SpecimenSnapshot))completion;
@end
