// Display a cube, using glDrawElements

#include "common.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>
#include <vector>

const char *WINDOW_TITLE = "Running Robot";
const double FRAME_RATE_MS = 1000.0/60.0;

typedef glm::vec4  color4;
typedef glm::vec4  point4;

std::vector<glm::vec4> icosphere_vertices;
std::vector<GLuint> icosphere_indices;

long long ms;
GLfloat the_time;

GLuint cube_vao, sphere_vao, square_vao, pyramid_vao;

point4 cube_vertices[8] = {
   point4(-0.5, -0.5,  0.5, 1.0),
   point4(-0.5,  0.5,  0.5, 1.0),
   point4(0.5,  0.5,  0.5, 1.0),
   point4(0.5, -0.5,  0.5, 1.0),
   point4(-0.5, -0.5, -0.5, 1.0),
   point4(-0.5,  0.5, -0.5, 1.0),
   point4(0.5,  0.5, -0.5, 1.0),
   point4(0.5, -0.5, -0.5, 1.0)
};

GLuint cube_indices[] = {
   1, 0, 3, 1, 3, 2,
   2, 3, 7, 2, 7, 6,
   3, 0, 4, 3, 4, 7,
   6, 5, 1, 6, 1, 2,
   4, 5, 6, 4, 6, 7,
   5, 4, 0, 5, 0, 1
};

point4 square_vertices[4] = {
   point4(-0.5, 0.5,  0.0, 1.0),
   point4(0.5,  0.5,  0.0, 1.0),
   point4(0.5,  -0.5,  0.0, 1.0),
   point4(-0.5, -0.5,  0.0, 1.0),
};

GLuint square_indices[] = {
	0,1,2,
	0,3,2
};

point4 pyramid_vertices[5] = {
	point4(-0.5, -0.5, -0.5,  1.0),
	point4(0.5,  -0.5, -0.5,  1.0),
	point4(0.5,  -0.5, 0.5, 1.0),
	point4(-0.5, -0.5, 0.5,  1.0),
	point4(0.0, 0.5, 0.0, 1.0),
};

GLuint pyramid_indices[] = {
	0,1,2,
	0,2,3,

	0,4,1,
	1,4,2,
	2,4,3,
	3,4,0,
};

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Xaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

float distance_count = 0.0;
float floor_distance = 0.0;
float floor_scale = 50.0;

GLuint  ModelView, Projection, ColorLocation, IsFloorLocation, TimeLocation;
color4 Color;

glm::mat4 eps_scale;


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

void draw_icosphere(glm::mat4 model_view) {
	glBindVertexArray(sphere_vao);

	// draw icosphere
	glUniform4fv(ColorLocation, 1, glm::value_ptr(color4(0.0, 0.0, 0.0, 1.0)));
	glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view));
	for (int i = 0; i < icosphere_indices.size(); i += 3)
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void *)(i * sizeof(GLuint)));

	// draw outline
	glUniform4fv(ColorLocation, 1, glm::value_ptr(color4(0.5, 0.5, 0.5, 1.0)));
	glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view*eps_scale));
	for (int i = 0; i < icosphere_indices.size(); i += 3)
		glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, (void *)(i * sizeof(GLuint)));
}

color4 default_color = color4(0.5, 0.5, 0.5, 1.0);
void draw_cube(glm::mat4 model_view, color4 color=default_color) {
	glBindVertexArray(cube_vao);

	// draw cube
	glUniform4fv(ColorLocation, 1, glm::value_ptr(color));
	glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view));
	for (int i = 0; i < sizeof(cube_indices) / sizeof(GLuint); i += 3)
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void *)(i * sizeof(GLuint)));

	// draw outline
	glUniform4fv(ColorLocation, 1, glm::value_ptr(color4(0.0, 0.0, 0.0, 1.0)));
	glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view*eps_scale));
	for (int i = 0; i < sizeof(cube_indices) / sizeof(GLuint); i += 3)
		glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, (void *)(i * sizeof(GLuint)));
}

