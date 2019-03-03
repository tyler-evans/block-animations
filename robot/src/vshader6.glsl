#version 150

in vec4 vPosition;
uniform mat4 ModelView, Projection;
uniform vec4 SetColor;
uniform int IsFloorInput;
uniform float SetTime;

out vec4 color;
out vec2 uv;
flat out float time;
flat out int is_floor;

void main()
{
    gl_Position = Projection * ModelView * vPosition;
    color = SetColor;
    is_floor = IsFloorInput;
    uv = normalize(vPosition.xy);
    time = SetTime;
}

