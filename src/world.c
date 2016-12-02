#include "cubeworld.h"

void world_init(t_world *world)
{
	memset(world, 0, sizeof(*world));
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
	while (lst)
	{
		if (lst->chunk->x == x && lst->chunk->z == z)
		{
			if (prv)
				prv->next = lst->next;
			else
				world->chunks = lst->next;
			free(lst);
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
		chunk_render(lst->chunk);
		lst = lst->next;
	}
	glTranslated(world->player.x, world->player.y, world->player.z);
	glRotatef(-world->player.ry, 0, 1, 0);
	glRotatef(-world->player.rx, 1, 0, 0);
	glRotatef(-world->player.rz, 0, 0, 1);
}
