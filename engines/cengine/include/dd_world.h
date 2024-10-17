#ifndef DD_WORLD_H
#define DD_WORLD_H

#ifdef __cplusplus
extern "C" {
#endif

/* data that each world has
 */
extern struct dd_world {
	void (*create)(struct dd_world *);
	void (*onload)(struct dd_world *);
	void (*update)(struct dd_world *, float);
	void (*resize)(struct dd_world *);
	void (*draw)(struct dd_world *);

	void (*input)(struct dd_world *, int button, int type);
	void (*clean)(struct dd_world *);

} *cworld, *nworld;

/*
 * new world data
 */
extern void (*nworld_constructor)(struct dd_world*);
extern int nworld_size;
extern int nworld_ready;
extern int nworld_loading;

/* constructor destructor
 */
struct dd_world *dd_world_create_dynamic();

/*
 * handy function to change between worlds
 *
 */
#define dd_world_prepare(WORLD, PERCENT) nworld_constructor = WORLD ## _create; nworld_size = sizeof(struct WORLD); nworld_ready = 0; avdl_assetManager_setPercentage(PERCENT);
#define dd_world_ready() nworld_ready = 1;
#define dd_world_prepareReady(WORLD, PERCENT) dd_world_prepare(WORLD, PERCENT); dd_world_ready();
void dd_world_change(int size, void (*constructor)(struct dd_world *));

void dd_world_create(struct dd_world *);
void dd_world_onload(struct dd_world *);
void dd_world_update(struct dd_world *, float dt);
void dd_world_draw(struct dd_world *);
void dd_world_input(struct dd_world *, int button, int type);
void dd_world_clean(struct dd_world *);

// default world
extern int dd_default_world_size;
extern void (*dd_default_world_constructor)(struct dd_world *);
#define dd_world_set_default(WORLD) dd_default_world_size = sizeof(struct WORLD); dd_default_world_constructor = WORLD ## _create;

#ifdef __cplusplus
}
#endif

#endif
