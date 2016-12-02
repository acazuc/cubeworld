#include "cubeworld.h"

#define BLOCK_WIDTH (.5)

void block_init(t_block *block, t_chunk *chunk, int32_t x, int32_t y, int32_t z, uint8_t type)
{
	memset(block, 0, sizeof(*block));
	block->chunk = chunk;
	block->x = x;
	block->y = y;
	block->z = z;
	block->cx = x - chunk->x;
	block->cz = z - chunk->z;
	block->type = type;
	for (uint8_t i = 0; i < 6; ++i)
		block->visibleFace[i] = 1;
	for (uint8_t face = 0; face < 6; ++face)
		for (uint8_t point = 0; point < 4; ++point)
			block->lights[face][point] = 128;
	block->transparent = type == 0;
}

void block_free(t_block *block)
{
	free(block);
}

static bool _block_calculate_is_transparent_same_chunk(t_block *block, int32_t addX, int32_t addY, int32_t addZ)
{
	t_block *tmp_block = chunk_block_get(block->chunk, block->cx + addX, block->y + addY, block->cz + addZ);
	return (!tmp_block || tmp_block->transparent);
}

static bool _block_calculate_is_transparent_other_chunk(t_chunk *chunk, int32_t newBlockX, int32_t newBlockY, int32_t newBlockZ)
{
	if (!chunk)
		return (1);
	t_block *tmp_block = chunk_block_get(chunk, newBlockX, newBlockY, newBlockZ);
	return (!tmp_block || tmp_block->transparent);
}

static bool _block_calculate_is_transparent(t_block *block, int32_t addX, int32_t addY, int32_t addZ)
{
	if (block->y + addY < 0 || block->y + addY >= CHUNK_HEIGHT - 1)
		return (1);
	bool different = (addX < 0 && block->cx < addX) || (addX > 0 && block->cx + addX > CHUNK_WIDTH - 1) || (addZ < 0 && block->cz < addZ) || (addZ > 0 && block->cz > CHUNK_WIDTH - 1);
	if (different)
	{
		return (1);
		int32_t newX = block->cx + addX;
		int32_t newY = block->y + addY;
		int32_t newZ = block->cz + addZ;
		t_chunk *new_chunk = block->chunk;
		while (newX < 0)
		{
			newX += CHUNK_WIDTH;
			if (!(new_chunk = new_chunk->chunkXLess))
				return (1);
		}
		while (newX >= CHUNK_WIDTH)
		{
			newX -= CHUNK_WIDTH;
			if (!(new_chunk = new_chunk->chunkXMore))
				return (1);
		}
		while (newZ < 0)
		{
			newZ += CHUNK_WIDTH;
			if (!(new_chunk = new_chunk->chunkZLess))
				return (1);
		}
		while (newZ >= CHUNK_WIDTH)
		{
			newZ -= CHUNK_WIDTH;
			if (!(new_chunk = new_chunk->chunkZMore))
				return (1);
		}
		return (_block_calculate_is_transparent_other_chunk(new_chunk, newX, newY, newZ));
	}
	return (_block_calculate_is_transparent_same_chunk(block, addX, addY, addZ));
}

