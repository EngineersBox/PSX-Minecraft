#pragma once

#ifndef PSX_MINECRAFT_ENTITY_H
#define PSX_MINECRAFT_ENTITY_H

#include <interface99.h>

#include "../util/inttypes.h"

typedef struct {
    i16 health;
    u16 on_fire;
} Entity;

#define IEntity_IFACE \
    vfuncDefault(void, damage, VSelf, i16 amount) \
    vfuncDefault(void, kill, VSelf)

void iEntityDamage(VSelf, i16 amount);
void IEntity_damage(VSelf, i16 amount);

void iEntityKill(VSelf);
void IEntity_kill(VSelf);

interface(IEntity);

#endif // PSX_MINECRAFT_ENTITY_H
