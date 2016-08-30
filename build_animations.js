var fs = require('fs');

var dir = fs.readdirSync('animations');

var includes = "";
var method_body = "";
var animations_body = "";
for (var i = dir.length - 1; i >= 0; i--) {
    var filename = dir[i];
    var typename = filename.split('.')[0];

    includes += '#include "animations/' + filename + '"\n';
    animations_body += '    animation_spec_t ' + typename +';\n';
    method_body +=
'    {\n\
    animation_builder_t builder = ' + typename + '_builder();\n\
    animation_spec_t* spec = &animations.' + typename + ';\n\
    spec->cycle_point = builder.cycle_point;\n\
    spec->frames.count = builder.frame_count;\n\
    spec->frames.values = PUSH_ARRAY(arena, (size_t)builder.frame_count, animation_frame_t);\n\
    tex2 texture = load_image(builder.bmp_filepath);\n\
    i32 frame_height = texture.height;\n\
    assert_(texture.width % builder.frame_count == 0);\n\
    i32 frame_width = texture.width / builder.frame_count;\n\
    v2 hotspot = builder.hotspot;\n\
\n\
    for (i32 i = 0; i < builder.frame_count; ++i) {\n\
        animation_frame_t frame = {0};\n\
        frame.duration = builder.frame_duration;\n\
        frame.texture = texture;\n\
        frame.source_rect.min_y = 0;\n\
        frame.source_rect.max_y = frame_height;\n\
        frame.source_rect.min_x = i * frame_width;\n\
        frame.source_rect.max_x = (i+1) * frame_width;\n\
        frame.pixel_size = VIRTUAL_PIXEL_SIZE;\n\
        frame.orientation = 0.0f;\n\
        frame.hotspot = hotspot;\n\
\n\
        spec->frames.set(i, frame);\n\
    }\n\
    }\n';
};

var source = "#ifndef ANIMATIONS_H_\n#define ANIMATIONS_H_\n" +
    includes +
    '\nstruct animations_list_t {\n'
    + animations_body +
    '};\n\n' +
    'animations_list_t load_animations(memory_arena_t* arena) {\n\
    animations_list_t animations = {0};\n' +
    method_body + '    return animations;\n}\n' +
    '#endif\n';

fs.writeFileSync('animations.h', source, 'utf8');