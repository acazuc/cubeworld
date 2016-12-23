#include "cubeworld.h"

void world_init(t_world *world)
{
	memset(world, 0, sizeof(*world));
	world->biome_noise_hgrid = 20;
	world->biome_noise_octaves = 512;
	world->biome_noise_gain = .20;
	simplex_noise_init(&world->noise, 512, .5, 1338);
}

void world_chunk_add(t_world *world, t_chunk *chunk)
{
	if (!world || !chunk)
		return;
	t_chunk_list *new = malloc(sizeof(*new));
	if (!new)
		ERROR("malloc failed");
	new->chunk = chunk;
	new->next = world->chunks;
	world->chunks = new;
}

t_chunk *world_chunk_get(t_world *world, int32_t x, int32_t z)
{
	t_chunk_list *lst = world->chunks;
	while (lst)
	{
		if (lst->chunk->x == x && lst->chunk->z == z)
			return (lst->chunk);
		lst = lst->next;
	}
	return (NULL);
}

void world_chunk_remove(t_world *world, int32_t x, int32_t z)
{
	t_chunk_list *prv = NULL;
	t_chunk_list *lst = world->chunks;
	t_chunk *tmp;
	while (lst)
	{
		if (lst->chunk->x == x && lst->chunk->z == z)
		{
			if (prv)
				prv->next = lst->next;
			else
				world->chunks = lst->next;
			tmp = lst->chunk;
			free(lst);
			chunk_free(tmp);
			return;
		}
	}
}

void world_render(t_world *world)
{
	glRotatef(world->player.rz, 0, 0, 1);
	glRotatef(world->player.rx, 1, 0, 0);
	glRotatef(world->player.ry, 0, 1, 0);
	glTranslated(-world->player.x, -world->player.y, -world->player.z);
	t_chunk_list *lst = world->chunks;
	while (lst)
	{
		/*t_vec3d player_vec = {0, 0, 1};
		vec3d_rotate_x(&player_vec, TO_RADIANS(world->player.rx));
		vec3d_rotate_y(&player_vec, TO_RADIANS(135 - world->player.ry));
		vec3d_rotate_z(&player_vec, TO_RADIANS(world->player.rz));
		t_vec3d player_2c_vec = {lst->chunk->x - world->player.x * 2, player_vec.y, lst->chunk->z - world->player.z * 2};
		if (vec3d_angle(&player_vec, &player_2c_vec) < TO_RADIANS(90))
		{*/
			chunk_render(lst->chunk);
		//}
		lst = lst->next;
	}
	/*glTranslated(world->player.x, world->player.y, world->player.z);
	glRotatef(-world->player.ry, 0, 1, 0);
	glRotatef(-world->player.rx, 1, 0, 0);
	glRotatef(-world->player.rz, 0, 0, 1);*/
}
