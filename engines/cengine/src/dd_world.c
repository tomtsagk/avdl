#include "dd_world.h"
#include <stdlib.h>
#include <stdio.h>

// Initialise extern variables
void (*nworld_constructor)(struct dd_world*) = 0;
int nworld_size = 0;
int nworld_ready = 0;
int nworld_loading = 0;
int dd_default_world_size = 0;
void (*dd_default_world_constructor)(struct dd_world *) = 0;
//struct dd_world *cworld = 0;
//struct dd_world *nworld = 0;

/* constructor creates an empty world
 */
struct dd_world *dd_world_create_dynamic() {
	struct dd_world *w = malloc(sizeof(struct dd_world));
	dd_world_create(w);
	return w;
}

/* destructor frees everything
 */
void dd_world_clean(struct dd_world *w) {
	//free(w);
}

// change to a new world, given size to allocate and constructor
void dd_world_change(int size, void (*constructor)(struct dd_world *)) {

	/*
	// free any previous world
	if (cworld) {
		cworld->clean(cworld);
		cworld = 0;
	}

	// allocate new world and construct it
	cworld = malloc(size);
	constructor(cworld);

	// reset new world data
	nworld_size = 0;
	nworld_constructor = 0;
	nworld_ready = 0;

	// notify new world it's all loaded
	cworld->onload(cworld);

	// resize the new world
	if (cworld->resize) {
		cworld->resize(cworld);
	}
	*/
}

void dd_world_create(struct dd_world *this) {
	this->create = 0;
	this->onload = dd_world_onload;
	this->update = 0;
	this->resize = 0;
	this->draw = 0;
	this->clean = dd_world_clean;

	this->input = 0;
}
void dd_world_onload(struct dd_world *this) {}

void dd_world_update(struct dd_world *this, float dt) {}
void dd_world_draw  (struct dd_world *this) {}

void dd_world_input(struct dd_world *this, int button, int type) {}
