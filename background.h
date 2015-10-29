#ifndef BACKGROUND_H__
#define BACKGROUND_H__

struct mote_t {
	i32 width;
	u32 color;
	v2 center;
	f32 velocity;
};

struct background_t {
	array<mote_t> motes;
	color_t background_color;
	tex2 texture;
};

tex2 load_empty_bmp(i32 width, i32 height, color_t initial_color);

void create_motes(background_t* background);

void update_motes(background_t* background,
                  render_group_t* render_group,
                  f32 dt);

#endif /* end of include guard: BACKGROUND_H__ */