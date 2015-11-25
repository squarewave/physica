#ifndef BACKGROUND_H__
#define BACKGROUND_H__

struct mote_attractor_t {
    v2 position;
};

struct mote_t {
	i32 width;
	u32 color;
	v2 center;
    v2 velocity;
    f32 z;
};

struct floaty_t {
    tex2 texture;
    rect_i source_rect;
    v2 center;
    f32 velocity;
    f32 z;
};

struct background_t {
	array<mote_t> motes;
    array<floaty_t> floaties;
    array<mote_attractor_t> attractors;
    array<tex2> textures;
	color_t background_color;
    f32 wind_x;
};

void create_background(game_state_t* game_state,
                       background_t* background);

void update_background(background_t* background,
                       render_group_t* render_group,
                       camera_t camera,
                       f32 dt);

#endif /* end of include guard: BACKGROUND_H__ */