#pragma once

#ifndef PSXMC_ENTITY_H
#define PSXMC_ENTITY_H

#include <interface99.h>
#include <stdbool.h>

#include "../util/preprocessor.h"
#include "../util/inttypes.h"

typedef struct {
    bool alive: 1;
    u8 _pad: 7;
} EntityFlags;

#define ENTITY_ABS_MAX_HEALTH INT16_MAX

typedef struct {
    EntityFlags flags;
    u16 health;
    // 0 = not on fire
    // 1+ = ticks on fire so far
    u16 on_fire;
} Entity;

#define IEntity_IFACE \
    vfuncDefault(bool, attackFrom, VSelf, const Entity* damage_source, const i16 amount) \
    vfuncDefault(void, damage, VSelf, const i16 amount) \
    vfuncDefault(void, kill, VSelf)

void entityInit(Entity* entity);

bool iEntityAttackFrom(VSelf, const Entity* damage_source, const i16 amount);
bool IEntity_attackFrom(VSelf, const Entity* damage_source, const i16 amount);

void iEntityDamage(VSelf, const i16 amount);
void IEntity_damage(VSelf, const i16 amount);

void iEntityKill(VSelf);
void IEntity_kill(VSelf);

interface(IEntity);

#endif // PSXMC_ENTITY_H
