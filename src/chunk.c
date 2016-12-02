#include "cubeworld.h"

void chunk_init(t_chunk *chunk, t_world *world, int32_t x, int32_t z)
{
	memset(chunk, 0, sizeof(*chunk));
	chunk->world = world;
	if (!(chunk->blocks = malloc(sizeof(*chunk->blocks) * CHUNK_WIDTH)))
		ERROR("malloc failed");
	for (uint32_t blockX = 0; blockX < CHUNK_WIDTH; ++blockX)
	{
		if (!(chunk->blocks[blockX] = malloc(sizeof(**chunk->blocks) * CHUNK_HEIGHT)))
			ERROR("malloc failed");
		for (uint32_t blockY = 0; blockY < CHUNK_HEIGHT; ++blockY)
		{
			if (!(chunk->blocks[blockX][blockY] = malloc(sizeof(***chunk->blocks) * CHUNK_WIDTH)))
				ERROR("malloc failed");
			for (uint32_t blockZ = 0; blockZ < CHUNK_WIDTH; ++blockZ)
			{
				if (!(chunk->blocks[blockX][blockY][blockZ] = malloc(sizeof(****chunk->blocks))))
					ERROR("malloc failed");
				block_init(chunk->blocks[blockX][blockY][blockZ], chunk, blockX + x, blockY, blockZ + z, 1);
			}
		}
	}
	for (uint32_t x = 0; x < CHUNK_WIDTH; ++x)
	{
		for (uint32_t y = 0; y < CHUNK_HEIGHT; ++y)
		{
			for (uint32_t z = 0; z < CHUNK_WIDTH; ++z)
			{
				block_calculate_visibility(chunk->blocks[x][y][z]);
			}
		}
	}
	chunk->x = x;
	chunk->z = z;
	if (!(chunk->glList = glGenLists(1)))
		ERROR("glGenList failed");
	chunk_redraw(chunk);
}

void chunk_free(t_chunk *chunk)
{
	for (uint32_t x = 0; x < CHUNK_WIDTH; ++x)
	{
		for (uint32_t y = 0; y < CHUNK_HEIGHT; ++y)
		{
			for (uint32_t z = 0; z < CHUNK_WIDTH; ++z)
			{
				free(chunk->blocks[x][y][z]);
			}
			free(chunk->blocks[x][y]);
		}
		free(chunk->blocks[x]);
	}
	free(chunk->blocks);
	free(chunk);
}

void chunk_redraw(t_chunk *chunk)
{
	glNewList(chunk->glList, GL_COMPILE);
	for (uint32_t x = 0; x < CHUNK_WIDTH; ++x)
	{
		for (uint32_t y = 0; y < CHUNK_HEIGHT; ++y)
		{
			for (uint32_t z = 0; z < CHUNK_WIDTH; ++z)
			{
				block_draw(chunk->blocks[x][y][z]);
			}
		}
	}
	glEndList();
}

void chunk_render(t_chunk *chunk)
{
	glCallList(chunk->glList);
}

t_block *chunk_block_get(t_chunk *chunk, int32_t x, int32_t y, int32_t z)
{
	if (x < 0 || x >= CHUNK_WIDTH || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_WIDTH)
		return (NULL);
	return (chunk->blocks[x][y][z]);
}