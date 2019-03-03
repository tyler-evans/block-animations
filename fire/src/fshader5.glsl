#version 150

in vec2 uv_pos;
flat in int use_texture;
uniform vec4 SetColor;

out vec4 color;


mat4 pattern = mat4(0.2, 0.4, 0.3, 0.8,
					0.8, 0.7, 0.1, 0.5,
					0.4, 0.2, 0.6, 0.4,
					0.3, 0.5, 0.4, 0.2);

float size = 4.0;

float scale = 16.0;

float u, v;
int i, j;

void main() 
{ 

	if(use_texture == 1){
		u = uv_pos[0];
		v = uv_pos[1];

		i = int(mod(scale*u, size));
	    j = int(mod(scale*v, size));

	    color = pattern[i][j]*SetColor;
	    color.a = SetColor.a;
    }
    else{
    	color = SetColor;
    }
    
}
