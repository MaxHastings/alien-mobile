#import "CreatureEditor.h"
#include "alienmobile/Creator.h"
#include "alienmobile/Rehearsal.h"
#include <memory>
#include <chrono>
using namespace alienmobile;
@interface BodyCanvas : UIView
@property(nonatomic) Genome* genome;
@property(nonatomic) float hue;
@property(nonatomic) NSInteger selected;
@property(nonatomic,copy) void (^changed)(void);
@property(nonatomic,copy) void (^willEdit)(void);
@property(nonatomic,copy) void (^rejected)(NSString*);
@end
@implementation BodyCanvas {
    float _dragScale;
    Vec2 _dragEdge;
    BOOL _dragging;
}
- (instancetype)initWithFrame:(CGRect)frame {
    if(self=[super initWithFrame:frame]) {
        UIPanGestureRecognizer* drag=[[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(reshape:)];
        drag.maximumNumberOfTouches=1;[self addGestureRecognizer:drag];
    }return self;
}
- (void)reshape:(UIPanGestureRecognizer*)gesture {
    if(gesture.state==UIGestureRecognizerStateBegan) {
        CGPoint location=[gesture locationInView:self],translation=[gesture translationInView:self];
        CGPoint start=CGPointMake(location.x-translation.x,location.y-translation.y);
        auto p=[self positions];float best=32;NSInteger chosen=-1;
        for(size_t i=1;i<p.size();++i){auto point=[self point:p[i]];float d=hypot(start.x-point.x,start.y-point.y);if(d<best){best=d;chosen=i;}}
        _dragging=chosen>=0;if(!_dragging)return;
        self.selected=chosen;_dragScale=0;_dragScale=[self scale];_dragEdge=self.genome->genes[0].nodes[chosen].relativePosition;
        if(self.willEdit)self.willEdit();
    }
    if(_dragging && (gesture.state==UIGestureRecognizerStateBegan || gesture.state==UIGestureRecognizerStateChanged || gesture.state==UIGestureRecognizerStateEnded)) {
        CGPoint delta=[gesture translationInView:self];Vec2 edge=_dragEdge+Vec2{float(delta.x/_dragScale),float(-delta.y/_dragScale)};
        float angle=round(atan2(edge.y,edge.x)/(M_PI/12))*(M_PI/12),distance=clamp(length(edge),.7f,2.f);
        BOOL moved=moveBodyCell(*self.genome,self.selected,{float(cos(angle)*distance),float(sin(angle)*distance)});
        [self setNeedsDisplay];[self setNeedsLayout];if(self.changed)self.changed();
        if(!moved && self.rejected)self.rejected(@"Cells need room: move this branch away from the body.");
    }
    if(gesture.state==UIGestureRecognizerStateEnded || gesture.state==UIGestureRecognizerStateCancelled){_dragging=NO;[self setNeedsLayout];[self setNeedsDisplay];}
}
- (std::vector<Vec2>)positions {std::vector<Vec2> p;for(auto const& n:self.genome->genes[0].nodes)p.push_back(n.parentNode<0?Vec2{}:p[n.parentNode]+n.relativePosition);return p;}
- (float)scale {if(_dragging && _dragScale>0)return _dragScale;auto p=[self positions];float r=2.5;for(auto v:p)r=std::max(r,std::max(std::abs(v.x),std::abs(v.y))+1);return std::min(self.bounds.size.width,self.bounds.size.height)/(2*r);}
- (CGPoint)point:(Vec2)p {float s=[self scale];return CGPointMake(self.bounds.size.width/2+p.x*s,self.bounds.size.height/2-p.y*s);}
- (void)layoutSubviews {
    [super layoutSubviews];if(_dragging)return;for(UIView* view in [self.subviews copy])[view removeFromSuperview];
    auto positions=[self positions];for(size_t i=0;i<positions.size();++i){auto p=[self point:positions[i]];UIButton* cell=[UIButton buttonWithType:UIButtonTypeCustom];cell.frame=CGRectMake(p.x-22,p.y-22,44,44);cell.tag=i;
        cell.accessibilityLabel=[NSString stringWithFormat:@"Cell %zu%@",i+1,i==self.selected?@", selected":@""];[cell addTarget:self action:@selector(selectCell:) forControlEvents:UIControlEventTouchUpInside];[self addSubview:cell];}
}
- (void)selectCell:(UIButton*)cell {self.selected=cell.tag;[self setNeedsDisplay];[self setNeedsLayout];if(self.changed)self.changed();}
- (BOOL)growAtAngle:(float)angle {
    auto p=[self positions];Vec2 edge{float(cos(angle)*1.15),float(sin(angle)*1.15)};auto target=p[self.selected]+edge;
    for(auto v:p)if(length(v-target)<.65f)return NO;
    if(self.willEdit)self.willEdit();
    if(!addBodyCell(*self.genome,self.selected,edge))return NO;
    self.selected=self.genome->genes[0].nodes.size()-1;[self setNeedsDisplay];[self setNeedsLayout];if(self.changed)self.changed();return YES;
}
- (void)drawRect:(CGRect)rect {
    auto p=[self positions];auto& nodes=self.genome->genes[0].nodes;auto c=UIGraphicsGetCurrentContext();
    UIColor* tint=[UIColor colorWithHue:self.hue saturation:.52 brightness:.96 alpha:1];
    CGContextSetLineWidth(c,3);CGContextSetStrokeColorWithColor(c,[tint colorWithAlphaComponent:.4].CGColor);
    for(size_t i=1;i<p.size();++i){auto a=[self point:p[nodes[i].parentNode]],b=[self point:p[i]];CGContextMoveToPoint(c,a.x,a.y);CGContextAddLineToPoint(c,b.x,b.y);CGContextStrokePath(c);}
    NSArray* symbols=@[@"·",@"◎",@"~",@"›",@"◉",@"◉",@"›",@"○",@"●",@"◇",@"·",@"◉",@"·",@"·"];
    for(size_t i=0;i<p.size();++i){auto a=[self point:p[i]];CGFloat r=std::min(16.f,[self scale]*.36f);CGContextSetFillColorWithColor(c,tint.CGColor);CGContextFillEllipseInRect(c,CGRectMake(a.x-r,a.y-r,2*r,2*r));
        if(i==self.selected){CGContextSetStrokeColorWithColor(c,UIColor.whiteColor.CGColor);CGContextSetLineWidth(c,2);CGContextStrokeEllipseInRect(c,CGRectMake(a.x-r-5,a.y-r-5,2*r+10,2*r+10));}
        if(nodes[i].behavior.role==CellRole::Motor){auto edge=nodes[i].relativePosition;float angle=atan2(edge.y,edge.x)+nodes[i].behavior.axisAngle;
            CGContextSetStrokeColorWithColor(c,tint.CGColor);CGContextSetLineWidth(c,2);CGPoint end=CGPointMake(a.x+cos(angle)*(r+13),a.y-sin(angle)*(r+13));CGContextMoveToPoint(c,a.x,a.y);CGContextAddLineToPoint(c,end.x,end.y);CGContextStrokePath(c);}
        NSString* symbol=symbols[NSUInteger(nodes[i].behavior.role)];[symbol drawAtPoint:CGPointMake(a.x-6,a.y-10) withAttributes:@{NSFontAttributeName:[UIFont boldSystemFontOfSize:17],NSForegroundColorAttributeName:UIColor.blackColor}];}
}
- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    CGPoint t=[touches.anyObject locationInView:self];auto p=[self positions];float best=32;NSInteger chosen=-1;
    for(size_t i=0;i<p.size();++i){auto a=[self point:p[i]];float d=hypot(t.x-a.x,t.y-a.y);if(d<best){best=d;chosen=i;}}
    if(chosen>=0)self.selected=chosen;
    else {auto a=[self point:p[self.selected]];float angle=round(atan2(a.y-t.y,t.x-a.x)/(M_PI/6))*(M_PI/6);
        [self growAtAngle:angle];}
    [self setNeedsDisplay];if(self.changed)self.changed();
}
@end
@interface RehearsalCanvas : UIView
@property(nonatomic) Rehearsal* rehearsal;
@property(nonatomic) float hue;
@end
@implementation RehearsalCanvas
- (void)drawRect:(CGRect)rect {
    if(!self.rehearsal)return;
    auto& run=*self.rehearsal;auto& w=run.world;auto c=UIGraphicsGetCurrentContext();
    float radius=5;
    for(auto const& cell:w.cells)radius=std::max(radius,std::max(std::abs(cell.position.x),std::abs(cell.position.y))+1);
    for(auto v:run.trail)radius=std::max(radius,std::max(std::abs(v.x),std::abs(v.y))+1);
    float scale=std::min(self.bounds.size.width,self.bounds.size.height)/(2*radius);
    auto point=[&](Vec2 v){return CGPointMake(self.bounds.size.width/2+v.x*scale,self.bounds.size.height/2-v.y*scale);};
    CGContextSetLineWidth(c,.5);CGContextSetStrokeColorWithColor(c,[UIColor colorWithWhite:1 alpha:.08].CGColor);
    for(int i=-int(radius);i<=int(radius);++i){auto a=point({float(i),-radius}),b=point({float(i),radius});CGContextMoveToPoint(c,a.x,a.y);CGContextAddLineToPoint(c,b.x,b.y);a=point({-radius,float(i)});b=point({radius,float(i)});CGContextMoveToPoint(c,a.x,a.y);CGContextAddLineToPoint(c,b.x,b.y);}CGContextStrokePath(c);
    UIColor* tint=[UIColor colorWithHue:self.hue saturation:.48 brightness:1 alpha:1];
    CGContextSetStrokeColorWithColor(c,[tint colorWithAlphaComponent:.4].CGColor);CGContextSetLineWidth(c,1.5);
    for(size_t i=0;i<run.trail.size();++i){auto p=point(run.trail[i]);if(!i)CGContextMoveToPoint(c,p.x,p.y);else CGContextAddLineToPoint(c,p.x,p.y);}CGContextStrokePath(c);
    auto start=point(run.startCenter);CGContextStrokeEllipseInRect(c,CGRectMake(start.x-4,start.y-4,8,8));
    CGContextSetFillColorWithColor(c,[UIColor colorWithRed:1 green:.73 blue:.3 alpha:.8].CGColor);
    for(auto const& mote:w.motes){auto p=point(mote.position);CGContextFillEllipseInRect(c,CGRectMake(p.x-1.5,p.y-1.5,3,3));}
    CGContextSetStrokeColorWithColor(c,[tint colorWithAlphaComponent:.5].CGColor);CGContextSetLineWidth(c,2);
    for(auto const& edge:w.connections){auto a=point(w.cells[edge.cellA].position),b=point(w.cells[edge.cellB].position);CGContextMoveToPoint(c,a.x,a.y);CGContextAddLineToPoint(c,b.x,b.y);}CGContextStrokePath(c);
    for(auto const& cell:w.cells){auto p=point(cell.position);float r=std::max(4.f,scale*w.config().cellRadius);
        // Keep the specimen hue stable while food uptake brightens that same
        // hue instead of switching the cell to a generic gold.
        UIColor* color=cell.starvationTimer>.2f ? [UIColor colorWithRed:1 green:.3 blue:.3 alpha:1] :
            [UIColor colorWithHue:self.hue saturation:.48 brightness:std::min(1.f,.96f+.16f*cell.absorptionFlash) alpha:1];
        CGContextSetShadowWithColor(c,CGSizeZero,7,[color colorWithAlphaComponent:.5].CGColor);
        CGContextSetFillColorWithColor(c,color.CGColor);CGContextFillEllipseInRect(c,CGRectMake(p.x-r,p.y-r,2*r,2*r));CGContextSetShadowWithColor(c,CGSizeZero,0,nil);
        if(cell.creatureId==run.subject){CGContextSetStrokeColorWithColor(c,[UIColor colorWithWhite:1 alpha:.7].CGColor);CGContextStrokeEllipseInRect(c,CGRectMake(p.x-r-2,p.y-r-2,2*r+4,2*r+4));}
        if(length(cell.motorThrust)>.001f){auto end=point(cell.position+cell.motorThrust*2);CGContextSetStrokeColorWithColor(c,UIColor.whiteColor.CGColor);CGContextMoveToPoint(c,p.x,p.y);CGContextAddLineToPoint(c,end.x,end.y);CGContextStrokePath(c);}
        if(cell.behavior.role==CellRole::EnergySensor && energySensor(w,&cell-w.cells.data())[EnergyIntensity]>.01f){CGContextSetStrokeColorWithColor(c,[UIColor colorWithRed:1 green:.8 blue:.3 alpha:.55].CGColor);CGContextStrokeEllipseInRect(c,CGRectMake(p.x-r-5,p.y-r-5,2*r+10,2*r+10));}
    }
    for(auto const& event:w.lifeEvents)if(event.cell!=kInvalidId && w.ecologicalTime-event.time<3){auto p=point(event.position);CGContextSetStrokeColorWithColor(c,UIColor.systemRedColor.CGColor);CGContextMoveToPoint(c,p.x-5,p.y-5);CGContextAddLineToPoint(c,p.x+5,p.y+5);CGContextMoveToPoint(c,p.x+5,p.y-5);CGContextAddLineToPoint(c,p.x-5,p.y+5);CGContextStrokePath(c);}
}
@end
@interface CreatureEditor () <UITextFieldDelegate>
@end
@implementation CreatureEditor {
    std::unique_ptr<Rehearsal> _rehearsal;RehearsalCanvas* _practice;NSTimer* _timer;UISegmentedControl* _mode;UILabel* _meaning;UILabel* _comparison;NSString* _originalResult;NSString* _editedResult;UIButton* _release;UIStackView* _roleControls;UIStackView* _editControls;
    Genome _source;SpecimenSnapshot _specimen;BOOL (^_completion)(SpecimenSnapshot);BodyCanvas* _canvas;UILabel* _detail;UITextField* _name;UISlider* _colorSlider;std::vector<Genome> _undo;std::vector<float> _undoHue;std::vector<bool> _undoRewired;BOOL _rewired;
}
- (instancetype)initWithSpecimen:(SpecimenSnapshot)s completion:(BOOL (^)(SpecimenSnapshot))completion {if(self=[super init]){_specimen=s;_source=s.genome;_specimen.genome=editableBody(s.genome);_completion=[completion copy];self.modalPresentationStyle=UIModalPresentationFullScreen;}return self;}
- (UIButton*)button:(NSString*)title action:(SEL)action tag:(NSInteger)tag {UIButton* b=[UIButton buttonWithType:UIButtonTypeSystem];[b setTitle:title forState:UIControlStateNormal];[b setTitleColor:[UIColor colorWithWhite:.94 alpha:1] forState:UIControlStateNormal];b.tag=tag;b.titleLabel.font=[UIFont systemFontOfSize:14 weight:UIFontWeightSemibold];b.backgroundColor=[UIColor colorWithWhite:1 alpha:.07];b.layer.cornerRadius=12;[b.heightAnchor constraintEqualToConstant:44].active=YES;[b addTarget:self action:action forControlEvents:UIControlEventTouchUpInside];return b;}
- (void)viewDidLoad {
    [super viewDidLoad];self.view.backgroundColor=[UIColor colorWithRed:.025 green:.045 blue:.065 alpha:1];self.modalInPresentation=YES;
    UIStackView* stack=[[UIStackView alloc] init];stack.axis=UILayoutConstraintAxisVertical;stack.spacing=8;stack.translatesAutoresizingMaskIntoConstraints=NO;
    UIScrollView* scroll=[[UIScrollView alloc] init];scroll.translatesAutoresizingMaskIntoConstraints=NO;scroll.alwaysBounceVertical=YES;[self.view addSubview:scroll];[scroll addSubview:stack];
    [NSLayoutConstraint activateConstraints:@[[scroll.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor],[scroll.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor],[scroll.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],[scroll.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor]]];
    _name=[[UITextField alloc] init];_name.text=[NSString stringWithUTF8String:_specimen.name.c_str()];_name.textColor=UIColor.whiteColor;_name.font=[UIFont systemFontOfSize:24 weight:UIFontWeightSemibold];_name.accessibilityLabel=@"Creature name";_name.returnKeyType=UIReturnKeyDone;_name.delegate=self;[stack addArrangedSubview:_name];
    UIStackView* colorRow=[[UIStackView alloc] init];colorRow.axis=UILayoutConstraintAxisHorizontal;colorRow.spacing=10;colorRow.alignment=UIStackViewAlignmentCenter;
    UILabel* colorLabel=[[UILabel alloc] init];colorLabel.text=@"Color";colorLabel.textColor=UIColor.lightGrayColor;colorLabel.font=[UIFont systemFontOfSize:14 weight:UIFontWeightSemibold];[colorLabel.widthAnchor constraintEqualToConstant:48].active=YES;
    _colorSlider=[[UISlider alloc] init];_colorSlider.minimumValue=0;_colorSlider.maximumValue=1;_colorSlider.value=_specimen.lineageHue;_colorSlider.accessibilityLabel=@"Creature color";_colorSlider.accessibilityHint=@"Choose the hue used by this creature and its descendants";_colorSlider.minimumTrackTintColor=[UIColor colorWithHue:_colorSlider.value saturation:.72 brightness:1 alpha:1];[_colorSlider addTarget:self action:@selector(colorEditStarted:) forControlEvents:UIControlEventTouchDown];[_colorSlider addTarget:self action:@selector(colorChanged:) forControlEvents:UIControlEventValueChanged];[colorRow addArrangedSubview:colorLabel];[colorRow addArrangedSubview:_colorSlider];[stack addArrangedSubview:colorRow];
    _meaning=[[UILabel alloc] init];_meaning.numberOfLines=3;_meaning.font=[UIFont systemFontOfSize:12];_meaning.textColor=UIColor.lightGrayColor;[stack addArrangedSubview:_meaning];
    _mode=[[UISegmentedControl alloc] initWithItems:@[@"Build",@"Try body",@"Original"]];_mode.selectedSegmentIndex=0;_mode.selectedSegmentTintColor=[UIColor colorWithRed:.1 green:.45 blue:.46 alpha:1];[_mode setTitleTextAttributes:@{NSForegroundColorAttributeName:UIColor.whiteColor} forState:UIControlStateNormal];[_mode addTarget:self action:@selector(modeChanged:) forControlEvents:UIControlEventValueChanged];[stack addArrangedSubview:_mode];
    UIView* stage=[[UIView alloc] init];stage.backgroundColor=[UIColor colorWithWhite:1 alpha:.025];stage.layer.cornerRadius=20;stage.clipsToBounds=YES;[stage.heightAnchor constraintEqualToConstant:220].active=YES;[stack addArrangedSubview:stage];
    _canvas=[[BodyCanvas alloc] init];_canvas.genome=&_specimen.genome;_canvas.hue=_specimen.lineageHue;_canvas.backgroundColor=UIColor.clearColor;_canvas.frame=CGRectMake(0,0,300,220);_canvas.autoresizingMask=UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;[stage addSubview:_canvas];
    _practice=[[RehearsalCanvas alloc] init];_practice.frame=_canvas.frame;_practice.autoresizingMask=_canvas.autoresizingMask;_practice.backgroundColor=UIColor.clearColor;_practice.hue=_specimen.lineageHue;_practice.hidden=YES;
    [stage addSubview:_practice];for(UIView* canvas in @[_canvas,_practice]){canvas.translatesAutoresizingMaskIntoConstraints=NO;[NSLayoutConstraint activateConstraints:@[[canvas.topAnchor constraintEqualToAnchor:stage.topAnchor],[canvas.bottomAnchor constraintEqualToAnchor:stage.bottomAnchor],[canvas.leadingAnchor constraintEqualToAnchor:stage.leadingAnchor],[canvas.trailingAnchor constraintEqualToAnchor:stage.trailingAnchor]]];}
    _practice.isAccessibilityElement=YES;_practice.accessibilityLabel=@"Practice tank: white outlines identify your body; offspring use the same physics";
    _detail=[[UILabel alloc] init];_detail.numberOfLines=3;[_detail.heightAnchor constraintGreaterThanOrEqualToConstant:48].active=YES;_detail.font=[UIFont systemFontOfSize:13];_detail.textColor=UIColor.lightGrayColor;[stack addArrangedSubview:_detail];
    _comparison=[[UILabel alloc] init];_comparison.numberOfLines=0;_comparison.font=[UIFont systemFontOfSize:12];_comparison.textColor=[UIColor colorWithWhite:.85 alpha:1];_comparison.hidden=YES;[stack addArrangedSubview:_comparison];
    NSArray* names=@[@"Body",@"Motor",@"Sensor",@"Storage",@"Attack",@"Digest"];
    int roles[]={0,3,4,8,6,7};
    _roleControls=[[UIStackView alloc] init];_roleControls.axis=UILayoutConstraintAxisVertical;_roleControls.spacing=8;[stack addArrangedSubview:_roleControls];
    for(int row=0;row<2;row++){UIStackView* line=[[UIStackView alloc] init];line.spacing=8;line.distribution=UIStackViewDistributionFillEqually;for(int col=0;col<3;col++){int i=row*3+col;[line addArrangedSubview:[self button:names[i] action:@selector(role:) tag:roles[i]]];}[_roleControls addArrangedSubview:line];}
    UIStackView* edit=[[UIStackView alloc] init];_editControls=edit;edit.spacing=8;edit.distribution=UIStackViewDistributionFillEqually;[edit addArrangedSubview:[self button:@"Grow" action:@selector(grow:) tag:0]];[edit addArrangedSubview:[self button:@"Turn" action:@selector(turn:) tag:0]];[edit addArrangedSubview:[self button:@"Remove tip" action:@selector(remove:) tag:0]];[stack addArrangedSubview:edit];
    UIStackView* experiment=[[UIStackView alloc] init];experiment.spacing=8;experiment.distribution=UIStackViewDistributionFillEqually;[experiment addArrangedSubview:[self button:@"Rewire…" action:@selector(rewire:) tag:0]];[experiment addArrangedSubview:[self button:@"Replay" action:@selector(replay:) tag:0]];[stack addArrangedSubview:experiment];
    UIButton* release=[self button:@"Save & release" action:@selector(releaseBody:) tag:0];_release=release;release.backgroundColor=[UIColor colorWithRed:.05 green:.38 blue:.39 alpha:1];[stack addArrangedSubview:release];UIStackView* bottom=[[UIStackView alloc] init];bottom.spacing=8;bottom.distribution=UIStackViewDistributionFillEqually;[bottom addArrangedSubview:[self button:@"Undo" action:@selector(undo:) tag:0]];[bottom addArrangedSubview:[self button:@"Cancel" action:@selector(cancel:) tag:0]];[stack addArrangedSubview:bottom];
    [NSLayoutConstraint activateConstraints:@[[stack.topAnchor constraintEqualToAnchor:scroll.contentLayoutGuide.topAnchor constant:12],[stack.bottomAnchor constraintEqualToAnchor:scroll.contentLayoutGuide.bottomAnchor constant:-16],[stack.leadingAnchor constraintEqualToAnchor:scroll.contentLayoutGuide.leadingAnchor constant:20],[stack.trailingAnchor constraintEqualToAnchor:scroll.contentLayoutGuide.trailingAnchor constant:-20],[stack.widthAnchor constraintEqualToAnchor:scroll.frameLayoutGuide.widthAnchor constant:-40]]];
    __weak CreatureEditor* owner=self;_canvas.changed=^{[owner update];};_canvas.willEdit=^{[owner remember];};_canvas.rejected=^(NSString* message){CreatureEditor* editor=owner;if(editor)editor->_detail.text=message;};[self update];
    _timer=[NSTimer scheduledTimerWithTimeInterval:1.0/30 repeats:YES block:^(NSTimer* timer){[owner tick];}];
}
- (void)dealloc {[_timer invalidate];}
- (void)viewDidDisappear:(BOOL)animated {[super viewDidDisappear:animated];if(self.isBeingDismissed || !self.presentingViewController)[_timer invalidate];}
- (void)tick {if(!_rehearsal || _mode.selectedSegmentIndex==0)return;for(int i=0;i<4;++i)_rehearsal->step();[_practice setNeedsDisplay];
    auto& r=*_rehearsal;NSString* evidence=[NSString stringWithUTF8String:r.message().c_str()];
    _detail.text=[NSString stringWithFormat:@"%@ · %.0f / 12s · moved %.1f cell widths\n%@",_mode.selectedSegmentIndex==2?@"Original":@"Your body",r.steps/120.f,length(r.lastCenter-r.startCenter)/(.68f),evidence];
    if(r.steps==r.durationSteps){
        auto body=r.world.findCreature(r.subject);BOOL intact=body && !body->fragment;
        unsigned cells=unsigned(r.world.cellIndicesForCreature(r.subject).size());
        NSString* result=[NSString stringWithFormat:@"%.1f widths · %@ · %u cells · %u grown young",length(r.lastCenter-r.startCenter)/(.68f),intact?@"alive":@"body lost",cells,r.completedChildren];
        if(_mode.selectedSegmentIndex==2)_originalResult=result;else _editedResult=result;
        _comparison.text=[NSString stringWithFormat:@"At 12s · Original: %@\nYour body: %@\nSame food and starting position. Ecosystem survival may differ.",_originalResult?:@"run Original to compare",_editedResult?:@"run Try body to compare"];
    }
    if(r.completedChildren)_detail.text=[_detail.text stringByAppendingFormat:@" · %u child%@",r.completedChildren,r.completedChildren==1?@"":@"ren"];
}
- (void)modeChanged:(UISegmentedControl*)control {
    [_name resignFirstResponder];BOOL building=_mode.selectedSegmentIndex==0;_canvas.hidden=!building;_practice.hidden=building;_roleControls.hidden=!building;_editControls.hidden=!building;_comparison.hidden=building;
    if(building){_practice.rehearsal=nullptr;_rehearsal.reset();[self update];return;}
    [self update];auto specimen=_specimen;specimen.genome=_mode.selectedSegmentIndex==2?_source:prepareCreatorRelease(_specimen.genome,_source);
    _practice.rehearsal=nullptr;_rehearsal=std::make_unique<Rehearsal>(specimen);_practice.rehearsal=_rehearsal.get();
    _meaning.text=@"Practice · same mechanics, finite food, no residents.\nWhite outline = your body; lines = paid thrust.\nOriginal and Try body restart under identical conditions.";[self tick];
}
- (void)replay:(UIButton*)button {if(_mode.selectedSegmentIndex==0)_mode.selectedSegmentIndex=1;[self modeChanged:_mode];}
- (void)rewire:(UIButton*)button {
    UIAlertController* alert=[UIAlertController alertControllerWithTitle:@"Rebuild this body's signals?" message:@"Replaces the controller in every cell using the current anatomy. It may help new organs communicate, but can erase inherited behavior. Geometry and organ properties stay yours. Try it, compare with Original, and Undo if needed." preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"Keep inherited control" style:UIAlertActionStyleCancel handler:nil]];
    [alert addAction:[UIAlertAction actionWithTitle:@"Rewire & try" style:UIAlertActionStyleDefault handler:^(UIAlertAction* action){[self remember];self->_specimen.genome=compileCreatorBody(self->_specimen.genome);self->_rewired=YES;self->_mode.selectedSegmentIndex=1;[self modeChanged:self->_mode];}]];[self presentViewController:alert animated:YES completion:nil];
}
- (void)colorEditStarted:(UISlider*)slider {[self remember];}
- (void)colorChanged:(UISlider*)slider {
    _specimen.lineageHue=slider.value;
    UIColor* swatch=[UIColor colorWithHue:slider.value saturation:.72 brightness:1 alpha:1];
    slider.minimumTrackTintColor=swatch;
    _canvas.hue=slider.value;_practice.hue=slider.value;
    [_canvas setNeedsDisplay];[_practice setNeedsDisplay];
}
- (void)update {auto& n=_specimen.genome.genes[0].nodes[_canvas.selected];NSArray* names=@[@"Body · connects and absorbs food",@"Constructor · grows offspring; keep this cell",@"Oscillator · inherited rhythm",@"Motor · paid thrust in the marked direction",@"Sensor · relays local food signals",@"Creature sensor",@"Attack · captures material on contact; Digest makes it usable",@"Digest · makes captured material usable; combine with Attack",@"Storage · holds more usable energy; still needs food",@"Defender",@"Memory",@"Obstacle sensor",@"Sender",@"Receiver"];
    BOOL canChange=_canvas.selected!=0 && !n.constructorCell;
    _roleControls.userInteractionEnabled=canChange;_roleControls.alpha=canChange?1:.4;
    for(UIStackView* row in _roleControls.arrangedSubviews)for(UIButton* button in row.arrangedSubviews)button.enabled=canChange;
    ((UIButton*)_editControls.arrangedSubviews[1]).enabled=_canvas.selected!=0;
    BOOL unfolded=creatorUnfoldsDevelopment(_specimen.genome,_source);
    _meaning.text=unfolded?@"Independent body copy: repeated parts become separate cells; growth timing resets. Undo restores the source.":@"Tap a cell to change its role; drag to reshape.\nInherited control stays. Try body to see the consequence.";
    if(_rewired && !unfolded)_meaning.text=@"Signals explicitly rebuilt for this anatomy.\nTry body, compare with Original, or Undo to restore.";
    [_release setTitle:unfolded?@"Save independent body & release":@"Save & release" forState:UIControlStateNormal];
    _detail.text=names[NSUInteger(n.behavior.role)];
    if(n.behavior.role==CellRole::Motor && n.behavior.motorMode!=MotorMode::Thrust)_detail.text=n.behavior.motorMode==MotorMode::Bending?@"Bending motor · changes joint angle; does not swim alone":@"Contractile motor · changes link length; does not swim alone";[_canvas setNeedsDisplay];[_canvas setNeedsLayout];}
