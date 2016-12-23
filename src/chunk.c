#include "cubeworld.h"

void chunk_init(t_chunk *chunk, t_world *world, int32_t x, int32_t z)
{
	memset(chunk, 0, sizeof(*chunk));
	chunk->world = world;
	chunk->x = x;
	chunk->z = z;
	pthread_mutex_init(&chunk->gl_mutex, NULL);
	chunk->vao_colors_size = 0;
	chunk->vao_colors_pos = 0;
	chunk->vao_colors = NULL;
	chunk->vao_vertex_size = 0;
	chunk->vao_vertex_pos = 0;
	chunk->vao_vertex = NULL;
	chunk->chunkXLess = world_chunk_get(world, x - CHUNK_WIDTH, z);
	chunk->chunkXMore = world_chunk_get(world, x + CHUNK_WIDTH, z);
	chunk->chunkZLess = world_chunk_get(world, x, z - CHUNK_WIDTH);
	chunk->chunkZMore = world_chunk_get(world, x, z + CHUNK_WIDTH);
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
			//uint32_t noiseIndex = perlin_noise(blockX + x, blockZ + z, world->biome_noise_gain, world->biome_noise_octaves, world->biome_noise_hgrid) * CHUNK_HEIGHT / 2  + CHUNK_HEIGHT / 2;
			for (uint32_t blockY = 0; blockY < CHUNK_HEIGHT; ++blockY)
			{
				uint8_t blockType = 0;
				if (blockY < noiseIndex)
				{
					if (blockY < CHUNK_HEIGHT / 3)
						blockType = 1;
					else if (blockY == CHUNK_HEIGHT / 3)
						blockType = 2;
					else if (blockY <= CHUNK_HEIGHT / 2)
						blockType = 3;
					else
						blockType = 4;
					if (!(chunk->blocks[blockX][blockY][blockZ] = malloc(sizeof(****chunk->blocks))))
						ERROR("malloc failed");
					block_init(chunk->blocks[blockX][blockY][blockZ], chunk, blockX + x, blockY, blockZ + z, blockType);
				}
				else
				{
					chunk->blocks[blockX][blockY][blockZ] = NULL;
				}
			}
		}
	}
	/*if (!(chunk->glList = glGenLists(1)))
		ERROR("glGenList failed");*/
	chunk_rebuild(chunk);
	if (chunk->chunkXLess)
		chunk_rebuild_borders(chunk->chunkXLess, CHUNK_BORDER_X_MORE);
	if (chunk->chunkXMore)
		chunk_rebuild_borders(chunk->chunkXMore, CHUNK_BORDER_X_LESS);
	if (chunk->chunkZLess)
		chunk_rebuild_borders(chunk->chunkZLess, CHUNK_BORDER_Z_MORE);
	if (chunk->chunkZMore)
		chunk_rebuild_borders(chunk->chunkZMore, CHUNK_BORDER_Z_LESS);
}

void chunk_rebuild(t_chunk *chunk)
{
	for (uint32_t blockX = 0; blockX < CHUNK_WIDTH; ++blockX)
	{
		for (uint32_t blockY = 0; blockY < CHUNK_HEIGHT; ++blockY)
		{
			for (uint32_t blockZ = 0; blockZ < CHUNK_WIDTH; ++blockZ)
			{
				if (chunk->blocks[blockX][blockY][blockZ])
				{
					t_vec3i pos = {chunk->x + blockX, blockY, chunk->z + blockZ};
					block_calculate_visibility(chunk->blocks[blockX][blockY][blockZ], &pos);
					block_calculate_light(chunk->blocks[blockX][blockY][blockZ], &pos);
				}
			}
		}
	}
	chunk_redraw(chunk);
}

