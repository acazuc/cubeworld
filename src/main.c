#include "cubeworld.h"

t_env *g_env = NULL;

int main()
{
	t_env env;

	g_env = &env;
	srand(time(NULL));
	if (!glfwInit())
		ERROR("Can't init glfw");
	printf("%lu, %lu\n", sizeof(struct s_block), sizeof(struct s_block_dev));
	window_create(&env);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearDepth(1);
	glClearColor(0.48, 0.65, 0.99, 0);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	world_init(&env.world);
	env.world.player.y = 50;
	//pthread_create(&env.world.chunk_loader, NULL, chunk_loader, &env.world);
	while (!glfwWindowShouldClose(env.window.glfw_window))
	{
		chunk_loader(&env.world);
		windowResizeListener(env.window.glfw_window, env.window.width, env.window.height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		player_orientate(&env.world.player);
		player_move(&env.world.player);
		world_render(&env.world);
		glfwSwapBuffers(env.window.glfw_window);
		g_env->window.deltaX = 0;
		g_env->window.deltaY = 0;
		glfwPollEvents();
	}
	return (EXIT_SUCCESS);
}

void windowErrorListener(int32_t id, const char *error)
{
	(void)id;
	ERROR(error);
}

void windowCharListener(GLFWwindow *window, uint32_t codePoint)
{
	(void)window;
	(void)codePoint;
}

void windowKeyListener(GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods)
{
	if (key == GLFW_KEY_ESCAPE)
		exit(EXIT_SUCCESS);
	(void)window;
	(void)key;
	(void)scancode;
	(void)action;
	(void)mods;
}

void windowScrollListener(GLFWwindow *window, double xoffset, double yoffset)
{
	(void)window;
	(void)xoffset;
	(void)yoffset;
}

void windowMouseListener(GLFWwindow *window, int32_t button, int32_t action, int32_t mods)
{
	(void)window;
	(void)button;
	(void)action;
	(void)mods;
}

void windowCursorListener(GLFWwindow *window, double x, double y)
{
	g_env->window.deltaX = x - g_env->window.mouseX;
	g_env->window.deltaY = y - g_env->window.mouseY;
	g_env->window.mouseX = x;
	g_env->window.mouseY = y;
	(void)window;
	(void)x;
	(void)y;
}

void windowResizeListener(GLFWwindow *window, int32_t width, int32_t height)
{
	(void)window;
	g_env->window.width = width;
	g_env->window.height = height;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (float)width / (float)height, .01, 2000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

void windowFocusListener(GLFWwindow *window, int32_t focused)
{
	(void)window;
	(void)focused;
}
