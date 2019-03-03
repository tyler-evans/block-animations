// Display a cube, using glDrawElements

#include "common.h"
#include <chrono>
#include <algorithm>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const char *WINDOW_TITLE = "Minecraft Fire";
const double FRAME_RATE_MS = 1000.0/60.0;
const int num_particles = 12;

typedef glm::vec4  color4;
typedef glm::vec4  point4;
typedef glm::vec3  point3;
typedef glm::vec2  point2;

long long ms;
float prev_time = 0.0;
float curr_time;
float time_delta;

int prev_button = 1;
float angle_sign = 1.0;

enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Yaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };
GLuint  ModelView, Projection, SetColor, UseTexture;

color4 brown = color4(0.6, 0.3, 0.0, 1.0);



point4 vertices[] = {
   point4(-0.5, -0.5,  0.5, 1.0),
   point4(-0.5,  0.5,  0.5, 1.0),
   point4(0.5,  0.5,  0.5, 1.0),
   point4(0.5, -0.5,  0.5, 1.0),

   point4(-0.5, -0.5, -0.5, 1.0),
   point4(-0.5,  0.5, -0.5, 1.0),
   point4(0.5,  0.5, -0.5, 1.0),
   point4(0.5, -0.5, -0.5, 1.0),

   point4(0.5, -0.5, -0.5, 1.0),
   point4(0.5, -0.5, 0.5, 1.0),
   point4(0.5, 0.5, 0.5, 1.0),
   point4(0.5, 0.5, -0.5, 1.0),

   point4(-0.5, -0.5, -0.5, 1.0),
   point4(-0.5, -0.5, 0.5, 1.0),
   point4(-0.5, 0.5, 0.5, 1.0),
   point4(-0.5, 0.5, -0.5, 1.0),

   point4(-0.5, 0.5, -0.5, 1.0),
   point4(-0.5, 0.5, 0.5, 1.0),
   point4(0.5, 0.5, 0.5, 1.0),
   point4(0.5, 0.5, -0.5, 1.0),

   point4(-0.5, -0.5, -0.5, 1.0),
   point4(-0.5, -0.5, 0.5, 1.0),
   point4(0.5, -0.5, 0.5, 1.0),
   point4(0.5, -0.5, -0.5, 1.0),
};

GLuint indices[] = {
	1, 0, 3, 1, 3, 2,
	5, 4, 7, 5, 7, 6,
	9, 8, 11, 9, 11, 10,
	13, 12, 15, 13, 15, 14,
	17, 16, 19, 17, 19, 18,
	21, 20, 23, 21, 23, 22,
};

point2 uv_points[] = {
	point2(0.0, 0.0),
	point2(0.0, 1.0),
	point2(1.0, 1.0),
	point2(1.0, 0.0),

	point2(0.0, 0.0),
	point2(0.0, 1.0),
	point2(1.0, 1.0),
	point2(1.0, 0.0),

	point2(0.0, 0.0),
	point2(0.0, 1.0),
	point2(1.0, 1.0),
	point2(1.0, 0.0),

	point2(0.0, 0.0),
	point2(0.0, 1.0),
	point2(1.0, 1.0),
	point2(1.0, 0.0),

	point2(0.0, 0.0),
	point2(0.0, 1.0),
	point2(1.0, 1.0),
	point2(1.0, 0.0),

	point2(0.0, 0.0),
	point2(0.0, 1.0),
	point2(1.0, 1.0),
	point2(1.0, 0.0),
};


glm::mat4 gen_rotate(GLfloat rot_x, GLfloat rot_y, GLfloat rot_z) {
	glm::mat4 rotate;
	rotate = glm::rotate(rotate, glm::radians(rot_x), glm::vec3(1, 0, 0));
	rotate = glm::rotate(rotate, glm::radians(rot_y), glm::vec3(0, 1, 0));
	rotate = glm::rotate(rotate, glm::radians(rot_z), glm::vec3(0, 0, 1));
	return rotate;
}

