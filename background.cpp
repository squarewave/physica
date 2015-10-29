#include "background.h"
#include "random.h"

global u32 palette[22] = {
	0xffd7eaf1, // 0: sky blue
	0xffd7eaf1, // 0: sky blue
	0xffd7eaf1, // 0: sky blue
	0xfff8fde3, // 1: cloud white
	0xfff8fde3, // 1: cloud white
	0xfff8fde3, // 1: cloud white
	0xfff8fde3, // 1: cloud white
	0xfff8fde3, // 1: cloud white
	0xfff8fde3, // 1: cloud white
	0xff8b946d, // 2: dark grass green
	0xff8b946d, // 2: dark grass green
	0xffd9bb73, // 3: orange-ish
	0xffd9bb73, // 3: orange-ish
	0xffd9bb73, // 3: orange-ish
	0xffd9bb73, // 3: orange-ish
	0xff8f8672, // 4: brown
	0xff8f8672, // 4: brown
	0xffd4e3ec, // 5: light blue
	0xffd4e3ec, // 5: light blue
	0xffd4e3ec, // 5: light blue
	0xffd4e3ec, // 5: light blue
	0xffd4e3ec, // 5: light blue
};

tex2 load_empty_bmp(i32 width, i32 height, color_t initial_color) {
	tex2 result = {};
	result.width = width;
	result.height = height;
	result.pitch = width;

	const i32 bytes_per_pixel = 4;
    u32 *pixels = (u32*)malloc(width * height * bytes_per_pixel);

    u32 u32_initial_color = from_rgb(initial_color);

    for (i32 y = 0; y < height; ++y) {
    	for (i32 x = 0; x < width; ++x) {
    		pixels[y * width + x] = u32_initial_color;
    	}
    }

    result.pixels = pixels;

    return result;
}

void create_motes(background_t* background) {
	for (i32 i = 0; i < background->motes.count; ++i) {
		mote_t* mote = background->motes.at(i);

		mote->width = random_i32(1, 5);
		mote->color = palette[random_i32(0, ARRAY_SIZE(palette))];
		mote->center = v2 {
			random_f32(0, (f32)background->texture.width),
			(f32)random_i32(0, background->texture.height)
		};
		mote->velocity = random_f32(-2.0f, 2.0f);
	}
}

void update_motes(background_t* background,
                  render_group_t* render_group,
                  f32 dt) {
	i32 width = background->texture.width;
	i32 height = background->texture.height;
    u32 u32_background_color = from_rgb(background->background_color);

    for (i32 y = 0; y < height; ++y) {
    	for (i32 x = 0; x < width; ++x) {
    		background->texture.pixels[y * width + x] = u32_background_color;
    	}
    }

	for (i32 i = 0; i < background->motes.count; ++i) {
		mote_t* mote = background->motes.at(i);

		mote->center.x += mote->velocity * dt;

		// push_rect(render_group,
		//           to_rgb(mote->color),
		//           mote->center * (1.0f / 30.0f),
		//           v2 { (f32)mote->width, 1.0f} * (1.0f / 30.0f),
		//           0.0f,)

		i32 start_x = sround(mote->center.x - (f32)mote->width / 2.0f);
		i32 y = sround(mote->center.y);

		u32 color = mote->color;
		i32 mote_width = mote->width;

		mote_width = fmin(start_x + mote_width, width) - start_x;
		start_x = fmax(start_x, 0);

		u32* pixel = background->texture.pixels + (y * width) + start_x;
		for (int i = 0; i < mote_width; ++i) {
			*pixel = color;
			pixel++;
		}
	}
}