void block_calculate_visibility(t_block *block)
{
	block->visibleFace[0] = _block_calculate_is_transparent(block, 0, 0, -1);
	block->visibleFace[1] = _block_calculate_is_transparent(block, 0, 0, 1);
	block->visibleFace[4] = _block_calculate_is_transparent(block, 0, -1, 0);
	block->visibleFace[5] = _block_calculate_is_transparent(block, 0, 1, 0);
	if (block->cx == 0)
		block->visibleFace[2] = _block_calculate_is_transparent_other_chunk(block->chunk->chunkXLess, CHUNK_WIDTH - 1, block->y, block->z);
	else
		block->visibleFace[2] = _block_calculate_is_transparent_same_chunk(block, -1, 0, 0);
	if (block->cx == CHUNK_WIDTH - 1)
		block->visibleFace[3] = _block_calculate_is_transparent_other_chunk(block->chunk->chunkXMore, 0, block->y, block->z);
	else
		block->visibleFace[3] = _block_calculate_is_transparent_same_chunk(block, 1, 0, 0);
	/*if (block->z == 0)
		block->visibleFace[0] = _block_calculate_is_transparent_other_chunk(block->chunk->chunkZLess, block->x, block->y, CHUNK_WIDTH - 1);
	else
		block->visibleFace[0] = _block_calculate_is_transparent_same_chunk(block, 0, 0, -1);
	if (block->cz == CHUNK_WIDTH - 1)
		block->visibleFace[1] = _block_calculate_is_transparent_other_chunk(block->chunk->chunkZMore, block->x, block->y, 0);
	else
		block->visibleFace[1] = _block_calculate_is_transparent_same_chunk(block, 0, 0, 1);*/
}

