#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>

#include "shader.hpp"

int window_width = 1024;
int window_height = 768;
int decoration = 1;
int fullscreen = 0;
int print = 0;
const int max_particle_num = 100000;
int particle_num = 200;
float particle_speed = 1;
float max_dist = 0.1;

struct Particle {
	glm::vec2 pos, speed;
	glm::vec4 color;
};

int digit( int n ) {
	int i = 0;
	while( n ) {
		n /= 10;
		i++;
	}
	return i;
}

int main( int argc, char** argv ) {
	int oc; // 选项字符
	while( ( oc = getopt( argc, argv, "dfn:pr:" ) ) != -1 ) {
		switch( oc ) {
			case 'd':
				decoration = 0;
				break;
			case 'f':
				fullscreen = 1;
				break;
			case 'n':
				particle_num = atoi(optarg);
				break;
			case 'p':
				print = 0;
				break;
			case 'r':
				window_width = atoi(optarg);
				window_height = atoi( optarg + digit( window_width ) + 1 );
				break;
		}
	}

	if( !glfwInit() ) {
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwWindowHint( GLFW_SAMPLES, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	glfwWindowHint( GLFW_DECORATED, decoration);

	window = glfwCreateWindow( window_width, window_height, "point-line", fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL );
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = true; // Needed for core profile
	if ( glewInit() != GLEW_OK ) {
		fprintf( stderr, "Failed to initialize GLEW\n" );
		return -1;
	}

	glfwSetInputMode( window, GLFW_STICKY_KEYS, GL_TRUE );

	glClearColor(0.1, 0.1, 0.1, 1.0);

	GLuint VertexArrayID;
	glGenVertexArrays( 1, &VertexArrayID );
	glBindVertexArray( VertexArrayID );

	GLuint programID_line = LoadShaders( "vertex.glsl", "fragment_line.glsl", "geometry_line.glsl" );
	GLuint programID_point = LoadShaders( "vertex.glsl", "fragment_point.glsl", NULL );

	GLuint xposID = glGetUniformLocation(programID_line, "xpos");
	GLuint yposID = glGetUniformLocation(programID_line, "ypos");

	Particle particles[max_particle_num];
	srand( time( NULL ) );
	for( int i = 0; i < particle_num; i++ ) {
		particles[i].pos = glm::vec2( ( rand() % 2000 - 1000.0f ) / 1000.0f, ( rand() % 2000 - 1000.0f ) / 1000.0f );
		particles[i].speed = glm::vec2( ( rand() % 2000 - 1000.0f ) / 1000000.0f * particle_speed, ( rand() % 2000 - 1000.0f ) / 1000000.0f * particle_speed );
		particles[i].color = glm::vec4( rand() % 2000 / 2000.0f, rand() % 2000 / 2000.0f, rand() % 2000 / 2000.0f, rand() % 2000 / 2000.0f );
	}

	GLfloat g_vertex_buffer_data[max_particle_num][2];
	GLfloat g_color_buffer_data[max_particle_num][4];

	GLuint vertexbuffer;
	glGenBuffers( 1, &vertexbuffer );

	GLuint colorbuffer;
	glGenBuffers( 1, &colorbuffer );

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	double time_current;
	double time_last;
	int frame_count = 0;
	glfwSetTime(0);
	do {
		glClear( GL_COLOR_BUFFER_BIT );

		for( int i = 0; i < particle_num; i++ ) {
//			particles[i].speed += glm::vec2( sin( glfwGetTime() ) / 100000, cos( glfwGetTime() ) / 100000 );
//			particles[i].color += glm::vec4( std::abs( sin( glfwGetTime() ) ) / 1000, std::abs( cos( glfwGetTime() ) ) / 1000, std::abs( sin( glfwGetTime() ) ) / 1000 * std::abs( cos( glfwGetTime() ) ) / 1000, 0 );
//			particles[i].color += glm::vec4( sin(particles[i].pos.x * 10), sin(particles[i].pos.y * 10), 0, 0 );
			particles[i].pos += particles[i].speed;
			if( std::abs(particles[i].pos.x) > 1.0 || std::abs(particles[i].pos.y) > 1.0 || particles[i].speed == glm::vec2( 0.0, 0.0 ) ) {
				switch( rand() % 4 ) {
					case 0 :
						particles[i].pos = glm::vec2( ( rand() % 2000 - 1000.0f ) / 1000.0f,-1.0 );
						particles[i].speed = glm::vec2( ( rand() % 2000 - 1000.0f ) / 1000000.0f * particle_speed, std::abs( ( rand() % 2000 - 1000.0f ) / 1000000.0f * particle_speed ) );
						break;
					case 1 :
						particles[i].pos = glm::vec2( ( rand() % 2000 - 1000.0f ) / 1000.0f, 1.0 );
						particles[i].speed = glm::vec2( ( rand() % 2000 - 1000.0f ) / 1000000.0f * particle_speed, -std::abs( ( rand() % 2000 - 1000.0f ) / 1000000.0f * particle_speed ) );
						break;
					case 2 :
						particles[i].pos = glm::vec2( -1.0, ( rand() % 2000 - 1000.0f ) / 1000.0f );
						particles[i].speed = glm::vec2(  std::abs( ( rand() % 2000 - 1000.0f ) / 1000000.0f * particle_speed ), ( rand() % 2000 - 1000.0f ) / 1000000.0f * particle_speed );
						break;
					case 3 :
						particles[i].pos = glm::vec2(  1.0, ( rand() % 2000 - 1000.0f ) / 1000.0f );
						particles[i].speed = glm::vec2( -std::abs( ( rand() % 2000 - 1000.0f ) / 1000000.0f * particle_speed ), ( rand() % 2000 - 1000.0f ) / 1000000.0f * particle_speed );
						break;
				}
				particles[i].color = glm::vec4( rand() % 2000 / 2000.0f, rand() % 2000 / 2000.0f, rand() % 2000 / 2000.0f, rand() % 2000 / 2000.0f );
			}
		}
		for( int i = 0; i < particle_num; i++ ) {
			g_vertex_buffer_data[i][0] = particles[i].pos.x;
			g_vertex_buffer_data[i][1] = particles[i].pos.y;
			g_color_buffer_data[i][0] = particles[i].color.r;
			g_color_buffer_data[i][1] = particles[i].color.g;
			g_color_buffer_data[i][2] = particles[i].color.b;
			g_color_buffer_data[i][3] = particles[i].color.a;
		}
		
		glEnableVertexAttribArray( 0 );
		glEnableVertexAttribArray( 1 );

		glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
		glBufferData( GL_ARRAY_BUFFER, particle_num * sizeof( float ) * 2, g_vertex_buffer_data, GL_DYNAMIC_DRAW );
		glVertexAttribPointer(
			0,		 // The attribute we want to configure
			2,		 // size
			GL_FLOAT,  // type
			GL_FALSE,  // normalized?
			0,		 // stride
			( void* )0 // array buffer offset
		);

		glBindBuffer( GL_ARRAY_BUFFER, colorbuffer );
		glBufferData( GL_ARRAY_BUFFER, particle_num * sizeof( float ) * 4, g_color_buffer_data, GL_DYNAMIC_DRAW );
		glVertexAttribPointer(
			1,		 // The attribute we want to configure
			4,		 // size
			GL_FLOAT,  // type
			GL_FALSE,  // normalized?
			0,		 // stride
			( void* )0 // array buffer offset
		);

		glUseProgram( programID_line );
		double xpos, ypos;
		glfwGetCursorPos( window, &xpos, &ypos );
		glUniform1f( xposID, xpos / window_width * 2 - 1 );
		glUniform1f( yposID, 1 - ypos / window_height * 2 );
		glUniform1i( glGetUniformLocation( programID_line, "window_width" ), window_width );
		glUniform1i( glGetUniformLocation( programID_line, "window_height" ), window_height );
		glDrawArrays( GL_POINTS, 0, particle_num );



		glUseProgram( programID_point );
		glDrawArrays( GL_POINTS, 0, particle_num );



		float dist;
		int vertex_num = 0;
		for( int i = 0; i < particle_num; i++ )
			for( int j = i + 1; j < particle_num; j++ )
				if( ( dist = glm::distance( glm::vec2(particles[i].pos.x * 1024 / 768, particles[i].pos.y), glm::vec2(particles[j].pos.x * 1024 / 768, particles[j].pos.y) ) ) <= max_dist ) {
					g_vertex_buffer_data[vertex_num][0] = particles[i].pos.x;
					g_vertex_buffer_data[vertex_num][1] = particles[i].pos.y;
					g_color_buffer_data[vertex_num][0] = particles[i].color.r;
					g_color_buffer_data[vertex_num][1] = particles[i].color.g;
					g_color_buffer_data[vertex_num][2] = particles[i].color.b;
					g_color_buffer_data[vertex_num++][3] = ( 1.0 - dist / max_dist );
					g_vertex_buffer_data[vertex_num][0] = particles[j].pos.x;
					g_vertex_buffer_data[vertex_num][1] = particles[j].pos.y;
					g_color_buffer_data[vertex_num][0] = particles[j].color.r;
					g_color_buffer_data[vertex_num][1] = particles[j].color.g;
					g_color_buffer_data[vertex_num][2] = particles[j].color.b;
					g_color_buffer_data[vertex_num++][3] = ( 1.0 - dist / max_dist );
				}
		glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
		glBufferData( GL_ARRAY_BUFFER, vertex_num * sizeof( float ) * 2, g_vertex_buffer_data, GL_DYNAMIC_DRAW );
		glVertexAttribPointer(
			0,		 // The attribute we want to configure
			2,		 // size
			GL_FLOAT,  // type
			GL_FALSE,  // normalized?
			0,		 // stride
			( void* )0 // array buffer offset
		);
		glBindBuffer( GL_ARRAY_BUFFER, colorbuffer );
		glBufferData( GL_ARRAY_BUFFER, vertex_num * sizeof( float ) * 4, g_color_buffer_data, GL_DYNAMIC_DRAW );
		glVertexAttribPointer(
			1,		 // The attribute we want to configure
			4,		 // size
			GL_FLOAT,  // type
			GL_FALSE,  // normalized?
			0,		 // stride
			( void* )0 // array buffer offset
		);
		glDrawArrays( GL_LINES, 0, vertex_num);
/*		for( int i = 0; i < particle_num; i++ )
			for( int j = i + 1; j < particle_num; j++ )
				if( ( dist = glm::distance( glm::vec2(particles[i].pos.x * 1024 / 768, particles[i].pos.y), glm::vec2(particles[j].pos.x * 1024 / 768, particles[j].pos.y) ) ) <= max_dist ) {
					vertex_num = 0;
					g_vertex_buffer_data[vertex_num][0] = particles[i].pos.x;
					g_vertex_buffer_data[vertex_num][1] = particles[i].pos.y;
					g_color_buffer_data[vertex_num][0] = particles[i].color.r;
					g_color_buffer_data[vertex_num][1] = particles[i].color.g;
					g_color_buffer_data[vertex_num][2] = particles[i].color.b;
					g_color_buffer_data[vertex_num++][3] = ( 1.0 - dist / max_dist );
					g_vertex_buffer_data[vertex_num][0] = particles[j].pos.x;
					g_vertex_buffer_data[vertex_num][1] = particles[j].pos.y;
					g_color_buffer_data[vertex_num][0] = particles[j].color.r;
					g_color_buffer_data[vertex_num][1] = particles[j].color.g;
					g_color_buffer_data[vertex_num][2] = particles[j].color.b;
					g_color_buffer_data[vertex_num++][3] = ( 1.0 - dist / max_dist );
					glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
					glBufferData( GL_ARRAY_BUFFER, vertex_num * sizeof( float ) * 2, g_vertex_buffer_data, GL_DYNAMIC_DRAW );
					glVertexAttribPointer(
						0,		 // The attribute we want to configure
						2,		 // size
						GL_FLOAT,  // type
						GL_FALSE,  // normalized?
						0,		 // stride
						( void* )0 // array buffer offset
					);
					glBindBuffer( GL_ARRAY_BUFFER, colorbuffer );
					glBufferData( GL_ARRAY_BUFFER, vertex_num * sizeof( float ) * 4, g_color_buffer_data, GL_DYNAMIC_DRAW );
					glVertexAttribPointer(
						1,		 // The attribute we want to configure
						4,		 // size
						GL_FLOAT,  // type
						GL_FALSE,  // normalized?
						0,		 // stride
						( void* )0 // array buffer offset
					);
					glDrawArrays( GL_LINES, 0, vertex_num);
			}*/



		glDisableVertexAttribArray( 0 );
		glDisableVertexAttribArray( 1 );

		time_current = glfwGetTime();
		double time_accurate = (++frame_count) / 60.0;
		double time_delta = time_accurate - time_current;
		time_delta = time_delta > 0 ? time_delta : 0;
		if(print) printf( "%lf %lf %lf\n", time_accurate, time_current, time_delta );
		usleep(time_delta * 1000000);
		glfwSwapBuffers( window );
		glfwPollEvents();
	} while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose( window ) == 0 );

	glDeleteBuffers( 1, &vertexbuffer );
	glDeleteBuffers( 1, &colorbuffer );
	glDeleteVertexArrays( 1, &VertexArrayID );
	glDeleteProgram( programID_line );

	glfwTerminate();

	return 0;
}
	