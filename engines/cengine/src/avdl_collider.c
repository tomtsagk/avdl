#include "avdl_collider.h"

void avdl_collider_create(struct avdl_collider *o) {
	o->type = AVDL_COLLIDER_TYPE_POINT;
}

void avdl_collider_clean(struct avdl_collider *o) {
}
