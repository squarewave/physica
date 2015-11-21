#ifndef CONSTANTS_H
#define CONSTANTS_H

const u32 FRAME_RATE = 60;
const u32 START_WIDTH = 1366;
const u32 START_HEIGHT = 768;
const f32 TARGET_VIEW_HEIGHT_IN_METERS = 7.0f;
const f32 VIRTUAL_PIXEL_SIZE = 1.0f / 32.0f;
const f32 INV_VIRTUAL_PIXEL_SIZE = 1.0f / VIRTUAL_PIXEL_SIZE;
const f32 PARALLAX_COEFFICIENT = 0.2f;
const f32 TIME_TO_ROTATE = 0.3f;
const i32 MAX_PARTICLES = 20000;

inline f32 vpixels_to_meters(f32 meters) {
    return VIRTUAL_PIXEL_SIZE * meters;
}

inline f32 meters_to_vpixels(f32 meters) {
    return INV_VIRTUAL_PIXEL_SIZE * meters;
}

#endif