void _block_calculate_ambient_occlusion_light(t_block *block)
{
	if (block->transparent)
		return;
	#define LESS 32
	if (block->visibleFace[0])
	{
		if (block->cz > 0)
		{
			if ( !_block_calculate_is_transparent_same_chunk(block, 1, -1, -1))
				block->lights[0][3] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, 0, -1, -1))
			{
				block->lights[0][3] -= LESS;
				block->lights[0][0] -= LESS;
			}
			if (!_block_calculate_is_transparent_same_chunk(block, -1, -1, -1))
				block->lights[0][0] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, -1, 1, -1))
				block->lights[0][1] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, 0, 1, -1))
			{
				block->lights[0][1] -= LESS;
				block->lights[0][2] -= LESS;
			}
			if (!_block_calculate_is_transparent_same_chunk(block, 1, 1, -1))
				block->lights[0][2] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, 1, 0, -1))
			{
				block->lights[0][2] -= LESS;
				block->lights[0][3] -= LESS;
			}
			if (!_block_calculate_is_transparent_same_chunk(block, -1, 0, -1))
			{
				block->lights[0][0] -= LESS;
				block->lights[0][1] -= LESS;
			}
		}
	}
	if (block->visibleFace[1])
	{
		if (block->cz < CHUNK_WIDTH - 1)
		{
			if (!_block_calculate_is_transparent_same_chunk(block, 1, -1, 1))
				block->lights[1][3] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, 0, -1, 1))
			{
				block->lights[1][3] -= LESS;
				block->lights[1][0] -= LESS;
			}
			if (!_block_calculate_is_transparent_same_chunk(block, -1, -1, 1))
				block->lights[1][0] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, -1, 1, 1))
				block->lights[1][1] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, 0, 1, 1))
			{
				block->lights[1][1] -= LESS;
				block->lights[1][2] -= LESS;
			}
			if (!_block_calculate_is_transparent_same_chunk(block, 1, 1, 1))
				block->lights[1][2] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, 1, 0, 1))
			{
				block->lights[1][2] -= LESS;
				block->lights[1][3] -= LESS;
			}
			if (!_block_calculate_is_transparent_same_chunk(block, -1, 0, 1))
			{
				block->lights[1][0] -= LESS;
				block->lights[1][1] -= LESS;
			}
		}
	}
	if (block->visibleFace[2])
	{
		if (block->cx > 0)
		{
			if (!_block_calculate_is_transparent_same_chunk(block, -1, -1, 1))
				block->lights[2][3] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, -1, -1, 1))
			{
				block->lights[2][3] -= LESS;
				block->lights[2][0] -= LESS;
			}
			if (!_block_calculate_is_transparent_same_chunk(block, -1, -1, -1))
				block->lights[2][0] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, -1, 1, -1))
				block->lights[2][1] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, -1, 1, 0))
			{
				block->lights[2][1] -= LESS;
				block->lights[2][2] -= LESS;
			}
			if (!_block_calculate_is_transparent_same_chunk(block, -1, 1, 1))
				block->lights[2][2] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, -1, 0, 1))
			{
				block->lights[2][2] -= LESS;
				block->lights[2][3] -= LESS;
			}

			if (!_block_calculate_is_transparent_same_chunk(block, -1, 0, -1))
			{
				block->lights[2][0] -= LESS;
				block->lights[2][1] -= LESS;
			}
		}
	}
	if (block->visibleFace[3])
	{
		if (block->cx < CHUNK_WIDTH - 1)
		{
			if (!_block_calculate_is_transparent_same_chunk(block, 1, -1, 1))
				block->lights[3][3] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, 1, -1, 1))
			{
				block->lights[3][3] -= LESS;
				block->lights[3][0] -= LESS;
			}
			if (!_block_calculate_is_transparent_same_chunk(block, 1, -1, -1))
				block->lights[3][0] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, 1, 1, -1))
				block->lights[3][1] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, 1, 1, 0))
			{
				block->lights[3][1] -= LESS;
				block->lights[3][2] -= LESS;
			}
			if (!_block_calculate_is_transparent_same_chunk(block, 1, 1, 1))
				block->lights[3][2] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, 1, 0, 1))
			{
				block->lights[3][2] -= LESS;
				block->lights[3][3] -= LESS;
			}

			if (!_block_calculate_is_transparent_same_chunk(block, 1, 0, -1))
			{
				block->lights[3][0] -= LESS;
				block->lights[3][1] -= LESS;
			}
		}
	}
	if (block->visibleFace[4])
	{
		if (block->y > 0)
		{
			if (!_block_calculate_is_transparent_same_chunk(block, -1, -1, -1))
				block->lights[4][1] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, -1, -1, 0))
			{
				block->lights[4][0] -= LESS;
				block->lights[4][1] -= LESS;
			}
			if (!_block_calculate_is_transparent_same_chunk(block, -1, -1, 1))
				block->lights[4][0] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, 1, -1, -1))
				block->lights[4][2] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, 1, -1, 0))
			{
				block->lights[4][2] -= LESS;
				block->lights[4][3] -= LESS;
			}
			if (!_block_calculate_is_transparent_same_chunk(block, 1, -1, 1))
				block->lights[4][3] -= LESS;
			if (!_block_calculate_is_transparent_same_chunk(block, 0, -1, -1))
			{
				block->lights[4][1] -= LESS;
				block->lights[4][2] -= LESS;
			}
			if (!_block_calculate_is_transparent_same_chunk(block, 0, -1, 1))
			{
				block->lights[4][3] -= LESS;
				block->lights[4][0] -= LESS;
			}
		}
	}
	if (block->visibleFace[5])
	{
		if (block->y < CHUNK_HEIGHT - 1)
		{
			if (!_block_calculate_is_transparent(block, -1, 1, 1))
				block->lights[5][0] -= LESS;
			if (!_block_calculate_is_transparent(block, -1, 1, 0))
			{
				block->lights[5][0] -= LESS;
				block->lights[5][1] -= LESS;
			}
			if (!_block_calculate_is_transparent(block, -1, 1, -1))
				block->lights[5][1] -= LESS;
			if (!_block_calculate_is_transparent(block, 1, 1, -1))
				block->lights[5][2] -= LESS;
			if (!_block_calculate_is_transparent(block, 1, 1, 0))
			{
				block->lights[5][2] -= LESS;
				block->lights[5][3] -= LESS;
			}
			if (!_block_calculate_is_transparent(block, 1, 1, 1))
				block->lights[5][3] -= LESS;
			if (!_block_calculate_is_transparent(block, 0, 1, -1))
			{
				block->lights[5][1] -= LESS;
				block->lights[5][2] -= LESS;
			}
			if (!_block_calculate_is_transparent(block, 0, 1, 1))
			{
				block->lights[5][3] -= LESS;
				block->lights[5][0] -= LESS;
			}
			/*if (block->cx > 0)
			{
				if (block->cz < CHUNK_WIDTH - 1 && !_block_calculate_is_transparent_same_chunk(block, -1, 1, 1))
					block->lights[5][0] -= LESS;
				if (!_block_calculate_is_transparent_same_chunk(block, -1, 1, 0))
				{
					block->lights[5][0] -= LESS;
					block->lights[5][1] -= LESS;
				}
				if (block->cz > 0 && !_block_calculate_is_transparent_same_chunk(block, -1, 1, -1))
					block->lights[5][1] -= LESS;
			}
			if (block->cx < CHUNK_WIDTH - 1)
			{
				if (block->cz > 0 && !_block_calculate_is_transparent_same_chunk(block, 1, 1, -1))
					block->lights[5][2] -= LESS;
				if (!_block_calculate_is_transparent_same_chunk(block, 1, 1, 0))
				{
					block->lights[5][2] -= LESS;
					block->lights[5][3] -= LESS;
				}
				if (block->cz < CHUNK_WIDTH - 1 && !_block_calculate_is_transparent_same_chunk(block, 1, 1, 1))
					block->lights[5][3] -= LESS;
			}
			if (block->cz > 0 && !_block_calculate_is_transparent_same_chunk(block, 0, 1, -1))
			{
				block->lights[5][1] -= LESS;
				block->lights[5][2] -= LESS;
			}
			if (block->cz < CHUNK_WIDTH - 1 && !_block_calculate_is_transparent_same_chunk(block, 0, 1, 1))
			{
				block->lights[5][3] -= LESS;
				block->lights[5][0] -= LESS;
			}*/
		}
	}
}