void draw_floor(glm::mat4 model_view, color4 color = color4(0.5, 0.5, 0.5, 1.0)) {
	glBindVertexArray(square_vao);
	glUniform1i(IsFloorLocation, 1);

	glUniform1f(TimeLocation, the_time);

	// draw square
	glUniform4fv(ColorLocation, 1, glm::value_ptr(color));
	glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view));
	for (int i = 0; i < sizeof(square_indices) / sizeof(GLuint); i += 3)
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void *)(i * sizeof(GLuint)));

	glUniform1i(IsFloorLocation, 0);
}

void draw_pyramid(glm::mat4 model_view) {
	glBindVertexArray(pyramid_vao);

	// draw pyramid
	glUniform4fv(ColorLocation, 1, glm::value_ptr(color4(0.8, 0.2, 0.2, 1.0)));
	glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view));
	for (int i = 0; i < sizeof(pyramid_vertices) / sizeof(GLuint); i += 3)
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void *)(i * sizeof(GLuint)));

	// draw outline
	glUniform4fv(ColorLocation, 1, glm::value_ptr(color4(0.0, 0.0, 0.0, 1.0)));
	glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view*eps_scale));
	for (int i = 0; i < sizeof(pyramid_vertices) / sizeof(GLuint); i += 3)
		glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, (void *)(i * sizeof(GLuint)));
}

void draw_arm(glm::mat4 model_view, float elbow_deg, float shoulder_deg, bool do_end=true) {
	glm::mat4 elbow_joint_rot, shoulder_joint_rot;
	elbow_joint_rot = glm::rotate(elbow_joint_rot, glm::radians((GLfloat) elbow_deg), glm::vec3(0, 0, 1));
	shoulder_joint_rot = glm::rotate(shoulder_joint_rot, glm::radians((GLfloat) shoulder_deg), glm::vec3(0, 0, 1));

	draw_cube(model_view * shoulder_joint_rot * gen_trans(0.0, -1.0, 0.0) * gen_scale(1.0, 2.0, 1.0));
	draw_cube(model_view * shoulder_joint_rot * gen_trans(0.0, -2.0, 0.0) * elbow_joint_rot * gen_trans(0.0, -1.0, 0.0) * gen_scale(1.0, 2.0, 1.0));
	draw_icosphere(model_view * shoulder_joint_rot * gen_trans(0.0, -2.0, 0.0) * elbow_joint_rot * gen_scale(0.75, 0.75, 0.75));

	draw_pyramid(model_view * shoulder_joint_rot * gen_trans(0.0, -2.0, 0.0) * elbow_joint_rot * gen_trans(0.0, -2.0, 0.0));
	if (do_end)
		draw_pyramid(model_view * shoulder_joint_rot * gen_trans(0.0, -2.0, 0.0) * elbow_joint_rot * gen_trans(0.0, -2.4, 0.0) * gen_rotate(180.0, 0.0, 0.0));
}

// Generate an icosphere
// COMP 4490 @ umanitoba.ca Winter 2018

