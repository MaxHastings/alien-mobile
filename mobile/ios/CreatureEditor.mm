#import "CreatureEditor.h"
#include "alienmobile/Creator.h"
using namespace alienmobile;
@interface BodyCanvas : UIView
@property(nonatomic) Genome* genome;
@property(nonatomic) float hue;
@property(nonatomic) NSInteger selected;
@property(nonatomic,copy) void (^changed)(void);
@property(nonatomic,copy) void (^willEdit)(void);
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
        moveBodyCell(*self.genome,self.selected,{float(cos(angle)*distance),float(sin(angle)*distance)});
        [self setNeedsDisplay];[self setNeedsLayout];if(self.changed)self.changed();
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
@interface CreatureEditor () <UITextFieldDelegate>
@end
@implementation CreatureEditor {
    SpecimenSnapshot _specimen;void (^_completion)(SpecimenSnapshot);BodyCanvas* _canvas;UILabel* _detail;UITextField* _name;std::vector<Genome> _undo;
}
- (instancetype)initWithSpecimen:(SpecimenSnapshot)s completion:(void (^)(SpecimenSnapshot))completion {if(self=[super init]){_specimen=s;_specimen.genome=editableBody(s.genome);_completion=[completion copy];}return self;}
- (UIButton*)button:(NSString*)title action:(SEL)action tag:(NSInteger)tag {UIButton* b=[UIButton buttonWithType:UIButtonTypeSystem];[b setTitle:title forState:UIControlStateNormal];[b setTitleColor:[UIColor colorWithWhite:.94 alpha:1] forState:UIControlStateNormal];b.tag=tag;b.titleLabel.font=[UIFont systemFontOfSize:14 weight:UIFontWeightSemibold];b.backgroundColor=[UIColor colorWithWhite:1 alpha:.07];b.layer.cornerRadius=12;[b.heightAnchor constraintEqualToConstant:44].active=YES;[b addTarget:self action:action forControlEvents:UIControlEventTouchUpInside];return b;}
- (void)viewDidLoad {
    [super viewDidLoad];self.view.backgroundColor=[UIColor colorWithRed:.025 green:.045 blue:.065 alpha:1];self.modalInPresentation=YES;
    UIStackView* stack=[[UIStackView alloc] init];stack.axis=UILayoutConstraintAxisVertical;stack.spacing=12;stack.translatesAutoresizingMaskIntoConstraints=NO;[self.view addSubview:stack];
    _name=[[UITextField alloc] init];_name.text=[NSString stringWithUTF8String:_specimen.name.c_str()];_name.textColor=UIColor.whiteColor;_name.font=[UIFont systemFontOfSize:24 weight:UIFontWeightSemibold];_name.accessibilityLabel=@"Creature name";_name.returnKeyType=UIReturnKeyDone;_name.delegate=self;[stack addArrangedSubview:_name];
    UILabel* tip=[[UILabel alloc] init];tip.text=@"Tap a cell to change its role. Drag it to reshape.\nTap empty space to grow from the selected cell.";tip.numberOfLines=2;tip.font=[UIFont systemFontOfSize:14];tip.textColor=UIColor.lightGrayColor;[stack addArrangedSubview:tip];
    _canvas=[[BodyCanvas alloc] init];_canvas.genome=&_specimen.genome;_canvas.hue=_specimen.lineageHue;_canvas.backgroundColor=[UIColor colorWithWhite:1 alpha:.025];_canvas.layer.cornerRadius=20;[stack addArrangedSubview:_canvas];[_canvas.heightAnchor constraintGreaterThanOrEqualToConstant:170].active=YES;
    _detail=[[UILabel alloc] init];_detail.numberOfLines=2;_detail.font=[UIFont systemFontOfSize:13];_detail.textColor=UIColor.lightGrayColor;[stack addArrangedSubview:_detail];
    NSArray* names=@[@"Body",@"Motor",@"Sensor",@"Storage",@"Attack",@"Digest"];
    int roles[]={0,3,4,8,6,7};
    for(int row=0;row<2;row++){UIStackView* line=[[UIStackView alloc] init];line.spacing=8;line.distribution=UIStackViewDistributionFillEqually;for(int col=0;col<3;col++){int i=row*3+col;[line addArrangedSubview:[self button:names[i] action:@selector(role:) tag:roles[i]]];}[stack addArrangedSubview:line];}
    UIStackView* edit=[[UIStackView alloc] init];edit.spacing=8;edit.distribution=UIStackViewDistributionFillEqually;[edit addArrangedSubview:[self button:@"Grow" action:@selector(grow:) tag:0]];[edit addArrangedSubview:[self button:@"Turn" action:@selector(turn:) tag:0]];[edit addArrangedSubview:[self button:@"Remove tip" action:@selector(remove:) tag:0]];[stack addArrangedSubview:edit];
    UIButton* release=[self button:@"Save & release" action:@selector(releaseBody:) tag:0];release.backgroundColor=[UIColor colorWithRed:.05 green:.38 blue:.39 alpha:1];[stack addArrangedSubview:release];UIStackView* bottom=[[UIStackView alloc] init];bottom.spacing=8;bottom.distribution=UIStackViewDistributionFillEqually;[bottom addArrangedSubview:[self button:@"Undo" action:@selector(undo:) tag:0]];[bottom addArrangedSubview:[self button:@"Cancel" action:@selector(cancel:) tag:0]];[stack addArrangedSubview:bottom];
    [NSLayoutConstraint activateConstraints:@[[stack.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor constant:20],[stack.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor constant:-16],[stack.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:20],[stack.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-20]]];
    __weak CreatureEditor* owner=self;_canvas.changed=^{[owner update];};_canvas.willEdit=^{[owner remember];};[self update];
}
- (void)update {auto& n=_specimen.genome.genes[0].nodes[_canvas.selected];NSArray* names=@[@"Body · connects and absorbs food",@"Constructor · grows offspring; keep this cell",@"Oscillator · inherited rhythm",@"Motor · paid thrust in the marked direction",@"Sensor · relays local food signals",@"Creature sensor",@"Attack · extracts on physical contact",@"Digest · converts captured material",@"Storage · larger energy reservoir",@"Defender",@"Memory",@"Obstacle sensor",@"Sender",@"Receiver"];
    _detail.text=names[NSUInteger(n.behavior.role)];[_canvas setNeedsDisplay];[_canvas setNeedsLayout];}
