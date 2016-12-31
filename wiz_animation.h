#ifndef WIZ_ANIMATION_H__
#define WIZ_ANIMATION_H__

const i32 WIZ_WIDTH = 18;
const i32 WIZ_HEIGHT = 24;
const f32 WIZ_PIXEL_SIZE = 0.05f;

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

animation_spec_ wiz_walking_right(vec<animation_frame_>* frames, tex2 wiz_bmp) {
	animation_spec_ result;
	result.frames.count = 6;
	result.frames.values = frames->at(frames->count);

	for (int i = 0; i < result.frames.count; ++i) {
		i32 index = frames->push_unassigned();
		animation_frame_* frame = frames->at(index);

		rect_i source_rect;
		source_rect.min_x = WIZ_WIDTH * i;
		source_rect.min_y = wiz_bmp.height - WIZ_HEIGHT;
		source_rect.max_x = source_rect.min_x + WIZ_WIDTH;
		source_rect.max_y = source_rect.min_y + WIZ_HEIGHT;

		frame->source_rect = source_rect;
		frame->duration = 0.15f;
		frame->pixel_size = WIZ_PIXEL_SIZE;
		frame->orientation = 0.0f;
		frame->hotspot = v2{ 7.0f, 6.0f };
	    frame->texture = wiz_bmp;
	}

	return result;
}

animation_spec_ wiz_standing_right(vec<animation_frame_>* frames, tex2 wiz_bmp) {
	animation_spec_ result;
	result.frames.count = 1;
	result.frames.values = frames->at(frames->count);

	for (int i = 0; i < result.frames.count; ++i) {
		i32 index = frames->push_unassigned();
		animation_frame_* frame = frames->at(index);

		rect_i source_rect;
		source_rect.min_x = WIZ_WIDTH * i;
		source_rect.min_y = wiz_bmp.height - WIZ_HEIGHT;
		source_rect.max_x = source_rect.min_x + WIZ_WIDTH;
		source_rect.max_y = source_rect.min_y + WIZ_HEIGHT;

		frame->source_rect = source_rect;
		frame->duration = 0.15f;
		frame->pixel_size = WIZ_PIXEL_SIZE;
		frame->orientation = 0.0f;
		frame->hotspot = v2{ 7.0f, 6.0f };
	    frame->texture = wiz_bmp;
	}

	return result;
}

animation_spec_ wiz_jumping_right(vec<animation_frame_>* frames, tex2 wiz_bmp) {
	animation_spec_ result;
	result.frames.count = 1;
	result.frames.values = frames->at(frames->count);

	for (int i = 0; i < result.frames.count; ++i) {
		i32 index = frames->push_unassigned();
		animation_frame_* frame = frames->at(index);

		rect_i source_rect;
		source_rect.min_x = WIZ_WIDTH * i;
		source_rect.min_y = wiz_bmp.height - (3 * WIZ_HEIGHT);
		source_rect.max_x = source_rect.min_x + WIZ_WIDTH;
		source_rect.max_y = source_rect.min_y + WIZ_HEIGHT;

		frame->source_rect = source_rect;
		frame->duration = 0.15f;
		frame->pixel_size = WIZ_PIXEL_SIZE;
		frame->orientation = 0.0f;
		frame->hotspot = v2{ 7.0f, 6.0f };
	    frame->texture = wiz_bmp;
	}

	return result;
}

animation_spec_ wiz_walking_left(vec<animation_frame_>* frames, tex2 wiz_bmp) {
	animation_spec_ result;
	result.frames.count = 6;
	result.frames.values = frames->at(frames->count);

	for (int i = 0; i < result.frames.count; ++i) {
		i32 index = frames->push_unassigned();
		animation_frame_* frame = frames->at(index);

		rect_i source_rect;
		source_rect.min_x = WIZ_WIDTH * i;
		source_rect.min_y = wiz_bmp.height - (2 * WIZ_HEIGHT);
		source_rect.max_x = source_rect.min_x + WIZ_WIDTH;
		source_rect.max_y = source_rect.min_y + WIZ_HEIGHT;

		frame->source_rect = source_rect;
		frame->duration = 0.15f;
		frame->pixel_size = WIZ_PIXEL_SIZE;
		frame->orientation = 0.0f;
		frame->hotspot = v2{ 11.0f, 6.0f };
	    frame->texture = wiz_bmp;
	}

	return result;
}

animation_spec_ wiz_standing_left(vec<animation_frame_>* frames, tex2 wiz_bmp) {
	animation_spec_ result;
	result.frames.count = 1;
	result.frames.values = frames->at(frames->count);

	for (int i = 0; i < result.frames.count; ++i) {
		i32 index = frames->push_unassigned();
		animation_frame_* frame = frames->at(index);

		rect_i source_rect;
		source_rect.min_x = WIZ_WIDTH * i;
		source_rect.min_y = wiz_bmp.height - (2 * WIZ_HEIGHT);
		source_rect.max_x = source_rect.min_x + WIZ_WIDTH;
		source_rect.max_y = source_rect.min_y + WIZ_HEIGHT;

		frame->source_rect = source_rect;
		frame->duration = 0.15f;
		frame->pixel_size = WIZ_PIXEL_SIZE;
		frame->orientation = 0.0f;
		frame->hotspot = v2{ 11.0f, 6.0f };
	    frame->texture = wiz_bmp;
	}

	return result;
}

animation_spec_ wiz_jumping_left(vec<animation_frame_>* frames, tex2 wiz_bmp) {
	animation_spec_ result;
	result.frames.count = 1;
	result.frames.values = frames->at(frames->count);

	for (int i = 0; i < result.frames.count; ++i) {
		i32 index = frames->push_unassigned();
		animation_frame_* frame = frames->at(index);

		rect_i source_rect;
		source_rect.min_x = WIZ_WIDTH * (i + 1);
		source_rect.min_y = wiz_bmp.height - (3 * WIZ_HEIGHT);
		source_rect.max_x = source_rect.min_x + WIZ_WIDTH;
		source_rect.max_y = source_rect.min_y + WIZ_HEIGHT;

		frame->source_rect = source_rect;
		frame->duration = 0.15f;
		frame->pixel_size = WIZ_PIXEL_SIZE;
		frame->orientation = 0.0f;
		frame->hotspot = v2{ 11.0f, 6.0f };
	    frame->texture = wiz_bmp;
	}

	return result;
}

#endif /* end of include guard: WIZ_ANIMATION_H__ */