// These definitions are from the OpenGL Red Book example 2-13
#define X .525731112119133606
#define Z .850650808352039932
static GLfloat vdata[12][3] = {
   {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},
   {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},
   {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0}
};
static GLuint tindices[20][3] = {
   {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},
   {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},
   {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
   {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11}
};

void icosphere(int sub, std::vector<glm::vec4> &vertices, std::vector<GLuint> &indices) {
	for (GLfloat *v : vdata) {
		vertices.push_back(glm::vec4(v[0], v[1], v[2], 1));
	}
	indices.assign((GLuint *)tindices, (GLuint *)tindices + (sizeof(tindices) / sizeof(GLuint)));

	// use a regular subdivision (each tri into 4 equally-sized)
	for (int s = 0; s < sub; s++) {
		int isize = indices.size();
		for (int tri = 0; tri < isize; tri += 3) {
			int i0 = indices[tri];
			int i1 = indices[tri + 1];
			int i2 = indices[tri + 2];

			// create midpoints and "push" them out to the unit sphere
			// (but exclude the homogeneous coordinate)
			glm::vec3 midpoint0(glm::normalize(glm::vec3(vertices[i0] + vertices[i1]) * 0.5f));
			glm::vec3 midpoint1(glm::normalize(glm::vec3(vertices[i1] + vertices[i2]) * 0.5f));
			glm::vec3 midpoint2(glm::normalize(glm::vec3(vertices[i2] + vertices[i0]) * 0.5f));

			// add the midpoints to the vertices list
			int m0 = vertices.size();
			int m1 = m0 + 1;
			int m2 = m0 + 2;
			vertices.push_back(glm::vec4(midpoint0, 1));
			vertices.push_back(glm::vec4(midpoint1, 1));
			vertices.push_back(glm::vec4(midpoint2, 1));

			// now the four triangles
			indices[tri + 1] = m0;
			indices[tri + 2] = m2;
			indices.push_back(m0);
			indices.push_back(i1);
			indices.push_back(m1);
			indices.push_back(m0);
			indices.push_back(m1);
			indices.push_back(m2);
			indices.push_back(m2);
			indices.push_back(m1);
			indices.push_back(i2);
		}
	}
}

void setup_buffers(GLuint &vao, int vertices_size, GLvoid *vertices, int indices_size, GLvoid *indices, GLuint program) {
	GLuint buffer;
	GLuint vPosition = glGetAttribLocation(program, "vPosition");

	// setup vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	// Create and initialize a buffer object
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	// Another for the index buffer
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	// buffer the data
	glBufferData(GL_ARRAY_BUFFER, vertices_size, vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, indices, GL_STATIC_DRAW);
	// target shaders
	glUseProgram(program);
	// set up vertex arrays
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	
}

// OpenGL initialization
void init()
{
	icosphere(1, icosphere_vertices, icosphere_indices);
	eps_scale = glm::scale(eps_scale, glm::vec3(1.001, 1.001, 1.001));

	GLuint program = InitShader("vshader6.glsl", "fshader5.glsl");
	
	setup_buffers(cube_vao, sizeof(cube_vertices), cube_vertices, sizeof(cube_indices), cube_indices, program);
	setup_buffers(sphere_vao, sizeof(glm::vec4)*icosphere_vertices.size(), &icosphere_vertices[0], sizeof(GLuint)*icosphere_indices.size(), &icosphere_indices[0], program);
	setup_buffers(square_vao, sizeof(square_vertices), square_vertices, sizeof(square_indices), square_indices, program);
	setup_buffers(pyramid_vao, sizeof(pyramid_vertices), pyramid_vertices, sizeof(pyramid_indices), pyramid_indices, program);

	ModelView = glGetUniformLocation(program, "ModelView");
	Projection = glGetUniformLocation(program, "Projection");
	ColorLocation = glGetUniformLocation(program, "SetColor");

	IsFloorLocation = glGetUniformLocation(program, "IsFloorInput");
	glUniform1i(IsFloorLocation, 0);

	TimeLocation = glGetUniformLocation(program, "SetTime");

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);
}



float wave(float min, float max, float x) {
	return 0.5*(max - min)*(sin(x) + 1.0) + min;
}



