#include "cubeworld.h"

#define SPEED_FACTOR 0.24
#define ORIENTATE_FACTOR 0.1

void player_orientate(t_player *player)
{
	player->ry += window_get_delta_x() * ORIENTATE_FACTOR;
	player->rx += window_get_delta_y() * ORIENTATE_FACTOR;
	player->ry = fmod(player->ry, 360);
	if (player->rx > 90)
		player->rx = 90;
	else if (player->rx < -90)
		player->rx = -90;
}

void player_move(t_player *player)
{
	if (window_is_key_down(GLFW_KEY_SPACE))
		player->y += SPEED_FACTOR;
	if (window_is_key_down(GLFW_KEY_LEFT_SHIFT))
		player->y -= SPEED_FACTOR;
	bool left = window_is_key_down(GLFW_KEY_A);
	bool right = window_is_key_down(GLFW_KEY_D);
	bool front = window_is_key_down(GLFW_KEY_W);
	bool back = window_is_key_down(GLFW_KEY_S);
	if (left && right)
	{
		left = 0;
		right = 0;
	}
	if (front && back)
	{
		front = 0;
		back = 0;
	}
	if (!left && !right && !front && !back)
		return;
	double angle = 0;
	if (back)
	{
		if (left)
			angle = 135;
		else if (right)
			angle = 45;
		else
			angle = 90;
	}
	else if (front)
	{
		if (left)
			angle = -135;
		else if (right)
			angle = -45;
		else
			angle = -90;
	}
	else if (left)
		angle = 180;
	else if (right)
		angle = 0;
	angle += player->ry;
	double diffX = cos(TO_RADIANS(angle)) * SPEED_FACTOR;
	double diffZ = sin(TO_RADIANS(angle)) * SPEED_FACTOR;
	player->x += diffX;
	player->z += diffZ;
}
