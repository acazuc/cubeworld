#ifndef CUBEWORLD_H
# define CUBEWORLD_H

# include <GL/glu.h>
# include <GL/gl.h>
# include <GLFW/glfw3.h>
# include <pthread.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <stdint.h>
# include <stdio.h>
# include <math.h>
# include <time.h>

# define LOG(s) {printf("[LOG] %s (%s:%d)\n", s, __FILE__, __LINE__);}
# define ERROR(s) {printf("[ERROR] %s (%s:%d)\n", s, __FILE__, __LINE__);exit(EXIT_FAILURE);}

# define TO_RADIANS(angle) (angle / 180. * M_PI)
# define TO_DEGREES(angle) (angle / M_PI / 180.)

# define WINDOW_DEFAULT_WIDTH 1920
# define WINDOW_DEFAULT_HEIGHT 1000
# define WORLD_HEIGHT 256
# define CHUNK_WIDTH 16
# define CHUNK_HEIGHT WORLD_HEIGHT
# define CHUNK_BORDER_X_LESS 1
# define CHUNK_BORDER_X_MORE 2
# define CHUNK_BORDER_Z_LESS 4
# define CHUNK_BORDER_Z_MORE 8
# define LOAD_DISTANCE 8

# define BLOCK_FACE_FRONT 1
# define BLOCK_FACE_BACK 2
# define BLOCK_FACE_LEFT 4
# define BLOCK_FACE_RIGHT 8
# define BLOCK_FACE_BOTTOM 16
# define BLOCK_FACE_TOP 32

# define true 1
# define false 0

typedef struct s_simplex_noise_octave t_simplex_noise_octave;
typedef struct s_simplex_noise_grad t_simplex_noise_grad;
typedef struct s_simplex_noise t_simplex_noise;
typedef struct s_chunk_list t_chunk_list;
typedef struct s_window t_window;
typedef struct s_player t_player;
typedef struct s_entity t_entity;
typedef struct s_world t_world;
typedef struct s_chunk t_chunk;
typedef struct s_block t_block;
typedef struct s_vec3d t_vec3d;
typedef struct s_vec3i t_vec3i;
typedef struct s_env t_env;
typedef char bool;


void *chunk_loader(void *data);
int main();
void window_create(t_env *env);
bool window_is_key_down(int key);
int32_t window_get_delta_x();
int32_t window_get_delta_y();
void windowErrorListener(int32_t id, const char *error);
void windowCharListener(GLFWwindow *window, uint32_t codePoint);
void windowKeyListener(GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods);
void windowScrollListener(GLFWwindow *window, double xoffset, double yoffset);
void windowMouseListener(GLFWwindow *window, int32_t button, int32_t action, int32_t mods);
void windowCursorListener(GLFWwindow *window, double x, double y);
void windowResizeListener(GLFWwindow *window, int32_t width, int32_t height);
void windowFocusListener(GLFWwindow *window, int32_t focused);

void world_init(t_world *world);
void world_chunk_add(t_world *world, t_chunk *chunk);
t_chunk *world_chunk_get(t_world *world, int32_t x, int32_t z);
void world_chunk_remove(t_world *world, int32_t x, int32_t z);
void world_render(t_world *world);

void chunk_init(t_chunk *chunk, t_world *world, int32_t x, int32_t z);
void chunk_free(t_chunk *chunk);
void chunk_redraw(t_chunk *chunk);
void chunk_render(t_chunk *chunk);
t_block *chunk_block_get(t_chunk *chunk, int32_t x, int32_t y, int32_t z);
void chunk_rebuild(t_chunk *chunk);
void chunk_rebuild_borders(t_chunk *chunk, uint8_t borders);

void block_init(t_block *block, t_chunk *chunk, int32_t x, int32_t y, int32_t z, uint8_t type);
void block_free(t_block *block);
void block_draw(t_block *block, t_vec3i *pos);
void block_calculate_visibility(t_block *block, t_vec3i *pos);
void block_calculate_light(t_block *block, t_vec3i *pos);

void player_move(t_player *player);
void player_orientate(t_player *player);

void simplex_noise_init(t_simplex_noise *noise, uint32_t largest_feature, double persistance, int32_t seed);
double simplex_noise_get2(t_simplex_noise *noise, int32_t x, int32_t y);
double simplex_noise_get3(t_simplex_noise *noise, int32_t x, int32_t y, int32_t z);

