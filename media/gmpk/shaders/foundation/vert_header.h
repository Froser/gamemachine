uniform mat4 GM_shadow_matrix;
uniform mat4 GM_view_matrix;
uniform mat4 GM_model_matrix;
uniform mat4 GM_projection_matrix;

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec4 tangent;
layout (location = 4) in vec4 bitangent;
layout (location = 5) in vec2 lightmapuv;
layout (location = 6) in vec4 color;