void block_calculate_light(t_block *block)
{
	_block_calculate_ambient_occlusion_light(block);
}

void block_draw(t_block *block)
{
	if (block->transparent)
		return;
	if (block->visibleFace[0])
	{
		if (block->lights[0][1] + block->lights[0][3] > block->lights[0][0] + block->lights[0][2])
		{
			glColor3ub(block->lights[0][1], block->lights[0][1], block->lights[0][1]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[0][2], block->lights[0][2], block->lights[0][2]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[0][3], block->lights[0][3], block->lights[0][3]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[0][3], block->lights[0][3], block->lights[0][3]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[0][0], block->lights[0][0], block->lights[0][0]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[0][1], block->lights[0][1], block->lights[0][1]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		}
		else
		{
			glColor3ub(block->lights[0][0], block->lights[0][0], block->lights[0][0]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[0][1], block->lights[0][1], block->lights[0][1]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[0][2], block->lights[0][2], block->lights[0][2]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[0][3], block->lights[0][3], block->lights[0][3]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[0][0], block->lights[0][0], block->lights[0][0]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[0][2], block->lights[0][2], block->lights[0][2]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		}
	}
	if (block->visibleFace[1])
	{
		if (block->lights[1][1] + block->lights[1][3] > block->lights[1][0] + block->lights[1][2])
		{
			glColor3ub(block->lights[1][1], block->lights[1][1], block->lights[1][1]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[1][2], block->lights[1][2], block->lights[1][2]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[1][3], block->lights[1][3], block->lights[1][3]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[1][3], block->lights[1][3], block->lights[1][3]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[1][0], block->lights[1][0], block->lights[1][0]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[1][1], block->lights[1][1], block->lights[1][1]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
		}
		else
		{
			glColor3ub(block->lights[1][0], block->lights[1][0], block->lights[1][0]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[1][1], block->lights[1][1], block->lights[1][1]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[1][2], block->lights[1][2], block->lights[1][2]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[1][3], block->lights[1][3], block->lights[1][3]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[1][0], block->lights[1][0], block->lights[1][0]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[1][2], block->lights[1][2], block->lights[1][2]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
		}
	}
	if (block->visibleFace[2])
	{
		if (block->lights[2][1] + block->lights[2][3] > block->lights[2][0] + block->lights[2][2])
		{
			glColor3ub(block->lights[2][1], block->lights[2][1], block->lights[2][1]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[2][2], block->lights[2][2], block->lights[2][2]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[2][3], block->lights[2][3], block->lights[2][3]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[2][3], block->lights[2][3], block->lights[2][3]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[2][0], block->lights[2][0], block->lights[2][0]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[2][1], block->lights[2][1], block->lights[2][1]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		}
		else
		{
			glColor3ub(block->lights[2][0], block->lights[2][0], block->lights[2][0]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[2][1], block->lights[2][1], block->lights[2][1]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[2][2], block->lights[2][2], block->lights[2][2]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[2][3], block->lights[2][3], block->lights[2][3]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[2][0], block->lights[2][0], block->lights[2][0]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[2][2], block->lights[2][2], block->lights[2][2]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
		}
	}
	if (block->visibleFace[3])
	{
		if (block->lights[3][1] + block->lights[3][3] > block->lights[3][0] + block->lights[3][2])
		{
			glColor3ub(block->lights[3][1], block->lights[3][1], block->lights[3][1]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[3][2], block->lights[3][2], block->lights[3][2]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[3][3], block->lights[3][3], block->lights[3][3]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[3][3], block->lights[3][3], block->lights[3][3]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[3][0], block->lights[3][0], block->lights[3][0]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[3][1], block->lights[3][1], block->lights[3][1]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		}
		else
		{
			glColor3ub(block->lights[3][0], block->lights[3][0], block->lights[3][0]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[3][1], block->lights[3][1], block->lights[3][1]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[3][2], block->lights[3][2], block->lights[3][2]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[3][3], block->lights[3][3], block->lights[3][3]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[3][0], block->lights[3][0], block->lights[3][0]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[3][2], block->lights[3][2], block->lights[3][2]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
		}
	}
	if (block->visibleFace[4])
	{
		if (block->lights[4][1] + block->lights[4][3] > block->lights[4][0] + block->lights[4][2])
		{
			glColor3ub(block->lights[4][1], block->lights[4][1], block->lights[4][1]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[4][2], block->lights[4][2], block->lights[4][2]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[4][3], block->lights[4][3], block->lights[4][3]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[4][3], block->lights[4][3], block->lights[4][3]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[4][0], block->lights[4][0], block->lights[4][0]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[4][1], block->lights[4][1], block->lights[4][1]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		}
		else
		{
			glColor3ub(block->lights[4][0], block->lights[4][0], block->lights[4][0]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[4][1], block->lights[4][1], block->lights[4][1]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[4][2], block->lights[4][2], block->lights[4][2]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[4][3], block->lights[4][3], block->lights[4][3]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[4][0], block->lights[4][0], block->lights[4][0]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[4][2], block->lights[4][2], block->lights[4][2]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		}
	}
	if (block->visibleFace[5])
	{
		if (block->lights[5][1] + block->lights[5][3] > block->lights[5][0] + block->lights[5][2])
		{
			glColor3ub(block->lights[5][1], block->lights[5][1], block->lights[5][1]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[5][2], block->lights[5][2], block->lights[5][2]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[5][3], block->lights[5][3], block->lights[5][3]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[5][3], block->lights[5][3], block->lights[5][3]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[5][0], block->lights[5][0], block->lights[5][0]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[5][1], block->lights[5][1], block->lights[5][1]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		}
		else
		{
			glColor3ub(block->lights[5][0], block->lights[5][0], block->lights[5][0]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[5][1], block->lights[5][1], block->lights[5][1]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[5][2], block->lights[5][2], block->lights[5][2]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor3ub(block->lights[5][3], block->lights[5][3], block->lights[5][3]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[5][0], block->lights[5][0], block->lights[5][0]);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor3ub(block->lights[5][2], block->lights[5][2], block->lights[5][2]);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		}
	}
}
