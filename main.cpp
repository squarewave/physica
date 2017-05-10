// #pragma clang diagnostic push
// #pragma ide diagnostic ignored "UnusedImportStatement"

#include "assert.h"

// trying this out:
// http://buffered.io/posts/the-magic-of-unity-builds/
#ifdef _WIN32
    #include "win32sdl_platform.cpp"
#else
    #include "linuxsdl_platform.cpp"
#endif

#include "renderer.cpp"
#include "hashmap.cpp"
#include "physica.cpp"
#include "game.cpp"
#include "player.cpp"
#include "tile.cpp"
#include "sim_entity.cpp"
#include "background.cpp"
#include "bogger.cpp"
#include "debug.cpp"
#include "tools.cpp"
#include "enemies.cpp"
#include "npcs.cpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define EASYTAB_IMPLEMENTATION
#include "easytab.h"

// #pragma clang diagnostic pop
