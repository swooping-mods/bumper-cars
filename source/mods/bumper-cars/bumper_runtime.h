/* Presentation and contacts remain local feature policy; native physics runs normally. */
static PTR find_m,position_m,transform_m,velocity_m,force_m,component_m,primitive_m,parent_m,local_pos_m,scale_m,enabled_m,destroy_m,go_transform_m,spec_m,vehicle_world_m;
static DRoot active_vehicle,active_world,active_walker,bars[4];static float next_scan,last_clock;static int enabled,cleaning;
static struct {DRoot vehicle;float until;} hits[128];
static float square_root(float x){float n=x>1?x:1;for(int i=0;i<12;i++)n=.5f*(n+x/n);return n;}
static float dot(DVec a,DVec b){return a.x*b.x+a.y*b.y+a.z*b.z;}
static DVec sub(DVec a,DVec b){return (DVec){a.x-b.x,a.y-b.y,a.z-b.z};}
static int finite_float(float f){return f<=3.402823466e38F&&f>=-3.402823466e38F;}
static int vector_read(PTR m,PTR self,DVec *out){
 int ok=1;if(!self)return 0;PTR box=call(m,self,0,&ok);PTR value=ok&&box?unbox(box):0;
 if(!value)return 0;dcopy(out,value,sizeof(*out));return finite_float(out->x)&&finite_float(out->y)&&finite_float(out->z);
}
static int target_alive(PTR target,int *ok){
 if(!target)return 0;void *args[]={target};PTR box=call(alive_m,0,args,ok);unsigned char *value=*ok&&box?unbox(box):0;
 if(!value){*ok=0;return 0;}return *value!=0;
}
static PTR transform(PTR component,int *ok){return component?call(transform_m,component,0,ok):0;}
static void vector_set(PTR m,PTR self,DVec v,int *ok){if(!*ok||!self){*ok=0;return;}void *a[]={&v};call(m,self,a,ok);}
static void clear_hits(void){for(int i=0;i<128;i++){ddrop(&hits[i].vehicle);hits[i].until=0;}}
/* Failed Destroy/liveness calls retain their roots and block re-equipping. */
static int teardown(void){
 int complete=1;enabled=0;cleaning=1;
 for(int i=0;i<4;i++)if(bars[i]){int ok=1;PTR go=dget(bars[i]);if(target_alive(go,&ok)){void *a[]={go};call(destroy_m,0,a,&ok);}if(ok)ddrop(bars+i);else complete=0;}
 if(!complete)return 0;ddrop(&active_vehicle);ddrop(&active_world);ddrop(&active_walker);clear_hits();next_scan=0;cleaning=0;return 1;
}
static int kit(PTR walker,PTR world,PTR vehicle){
 int ok=1;PTR actual_world=call(vehicle_world_m,vehicle,0,&ok);if(!ok||actual_world!=world)return 0;PTR parent=transform(vehicle,&ok);if(!ok||!parent)return 0;PTR spec=call(spec_m,vehicle,0,&ok);if(!ok||!spec)return 0;
 float width=2,depth=3;PTR wf=get_field(object_class(spec),"width"),df=get_field(object_class(spec),"depth");if(wf)field_value(spec,wf,&width);if(df)field_value(spec,df,&depth);
 if(!(width>=.5f&&width<=20&&depth>=.5f&&depth<=30))return 0;
 active_vehicle=dhold(vehicle);active_world=dhold(world);active_walker=dhold(walker);if(!active_vehicle||!active_world||!active_walker){teardown();return 0;}
 PTR collider_type=dtype("UnityEngine","Collider");if(!collider_type){teardown();return 0;}
 for(int i=0;i<4&&ok;i++){
  int cube=3;void *a[]={&cube};PTR go=call(primitive_m,0,a,&ok);if(!go){ok=0;break;}bars[i]=dhold(go);if(!bars[i]){int cleanup_ok=1;void *d[]={go};call(destroy_m,0,d,&cleanup_ok);ok=0;break;}
  unsigned char no=0;void *ca[]={collider_type};PTR collider=ok?call(component_m,go,ca,&ok):0;
  if(!ok||!collider){ok=0;break;}void *ea[]={&no};call(enabled_m,collider,ea,&ok);if(!ok)break;
  PTR tr=call(go_transform_m,go,0,&ok);if(!ok||!tr){ok=0;break;}void *pa[]={parent,&no};call(parent_m,tr,pa,&ok);
  DVec pos=i<2?(DVec){0,.55f,(i?1:-1)*(depth*.5f+.12f)}:(DVec){(i==2?1:-1)*(width*.5f+.12f),.55f,0};
  DVec scale=i<2?(DVec){width+.45f,.35f,.25f}:(DVec){.25f,.35f,depth+.45f};vector_set(local_pos_m,tr,pos,&ok);vector_set(scale_m,tr,scale,&ok);
 }
 if(!ok){teardown();return 0;}clear_hits();next_scan=0;enabled=1;return 1;
}
static int hit_slot(PTR other){
 int slot=-1;
 for(int i=0;i<128;i++){
  if(hits[i].vehicle&&clock_now>=hits[i].until)ddrop(&hits[i].vehicle);
  if(dget(hits[i].vehicle)==other)return -1;
  if(!hits[i].vehicle&&slot<0)slot=i;
 }return slot;
}
static void bump(PTR vehicle){
 float hit_until=0;if(!vehicle_read_deadline(clock_now,.4f,&hit_until))return;
 int ok=1;PTR body=0;field_value(vehicle,body_f,&body);if(!alive(body))return;
 DVec origin,velocity;PTR tr=transform(vehicle,&ok);if(!ok||!vector_read(position_m,tr,&origin)||!vector_read(velocity_m,body,&velocity))return;
 PTR type=dtype("MoonCleaners","MoonPrototypeVehicle");if(!type)return;int inactive=0,sort=0;void *fa[]={type,&inactive,&sort};PTR array=call(find_m,0,fa,&ok);if(!ok||!array)return;
 DRoot scan=dhold(array);if(!scan)return;U64 count=da.array_length(array);if(count>128)count=128;
 for(U64 i=0;i<count;i++){
  PTR slot=darray(array,i),other=slot?*(PTR*)slot:0;if(other==vehicle||!alive(other))continue;
  /* Enumeration spans loaded scenes; never push a car belonging to another world. */
  ok=1;PTR other_world=call(vehicle_world_m,other,0,&ok);if(!ok||other_world!=dget(active_world))continue;
  int hit=hit_slot(other);if(hit<0)continue;
  PTR other_body=0;field_value(other,body_f,&other_body);if(other_body==body||!alive(other_body))continue;
  DVec position,ov;ok=1;tr=transform(other,&ok);if(!ok||!vector_read(position_m,tr,&position)||!vector_read(velocity_m,other_body,&ov))continue;
  DVec delta=sub(position,origin);float d2=dot(delta,delta);if(!(d2>=.04f&&d2<=12.25f))continue;
  float distance=square_root(d2);DVec n={delta.x/distance,delta.y/distance,delta.z/distance};float closing=dot(sub(velocity,ov),n);if(!finite_float(closing)||closing<.5f)continue;
  /* Reserve a rooted identity before either non-transactional native force call. */
  hits[hit].vehicle=dhold(other);if(!hits[hit].vehicle)continue;hits[hit].until=hit_until;
  float power=closing*.7f+1;if(power>6)power=6;DVec push={n.x*power,n.y*power,n.z*power},back={-push.x,-push.y,-push.z};int mode=2;void *a[]={&push,&mode},*b[]={&back,&mode};
  call(force_m,other_body,a,&ok);if(ok)call(force_m,body,b,&ok);event(ok?"bumper-contact":"bumper-call-failed");break;
 }ddrop(&scan);
}
static int before(int id,Context *c){
 (void)id;if(!on_thread())return 0;PTR local=0;
 if(!moon_local_actor_read(&local)||!local){(void)moon_input_action("bumper-cars.toggle","Alt+B",0);if(enabled||cleaning)teardown();return 0;}if(local!=(PTR)c->rcx)return 0;
 if(cleaning){(void)moon_input_action("bumper-cars.toggle","Alt+B",0);teardown();return 0;}
 int ok=1;PTR box=call(time_m,0,0,&ok);float *time=ok&&box?unbox(box):0;
 float scan_until=0,hit_until=0;
 if(!time||!vehicle_read_deadline(*time,.1f,&scan_until)||!vehicle_read_deadline(*time,.4f,&hit_until)){
  (void)moon_input_action("bumper-cars.toggle","Alt+B",0);if(enabled)teardown();return 0;
 }clock_now=*time;
 if(clock_now<last_clock){clear_hits();next_scan=0;}last_clock=clock_now;
 PTR world=0,vehicle=0;int ready=input_ready(local,&world,&vehicle,1);
 int edge=moon_input_action("bumper-cars.toggle","Alt+B",ready).pressed;
 if(enabled&&(!ready||local!=dget(active_walker)||vehicle!=dget(active_vehicle)||world!=dget(active_world))){teardown();return 0;}
 if(ready&&edge){if(enabled){event(teardown()?"bumper-kit-removed":"bumper-cleanup-pending");}else event(kit(local,world,vehicle)?"bumper-kit-equipped":"bumper-kit-failed");}
 if(enabled&&clock_now>=next_scan){next_scan=scan_until;bump(vehicle);}return 0;
}
