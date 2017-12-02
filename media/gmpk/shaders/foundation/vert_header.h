
layout (location = 0) in vec4 position;
layout (location = 1) in vec4 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec4 tangent;
layout (location = 4) in vec4 bitangent;
layout (location = 5) in vec2 lightmapuv;
layout (location = 6) in vec4 color;

out vec4 _position;
out vec4 _normal;
out vec2 _uv;
out vec4 _tangent;
out vec4 _bitangent;
out vec2 _lightmapuv;
out vec4 _color;