- (void)role:(UIButton*)b {[self remember];if(!changeOrgan(_specimen.genome,_canvas.selected,CellRole(b.tag)))_detail.text=@"The constructor stays so offspring can develop.";else [self update];}
- (void)turn:(UIButton*)b {[self remember];auto copy=_specimen.genome;auto& n=copy.genes[0].nodes[_canvas.selected];if(n.behavior.role==CellRole::Motor){n.behavior.axisAngle+=M_PI/4;if(n.behavior.axisAngle>3.14159)n.behavior.axisAngle-=2*M_PI;}else if(_canvas.selected){auto v=n.relativePosition;float a=M_PI/6;moveBodyCell(copy,_canvas.selected,{float(v.x*cos(a)-v.y*sin(a)),float(v.x*sin(a)+v.y*cos(a))});}if(validCreatorBody(copy))_specimen.genome=copy;[self update];}
- (void)remove:(UIButton*)b {[self remember];if(removeBodyCell(_specimen.genome,_canvas.selected)){_canvas.selected=0;[self update];}else _detail.text=@"Choose an outer tip. Keep the constructor and two cells.";}
- (BOOL)textFieldShouldReturn:(UITextField*)field {[field resignFirstResponder];return YES;}
- (void)remember {_undo.push_back(_specimen.genome);if(_undo.size()>30)_undo.erase(_undo.begin());}
- (void)undo:(UIButton*)button {if(_undo.empty())return;_specimen.genome=_undo.back();_undo.pop_back();_canvas.selected=std::min<NSInteger>(_canvas.selected,_specimen.genome.genes[0].nodes.size()-1);[self update];}
- (void)grow:(UIButton*)button {auto p=[_canvas positions];auto v=p[_canvas.selected];float start=atan2(v.y,v.x);for(int i=0;i<12;++i)if([_canvas growAtAngle:start+i*M_PI/6])return;_detail.text=@"No room here. Choose another cell to grow from.";}
- (void)releaseBody:(UIButton*)b {if(!validCreatorBody(_specimen.genome))return;NSString* name=[_name.text stringByTrimmingCharactersInSet:NSCharacterSet.whitespaceAndNewlineCharacterSet];_specimen.name=name.length?std::string([name substringWithRange:[name rangeOfComposedCharacterSequencesForRange:NSMakeRange(0,MIN(name.length,40))]].UTF8String):"My creature";_specimen.ecology="Mine";_specimen.initialEnergy=1.f;auto specimen=_specimen;auto completion=_completion;[self dismissViewControllerAnimated:YES completion:^{completion(specimen);}];}
- (void)cancel:(UIButton*)b {[self dismissViewControllerAnimated:YES completion:self.onCancel];}
@end