glm::mat4 gen_trans(float x, float y, float z) {
	glm::mat4 trans;
	return glm::translate(trans, glm::vec3(x, y, z));
}

glm::mat4 gen_scale(float x, float y, float z) {
	glm::mat4 scale;
	return glm::scale(scale, glm::vec3(x, y, z));
}

void draw_cube(glm::mat4 model_view, color4 color, int use_texture) {
	glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view));
	glUniform4f(SetColor, color[0], color[1], color[2], color[3]);
	glUniform1i(UseTexture, use_texture);
	for (int i = 0; i < sizeof(indices) / sizeof(GLuint); i += 3) {
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void *)(i * sizeof(GLuint)));
	}
}


class Particle {

	float max_distance = float((rand() % 6) + 5) / 10.0;
	float color_scale;
	point2 initial_position_shift = point2(
		float(2 * (rand() % 2) - 1)*float(rand() % 10)*0.005,
		float(2 * (rand() % 2) - 1)*float(rand() % 10)*0.005
	);
	float speed = float((rand() % 100) + 50) * 0.01;

	glm::vec3 rot_axis = glm::vec3(rand() % 2, rand() % 2, rand() % 2);
	float angular_speed = float(rand() % 5) + 5.0;
	float angular_theta = 0.0;
	glm::mat4 rot_matrix;

	float theta = glm::radians(float(rand() % 360));
	float incline = glm::radians(float(rand() % 35));
	point3 direction;
	point3 position = point3(0.0, 0.2, 0.0);
	color4 orange = color4(1.0, 0.6, 0.0, 1.0);
	glm::mat4 scale = gen_scale(0.1, 0.1, 0.1);

public:
	void draw(glm::mat4 model_view) {
		color_scale = std::max(distance() - 0.3, 0.0);
		orange[3] = 1.0 - distance(); // set alpha
		draw_cube(model_view * gen_trans(position[0], position[1], position[2]) * rot_matrix * scale, orange + point4(color_scale, color_scale, color_scale, 0.0), 0);
	}

	void update(float time_delta) {
		direction = point3(sin(incline)*sin(theta), cos(incline), sin(incline)*cos(theta));
		position += speed * direction * time_delta;
		angular_theta += angular_speed * time_delta;
		rot_matrix = glm::rotate(rot_matrix, glm::radians(angular_theta), rot_axis);
	}
	float distance() {
		return glm::length(position - glm::vec3(0.0, 0.2, 0.0));
	}
	bool past_life() {
		return distance() > max_distance;
	}

};


class ParticleSystem {
public:
	int num_particles;
	Particle *particles;

	ParticleSystem(int num) {
		num_particles = num;
		particles = new Particle[num_particles];
		for (int i = 0; i < num_particles; i++)
			particles[i] = Particle();
	}

	void draw(glm::mat4 model_view) {
		for (int i = 0; i < num_particles; i++)
			particles[i].draw(model_view);
	}

	void update(float time_delta) {
		for (int i = 0; i < num_particles; i++)
			particles[i].update(time_delta);
	}

	void dropout() {
		if (rand() % 2)
			particles[rand() % num_particles] = Particle();
	}

	void prune_system() {
		for (int i = 0; i < num_particles; i++) {
			if (particles[i].past_life()) {
				particles[i] = Particle();
			}
		}
	}
};
ParticleSystem particle_system = ParticleSystem(num_particles);

//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
   // Create a vertex array object
   GLuint vao = 0;
   glGenVertexArrays( 1, &vao );
   glBindVertexArray( vao );

   GLuint buffer;

   // Create and initialize a buffer object
   glGenBuffers( 1, &buffer );
   glBindBuffer( GL_ARRAY_BUFFER, buffer );
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(uv_points), NULL, GL_STATIC_DRAW);
   glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
   glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(uv_points), uv_points);

   // Another for the index buffer
   glGenBuffers( 1, &buffer );
   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffer );
   glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW );

   // Load shaders and use the resulting shader program
   GLuint program = InitShader( "vshader6.glsl", "fshader5.glsl" );
   glUseProgram( program );

   // set up vertex arrays
   GLuint vPosition = glGetAttribLocation( program, "vPosition" );
   glEnableVertexAttribArray( vPosition );
   glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

   GLuint uv = glGetAttribLocation(program, "uv");
   glEnableVertexAttribArray(uv);
   glVertexAttribPointer(uv, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vertices)));

   ModelView = glGetUniformLocation( program, "ModelView" );
   Projection = glGetUniformLocation( program, "Projection" );
   SetColor = glGetUniformLocation(program, "SetColor");
   UseTexture = glGetUniformLocation(program, "UseTexture");

   glEnable( GL_DEPTH_TEST );
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glClearColor( 0.9, 0.9, 0.9, 1.0 );
}

