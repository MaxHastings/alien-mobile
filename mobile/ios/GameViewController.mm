#import "GameViewController.h"

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#import "Renderer.h"
#import "CreatureEditor.h"
#include "alienmobile/GenomeIO.h"
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cmath>

static UIImage* specimenThumbnail(alienmobile::SpecimenSnapshot const& specimen)
{
    constexpr CGFloat size=88;
    UIGraphicsBeginImageContextWithOptions(CGSizeMake(size,size),NO,0);
    CGContextRef context=UIGraphicsGetCurrentContext();
    std::vector<alienmobile::GenomeNode> nodes;
    alienmobile::DevelopmentCursor cursor;
    while(auto node=cursor.next(specimen.genome))nodes.push_back(node->physical);
    std::vector<alienmobile::Vec2> positions(nodes.size());
    for(std::size_t i=1;i<nodes.size();++i)
        positions[i]=positions[nodes[i].parentNode]+nodes[i].relativePosition;
    float minX=0,maxX=0,minY=0,maxY=0;
    for(auto p:positions){minX=std::min(minX,p.x);maxX=std::max(maxX,p.x);minY=std::min(minY,p.y);maxY=std::max(maxY,p.y);}
    alienmobile::Vec2 center{(minX+maxX)*.5f,(minY+maxY)*.5f};
    for(auto& p:positions)p-=center;
    auto scale=float((size-14)/std::max(2.f,std::max(maxX-minX,maxY-minY))),mid=float(size*.5f);
    float radius=std::min(5.f,.34f*scale);
    UIColor* color=[UIColor colorWithHue:specimen.lineageHue saturation:.72 brightness:1 alpha:1];
    CGContextSetStrokeColorWithColor(context,[color colorWithAlphaComponent:.55].CGColor);
    CGContextSetLineWidth(context,2.2);
    for(std::size_t i=1;i<nodes.size();++i) {
        auto a=positions[nodes[i].parentNode],b=positions[i];
        CGContextMoveToPoint(context,mid+a.x*scale,mid-a.y*scale);
        CGContextAddLineToPoint(context,mid+b.x*scale,mid-b.y*scale);
        CGContextStrokePath(context);
    }
    for(std::size_t i=0;i<positions.size();++i) {
        auto p=positions[i];float x=mid+p.x*scale,y=mid-p.y*scale;
        CGContextSetFillColorWithColor(context,color.CGColor);
        CGContextFillEllipseInRect(context,CGRectMake(x-radius,y-radius,2*radius,2*radius));
        CGContextSetStrokeColorWithColor(context,[UIColor colorWithWhite:1 alpha:.8].CGColor);
        CGContextSetLineWidth(context,.9);
        auto role=nodes[i].behavior.role;
        if(role==alienmobile::CellRole::Depot || role==alienmobile::CellRole::Constructor)
            CGContextStrokeEllipseInRect(context,CGRectMake(x-radius*.55f,y-radius*.55f,radius*1.1f,radius*1.1f));
        if(role==alienmobile::CellRole::Motor || role==alienmobile::CellRole::Attacker) {
            CGContextMoveToPoint(context,x-radius*.4f,y+radius*.35f);CGContextAddLineToPoint(context,x+radius*.5f,y);
            CGContextAddLineToPoint(context,x-radius*.4f,y-radius*.35f);CGContextStrokePath(context);
        }
        if(role==alienmobile::CellRole::EnergySensor || role==alienmobile::CellRole::CreatureSensor) {
            CGContextSetFillColorWithColor(context,UIColor.whiteColor.CGColor);
            CGContextFillEllipseInRect(context,CGRectMake(x-1,y-1,2,2));
        }
    }
    UIImage* image=UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return image;
}

struct NativeGameState {
    alienmobile::SimulationConfig config;
    alienmobile::World world;
    alienmobile::Simulation simulation;