void vec3d_rotate_x(t_vec3d *vector, double angle);
void vec3d_rotate_y(t_vec3d *vector, double angle);
void vec3d_rotate_z(t_vec3d *vector, double angle);
void vec3d_rotate(t_vec3d *vector, t_vec3d *rotation);
void vec3d_unrotate(t_vec3d *vector, t_vec3d *rotation);
double vec3d_angle(t_vec3d *v1, t_vec3d *v2);
double vec3d_dot(t_vec3d *v1, t_vec3d *v2);
double vec3d_size(t_vec3d *vector);

float perlin_noise(int32_t x, int32_t y, float gain, int32_t octaves, int32_t hgrid);

void frustum_update(t_world *world);
bool frustum_cube(t_world *world, float x1, float y1, float z1, float x2, float y2, float z2);

struct s_simplex_noise_octave
{
	short perm[512];
	short permMod12[512];
};

struct s_simplex_noise_grad
{
	double x;
	double y;
	double z;
};

struct s_simplex_noise
{
	t_simplex_noise_octave *octaves;
	double *frequencies;
	double *amplitudes;
	double persistence;
	uint32_t largest_feature;
	uint32_t octaves_number;
	int32_t seed;
};

struct s_window
{
	GLFWwindow *glfw_window;
	int32_t width;
	int32_t height;
	int32_t mouseX;
	int32_t mouseY;
	int32_t deltaX;
	int32_t deltaY;
};

struct s_player
{
	double rx;
	double ry;
	double rz;
	double x;
	double y;
	double z;
};

struct s_entity
{
	double dx;
	double dy;
	double dz;
	double x;
	double y;
	double z;
};

struct s_world
{
	t_player player;
	t_chunk_list *chunks;
	t_entity **entities;
	t_simplex_noise noise;
	pthread_t chunk_loader;
	float biome_noise_gain;
	int32_t biome_noise_octaves;
	int32_t biome_noise_hgrid;
	float frustum[6][4];
};

struct s_block
{
	t_chunk *chunk;
	struct
	{
		uint8_t f1p1 : 4;
		uint8_t f1p2 : 4;
		uint8_t f1p3 : 4;
		uint8_t f1p4 : 4;
		uint8_t f2p1 : 4;
		uint8_t f2p2 : 4;
		uint8_t f2p3 : 4;
		uint8_t f2p4 : 4;
		uint8_t f3p1 : 4;
		uint8_t f3p2 : 4;
		uint8_t f3p3 : 4;
		uint8_t f3p4 : 4;
		uint8_t f4p1 : 4;
		uint8_t f4p2 : 4;
		uint8_t f4p3 : 4;
		uint8_t f4p4 : 4;
		uint8_t f5p1 : 4;
		uint8_t f5p2 : 4;
		uint8_t f5p3 : 4;
		uint8_t f5p4 : 4;
		uint8_t f6p1 : 4;
		uint8_t f6p2 : 4;
		uint8_t f6p3 : 4;
		uint8_t f6p4 : 4;
	} lights;
	uint8_t visibleFace;
	uint8_t type;
};

struct s_block_dev
{
	t_chunk *chunk;
	int32_t x;
	int32_t y;
	int32_t z;
	uint8_t visibleFace;
	uint8_t type;
};

struct s_chunk
{
	t_world *world;
	t_chunk *chunkXLess;
	t_chunk *chunkXMore;
	t_chunk *chunkZLess;
	t_chunk *chunkZMore;
	t_block *blocks[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_WIDTH];
	pthread_mutex_t gl_mutex;
	uint32_t vao_colors_size;
	uint32_t vao_colors_pos;
	uint8_t *vao_colors;
	uint32_t vao_vertex_size;
	uint32_t vao_vertex_pos;
	int32_t *vao_vertex;
	int32_t x;
	int32_t z;
	GLuint glList;
};

struct s_chunk_list
{
	t_chunk *chunk;
	t_chunk_list *next;
};

struct s_vec3d
{
	double x;
	double y;
	double z;
};

struct s_vec3i
{
	int32_t x;
	int32_t y;
	int32_t z;
};

struct s_env
{
	t_window window;
	t_world world;
};

#endif
