#include "cubeworld.h"

t_env *g_env;

void window_create(t_env *env)
{
	memset(&env->window, 0, sizeof(env->window));
	if (!(env->window.glfw_window = glfwCreateWindow(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT, "Cubeworld", NULL, NULL)))
		ERROR("Window: can't create window");
	windowResizeListener(env->window.glfw_window, WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);
	glfwSwapInterval(1);
	glfwMakeContextCurrent(env->window.glfw_window);
	env->window.width = WINDOW_DEFAULT_WIDTH;
	env->window.height = WINDOW_DEFAULT_HEIGHT;
	glfwSetErrorCallback(windowErrorListener);
	glfwSetCharCallback(env->window.glfw_window, windowCharListener);
	glfwSetKeyCallback(env->window.glfw_window, windowKeyListener);
	glfwSetScrollCallback(env->window.glfw_window, windowScrollListener);
	glfwSetMouseButtonCallback(env->window.glfw_window, windowMouseListener);
	glfwSetCursorPosCallback(env->window.glfw_window, windowCursorListener);
	glfwSetFramebufferSizeCallback(env->window.glfw_window, windowResizeListener);
	glfwSetWindowFocusCallback(env->window.glfw_window, windowFocusListener);
	glfwSetInputMode(env->window.glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

bool window_is_key_down(int key)
{
	return (glfwGetKey(g_env->window.glfw_window, key) == GLFW_PRESS);
}

int32_t window_get_delta_x()
{
	return (g_env->window.deltaX);
}

int32_t window_get_delta_y()
{
	return (g_env->window.deltaY);
}
