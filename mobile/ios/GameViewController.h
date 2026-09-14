#import <UIKit/UIKit.h>
#include <vector>
#include "alienmobile/Genome.h"
struct NativeGameState;
@class Renderer;
@interface GameViewController : UIViewController <UIGestureRecognizerDelegate> {
    NativeGameState* _state;
    Renderer* _renderer;
    UIButton *_editButton, *_wideButton, *_childButton, *_saveButton, *_speedButton, *_pauseButton, *_mutagenButton, *_currentButton, *_catalogButton, *_foodButton;
    BOOL _paused, _creatorOpen;
    CFTimeInterval _noticeUntil;
    bool _libraryReadOnly;
    UILabel *_hint;
    UIView* _catalogTray;
    NSTimer* _statusTimer;
    std::vector<alienmobile::SpecimenSnapshot> _catalog;
    NSUInteger _builtInCount;
    NSInteger _selectedSpecimen, _tool, _speedIndex;
}
@end
