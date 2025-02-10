#pragma once

#ifndef PSXMC_ENTITY_H
#define PSXMC_ENTITY_H

#include <interface99.h>
#include <stdbool.h>
#include <psxgte.h>
#include <psxgpu.h>

#include "../util/preprocessor.h"
#include "../util/inttypes.h"
#include "../physics/physics_object.h"

FWD_DECL typedef struct World World;

typedef struct EntityFlags {
    bool alive: 1;
    u8 _pad: 7;
} EntityFlags;

#define ENTITY_ABS_MAX_HEALTH INT16_MAX

typedef struct Entity {
    PhysicsObject physics_object;
    EntityFlags flags;
    u8 health: 5;
    u8 armour: 5;
    u8 air: 5;
    u8 _pad: 1;
    // 0 = not on fire
    // 1+ = ticks on fire so far
    u16 on_fire;
} Entity;

typedef struct DamageContext {
    SVECTOR direction;
    i16 amount;
    bool knockback: 1;
    u8 _pad: 7;
} DamageContext;

FWD_DECL typedef struct IEntityVTable IEntityVTable;

#define IEntity_IFACE \
    vfuncDefault(bool, attackFrom, VSelf, IEntityVTable* vtable, const Entity* damage_source, const DamageContext* ctx) \
    vfuncDefault(void, damage, VSelf, const i16 amount) \
    vfuncDefault(void, kill, VSelf)

void entityInit(Entity* entity);

bool iEntityAttackFrom(VSelf, IEntityVTable* vtable, const Entity* damage_source, const DamageContext* ctx);
bool IEntity_attackFrom(VSelf, IEntityVTable* vtable, const Entity* damage_source, const DamageContext* ctx);

void iEntityDamage(VSelf, const i16 amount);
void IEntity_damage(VSelf, const i16 amount);

void iEntityKill(VSelf);
void IEntity_kill(VSelf);

interface(IEntity);

#endif // PSXMC_ENTITY_H
