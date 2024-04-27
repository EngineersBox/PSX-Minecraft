#include "entity.h"

#include "../math/math_utils.h"

void entityInit(Entity* entity) {
    entity->flags = (EntityFlags) {
        .alive = true,
    };
    entity->health = 0;
    entity->on_fire = 0;
}

bool iEntityAttackFrom(VSelf, const Entity* damage_source, const i16 amount) __attribute__((alias("IEntity_attackFrom")));
bool IEntity_attackFrom(VSelf, const Entity* damage_source, const i16 amount) {
    // TODO: Implement this
    return false;
}

void iEntityDamage(VSelf, const i16 amount) __attribute__((alias("IEntity_damage")));
void IEntity_damage(VSelf, const i16 amount) {
    VSELF(Entity);
    // Bounded to [0,UINT16_MAX] to ensure no integer over/underflow
    self->health = max(min(self->health - amount, ENTITY_ABS_MAX_HEALTH), 0);
}

void iEntityKill(VSelf) __attribute__((alias("IEntity_kill")));
void IEntity_kill(VSelf) {
    VSELF(Entity);
    self->health = 0;
    self->flags.alive = false;
}