void display( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glm::mat4 view_trans, rot, scale, model_view;
	rot = gen_rotate(0.0, Theta[Yaxis], 0.0);

	const glm::vec3 viewer_pos( 0.0, 0.5, 1.8 );
	view_trans = glm::translate(view_trans, -viewer_pos);

	scale = gen_scale(0.1, 0.1, 0.1);


	// floor
	draw_floor(view_trans * rot * gen_rotate(90.0, 0.0, 0.0) * gen_scale(4.0, 4.0, 4.0));


	float scaled_time = the_time*6.0;

	float left_shoulder_deg, left_elbow_deg;
	left_shoulder_deg = wave(-45.0, 45.0, scaled_time);
	left_elbow_deg = wave(30.0, 90.0, scaled_time);

	float right_shoulder_deg, right_elbow_deg;
	right_shoulder_deg = -wave(-45.0, 45.0, scaled_time);
	right_elbow_deg = 90.0 - wave(0.0, 40.0, scaled_time);

	float left_knee_deg, right_knee_deg;
	left_knee_deg = -wave(0.0, 80.0, scaled_time);
	right_knee_deg = -wave(0.0, 80.0, -scaled_time);

	// robot
	model_view = gen_trans(0.0, 0.63 + 0.04*(sin(2*scaled_time)+0.8), 0.0) * view_trans * gen_rotate(0.0, 90.0, 0.0) * rot * scale * gen_scale(0.8, 0.8, 0.8);

	// arms
	draw_arm(model_view * gen_trans(-2.0, 0.0, 0.0) * gen_rotate(0.0, -90.0, 0.0), left_elbow_deg, left_shoulder_deg);
	draw_icosphere(model_view * gen_trans(-1.0, 0.0, 0.0) * gen_rotate(-left_shoulder_deg, 0.0, 0.0) * gen_scale(1.5, 0.5, 0.5));
	draw_icosphere(model_view * gen_trans(-2.0, 0.0, 0.0) * gen_rotate(-left_shoulder_deg, 0.0, 0.0) * gen_scale(0.6, 0.6, 0.6));

	draw_arm(model_view * gen_trans(2.0, 0.0, 0.0) * gen_rotate(0.0, -90.0, 0.0), right_elbow_deg, right_shoulder_deg);
	draw_icosphere(model_view * gen_trans(1.0, 0.0, 0.0) * gen_rotate(-right_shoulder_deg, 0.0, 0.0) * gen_scale(1.5, 0.5, 0.5));
	draw_icosphere(model_view * gen_trans(2.0, 0.0, 0.0) * gen_rotate(-right_shoulder_deg, 0.0, 0.0) * gen_scale(0.6, 0.6, 0.6));

	// body
	draw_cube(model_view * gen_trans(0.0, -1.5, 0.0) * gen_scale(2.1, 4.0, 1.5), color4(0.3, 0.3, 0.3, 1.0));

	// left leg
	draw_arm(model_view * gen_trans(-0.8, -3.5, 0.0) * gen_rotate(0.0, -90.0, 0.0), left_knee_deg, right_shoulder_deg, false);
	draw_icosphere(model_view * gen_trans(-0.8, -3.5, 0.0) * gen_rotate(-right_shoulder_deg, 0.0, 0.0) * gen_scale(0.7, 0.7, 0.7));

	// right leg
	draw_arm(model_view * gen_trans(0.8, -3.5, 0.0) * gen_rotate(0.0, -90.0, 0.0), right_knee_deg, left_shoulder_deg, false);
	draw_icosphere(model_view * gen_trans(0.8, -3.5, 0.0) * gen_rotate(-left_shoulder_deg, 0.0, 0.0) * gen_scale(0.7, 0.7, 0.7));

	// head
	draw_icosphere(model_view * gen_trans(0.0, 0.75, 0.0) * gen_scale(0.4, 0.4, 0.4));
	draw_cube(model_view * gen_trans(0.0, 1.25, 0.0) * gen_scale(1.2, 1.2, 1.2));

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
       switch( button ) {
          case GLUT_LEFT_BUTTON:    Axis = Xaxis;  break;
          case GLUT_MIDDLE_BUTTON:  Axis = Yaxis;  break;
          case GLUT_RIGHT_BUTTON:   Axis = Zaxis;  break;
       }
    }
}

//----------------------------------------------------------------------------

void
update( void )
{
	ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();

	the_time = (ms % 1000000) / 1000.0;

	Axis = Yaxis;
    Theta[Axis] += 0.5;
    if ( Theta[Axis] > 360.0 ) {
       Theta[Axis] -= 360.0;
    }

}

//----------------------------------------------------------------------------

void
reshape( int width, int height )
{
   glViewport( 0, 0, width, height );

   GLfloat aspect = GLfloat(width)/height;
   //glm::mat4  projection = glm::perspective( glm::radians(45.0f), aspect, 0.5f, 3.0f );
   glm::mat4  projection = glm::perspective(glm::radians(45.0f), aspect, 0.5f, 5.0f);

   glUniformMatrix4fv( Projection, 1, GL_FALSE, glm::value_ptr(projection) );
}
