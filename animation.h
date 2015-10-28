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
    f32 speed;
    f32 z;
    v2 position;
    animation_spec_t* spec;
};

struct animation_group_t {
	vec<animation_t> animations;
	vec<i32> freed;
};

inline i32
add_animation(animation_group_t* animation_group,
              animation_spec_t* spec,
              f32 z) {
	i32 index;
	if (animation_group->freed.count) {
		index = animation_group->freed.pop();
	} else {
		index = animation_group->animations.push_unassigned();
	}

	animation_t* animation = animation_group->animations.at(index);
	animation->freed = false;
	animation->frame_progress = 0.0f;
	animation->frame_index = 0;
	animation->spec = spec;
	animation->speed = 1.0f;
	animation->z = z;

	return index;
}

inline void
remove_animation(animation_group_t* animation_group, i32 index) {
	assert(index < animation_group->animations.count);
	animation_t* animation = animation_group->animations.at(index);
	assert(!animation->freed);
	animation->freed = true;
	animation_group->freed.push(index);

	assert(false); //TODO(doug): ensure that the render object is removed from the group
}

void
update_animations(animation_group_t* animation_group,
                  render_group_t* render_group,
                  f32 dt) {
	for (int i = 0; i < animation_group->animations.count; ++i) {
		animation_t* animation = animation_group->animations.at(i);
		animation->frame_progress += animation->speed * dt;
		animation->frame_index %= animation->spec->frames.count;
		f32 current_frame_duration = animation->spec->frames[animation->frame_index].duration;
		if (animation->frame_progress > current_frame_duration) {
			animation->frame_progress -= current_frame_duration;
			animation->frame_index++;
			animation->frame_index %= animation->spec->frames.count;
		}

		animation_frame_t frame = animation->spec->frames[animation->frame_index];

		push_texture(render_group,
		             animation->position,
		             frame.hotspot,
		             frame.pixel_size,
		             frame.texture,
		             frame.orientation,
		             animation->z);
	}
}

#endif /* end of include guard: ANIMATION_H__ */
