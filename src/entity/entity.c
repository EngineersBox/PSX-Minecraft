#include "entity.h"

#include "../core/std/stdlib.h"
#include "../math/math_utils.h"
#include "../util/memory.h"
#include "../logging/logging.h"

Entity* entityNew() {
    Entity* entity = malloc(sizeof(Entity));
    zeroed(entity);
    return entity;
}

void entityInit(Entity* entity) {
    entity->flags = (EntityFlags) {
        .alive = true,
    };
    entity->health = 0;
    entity->on_fire = 0;
    entity->health = 0;
    entity->armour = 0;
    entity->air = 0;
}

bool iEntityAttackFrom(VSelf,
                       IEntityVTable* vtable,
                       const Entity* damage_source,
                       const DamageContext* ctx) ALIAS("IEntity_attackFrom");
bool IEntity_attackFrom(VSelf,
                        IEntityVTable* vtable,
                        MAYBE_UNUSED const Entity* damage_source,
                        const DamageContext* ctx) {
    VSELF(Entity);
    if (self->health <= 0) {
        return false;
    }
    vtable->damage(self, ctx->amount);
    UNIMPLEMENTED();
    // TODO: Apply dmanage based on context
    return false;
}

void iEntityDamage(VSelf, const i16 amount) ALIAS("IEntity_damage");
void IEntity_damage(VSelf, const i16 amount) {
    VSELF(Entity);
    // Bounded to [0,INT16_MAX] to ensure no integer over/underflow
    self->health = max(min((i16)self->health - amount, ENTITY_ABS_MAX_HEALTH), 0);
    if (self->health == 0) {
        self->flags.alive = false;
    }
}

void iEntityKill(VSelf) ALIAS("IEntity_kill");
void IEntity_kill(VSelf) {
    VSELF(Entity);
    self->health = 0;
    self->flags.alive = false;
}
