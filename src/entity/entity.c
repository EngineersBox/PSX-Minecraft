#include "entity.h"

void iEntityDamage(VSelf, i16 amount) __attribute__((alias("IEntity_damage")));
void IEntity_damage(VSelf, i16 amount) {
    VSELF(Entity);
    self->health -= amount;
}
