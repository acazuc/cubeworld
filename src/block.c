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
	block->transparent = type == 0;
}

void block_free(t_block *block)
{
	free(block);
}

static bool _block_calculate_visiblity_other_chunk(t_block *block, t_chunk *chunk, int32_t newBlockX, int32_t newBlockZ)
{
	if (!chunk)
		return (1);
	t_block *tmp_block = chunk_block_get(chunk, newBlockX, block->y, newBlockZ);
	return (!tmp_block || tmp_block->transparent);
}

void block_calculate_visibility(t_block *block)
{
	t_block *tmp_block;
	if (block->x == 0)
		block->visibleFace[2] = _block_calculate_visiblity_other_chunk(block, block->chunk->chunkXLess, 15, block->z);
	else
	{
		tmp_block = chunk_block_get(block->chunk, block->cx - 1, block->y, block->cz);
		block->visibleFace[2] = !tmp_block || tmp_block->transparent;
	}
	if (block->x == block->chunk->x + CHUNK_WIDTH - 1)
		block->visibleFace[3] = _block_calculate_visiblity_other_chunk(block, block->chunk->chunkXMore, 0, block->z);
	else
	{
		tmp_block = chunk_block_get(block->chunk, block->cx + 1, block->y, block->cz);
		block->visibleFace[3] = !tmp_block || tmp_block->transparent;
	}
	if (block->y == 0)
		block->visibleFace[4] = 1;
	else
	{
		tmp_block = chunk_block_get(block->chunk, block->cx, block->y - 1, block->cz);
		block->visibleFace[4] = !tmp_block || tmp_block->transparent;
	}
	if (block->y == CHUNK_HEIGHT - 1)
		block->visibleFace[5] = 1;
	else
	{
		tmp_block = chunk_block_get(block->chunk, block->cx, block->y + 1, block->cz);
		block->visibleFace[5] = !tmp_block || tmp_block->transparent;
	}
	if (block->z == 0)
		block->visibleFace[0] = _block_calculate_visiblity_other_chunk(block, block->chunk->chunkZLess, block->x, 15);
	else
	{
		tmp_block = chunk_block_get(block->chunk, block->cx, block->y, block->cz - 1);
		block->visibleFace[0] = !tmp_block || tmp_block->transparent;
	}
	if (block->z == block->chunk->z + CHUNK_WIDTH - 1)
		block->visibleFace[1] = _block_calculate_visiblity_other_chunk(block, block->chunk->chunkZMore, block->x, 0);
	else
	{
		tmp_block = chunk_block_get(block->chunk, block->cx, block->y, block->cz + 1);
		block->visibleFace[1] = !tmp_block || tmp_block->transparent;
	}
}

void block_draw(t_block *block)
{
	if (block->transparent)
		return;
	//glBegin(GL_QUADS);
	if (block->visibleFace[0])
	{
		glColor3f(block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT);
		glTexCoord2f(0, 0);
		glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glColor3f((block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT);
		glTexCoord2f(0, 1);
		glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(1, 1);
		glColor3f((block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT);
		glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(1, 0);
		glColor3f(block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT);
		glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
	}
	if (block->visibleFace[1])
	{
		//glColor3f(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
		glTexCoord2f(1, 0);
		glColor3f(block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT);
		glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
		glTexCoord2f(1, 1);
		glColor3f((block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT);
		glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
		glTexCoord2f(0, 1);
		glColor3f((block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT);
		glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
		glTexCoord2f(0, 0);
		glColor3f(block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT);
		glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
	}
	if (block->visibleFace[2])
	{
		glTexCoord2f(1, 0);
		glColor3f(block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT);
		glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(1, 1);
		glColor3f((block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT);
		glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(0, 1);
		glColor3f((block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT);
		glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
		glTexCoord2f(0, 0);
		glColor3f(block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT);
		glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
	}
	if (block->visibleFace[3])
	{
		glTexCoord2f(0, 0);
		glColor3f(block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT);
		glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(0, 1);
		glColor3f((block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT);
		glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(1, 1);
		glColor3f((block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT);
		glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
		glTexCoord2f(1, 0);
		glColor3f(block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT);
		glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
	}
	if (block->visibleFace[4])
	{
		glTexCoord2f(0, 0);
		glColor3f(block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT);
		glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
		glTexCoord2f(0, 1);
		glColor3f(block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT);
		glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(1, 1);
		glColor3f(block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT);
		glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(1, 0);
		glColor3f(block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT, block->y / (float)CHUNK_HEIGHT);
		glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
	}
	if (block->visibleFace[5])
	{
		glTexCoord2f(0, 0);
		glColor3f((block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT);
		glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
		glTexCoord2f(0, 1);
		glColor3f((block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT);
		glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(1, 1);
		glColor3f((block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT);
		glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
		glTexCoord2f(1, 0);
		glColor3f((block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT, (block->y + 1) / (float)CHUNK_HEIGHT);
		glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
	}
	//glEnd();
}
