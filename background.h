#ifndef BACKGROUND_H_
#define BACKGROUND_H_

struct mote_attractor_ {
    v2 position;
};

struct mote_ {
	i32 width;
	u32 color;
	v2 center;
    v2 velocity;
    f32 z;
};

struct floaty_ {
    tex2 texture;
    rect_i source_rect;
    v2 center;
    f32 velocity;
    f32 z;
};

struct background_ {
	array<mote_> motes;
    array<floaty_> floaties;
    array<mote_attractor_> attractors;
    array<tex2> textures;
	color_ background_color;
    f32 wind_x;
};

void create_background(game_state_* game_state,
                       background_* background);

void update_background(background_* background,
                       render_group_* render_group,
                       camera_ camera,
                       f32 dt);

#endif /* end of include guard: BACKGROUND_H_ */
