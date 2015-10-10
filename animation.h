#ifndef ANIMATION_H__
#define ANIMATION_H__

struct animation_frame_t {
    f32 duration;
    u32 pixel_size;
    tex2 texture;
    f32 hotspot_x, hotspot_y;
};

struct animation_t {
    u32 frame_count;
    f32 frame_progress;
    animation_frame_t* frames;
    u32 frame_index;
};

#endif /* end of include guard: ANIMATION_H__ */
