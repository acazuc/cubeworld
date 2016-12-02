#include "cubeworld.h"

#define BLOCK_WIDTH (.5)

void block_init(t_block *block, t_chunk *chunk, int32_t x, int32_t y, int32_t z, uint8_t type)
{
	memset(block, 0, sizeof(*block));
	block->chunk = chunk;
	block->x = x;
	block->y = y;
	block->z = z;
	block->type = type;
	for (uint8_t i = 0; i < 6; ++i)
		block->visibleFace[i] = 1;
}

void block_free(t_block *block)
{
	free(block);
}

static bool _block_calculate_visiblity_other_chunk(t_block *block, int32_t addX, int32_t addZ, int32_t newBlockX, int32_t newBlockZ)
{
	t_chunk *tmp_chunk = world_chunk_get(block->chunk->world, block->chunk->x + addX, block->chunk->z +addZ);
	if (!tmp_chunk)
		return (1);
	else
	{
		t_block *tmp_block = chunk_block_get(tmp_chunk, newBlockX, block->y, newBlockZ);
		return (!tmp_block || tmp_block->transparent);
	}
}

void block_calculate_visibility(t_block *block)
{
	t_block *tmp_block;
	if (block->x == 0)
		block->visibleFace[2] = _block_calculate_visiblity_other_chunk(block, -CHUNK_WIDTH, 0, 15, block->z);
	else
	{
		tmp_block = chunk_block_get(block->chunk, block->x - 1, block->y, block->z);
		block->visibleFace[2] = !tmp_block || tmp_block->transparent;
	}
	if (block->x == block->chunk->x + CHUNK_WIDTH - 1)
		block->visibleFace[3] = _block_calculate_visiblity_other_chunk(block, CHUNK_WIDTH, 0, 0, block->z);
	else
	{
		tmp_block = chunk_block_get(block->chunk, block->x + 1, block->y, block->z);
		block->visibleFace[3] = !tmp_block || tmp_block->transparent;
	}
	if (block->y == 0)
		block->visibleFace[4] = 1;
	else
	{
		tmp_block = chunk_block_get(block->chunk, block->x, block->y + 1, block->z);
		block->visibleFace[4] = !tmp_block || tmp_block->transparent;
	}
	if (block->y == CHUNK_HEIGHT - 1)
		block->visibleFace[5] = 1;
	else
	{
		tmp_block = chunk_block_get(block->chunk, block->x, block->y - 1, block->z);
		block->visibleFace[5] = !tmp_block || tmp_block->transparent;
	}
	if (block->z == 0)
		block->visibleFace[0] = _block_calculate_visiblity_other_chunk(block, 0, -CHUNK_WIDTH, block->x, 15);
	else
	{
		tmp_block = chunk_block_get(block->chunk, block->x, block->y, block->z - 1);
		block->visibleFace[0] = !tmp_block || tmp_block->transparent;
	}
	if (block->z == block->chunk->z + CHUNK_WIDTH - 1)
		block->visibleFace[1] = _block_calculate_visiblity_other_chunk(block, 0, CHUNK_WIDTH, block->x, 0);
	else
	{
		tmp_block = chunk_block_get(block->chunk, block->x, block->y, block->z + 1);
		block->visibleFace[1] = !tmp_block || tmp_block->transparent;
	}
}

void block_draw(t_block *block)
{
	glBegin(GL_QUADS);
	glColor3f(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
	if (block->visibleFace[0])
	{
		glTexCoord2f(0, 0);
		glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(0, 1);
		glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(1, 1);
		glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(1, 0);
		glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
	}
	glColor3f(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
	if (block->visibleFace[1])
	{
		glTexCoord2f(1, 0);
		glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
		glTexCoord2f(1, 1);
		glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
		glTexCoord2f(0, 1);
		glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
		glTexCoord2f(0, 0);
		glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
	}
	glColor3f(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
	if (block->visibleFace[2])
	{
		glTexCoord2f(1, 0);
		glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(1, 1);
		glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(0, 1);
		glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
		glTexCoord2f(0, 0);
		glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
	}
	glColor3f(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
	if (block->visibleFace[3])
	{
		glTexCoord2f(0, 0);
		glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(0, 1);
		glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(1, 1);
		glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
		glTexCoord2f(1, 0);
		glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
	}
	glColor3f(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
	if (block->visibleFace[4])
	{
		glTexCoord2f(0, 0);
		glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
		glTexCoord2f(0, 1);
		glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(1, 1);
		glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(1, 0);
		glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
	}
	glColor3f(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
	if (block->visibleFace[5])
	{
		glTexCoord2f(0, 0);
		glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
		glTexCoord2f(0, 1);
		glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(1, 1);
		glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(1, 0);
		glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
	}
	glEnd();
}
