/* M19: reversible bumper kit on the current car; host applies bounded impulses. */
#include "base.h"
static PTR find_m,position_m,transform_m,velocity_m,force_m,component_m,primitive_m,parent_m,local_pos_m,scale_m,enabled_m,destroy_m,go_transform_m,spec_m;
static DRoot active_vehicle,active_world,bars[4];static PTR last_hit;static float next_hit,next_scan;static int enabled;
static float square_root(float x){float n=x>1?x:1;for(int i=0;i<12;i++)n=.5f*(n+x/n);return n;}
static float dot(DVec a,DVec b){return a.x*b.x+a.y*b.y+a.z*b.z;}
static DVec sub(DVec a,DVec b){return (DVec){a.x-b.x,a.y-b.y,a.z-b.z};}
static PTR transform(PTR component,int *ok){return call(transform_m,component,0,ok);}
static void vector_set(PTR m,PTR self,DVec v,int *ok){void *a[]={&v};call(m,self,a,ok);}
static void teardown(void){for(int i=0;i<4;i++){PTR go=dget(bars[i]);if(alive(go)){int ok=1;void *a[]={go};call(destroy_m,0,a,&ok);}ddrop(bars+i);}ddrop(&active_vehicle);ddrop(&active_world);last_hit=0;enabled=0;}
static int kit(PTR world,PTR vehicle){
 int ok=1;PTR parent=transform(vehicle,&ok),spec=call(spec_m,vehicle,0,&ok);if(!ok||!parent||!spec)return 0;
 float width=2,depth=3;PTR wf=get_field(object_class(spec),"width"),df=get_field(object_class(spec),"depth");if(wf)field_value(spec,wf,&width);if(df)field_value(spec,df,&depth);
 if(!(width>=.5f&&width<=20&&depth>=.5f&&depth<=30))return 0;
 active_vehicle=dhold(vehicle);active_world=dhold(world);if(!active_vehicle||!active_world){teardown();return 0;}
 for(int i=0;i<4&&ok;i++){
  int cube=3;void *a[]={&cube};PTR go=call(primitive_m,0,a,&ok);if(!go){ok=0;break;}bars[i]=dhold(go);if(!bars[i]){void *d[]={go};call(destroy_m,0,d,&ok);ok=0;break;}
  PTR tr=call(go_transform_m,go,0,&ok);unsigned char no=0;void *pa[]={parent,&no};call(parent_m,tr,pa,&ok);
  DVec pos=i<2?(DVec){0,.55f,(i?1:-1)*(depth*.5f+.12f)}:(DVec){(i==2?1:-1)*(width*.5f+.12f),.55f,0};
  DVec scale=i<2?(DVec){width+.45f,.35f,.25f}:(DVec){.25f,.35f,depth+.45f};vector_set(local_pos_m,tr,pos,&ok);vector_set(scale_m,tr,scale,&ok);
  PTR collider_type=dtype("UnityEngine","Collider");void *ca[]={collider_type};PTR collider=call(component_m,go,ca,&ok);if(collider){void *ea[]={&no};call(enabled_m,collider,ea,&ok);}
 }
 if(!ok){teardown();return 0;}enabled=1;return 1;
}
static void bump(PTR vehicle){
 int ok=1;PTR body=0;field_value(vehicle,body_f,&body);if(!alive(body))return;
 PTR type=dtype("MoonCleaners","MoonPrototypeVehicle");int inactive=0,sort=0;void *fa[]={type,&inactive,&sort};PTR array=call(find_m,0,fa,&ok);if(!ok||!array)return;
 DVec origin=dvector(position_m,transform(vehicle,&ok),&ok),velocity=dvector(velocity_m,body,&ok);U64 count=da.array_length(array);if(count>128)count=128;
 for(U64 i=0;i<count&&ok;i++){
  PTR slot=darray(array,i),other=slot?*(PTR*)slot:0;if(other==vehicle||!alive(other)||(other==last_hit&&clock_now<next_hit))continue;
  PTR other_body=0;field_value(other,body_f,&other_body);if(!alive(other_body))continue;
  DVec delta=sub(dvector(position_m,transform(other,&ok),&ok),origin);float d2=dot(delta,delta);if(d2<.04f||d2>12.25f)continue;
  /* A conservative proximity bumper, not an OnCollision replacement. The
   * current car pushes away only when closing on another native host car. */
  float distance=square_root(d2);DVec n={delta.x/distance,delta.y/distance,delta.z/distance};DVec ov=dvector(velocity_m,other_body,&ok);float closing=dot(sub(velocity,ov),n);if(!ok||closing<.5f)continue;
  float power=closing*.7f+1;if(power>6)power=6;DVec push={n.x*power,n.y*power,n.z*power},back={-push.x,-push.y,-push.z};int mode=2;void *a[]={&push,&mode},*b[]={&back,&mode};call(force_m,other_body,a,&ok);call(force_m,body,b,&ok);last_hit=other;next_hit=clock_now+.4f;event(ok?"bumper-contact":"bumper-call-failed");break;
 }
}
static int before(int id,Context *c){
 (void)id;if(!on_thread())return 0;int ok=1;clock_now=dnumber(time_m,0,&ok);PTR world=0,vehicle=0;int edge=pressed('B'),ready=input_ready((PTR)c->rcx,&world,&vehicle,1);
 if(enabled&&(!ready||vehicle!=dget(active_vehicle)||world!=dget(active_world)))teardown();
 if(ready&&edge){if(enabled){teardown();event("bumper-kit-removed");}else event(kit(world,vehicle)?"bumper-kit-equipped":"bumper-kit-failed");}
 if(enabled&&clock_now>=next_scan){next_scan=clock_now+.1f;bump(vehicle);}return 0;
}
int MoonLab_ModStart(PTR domain,PTR game){
 if(!base_bind(domain,game,"bumper-cars.jsonl"))return 0;
#define M(F,T,N,S) F=dmethod("UnityEngine",T,N,S);if(!F){event(T "::" N " binding-failed");return 0;}
 M(find_m,"Object","FindObjectsByType","System.Type;UnityEngine.FindObjectsInactive;UnityEngine.FindObjectsSortMode");M(position_m,"Transform","get_position","");M(transform_m,"Component","get_transform","");M(velocity_m,"Rigidbody","get_linearVelocity","");M(force_m,"Rigidbody","AddForce","UnityEngine.Vector3;UnityEngine.ForceMode");M(component_m,"GameObject","GetComponent","System.Type");M(primitive_m,"GameObject","CreatePrimitive","UnityEngine.PrimitiveType");M(parent_m,"Transform","SetParent","UnityEngine.Transform;System.Boolean");M(local_pos_m,"Transform","set_localPosition","UnityEngine.Vector3");M(scale_m,"Transform","set_localScale","UnityEngine.Vector3");M(enabled_m,"Collider","set_enabled","System.Boolean");M(destroy_m,"Object","Destroy","UnityEngine.Object");M(go_transform_m,"GameObject","get_transform","");
#undef M
 spec_m=dmethod("MoonCleaners","MoonPrototypeVehicle","get_Spec","");if(!spec_m||!prepare_hook(method("MoonCleaners","MoonWalker","Update",0),100671733,0,game))return 0;before_hook=before;return install_hooks();
}