    NativeGameState()
        : config([] { auto c=alienmobile::evolutionPlaytestConfig();
            if(auto scenario=std::getenv("ALIEN_MOBILE_WORLD");scenario && std::strcmp(scenario,"beds")==0) {
                // Developer-only historical comparison fixture. Normal play
                // always uses the drifting-mote ecology from the config above.
                c.spatialResources=c.heterogeneousBeds=c.plantedFounders=c.gardenSeed=true;c.primitiveSeed=false;
            }
            c.randomSeed=(uint64_t(arc4random())<<32)|arc4random();
            if(auto seed=std::getenv("ALIEN_MOBILE_SEED"))c.randomSeed=std::strtoull(seed,nullptr,10);
#ifndef NDEBUG
            // Isolated visual integration fixture, never enabled in normal play.
            if(auto check=std::getenv("ALIEN_MOBILE_FOLLOWING_CHECK"); check && std::strcmp(check,"1")==0) {
                c=alienmobile::behavioralPlaytestConfig();
                c.constructionEnergy=10; c.initialRootEnergy=c.initialCellEnergy=1;
                c.energySourceRadius=2.2f; c.hazardStrength=0;
            }
#endif
            return c; }())
        , world(config)
        , simulation(world,config)
    {}
};

@implementation GameViewController
- (UIButton*)button:(NSString*)title action:(SEL)action {
    UIButton* b=[UIButton buttonWithType:UIButtonTypeSystem];
    [b setTitle:title forState:UIControlStateNormal];
    b.titleLabel.font=[UIFont systemFontOfSize:14 weight:UIFontWeightSemibold];
    [b setTitleColor:[UIColor colorWithWhite:.92 alpha:1] forState:UIControlStateNormal];
    b.backgroundColor=[UIColor colorWithRed:.055 green:.075 blue:.10 alpha:.94];
    b.layer.cornerRadius=14;b.translatesAutoresizingMaskIntoConstraints=NO;
    [b.heightAnchor constraintEqualToConstant:46].active=YES;
    [b addTarget:self action:action forControlEvents:UIControlEventTouchUpInside];
    return b;
}
- (void)viewDidLoad {
    [super viewDidLoad];self.view.backgroundColor=UIColor.blackColor;
    _state=new NativeGameState();_catalog=alienmobile::makeCuratedSpecimenCatalog();
    _builtInCount=_catalog.size();[self loadLibrary];
    _selectedSpecimen=-1;_tool=0;_speedIndex=0;_paused=NO;
    MTKView* metal=[[MTKView alloc] initWithFrame:self.view.bounds device:MTLCreateSystemDefaultDevice()];
    metal.autoresizingMask=UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;
    metal.colorPixelFormat=MTLPixelFormatBGRA8Unorm;
    metal.clearColor=MTLClearColorMake(.005,.008,.015,1);metal.preferredFramesPerSecond=60;
    [self.view addSubview:metal];
    _renderer=[[Renderer alloc] initWithView:metal world:_state->world simulation:_state->simulation];
    metal.delegate=_renderer;
    metal.isAccessibilityElement=YES;
    metal.accessibilityLabel=@"Ecosystem";
    metal.accessibilityHint=@"Use the selected tool at the center, stir the water, or zoom in to inspect life.";
    metal.accessibilityCustomActions=@[
        [[UIAccessibilityCustomAction alloc] initWithName:@"Use selected tool at center" target:self selector:@selector(accessibleUseCenter:)],
        [[UIAccessibilityCustomAction alloc] initWithName:@"Stir water at center" target:self selector:@selector(accessibleStir:)],
        [[UIAccessibilityCustomAction alloc] initWithName:@"Zoom in" target:self selector:@selector(accessibleZoom:)]];
    _catalogButton=[self button:@"Create" action:@selector(catalog:)];
    _foodButton=[self button:@"Food" action:@selector(food:)];
    _currentButton=[self button:@"Flow" action:@selector(flow:)];
    _mutagenButton=[self button:@"Mutagen" action:@selector(mutagen:)];
    _mutagenButton.accessibilityHint=@"Place one temporary exposure area. Changes may appear in offspring.";
    UIStackView* bar=[[UIStackView alloc] initWithArrangedSubviews:@[_catalogButton,_foodButton,_mutagenButton,_currentButton]];
    bar.axis=UILayoutConstraintAxisHorizontal;bar.distribution=UIStackViewDistributionFillEqually;bar.spacing=10;
    bar.translatesAutoresizingMaskIntoConstraints=NO;[self.view addSubview:bar];
    _speedButton=[self button:@"1×" action:@selector(speed:)];
    _pauseButton=[self button:@"Pause" action:@selector(pause:)];
    UIButton* wide=[self button:@"Wide" action:@selector(wide:)];
    UIButton* reset=[self button:@"↺" action:@selector(reset:)];
    UIStackView* top=[[UIStackView alloc] initWithArrangedSubviews:@[wide,_speedButton,_pauseButton,reset]];
    top.spacing=10;top.distribution=UIStackViewDistributionFillEqually;top.translatesAutoresizingMaskIntoConstraints=NO;
    [self.view addSubview:top];
    reset.accessibilityLabel=@"New experiment";wide.accessibilityLabel=@"Show whole world";
    _speedButton.accessibilityLabel=@"Time: 1×. Tap for 2× or 4×";
    _catalogButton.accessibilityLabel=@"Create: templates and my creatures";
    _hint=[[UILabel alloc] init];_hint.numberOfLines=2;_hint.textAlignment=NSTextAlignmentCenter;
    _hint.font=[UIFont systemFontOfSize:13 weight:UIFontWeightMedium];
    _hint.textColor=[UIColor colorWithWhite:.75 alpha:1];_hint.translatesAutoresizingMaskIntoConstraints=NO;
    [self.view addSubview:_hint];
    [NSLayoutConstraint activateConstraints:@[
        [top.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor constant:10],
        [top.trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor constant:-16],
        [top.widthAnchor constraintEqualToConstant:278],
        [bar.leadingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.leadingAnchor constant:16],
        [bar.trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor constant:-16],
        [bar.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor constant:-12],
        [_hint.leadingAnchor constraintEqualToAnchor:bar.leadingAnchor],
        [_hint.trailingAnchor constraintEqualToAnchor:bar.trailingAnchor],
        [_hint.bottomAnchor constraintEqualToAnchor:bar.topAnchor constant:-12],
    ]];
    _saveButton=[self button:@"Save specimen" action:@selector(saveFollowed:)];
    _childButton=[self button:@"Follow child" action:@selector(followChild:)];
    UIStackView* focusBar=[[UIStackView alloc] initWithArrangedSubviews:@[_saveButton,_childButton]];focusBar.spacing=8;focusBar.distribution=UIStackViewDistributionFillEqually;focusBar.translatesAutoresizingMaskIntoConstraints=NO;[self.view addSubview:focusBar];
    [NSLayoutConstraint activateConstraints:@[[focusBar.centerXAnchor constraintEqualToAnchor:self.view.centerXAnchor],[focusBar.bottomAnchor constraintEqualToAnchor:_hint.topAnchor constant:-10],[focusBar.widthAnchor constraintEqualToConstant:310]]];
    [self refreshTools];
    UIPanGestureRecognizer* pan=[[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(pan:)];
    pan.minimumNumberOfTouches=1;pan.maximumNumberOfTouches=1;pan.delegate=self;[metal addGestureRecognizer:pan];
    UIPanGestureRecognizer* camera=[[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(camera:)];
    camera.minimumNumberOfTouches=2;camera.maximumNumberOfTouches=2;camera.delegate=self;[metal addGestureRecognizer:camera];
    UIPinchGestureRecognizer* pinch=[[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(pinch:)];
    pinch.delegate=self;[metal addGestureRecognizer:pinch];
    UITapGestureRecognizer* tap=[[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(tap:)];
    [tap requireGestureRecognizerToFail:pan];[tap requireGestureRecognizerToFail:pinch];[metal addGestureRecognizer:tap];
    __weak GameViewController* weakSelf=self;
    _statusTimer=[NSTimer scheduledTimerWithTimeInterval:1 repeats:YES block:^(NSTimer* timer){
        GameViewController* owner=weakSelf;if(!owner)return;
        if(owner->_tool==0 && owner->_catalogTray==nil) [owner refreshTools];
    }];
    dispatch_async(dispatch_get_main_queue(), ^{[self catalog:nil];});
    if(auto requested=std::getenv("ALIEN_MOBILE_OBSERVATION_SPEED")) {
        unsigned speed=std::strtoul(requested,nullptr,10);
        if(speed==1||speed==2||speed==4||speed==8){[_renderer setObservationSpeed:speed];
            [_speedButton setTitle:[NSString stringWithFormat:@"%u×",speed] forState:UIControlStateNormal];}
    }
}
- (void)dealloc {[_statusTimer invalidate];_renderer=nil;delete _state;}
- (BOOL)prefersStatusBarHidden {return YES;}
- (void)refreshTools {
    _saveButton.hidden=(_tool!=0 || _catalogTray!=nil || !_state->world.findCreature([_renderer followedCreatureId]));
    _childButton.hidden=(_tool!=0 || _catalogTray!=nil || ![_renderer hasChild]);
    NSArray* buttons=@[_catalogButton,_foodButton,_currentButton,_mutagenButton];
    for(NSUInteger i=0;i<buttons.count;++i){UIButton* b=buttons[i];BOOL on=_tool==NSInteger(i+1);
        b.backgroundColor=on?[UIColor colorWithRed:.08 green:.35 blue:.40 alpha:1]:[UIColor colorWithRed:.055 green:.075 blue:.10 alpha:.94];
        b.accessibilityValue=on?@"Selected":@"";
    }
    _catalogButton.accessibilityLabel=_tool==1?@"Cancel specimen placement":@"Create: templates and my creatures";
    [_catalogButton setTitle:_tool==1?@"Cancel":@"Create" forState:UIControlStateNormal];
    if(_tool==1 && _selectedSpecimen>=0)_hint.text=[NSString stringWithFormat:@"Tap anywhere to release %@",[NSString stringWithUTF8String:_catalog[_selectedSpecimen].name.c_str()]];
    else if(_tool==2)_hint.text=@"Tap to scatter food · each handful is finite";
    else if(_tool==3)_hint.text=@"Drag to stir · two fingers move the camera";
    else if(_tool==4)_hint.text=@"Tap open water · 20 seconds of local exposure\nMay change future offspring";
    else if(_state->world.mutagen.remaining>0)_hint.text=[NSString stringWithFormat:@"Local exposure · %.0fs remaining\nOnly newly conceived offspring can inherit changes",std::ceil(_state->world.mutagen.remaining)];
    else {NSString* focus=[_renderer focusDescription];
        _hint.text=focus.length?focus:(_state->world.matureCreatureCount()==0?@"No mature life remains · introduce a specimen":@"Tap life to follow · drag to pan · pinch to zoom");}
}
- (void)clearTool {_creatorOpen=NO;[self updateTime];[_renderer endCurrent];_tool=0;_selectedSpecimen=-1;[_catalogTray removeFromSuperview];_catalogTray=nil;[self refreshTools];}
- (void)food:(UIButton*)sender {BOOL active=_tool==2;[self clearTool];_tool=active?0:2;[self refreshTools];}
- (void)mutagen:(UIButton*)sender {BOOL active=_tool==4;[self clearTool];_tool=active?0:4;[self refreshTools];}
- (void)flow:(UIButton*)sender {BOOL active=_tool==3;[self clearTool];_tool=active?0:3;[self refreshTools];}
- (void)wide:(UIButton*)sender {[_renderer resetCamera];
    [_renderer zoomByScale:.75 atPoint:CGPointMake(self.view.bounds.size.width*.5,self.view.bounds.size.height*.5) viewportSize:self.view.bounds.size];
    [self clearTool];}
- (void)updateTime {
    static constexpr unsigned speeds[]={1,2,4};
    unsigned value=speeds[_speedIndex];
    [_renderer setObservationSpeed:(_paused || _creatorOpen)?0:value];
    [_speedButton setTitle:[NSString stringWithFormat:@"%u×",value] forState:UIControlStateNormal];
    _pauseButton.enabled=!_creatorOpen;
    [_pauseButton setTitle:_creatorOpen?@"Paused":_paused?@"Play":@"Pause" forState:UIControlStateNormal];
    _pauseButton.accessibilityLabel=_paused?@"Resume simulation":@"Pause simulation";
    _speedButton.accessibilityLabel=[NSString stringWithFormat:@"Speed: %u×",value];
}
- (void)pause:(UIButton*)sender { _paused=!_paused;[self updateTime]; }
- (void)speed:(UIButton*)sender { _speedIndex=(_speedIndex+1)%3;[self updateTime]; }
- (void)reset:(UIButton*)sender {
    UIAlertController* alert=[UIAlertController alertControllerWithTitle:@"New experiment?" message:@"This replaces the current world. My creatures are kept." preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"Keep watching" style:UIAlertActionStyleCancel handler:nil]];
    [alert addAction:[UIAlertAction actionWithTitle:@"New world" style:UIAlertActionStyleDestructive handler:^(UIAlertAction* action){
        [self clearTool];self->_state->simulation.resetWithSeed((uint64_t(arc4random())<<32)|arc4random());
        [self->_renderer clearFamilyNames];[self->_renderer resetCamera];self->_speedIndex=0;self->_paused=NO;[self updateTime];[self refreshTools];
    }]];[self presentViewController:alert animated:YES completion:nil];
}
- (NSString*)libraryPath {
    NSString* dir=NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory,NSUserDomainMask,YES).firstObject;
    [[NSFileManager defaultManager] createDirectoryAtPath:dir withIntermediateDirectories:YES attributes:nil error:nil];
    return [dir stringByAppendingPathComponent:@"specimens-v1.json"];
}
- (void)loadLibrary {
    NSData* data=[NSData dataWithContentsOfFile:[self libraryPath]];if(!data || data.length>16*1024*1024)return;
    id records=[NSJSONSerialization JSONObjectWithData:data options:0 error:nil];if(![records isKindOfClass:NSArray.class])return;
    for(id record in records){if(_catalog.size()>=_builtInCount+100)break;
        if(![record isKindOfClass:NSDictionary.class] || ![record[@"dna"] isKindOfClass:NSString.class] || ![record[@"name"] isKindOfClass:NSString.class])continue;
        try {std::istringstream input([record[@"dna"] UTF8String]);auto genome=alienmobile::genomeio::readGenome(input);
            if(![record[@"hue"] isKindOfClass:NSNumber.class])continue;double hue=[record[@"hue"] doubleValue];if(!std::isfinite(hue)||hue<0||hue>1)continue;
            NSString* origin=[record[@"origin"] isKindOfClass:NSString.class]?record[@"origin"]:@"Mine";
            _catalog.push_back({[record[@"name"] UTF8String],genome,float(hue),1.f,[origin UTF8String]});
        }catch(std::exception const&) {continue;}}
}
- (BOOL)persistLibrary {
    NSMutableArray* records=[NSMutableArray array];
    for(size_t i=_builtInCount;i<_catalog.size();++i){auto const& s=_catalog[i];std::ostringstream output;alienmobile::genomeio::writeGenome(output,s.genome);
        [records addObject:@{@"name":[NSString stringWithUTF8String:s.name.c_str()],@"dna":[NSString stringWithUTF8String:output.str().c_str()],@"hue":@(s.lineageHue),@"origin":[NSString stringWithUTF8String:s.ecology.c_str()]}];}
    NSData* data=[NSJSONSerialization dataWithJSONObject:records options:0 error:nil];return [data writeToFile:[self libraryPath] options:NSDataWritingAtomic error:nil];
}
- (void)followChild:(UIButton*)sender {[_renderer followChild];[self refreshTools];}
- (void)saveFollowed:(UIButton*)sender {
    auto owner=_state->world.findCreature([_renderer followedCreatureId]);if(!owner)return;
    alienmobile::SpecimenSnapshot specimen;specimen.genome=owner->genome;specimen.lineageHue=owner->lineageHue;specimen.initialEnergy=1.f;
    specimen.name="Lineage "+std::to_string(owner->lineageId)+" · Gen "+std::to_string(owner->generation);specimen.ecology="Discovered · ancestor "+std::to_string(owner->ancestorId+1)+" · gen "+std::to_string(owner->generation);
    [self nameAndSave:specimen];
}
- (void)nameAndSave:(alienmobile::SpecimenSnapshot)specimen {
    if(_catalog.size()>=_builtInCount+100){_hint.text=@"My creatures is full · remove a saved entry first";return;}
    UIAlertController* alert=[UIAlertController alertControllerWithTitle:@"Preserve this genome" message:@"Reuse it from My creatures, even after a new world." preferredStyle:UIAlertControllerStyleAlert];
    [alert addTextFieldWithConfigurationHandler:^(UITextField* f){f.text=[NSString stringWithUTF8String:specimen.name.c_str()];}];
    [alert addAction:[UIAlertAction actionWithTitle:@"Cancel" style:UIAlertActionStyleCancel handler:nil]];
    [alert addAction:[UIAlertAction actionWithTitle:@"Save" style:UIAlertActionStyleDefault handler:^(UIAlertAction* action){auto copy=specimen;NSString* name=alert.textFields.firstObject.text;if(name.length)copy.name=[name substringWithRange:[name rangeOfComposedCharacterSequencesForRange:NSMakeRange(0,MIN(name.length,40))]].UTF8String;self->_catalog.push_back(copy);if(![self persistLibrary]){self->_catalog.pop_back();self->_hint.text=@"Could not save this specimen. Try again.";}else self->_hint.text=@"Saved to My creatures";}]];
    [self presentViewController:alert animated:YES completion:nil];
}
- (void)catalog:(UIButton*)sender {
    if(_catalogTray || _tool==1){[self clearTool];return;}[self clearTool];_creatorOpen=YES;[self updateTime];
    UIView* tray=[[UIView alloc] init];_catalogTray=tray;tray.translatesAutoresizingMaskIntoConstraints=NO;
    tray.backgroundColor=[UIColor colorWithRed:.035 green:.055 blue:.085 alpha:.99];tray.layer.cornerRadius=18;[self.view addSubview:tray];
    UIScrollView* scroll=[[UIScrollView alloc] init];scroll.translatesAutoresizingMaskIntoConstraints=NO;[tray addSubview:scroll];
    UIStackView* rows=[[UIStackView alloc] init];rows.axis=UILayoutConstraintAxisVertical;rows.spacing=8;rows.translatesAutoresizingMaskIntoConstraints=NO;[scroll addSubview:rows];
    for(size_t i=0;i<_catalog.size();++i){
        if(i==0 || i==_builtInCount){UILabel* heading=[[UILabel alloc] init];heading.text=i==0?@"Starting templates":@"My creatures";heading.textColor=UIColor.whiteColor;heading.font=[UIFont systemFontOfSize:21 weight:UIFontWeightSemibold];[rows addArrangedSubview:heading];}
        UIButton* card=[UIButton buttonWithType:UIButtonTypeCustom];card.tag=i;card.backgroundColor=[UIColor colorWithWhite:1 alpha:.045];card.layer.cornerRadius=14;
        [card.heightAnchor constraintEqualToConstant:88].active=YES;
        UIImageView* preview=[[UIImageView alloc] initWithImage:specimenThumbnail(_catalog[i])];preview.translatesAutoresizingMaskIntoConstraints=NO;[card addSubview:preview];
        UILabel* label=[[UILabel alloc] init];label.numberOfLines=3;label.textColor=UIColor.whiteColor;label.font=[UIFont systemFontOfSize:14 weight:UIFontWeightMedium];label.translatesAutoresizingMaskIntoConstraints=NO;
        NSString* name=[NSString stringWithUTF8String:_catalog[i].name.c_str()];NSString* type=i<_builtInCount?[NSString stringWithUTF8String:_catalog[i].ecology.c_str()]:[NSString stringWithFormat:@"%@ · place or edit",[NSString stringWithUTF8String:_catalog[i].ecology.c_str()]];
        label.text=[NSString stringWithFormat:@"%@\n%@",name,type];[card addSubview:label];card.accessibilityLabel=label.text;
        [NSLayoutConstraint activateConstraints:@[[preview.leadingAnchor constraintEqualToAnchor:card.leadingAnchor],[preview.widthAnchor constraintEqualToConstant:88],[preview.heightAnchor constraintEqualToConstant:88],[preview.centerYAnchor constraintEqualToAnchor:card.centerYAnchor],[label.leadingAnchor constraintEqualToAnchor:preview.trailingAnchor constant:8],[label.trailingAnchor constraintEqualToAnchor:card.trailingAnchor constant:-12],[label.centerYAnchor constraintEqualToAnchor:card.centerYAnchor]]];
        [card addTarget:self action:@selector(specimen:) forControlEvents:UIControlEventTouchUpInside];[rows addArrangedSubview:card];
    }
    [NSLayoutConstraint activateConstraints:@[[tray.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor constant:76],[tray.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:16],[tray.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-16],[tray.bottomAnchor constraintEqualToAnchor:_hint.topAnchor constant:-12],[scroll.topAnchor constraintEqualToAnchor:tray.topAnchor constant:16],[scroll.bottomAnchor constraintEqualToAnchor:tray.bottomAnchor constant:-16],[scroll.leadingAnchor constraintEqualToAnchor:tray.leadingAnchor constant:12],[scroll.trailingAnchor constraintEqualToAnchor:tray.trailingAnchor constant:-12],[rows.topAnchor constraintEqualToAnchor:scroll.contentLayoutGuide.topAnchor],[rows.bottomAnchor constraintEqualToAnchor:scroll.contentLayoutGuide.bottomAnchor],[rows.leadingAnchor constraintEqualToAnchor:scroll.contentLayoutGuide.leadingAnchor],[rows.trailingAnchor constraintEqualToAnchor:scroll.contentLayoutGuide.trailingAnchor],[rows.widthAnchor constraintEqualToAnchor:scroll.frameLayoutGuide.widthAnchor]]];
    _saveButton.hidden=YES;_childButton.hidden=YES;_hint.text=@"Make an ancestor. Watch its lineage. · Create closes";
}
- (void)specimen:(UIButton*)sender {
    NSInteger selected=sender.tag;auto specimen=_catalog[selected];
    UIAlertController* alert=[UIAlertController alertControllerWithTitle:[NSString stringWithUTF8String:specimen.name.c_str()] message:[NSString stringWithUTF8String:specimen.ecology.c_str()] preferredStyle:UIAlertControllerStyleActionSheet];
    [alert addAction:[UIAlertAction actionWithTitle:@"Place" style:UIAlertActionStyleDefault handler:^(UIAlertAction* a){[self clearTool];self->_selectedSpecimen=selected;self->_tool=1;self->_creatorOpen=YES;[self updateTime];[self refreshTools];}]];
    auto summary=alienmobile::measureDevelopment(specimen.genome);
    if(summary.complete() && summary.cells<=64 && _catalog.size()<_builtInCount+100)[alert addAction:[UIAlertAction actionWithTitle:@"Edit copy" style:UIAlertActionStyleDefault handler:^(UIAlertAction* a){[self clearTool];self->_creatorOpen=YES;[self updateTime];
        CreatureEditor* editor=[[CreatureEditor alloc] initWithSpecimen:specimen completion:^(alienmobile::SpecimenSnapshot result){self->_creatorOpen=NO;[self updateTime];self->_catalog.push_back(result);if(![self persistLibrary]){self->_catalog.pop_back();self->_hint.text=@"Could not save. Please try again.";return;}self->_selectedSpecimen=self->_catalog.size()-1;self->_tool=1;self->_creatorOpen=YES;[self updateTime];[self refreshTools];}];
        editor.onCancel=^{self->_creatorOpen=NO;[self updateTime];[self refreshTools];};
        [self presentViewController:editor animated:YES completion:nil];
    }]];
    if(selected>=_builtInCount)[alert addAction:[UIAlertAction actionWithTitle:@"Delete saved entry" style:UIAlertActionStyleDestructive handler:^(UIAlertAction* a){auto saved=self->_catalog[selected];self->_catalog.erase(self->_catalog.begin()+selected);if(![self persistLibrary])self->_catalog.insert(self->_catalog.begin()+selected,saved);[self clearTool];[self catalog:nil];}]];
    [alert addAction:[UIAlertAction actionWithTitle:@"Cancel" style:UIAlertActionStyleCancel handler:nil]];
    alert.popoverPresentationController.sourceView=sender;alert.popoverPresentationController.sourceRect=sender.bounds;
    [self presentViewController:alert animated:YES completion:nil];
}
- (void)tap:(UITapGestureRecognizer*)g {
    if(_catalogTray)return;
    [self useToolAtPoint:[g locationInView:self.view]];
}
- (BOOL)accessibleUseCenter:(UIAccessibilityCustomAction*)action {
    if(_catalogTray)return NO;
    return [self useToolAtPoint:CGPointMake(self.view.bounds.size.width*.5,self.view.bounds.size.height*.5)];
}
- (BOOL)accessibleStir:(UIAccessibilityCustomAction*)action {
    if(_catalogTray)return NO;
    CGSize s=self.view.bounds.size;
    [_renderer beginCurrentAtScreenPoint:CGPointMake(s.width*.4,s.height*.5) viewportSize:s];
    [_renderer extendCurrentToScreenPoint:CGPointMake(s.width*.6,s.height*.5) viewportSize:s];
    [_renderer endCurrent];return YES;
}
- (BOOL)accessibleZoom:(UIAccessibilityCustomAction*)action {
    CGSize s=self.view.bounds.size;
    [_renderer zoomByScale:1.8 atPoint:CGPointMake(s.width*.5,s.height*.5) viewportSize:s];return YES;
}
- (BOOL)useToolAtPoint:(CGPoint)p {
    CGSize size=self.view.bounds.size;BOOL ok=YES;
    if(_tool==1){ok=[_renderer placeSpecimen:_catalog[_selectedSpecimen] atScreenPoint:p viewportSize:size];if(ok)[self clearTool];else {
        auto summary=alienmobile::measureDevelopment(_catalog[_selectedSpecimen].genome);
        size_t reserved=_state->world.cells.size();
        for(auto const& c:_state->world.creatures)if(!c.mature && !c.fragment && !c.developmentFailed)
            reserved+=c.expectedCells>c.bodyNodes.size()?c.expectedCells-c.bodyNodes.size():0;
        if(reserved+summary.cells>_state->config.maxCellCount)[self offerFreshRelease];
    }}
    else if(_tool==2)ok=[_renderer scatterFoodAtScreenPoint:p viewportSize:size];
    else if(_tool==4){ok=[_renderer applyMutagenAtScreenPoint:p viewportSize:size];if(ok)[self clearTool];}
    else if(_tool==0){ok=[_renderer followAtScreenPoint:p viewportSize:size];[self refreshTools];}
    if(!ok && _tool!=0)_hint.text=_tool==1?@"No room here · try open water or start a new world":
        _tool==4?@"Wait for the current exposure to fade · place away from the edge":@"Scatter inside the world · let existing food clear";
    if(ok && _tool!=3) {
        UIView* ring=[[UIView alloc] initWithFrame:CGRectMake(p.x-14,p.y-14,28,28)];
        ring.userInteractionEnabled=NO;ring.layer.cornerRadius=14;ring.layer.borderWidth=1.5;
        ring.layer.borderColor=(_tool==2?[UIColor colorWithRed:1 green:.72 blue:.28 alpha:1]:[UIColor colorWithRed:.35 green:.9 blue:1 alpha:1]).CGColor;
        [self.view addSubview:ring];
        [UIView animateWithDuration:.65 animations:^{ring.transform=CGAffineTransformMakeScale(2.8,2.8);ring.alpha=0;} completion:^(BOOL finished){[ring removeFromSuperview];}];
    }
    UIImpactFeedbackGenerator* feedback=[[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleLight];[feedback impactOccurred];
    return ok;
}
- (void)offerFreshRelease {
    if(self.presentedViewController || _selectedSpecimen<0)return;
    NSInteger selected=_selectedSpecimen;
    UIAlertController* alert=[UIAlertController alertControllerWithTitle:@"A fresh world for this creature?" message:@"This tank has reached its population limit. Your saved creatures stay with you." preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"Keep this world" style:UIAlertActionStyleCancel handler:nil]];
    [alert addAction:[UIAlertAction actionWithTitle:@"New world & release" style:UIAlertActionStyleDefault handler:^(UIAlertAction* action){
        self->_state->simulation.resetWithSeed((uint64_t(arc4random())<<32)|arc4random());[self->_renderer clearFamilyNames];[self->_renderer resetCamera];
        BOOL placed=[self->_renderer placeSpecimen:self->_catalog[selected] atScreenPoint:CGPointMake(self.view.bounds.size.width*.5,self.view.bounds.size.height*.5) viewportSize:self.view.bounds.size];
        if(placed){[self clearTool];[self refreshTools];}else self->_hint.text=@"This body is too large for the tank · edit a smaller copy";
    }]];
    [self presentViewController:alert animated:YES completion:nil];
}
- (void)pan:(UIPanGestureRecognizer*)g {
    if(_catalogTray)return;
    if(_tool==3){
        if(g.state==UIGestureRecognizerStateBegan){
            CGPoint location=[g locationInView:self.view],translation=[g translationInView:self.view];
            [_renderer beginCurrentAtScreenPoint:CGPointMake(location.x-translation.x,location.y-translation.y) viewportSize:self.view.bounds.size];
            [_renderer extendCurrentToScreenPoint:location viewportSize:self.view.bounds.size];
        }
        if(g.state==UIGestureRecognizerStateChanged)[_renderer extendCurrentToScreenPoint:[g locationInView:self.view] viewportSize:self.view.bounds.size];
        if(g.state==UIGestureRecognizerStateEnded||g.state==UIGestureRecognizerStateCancelled||g.state==UIGestureRecognizerStateFailed)[_renderer endCurrent];
    }else if(g.state==UIGestureRecognizerStateChanged){[_renderer panByScreenTranslation:[g translationInView:self.view] viewportSize:self.view.bounds.size];[g setTranslation:CGPointZero inView:self.view];}
}
- (void)camera:(UIPanGestureRecognizer*)g {[_renderer endCurrent];if(g.state==UIGestureRecognizerStateChanged){[_renderer panByScreenTranslation:[g translationInView:self.view] viewportSize:self.view.bounds.size];[g setTranslation:CGPointZero inView:self.view];}}
- (void)pinch:(UIPinchGestureRecognizer*)g {[_renderer endCurrent];[_renderer zoomByScale:g.scale atPoint:[g locationInView:self.view] viewportSize:self.view.bounds.size];g.scale=1;}
- (BOOL)gestureRecognizer:(UIGestureRecognizer*)a shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer*)b {return NO;}
@end
