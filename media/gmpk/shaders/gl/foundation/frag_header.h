out vec4 _frag_color;
// 与vert_header.h对应
in vec4 _position;
in vec4 _normal;
in vec2 _uv;
in vec4 _tangent;
in vec4 _bitangent;
in vec2 _lightmapuv;
in vec4 _color;

uniform int GM_Debug_Normal = 0;
const int GM_Debug_Normal_Off = 0;
const int GM_Debug_Normal_WorldSpace = 1;
const int GM_Debug_Normal_EyeSpace = 2;

uniform int GM_ColorVertexOp = 0;
const int GM_VertexColorOp_NoColor = 0;
const int GM_VertexColorOp_Replace = 1;
const int GM_VertexColorOp_Multiply = 2;
const int GM_VertexColorOp_Add = 3;