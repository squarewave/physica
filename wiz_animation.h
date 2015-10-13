#ifndef WIZ_ANIMATION_H__
#define WIZ_ANIMATION_H__

const i32 WIZ_WIDTH = 18;
const i32 WIZ_HEIGHT = 24;

u32* inverted_coord(tex2 texture, v2i pos) {
    return texture.pixels + (texture.height - pos.y) * texture.pitch + pos.x;
}

tex2 load_wiz_bmp() {
    return load_bmp((char*)"assets/wiz.bmp", 1);
}

tex2 wiz_walking_0(tex2 wiz_bmp) {
    tex2 result;
    result.pitch = wiz_bmp.pitch;
    result.width = WIZ_WIDTH;
    result.height = WIZ_HEIGHT;
    v2i bottom_left = {0, WIZ_HEIGHT};
    result.pixels = inverted_coord(wiz_bmp, bottom_left);
    return result;
}

animation_spec_t wiz_walking_right(vec<animation_frame_t>* frames, tex2 wiz_bmp) {
	animation_spec_t result;
	result.frames.count = 6;
	result.frames.values = frames->at(frames->count);

	for (int i = 0; i < result.frames.count; ++i) {
		i32 index = frames->push_unassigned();
		animation_frame_t* frame = frames->at(index);
		frame->duration = 0.15f;
		frame->pixel_size = 3.0f;
		frame->orientation = 0.0f;
		frame->hotspot = v2{ 7.0f, 6.0f };

		tex2 texture;
		texture.pitch = wiz_bmp.pitch;
		texture.width = WIZ_WIDTH;
		texture.height = WIZ_HEIGHT;
	    v2i bottom_left = {WIZ_WIDTH * i, WIZ_HEIGHT};
	    texture.pixels = inverted_coord(wiz_bmp, bottom_left);
	    frame->texture = texture;
	}

	return result;
}

animation_spec_t wiz_standing_right(vec<animation_frame_t>* frames, tex2 wiz_bmp) {
	animation_spec_t result;
	result.frames.count = 1;
	result.frames.values = frames->at(frames->count);

	for (int i = 0; i < result.frames.count; ++i) {
		i32 index = frames->push_unassigned();
		animation_frame_t* frame = frames->at(index);
		frame->duration = 0.15f;
		frame->pixel_size = 3.0f;
		frame->orientation = 0.0f;
		frame->hotspot = v2{ 7.0f, 6.0f };

		tex2 texture;
		texture.pitch = wiz_bmp.pitch;
		texture.width = WIZ_WIDTH;
		texture.height = WIZ_HEIGHT;
	    v2i bottom_left = {WIZ_WIDTH * i, WIZ_HEIGHT};
	    texture.pixels = inverted_coord(wiz_bmp, bottom_left);
	    frame->texture = texture;
	}

	return result;
}

animation_spec_t wiz_jumping_right(vec<animation_frame_t>* frames, tex2 wiz_bmp) {
	animation_spec_t result;
	result.frames.count = 1;
	result.frames.values = frames->at(frames->count);

	for (int i = 0; i < result.frames.count; ++i) {
		i32 index = frames->push_unassigned();
		animation_frame_t* frame = frames->at(index);
		frame->duration = 0.15f;
		frame->pixel_size = 3.0f;
		frame->orientation = 0.0f;
		frame->hotspot = v2{ 7.0f, 6.0f };

		tex2 texture;
		texture.pitch = wiz_bmp.pitch;
		texture.width = WIZ_WIDTH;
		texture.height = WIZ_HEIGHT;
	    v2i bottom_left = {WIZ_WIDTH * i, 3 * WIZ_HEIGHT};
	    texture.pixels = inverted_coord(wiz_bmp, bottom_left);
	    frame->texture = texture;
	}

	return result;
}

animation_spec_t wiz_walking_left(vec<animation_frame_t>* frames, tex2 wiz_bmp) {
	animation_spec_t result;
	result.frames.count = 6;
	result.frames.values = frames->at(frames->count);

	for (int i = 0; i < result.frames.count; ++i) {
		i32 index = frames->push_unassigned();
		animation_frame_t* frame = frames->at(index);
		frame->duration = 0.15f;
		frame->pixel_size = 3.0f;
		frame->orientation = 0.0f;
		frame->hotspot = v2{ 11.0f, 6.0f };

		tex2 texture;
		texture.pitch = wiz_bmp.pitch;
		texture.width = WIZ_WIDTH;
		texture.height = WIZ_HEIGHT;
	    v2i bottom_left = {WIZ_WIDTH * i, 2 * WIZ_HEIGHT};
	    texture.pixels = inverted_coord(wiz_bmp, bottom_left);
	    frame->texture = texture;
	}

	return result;
}

animation_spec_t wiz_standing_left(vec<animation_frame_t>* frames, tex2 wiz_bmp) {
	animation_spec_t result;
	result.frames.count = 1;
	result.frames.values = frames->at(frames->count);

	for (int i = 0; i < result.frames.count; ++i) {
		i32 index = frames->push_unassigned();
		animation_frame_t* frame = frames->at(index);
		frame->duration = 0.15f;
		frame->pixel_size = 3.0f;
		frame->orientation = 0.0f;
		frame->hotspot = v2{ 11.0f, 6.0f };

		tex2 texture;
		texture.pitch = wiz_bmp.pitch;
		texture.width = WIZ_WIDTH;
		texture.height = WIZ_HEIGHT;
	    v2i bottom_left = {WIZ_WIDTH * i, 2 * WIZ_HEIGHT};
	    texture.pixels = inverted_coord(wiz_bmp, bottom_left);
	    frame->texture = texture;
	}

	return result;
}

animation_spec_t wiz_jumping_left(vec<animation_frame_t>* frames, tex2 wiz_bmp) {
	animation_spec_t result;
	result.frames.count = 1;
	result.frames.values = frames->at(frames->count);

	for (int i = 0; i < result.frames.count; ++i) {
		i32 index = frames->push_unassigned();
		animation_frame_t* frame = frames->at(index);
		frame->duration = 0.15f;
		frame->pixel_size = 3.0f;
		frame->orientation = 0.0f;
		frame->hotspot = v2{ 11.0f, 6.0f };

		tex2 texture;
		texture.pitch = wiz_bmp.pitch;
		texture.width = WIZ_WIDTH;
		texture.height = WIZ_HEIGHT;
	    v2i bottom_left = {WIZ_WIDTH * (i + 1), 3 * WIZ_HEIGHT};
	    texture.pixels = inverted_coord(wiz_bmp, bottom_left);
	    frame->texture = texture;
	}

	return result;
}

#endif /* end of include guard: WIZ_ANIMATION_H__ */
