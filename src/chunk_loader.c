#include "cubeworld.h"

static bool check_chunk(t_world *world, int32_t chunk_x, int32_t chunk_z)
{
	double part1 = pow((world->player.z - ((int)world->player.z % CHUNK_WIDTH)) - chunk_z, 2);
	double part2 = pow((world->player.x - ((int)world->player.x % CHUNK_WIDTH)) - chunk_x, 2);
	int distance = sqrt(part1 + part2);
	if (distance > LOAD_DISTANCE * CHUNK_WIDTH)
		return (false);
	if (world_chunk_get(world, chunk_x, chunk_z))
		return (false);
	t_chunk *chunk = malloc(sizeof(*chunk));
	if (!chunk)
		ERROR("malloc failed");
	chunk_init(chunk, world, chunk_x, chunk_z);
	world_chunk_add(world, chunk);
	return (true);
}

void *chunk_loader(void *data)
{
	t_world *world = (t_world*)data;
startLoop:
	{
		float playerX = world->player.x;
		float playerZ = world->player.z;
		int playerChunkX = playerX - (int)playerX % CHUNK_WIDTH;
		int playerChunkZ = playerZ - (int)playerZ % CHUNK_WIDTH;
		t_chunk_list *lst = world->chunks;
		t_chunk_list *prv = NULL;
		t_chunk *tmp;
		while (lst)
		{
			double part1 = pow(playerChunkZ - (lst->chunk->z + CHUNK_WIDTH / 2), 2);
			double part2 = pow(playerChunkX - (lst->chunk->x + CHUNK_WIDTH / 2), 2);
			int distance = sqrt(part1 + part2);
			if (distance > LOAD_DISTANCE * 1.5 * CHUNK_WIDTH)
			{
				tmp = lst->chunk;
				if (prv)
				{
					prv->next = lst->next;
					free(lst);
					lst = lst->next;
				}
				else
				{
					prv = lst->next; // use as tmp
					free(lst);
					lst = prv;
					world->chunks = prv;
					prv = NULL;
				}
				chunk_free(tmp);
				continue;
			}
			prv = lst;
			lst = lst->next;
		}
		if (check_chunk(world, playerChunkX, playerChunkZ))
			goto startLoop;
		for (int32_t i = 0; i <= LOAD_DISTANCE; ++i)
		{
			int32_t chunkX = playerChunkX - i * CHUNK_WIDTH;
			int32_t chunkZ = playerChunkZ - i * CHUNK_WIDTH;
			if (check_chunk(world, chunkX, chunkZ))
				goto startLoop;
			for (int32_t j = 0; j <= i * 2; ++j)
			{
				chunkX += CHUNK_WIDTH;
				if (check_chunk(world, chunkX, chunkZ))
					goto startLoop;
			}
			for (int32_t j = 0; j <= i * 2; ++j)
			{
				chunkZ += CHUNK_WIDTH;
				if (check_chunk(world, chunkX, chunkZ))
					goto startLoop;
			}
			for (int32_t j = 0; j <= i * 2; ++j)
			{
				chunkX -= CHUNK_WIDTH;
				if (check_chunk(world, chunkX, chunkZ))
					goto startLoop;
			}
			for (int32_t j = 0; j <= i * 2 - 1; ++j)
			{
				chunkZ -= CHUNK_WIDTH;
				if (check_chunk(world, chunkX, chunkZ))
					goto startLoop;
			}
		}
		struct timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = 10000000;
		nanosleep(&ts, NULL);
	}
	goto startLoop;
	return (NULL);
}
