#ifndef WIZ_ANIMATION_H__
#define WIZ_ANIMATION_H__

const i32 WIZ_WIDTH = 18;
const i32 WIZ_HEIGHT = 24;

u32* inverted_coord(tex2 texture, v2i pos) {
    return texture.pixels + (texture.height - 1 - pos.y) * texture.pitch + pos.x;
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

#endif /* end of include guard: WIZ_ANIMATION_H__ */
