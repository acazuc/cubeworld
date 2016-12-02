#ifndef CUBEWORLD_H
# define CUBEWORLD_H

# include <GL/glu.h>
# include <GL/gl.h>
# include <GLFW/glfw3.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <stdint.h>
# include <stdio.h>
# include <math.h>
# include <time.h>

# define LOG(s) {printf("[LOG] %s (%s:%d)", s, __FILE__, __LINE__);}
# define ERROR(s) {printf("[ERROR] %s (%s:%d)", s, __FILE__, __LINE__);exit(EXIT_FAILURE);}

# define TO_RADIANS(angle) (angle / 180. * M_PI)
# define TO_DEGREES(angle) (angle / M_PI / 180.)

# define WINDOW_DEFAULT_WIDTH 1280
# define WINDOW_DEFAULT_HEIGHT 720
# define WORLD_HEIGHT 256
# define CHUNK_WIDTH 16
# define CHUNK_HEIGHT WORLD_HEIGHT

typedef struct s_chunk_list t_chunk_list;
typedef struct s_window t_window;
typedef struct s_player t_player;
typedef struct s_entity t_entity;
typedef struct s_world t_world;
typedef struct s_chunk t_chunk;
typedef struct s_block t_block;
typedef struct s_env t_env;
typedef char bool;

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

void block_init(t_block *block, t_chunk *chunk, int32_t x, int32_t y, int32_t z, uint8_t type);
void block_free(t_block *block);
void block_draw(t_block *block);
void block_calculate_visibility(t_block *block);

void player_move(t_player *player);
void player_orientate(t_player *player);

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
};

struct s_chunk
{
	t_world *world;
	t_block ****blocks;
	int32_t x;
	int32_t z;
	GLuint glList;
};

struct s_chunk_list
{
	t_chunk *chunk;
	t_chunk_list *next;
};

struct s_block
{
	t_chunk *chunk;
	uint8_t visibleFace[6];
	uint8_t type;
	int32_t x;
	int32_t y;
	int32_t z;
	bool transparent;
};

struct s_env
{
	t_window window;
	t_world world;
};

#endif