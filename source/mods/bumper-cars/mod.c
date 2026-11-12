/* M19: reversible bumper kit on the current car; host applies bounded impulses. */
#include "base.h"
#include "bumper_runtime.h"
int MoonLab_ModStart(PTR domain,PTR game){
 if(!base_bind(domain,game,"bumper-cars.jsonl"))return 0;
#define M(F,T,N,S) F=dmethod("UnityEngine",T,N,S);if(!F){event(T "::" N " binding-failed");return 0;}
 M(find_m,"Object","FindObjectsByType","System.Type;UnityEngine.FindObjectsInactive;UnityEngine.FindObjectsSortMode");M(position_m,"Transform","get_position","");M(transform_m,"Component","get_transform","");M(velocity_m,"Rigidbody","get_linearVelocity","");M(force_m,"Rigidbody","AddForce","UnityEngine.Vector3;UnityEngine.ForceMode");M(component_m,"GameObject","GetComponent","System.Type");M(primitive_m,"GameObject","CreatePrimitive","UnityEngine.PrimitiveType");M(parent_m,"Transform","SetParent","UnityEngine.Transform;System.Boolean");M(local_pos_m,"Transform","set_localPosition","UnityEngine.Vector3");M(scale_m,"Transform","set_localScale","UnityEngine.Vector3");M(enabled_m,"Collider","set_enabled","System.Boolean");M(destroy_m,"Object","Destroy","UnityEngine.Object");M(go_transform_m,"GameObject","get_transform","");
#undef M
 vehicle_world_m=dmethod("MoonCleaners","MoonPrototypeVehicle","get_World","");
 spec_m=dmethod("MoonCleaners","MoonPrototypeVehicle","get_Spec","");if(!vehicle_world_m||!spec_m||!prepare_hook(method("MoonCleaners","MoonWalker","Update",0),100671733,0,game))return 0;before_hook=before;return install_hooks();
}
