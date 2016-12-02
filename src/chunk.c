#include "cubeworld.h"

void chunk_init(t_chunk *chunk, t_world *world, int32_t x, int32_t z)
{
	memset(chunk, 0, sizeof(*chunk));
	chunk->world = world;
	chunk->x = x;
	chunk->z = z;
	if ((chunk->chunkXLess = world_chunk_get(world, x - CHUNK_WIDTH, z)))
		chunk_rebuild(chunk->chunkXLess);
	if ((chunk->chunkXMore = world_chunk_get(world, x + CHUNK_WIDTH, z)))
		chunk_rebuild(chunk->chunkXMore);
	if ((chunk->chunkZLess = world_chunk_get(world, x, z - CHUNK_WIDTH)))
		chunk_rebuild(chunk->chunkZLess);
	if ((chunk->chunkZMore = world_chunk_get(world, x, z + CHUNK_WIDTH)))
		chunk_rebuild(chunk->chunkZMore);
	if (chunk->chunkXLess)
		chunk->chunkXLess->chunkXMore = chunk;
	if (chunk->chunkXMore)
		chunk->chunkXMore->chunkXLess = chunk;
	if (chunk->chunkZLess)
		chunk->chunkZLess->chunkZMore = chunk;
	if (chunk->chunkZMore)
		chunk->chunkZMore->chunkZLess = chunk;
	for (uint32_t blockX = 0; blockX < CHUNK_WIDTH; ++blockX)
	{
		for (uint32_t blockZ = 0; blockZ < CHUNK_WIDTH; ++blockZ)
		{
			uint32_t noiseIndex = (simplex_noise_get2(&world->noise, blockX + x, blockZ + z)) * CHUNK_HEIGHT / 2  + CHUNK_HEIGHT / 2;
			for (uint32_t blockY = 0; blockY < CHUNK_HEIGHT; ++blockY)
			{
				uint8_t blockType = 0;
				if (blockY < noiseIndex)
					blockType = 1;
				block_init(&chunk->blocks[blockX][blockY][blockZ], chunk, blockX + x, blockY, blockZ + z, blockType);
			}
		}
	}
	chunk_rebuild(chunk);
	if (!(chunk->glList = glGenLists(1)))
		ERROR("glGenList failed");
	chunk_redraw(chunk);
}

void chunk_rebuild(t_chunk *chunk)
{
	for (uint32_t blockX = 0; blockX < CHUNK_WIDTH; ++blockX)
	{
		for (uint32_t blockY = 0; blockY < CHUNK_HEIGHT; ++blockY)
		{
			for (uint32_t blockZ = 0; blockZ < CHUNK_WIDTH; ++blockZ)
			{
				block_calculate_visibility(&chunk->blocks[blockX][blockY][blockZ]);
				block_calculate_light(&chunk->blocks[blockX][blockY][blockZ]);
			}
		}
	}
}

void chunk_free(t_chunk *chunk)
{
	free(chunk);
}

void chunk_redraw(t_chunk *chunk)
{
	glNewList(chunk->glList, GL_COMPILE);
	glBegin(GL_TRIANGLES);
	for (uint32_t x = 0; x < CHUNK_WIDTH; ++x)
	{
		for (uint32_t y = 0; y < CHUNK_HEIGHT; ++y)
		{
			for (uint32_t z = 0; z < CHUNK_WIDTH; ++z)
			{
				block_draw(&chunk->blocks[x][y][z]);
			}
		}
	}
	glEnd();
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
	return (&chunk->blocks[x][y][z]);
}
