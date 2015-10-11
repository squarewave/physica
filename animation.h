#ifndef ANIMATION_H__
#define ANIMATION_H__

struct animation_frame_t {
    f32 duration;
    tex2 texture;
    f32 pixel_size;
    f32 orientation;
    v2 hotspot;
};

struct animation_spec_t {
    array<animation_frame_t> frames;
};

struct animation_t {
	b32 freed;
    f32 frame_progress;
    i32 frame_index;
    render_object_t* render_object;
    animation_spec_t* spec;
};

struct animation_group_t {
	vec<animation_t> animations;
	vec<i32> free_list;
};

inline i32
add_animation(render_group_t* render_group,
              animation_group_t* animation_group,
              animation_spec_t* spec) {
	i32 index;
	if (animation_group->free_list.count) {
		index = animation_group->free_list.pop();
	} else {
		index = animation_group->animations.push_unassigned();
	}

	animation_t* animation = animation_group->animations.at(index);
	animation->freed = false;
	animation->frame_progress = 0.0f;
	animation->frame_index = 0;
	animation->spec = spec;

	animation_frame_t first_frame = spec->frames[0];
	render_object_t* render_obj = push_texture(render_group,
	             v2{0,0},
	             first_frame.hotspot,
	             first_frame.pixel_size,
	             first_frame.texture,
	             first_frame.orientation);

	animation->render_object = render_obj;

	return index;
}

inline void
remove_animation(animation_group_t* animation_group, i32 index) {
	assert(index < animation_group->animations.count);
	animation_t* animation = animation_group->animations.at(index);
	assert(!animation->freed);
	animation->freed = true;
	animation_group->free_list.push(index);

	assert(false); //TODO(doug): ensure that the render object is removed from the group
}

void
update_animations(animation_group_t* animation_group, f32 dt) {
	for (int i = 0; i < animation_group->animations.count; ++i) {
		animation_t* animation = animation_group->animations.at(i);
		animation->frame_progress += dt;
		f32 current_frame_duration = animation->spec->frames[animation->frame_index].duration;
		if (animation->frame_progress > current_frame_duration) {
			animation->frame_progress -= current_frame_duration;
			animation->frame_index++;
			animation->frame_index %= animation->spec->frames.count;

			animation_frame_t frame = animation->spec->frames[animation->frame_index];
			animation->render_object->render_texture.texture = frame.texture;
			animation->render_object->render_texture.hotspot = frame.hotspot;
			animation->render_object->render_texture.orientation = frame.orientation;
			animation->render_object->render_texture.pixel_size = frame.pixel_size;
		}
	}
}

#endif /* end of include guard: ANIMATION_H__ */
