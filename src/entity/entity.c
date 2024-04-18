#include "entity.h"

void iEntityDamage(VSelf, i16 amount) __attribute__((alias("IEntity_damage")));
void IEntity_damage(VSelf, i16 amount) {
    VSELF(Entity);
    self->health -= amount;
}

void iEntityKill(VSelf) __attribute__((alias("IEntity_kill")));
void IEntity_kill(VSelf) {
    VSELF(Entity);
    self->health = 0;
}