void chunk_rebuild_borders(t_chunk *chunk, uint8_t borders)
{
	if (borders & CHUNK_BORDER_X_LESS)
	{
		for (uint32_t blockY = 0; blockY < CHUNK_HEIGHT; ++blockY)
		{
			for (uint32_t blockZ = 0; blockZ < CHUNK_WIDTH; ++blockZ)
			{
				if (chunk->blocks[0][blockY][blockZ])
				{
					t_vec3i pos = {chunk->x, blockY, chunk->z + blockZ};
					block_calculate_visibility(chunk->blocks[0][blockY][blockZ], &pos);
					block_calculate_light(chunk->blocks[0][blockY][blockZ], &pos);
				}
			}
		}
	}
	if (borders & CHUNK_BORDER_X_MORE)
	{
		for (uint32_t blockY = 0; blockY < CHUNK_HEIGHT; ++blockY)
		{
			for (uint32_t blockZ = 0; blockZ < CHUNK_WIDTH; ++blockZ)
			{
				if (chunk->blocks[CHUNK_WIDTH - 1][blockY][blockZ])
				{
					t_vec3i pos = {chunk->x + CHUNK_WIDTH - 1, blockY, chunk->z + blockZ};
					block_calculate_visibility(chunk->blocks[CHUNK_WIDTH - 1][blockY][blockZ], &pos);
					block_calculate_light(chunk->blocks[CHUNK_WIDTH - 1][blockY][blockZ], &pos);
				}
			}
		}
	}
	if (borders & CHUNK_BORDER_Z_LESS)
	{
		for (uint32_t blockX = 0; blockX < CHUNK_WIDTH; ++blockX)
		{
			for (uint32_t blockY = 0; blockY < CHUNK_HEIGHT; ++blockY)
			{
				if (chunk->blocks[blockX][blockY][0])
				{
					t_vec3i pos = {chunk->x + blockX, blockY, chunk->z};
					block_calculate_visibility(chunk->blocks[blockX][blockY][0], &pos);
					block_calculate_light(chunk->blocks[blockX][blockY][0], &pos);
				}
			}
		}
	}
	if (borders & CHUNK_BORDER_Z_MORE)
	{
		for (uint32_t blockX = 0; blockX < CHUNK_WIDTH; ++blockX)
		{
			for (uint32_t blockY = 0; blockY < CHUNK_HEIGHT; ++blockY)
			{
				if (chunk->blocks[blockX][blockY][CHUNK_WIDTH - 1])
				{
					t_vec3i pos = {chunk->x + blockX, blockY, chunk->z + CHUNK_WIDTH - 1};
					block_calculate_visibility(chunk->blocks[blockX][blockY][CHUNK_WIDTH - 1], &pos);
					block_calculate_light(chunk->blocks[blockX][blockY][CHUNK_WIDTH - 1], &pos);
				}
			}
		}
	}
	chunk_redraw(chunk);
}

void chunk_free(t_chunk *chunk)
{
	if (chunk->chunkXLess)
	{
		chunk->chunkXLess->chunkXMore = NULL;
		chunk_rebuild_borders(chunk->chunkXLess, CHUNK_BORDER_X_MORE);
	}
	if (chunk->chunkXMore)
	{
		chunk->chunkXMore->chunkXLess = NULL;
		chunk_rebuild_borders(chunk->chunkXMore, CHUNK_BORDER_X_LESS);
	}
	if (chunk->chunkZLess)
	{
		chunk->chunkZLess->chunkZMore = NULL;
		chunk_rebuild_borders(chunk->chunkZLess, CHUNK_BORDER_Z_MORE);
	}
	if (chunk->chunkZMore)
	{
		chunk->chunkZMore->chunkZLess = NULL;
		chunk_rebuild_borders(chunk->chunkZMore, CHUNK_BORDER_Z_LESS);
	}
	for (uint32_t x = 0; x < CHUNK_WIDTH; ++x)
	{
		for (uint32_t y = 0; y < CHUNK_HEIGHT; ++y)
		{
			for (uint32_t z = 0; z < CHUNK_WIDTH; ++z)
			{
				free(chunk->blocks[x][y][z]);
			}
		}
	}
	free(chunk);
}

void chunk_redraw(t_chunk *chunk)
{
	pthread_mutex_lock(&chunk->gl_mutex);
	///*
	free(chunk->vao_colors);
	free(chunk->vao_vertex);
	chunk->vao_colors_size = 0;
	chunk->vao_colors_pos = 0;
	if (!(chunk->vao_colors = malloc(1)))
		ERROR("malloc failed");
	chunk->vao_vertex_size = 0;
	chunk->vao_vertex_pos = 0;
	if (!(chunk->vao_vertex = malloc(1)))
		ERROR("malloc failed");
	//*/
	//glNewList(chunk->glList, GL_COMPILE);
	//glBegin(GL_TRIANGLES);
	for (uint32_t x = 0; x < CHUNK_WIDTH; ++x)
	{
		for (uint32_t y = 0; y < CHUNK_HEIGHT; ++y)
		{
			for (uint32_t z = 0; z < CHUNK_WIDTH; ++z)
			{
				if (chunk->blocks[x][y][z])
				{
					t_vec3i pos = {chunk->x + x, y, chunk->z + z};
					block_draw(chunk->blocks[x][y][z], &pos);
				}
			}
		}
	}
	//glEnd();
	//glEndList();
	pthread_mutex_unlock(&chunk->gl_mutex);
}

void chunk_render(t_chunk *chunk)
{
	if (!frustum_cube(chunk->world, chunk->x, 0, chunk->z, chunk->x + CHUNK_WIDTH, CHUNK_HEIGHT, chunk->z + CHUNK_WIDTH))
		return;
	pthread_mutex_lock(&chunk->gl_mutex);
	///*
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, chunk->vao_colors);
	glVertexPointer(3, GL_INT, 0, chunk->vao_vertex);
	glDrawArrays(GL_TRIANGLES, 0, chunk->vao_colors_pos / 4);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	//*/
	//glCallList(chunk->glList);
	pthread_mutex_unlock(&chunk->gl_mutex);
}

t_block *chunk_block_get(t_chunk *chunk, int32_t x, int32_t y, int32_t z)
{
	if (x < 0 || x >= CHUNK_WIDTH || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_WIDTH)
		return (NULL);
	return (chunk->blocks[x][y][z]);
}
