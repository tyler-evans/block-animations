#version 150

in vec4 vPosition;
in vec2 uv;
uniform mat4 ModelView, Projection;
uniform int UseTexture;

out vec2 uv_pos;
flat out int use_texture;


void main()
{
    gl_Position = Projection * ModelView * vPosition;
    uv_pos = uv;
    use_texture = UseTexture;
}