- (void)chooseRole:(CellRole)role {[self remember];if(!changeOrgan(_specimen.genome,_canvas.selected,role))_detail.text=@"The constructor stays so offspring can develop.";else [self update];}
- (void)role:(UIButton*)b {
    if(b.tag!=NSInteger(CellRole::EnergySensor)){[self chooseRole:CellRole(b.tag)];return;}
    UIAlertController* menu=[UIAlertController alertControllerWithTitle:@"Sense nearby…" message:@"Signals travel through the body. Motors respond using their marked directions." preferredStyle:UIAlertControllerStyleActionSheet];
    [menu addAction:[UIAlertAction actionWithTitle:@"Food" style:UIAlertActionStyleDefault handler:^(UIAlertAction* a){[self chooseRole:CellRole::EnergySensor];}]];
    [menu addAction:[UIAlertAction actionWithTitle:@"Creatures" style:UIAlertActionStyleDefault handler:^(UIAlertAction* a){[self chooseRole:CellRole::CreatureSensor];}]];
    [menu addAction:[UIAlertAction actionWithTitle:@"Cancel" style:UIAlertActionStyleCancel handler:nil]];
    menu.popoverPresentationController.sourceView=b;menu.popoverPresentationController.sourceRect=b.bounds;
    [self presentViewController:menu animated:YES completion:nil];
}
- (void)turn:(UIButton*)b {[self remember];auto copy=_specimen.genome;auto& n=copy.genes[0].nodes[_canvas.selected];if(n.behavior.role==CellRole::Motor){n.behavior.axisAngle+=M_PI/4;if(n.behavior.axisAngle>3.14159)n.behavior.axisAngle-=2*M_PI;}else if(_canvas.selected){auto v=n.relativePosition;float a=M_PI/6;moveBodyCell(copy,_canvas.selected,{float(v.x*cos(a)-v.y*sin(a)),float(v.x*sin(a)+v.y*cos(a))});}if(validCreatorBody(copy))_specimen.genome=copy;[self update];}
- (void)remove:(UIButton*)b {[self remember];if(removeBodyCell(_specimen.genome,_canvas.selected)){_canvas.selected=0;[self update];}else _detail.text=@"Choose an outer tip. The constructor must stay.";}
- (BOOL)textFieldShouldReturn:(UITextField*)field {[field resignFirstResponder];return YES;}
- (void)remember {_editedResult=nil;_comparison.text=@"";_undo.push_back(_specimen.genome);_undoHue.push_back(_specimen.lineageHue);_undoRewired.push_back(_rewired);if(_undo.size()>30){_undo.erase(_undo.begin());_undoHue.erase(_undoHue.begin());_undoRewired.erase(_undoRewired.begin());}}
- (void)undo:(UIButton*)button {if(_undo.empty())return;_editedResult=nil;_comparison.text=@"";_specimen.genome=_undo.back();_specimen.lineageHue=_undoHue.back();_undo.pop_back();_undoHue.pop_back();_rewired=_undoRewired.back();_undoRewired.pop_back();_canvas.selected=std::min<NSInteger>(_canvas.selected,_specimen.genome.genes[0].nodes.size()-1);_colorSlider.value=_specimen.lineageHue;_colorSlider.minimumTrackTintColor=[UIColor colorWithHue:_specimen.lineageHue saturation:.72 brightness:1 alpha:1];_canvas.hue=_specimen.lineageHue;_practice.hue=_specimen.lineageHue;if(_mode.selectedSegmentIndex==2)_mode.selectedSegmentIndex=1;[self modeChanged:_mode];}
- (void)grow:(UIButton*)button {auto p=[_canvas positions];auto v=p[_canvas.selected];float start=atan2(v.y,v.x);for(int i=0;i<12;++i)if([_canvas growAtAngle:start+i*M_PI/6])return;_detail.text=@"No room here. Choose another cell to grow from.";}
- (void)releaseBody:(UIButton*)b {if(!validCreatorBody(_specimen.genome))return;auto start=std::chrono::steady_clock::now();auto releaseGenome=prepareCreatorRelease(_specimen.genome,_source);NSLog(@"Creator DNA prepared in %.3f ms",std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count());NSString* name=[_name.text stringByTrimmingCharactersInSet:NSCharacterSet.whitespaceAndNewlineCharacterSet];_specimen.name=name.length?std::string([name substringWithRange:[name rangeOfComposedCharacterSequencesForRange:NSMakeRange(0,MIN(name.length,40))]].UTF8String):"My creature";_specimen.ecology=_rewired?"Mine · rewired signals":"Mine · inherited control";auto specimen=_specimen;specimen.genome=releaseGenome;
    if(!_completion(specimen)){UIAlertController* alert=[UIAlertController alertControllerWithTitle:@"Could not save" message:@"Your edits are still here. Free up device storage, then try Save & release again." preferredStyle:UIAlertControllerStyleAlert];[alert addAction:[UIAlertAction actionWithTitle:@"Keep editing" style:UIAlertActionStyleDefault handler:nil]];[self presentViewController:alert animated:YES completion:nil];return;}
    [self dismissViewControllerAnimated:YES completion:nil];}
- (void)cancel:(UIButton*)b {[self dismissViewControllerAnimated:YES completion:self.onCancel];}
@end
