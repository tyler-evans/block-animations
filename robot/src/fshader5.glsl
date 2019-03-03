#version 150

in vec4 color;
in vec2 uv;
flat in float time;
flat in int is_floor;

out vec4 fColor;

float u, v, result;
float size = 16.0;
float speed = 10.0;

void main() 
{
	if(is_floor == 1){
	    u = uv[0];
	    v = uv[1];

		result = mod(floor(size*u+speed*float(time)) + floor(size*v), 2.0);
	    
		fColor = vec4(0.0, result - 0.5, 0.2, 1.0);
	}
   	else{
   		fColor = color;
   	}
}