//----------------------------------------------------------------------------
void
display( void )
{
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   //  Generate the model-view matrix
   const glm::vec3 viewer_pos( 0.0, 0.5, 2.0 );
   glm::mat4 trans, rot, scale, model_view;
   trans = glm::translate(trans, -viewer_pos);
   rot = gen_rotate(Theta[Xaxis], Theta[Yaxis], Theta[Zaxis]);
   model_view = trans * rot;

   // Floor
   draw_cube(model_view * gen_scale(1.5, 0.001, 1.5), color4(0.0, 1.0, 0.0, 1.0), 1);

   // Logs
   draw_cube(model_view * gen_trans(-0.06, 0.15, 0.0) * gen_rotate(0.0, 0.0, 40.0) * gen_scale(0.5, 0.1, 0.1), brown, 1);
   draw_cube(model_view * gen_trans(0.06, 0.15, 0.0) * gen_rotate(0.0, 0.0, -40.0) * gen_scale(0.5, 0.1, 0.1), brown, 1);
   draw_cube(model_view * gen_trans(0.0, 0.15, 0.06) * gen_rotate(40.0, 90.0, 0.0) * gen_scale(0.5, 0.1, 0.1), brown, 1);
   draw_cube(model_view * gen_trans(0.0, 0.15, -0.06) * gen_rotate(-40.0, 90.0, 0.0) * gen_scale(0.5, 0.1, 0.1), brown, 1);

   particle_system.draw(model_view);
   particle_system.update(time_delta);
   particle_system.prune_system();

   glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
       case 033: // Escape Key
       case 'q': case 'Q':
          exit( EXIT_SUCCESS );
          break;
    }
}

//----------------------------------------------------------------------------

void
mouse( int button, int state, int x, int y )
{
    if ( state == GLUT_DOWN ) {
		if (button == prev_button) {
			Axis = -1;
			prev_button = -1;
		}
		else {
			switch (button) {
				case GLUT_MIDDLE_BUTTON:
					Axis = Yaxis;
					prev_button = button;
					break;
				case GLUT_LEFT_BUTTON:
					Axis = Xaxis;
					angle_sign = 1.0;
					prev_button = button;
					break;
				case GLUT_RIGHT_BUTTON:
					Axis = Xaxis;
					angle_sign = -1.0;
					prev_button = button;
					break;
			}
		}
     }
}

//----------------------------------------------------------------------------

void
update( void )
{
	ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();

	curr_time = (ms % 1000000) / 1000.0;
	time_delta = curr_time - prev_time;
	prev_time = curr_time;


	if (Axis != -1) {

		if (Axis == Xaxis)
			Theta[Axis] += 0.5 * angle_sign;
		else
			Theta[Axis] += 0.5;

		if (Theta[Axis] > 360.0) {
			Theta[Axis] -= 360.0;
		}

		Theta[Xaxis] = std::max(std::min(Theta[Xaxis], 70.0f), -10.0f);
	}
}

//----------------------------------------------------------------------------

void
reshape( int width, int height )
{
   glViewport( 0, 0, width, height );

   GLfloat aspect = GLfloat(width)/height;
   glm::mat4  projection = glm::perspective(glm::radians(60.0f), aspect, 0.5f, 5.0f);

   glUniformMatrix4fv( Projection, 1, GL_FALSE, glm::value_ptr(projection) );
}
