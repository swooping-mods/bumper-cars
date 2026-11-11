# Bumper Cars: behavior and implementation

Describes the exported implementation **0.1.0**. The linked source and metadata define this version; gameplay qualification is a separate requirement.

## Behavior

Alt+B equips or removes four visual bumper bars on the current car. Host-side proximity contacts run at 10 Hz, inspect at most 128 cars, and apply equal/opposite velocity-change impulses to closing neighbors within 3.5 m. Impulses are capped at 6 and repeated contacts have a 0.4-second cooldown. Bar dimensions follow the vehicle specification; visual colliders are disabled so the kit does not silently change compound mass or collision geometry.

## State and lifecycle

The kit and cooldowns are session-local. Focus/UI, vehicle and world changes remove mod-owned visuals. Native vehicle physics remains in place; this feature adds bounded arcade contact impulses and does not replace Vehicle Framework physics routing.

## Integration and configuration

No additional feature-mod runtime dependency is declared. Included shared headers are source reuse, not extra DLL dependencies.

The [module metadata](../source/mods/bumper-cars/mod.json) owns the version and declared integration contract. Native modules start at `MoonLab_ModStart`. Game/Unity integrations target the pinned Steam Windows x64 IL2CPP build.

## Implementation sources

- [mod.c](../source/mods/bumper-cars/mod.c)
- [base.h](../source/mods/bumper-cars/base.h)

## Verification boundary

No purchasable chassis, arena, scoring, saved kit or guest presentation protocol. Proximity response, actual collisions and co-op behavior remain separate game checks.

`python3 build.py --check` verifies the exported source hashes. It does not execute the mod or qualify the behaviors above. Installation and independent build prerequisites are in the [README](../README.md).
