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
	if (block->y < CHUNK_HEIGHT / 3)
	{
		block->red = 44;
		block->green = 130;
		block->blue = 201;
		block->alpha = 255;
	}
	else if (block->y == CHUNK_HEIGHT / 3)
	{
		block->red = 247;
		block->green = 218;
		block->blue = 100;
		block->alpha = 255;
	}
	else
	{
		block->red = 65;
		block->green = 168;
		block->blue = 95;
		block->alpha = 255;
	}
	if (type == 0)
		block->alpha = 0;
	block->visibleFace = block->alpha != 0 ? 0xff : 0;
}

void block_free(t_block *block)
{
	free(block);
}

static bool _block_calculate_is_transparent_same_chunk(t_block *block, int32_t addX, int32_t addY, int32_t addZ)
{
	t_block *tmp_block = chunk_block_get(block->chunk, block->cx + addX, block->y + addY, block->cz + addZ);
	return (!tmp_block || tmp_block->alpha != 255);
}

static bool _block_calculate_is_transparent_other_chunk(t_chunk *chunk, int32_t newBlockX, int32_t newBlockY, int32_t newBlockZ)
{
	if (!chunk)
		return (1);
	t_block *tmp_block = chunk_block_get(chunk, newBlockX, newBlockY, newBlockZ);
	return (!tmp_block || tmp_block->alpha != 255);
}

static bool _block_calculate_is_transparent(t_block *block, int32_t addX, int32_t addY, int32_t addZ)
{
	if (block->y + addY < 0 || block->y + addY >= CHUNK_HEIGHT)
		return (1);
	bool different = (addX < 0 && (int32_t)block->cx < -addX)
		|| (addX > 0 && (int32_t)block->cx + addX >= CHUNK_WIDTH)
		|| (addZ < 0 && (int32_t)block->cz < -addZ)
		|| (addZ > 0 && (int32_t)block->cz + addZ >= CHUNK_WIDTH);
	if (different)
	{
		int32_t newX = (int32_t)block->cx + addX;
		int32_t newY = block->y + addY;
		int32_t newZ = (int32_t)block->cz + addZ;
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
	if (!block->alpha)
		return;
	block->visibleFace = 0;
	if (_block_calculate_is_transparent(block, 0, 0, -1))
		block->visibleFace |= BLOCK_FACE_FRONT;
	if (_block_calculate_is_transparent(block, 0, 0, 1))
		block->visibleFace |= BLOCK_FACE_BACK;
	if (_block_calculate_is_transparent(block, -1, 0, 0))
		block->visibleFace |= BLOCK_FACE_LEFT;
	if (_block_calculate_is_transparent(block, 1, 0, 0))
		block->visibleFace |= BLOCK_FACE_RIGHT;
	if (_block_calculate_is_transparent(block, 0, -1, 0))
		block->visibleFace |= BLOCK_FACE_BOTTOM;
	if (_block_calculate_is_transparent(block, 0, 1, 0))
		block->visibleFace |= BLOCK_FACE_TOP;
}

void _block_calculate_ambient_occlusion_light(t_block *block)
{
	if (!block->alpha)
		return;
	for (uint8_t face = 0; face < 6; ++face)
		for (uint8_t point = 0; point < 4; ++point)
			block->lights[face][point] = 255;
	#define LESS 16
	if (block->visibleFace & BLOCK_FACE_FRONT)
	{
		if (block->cz > 0)
		{
			if (!_block_calculate_is_transparent(block, 1, -1, -1))
				block->lights[0][3] -= LESS;
			if (!_block_calculate_is_transparent(block, 0, -1, -1))
			{
				block->lights[0][3] -= LESS;
				block->lights[0][0] -= LESS;
			}
			if (!_block_calculate_is_transparent(block, -1, -1, -1))
				block->lights[0][0] -= LESS;
			if (!_block_calculate_is_transparent(block, -1, 1, -1))
				block->lights[0][1] -= LESS;
			if (!_block_calculate_is_transparent(block, 0, 1, -1))
			{
				block->lights[0][1] -= LESS;
				block->lights[0][2] -= LESS;
			}
			if (!_block_calculate_is_transparent(block, 1, 1, -1))
				block->lights[0][2] -= LESS;
			if (!_block_calculate_is_transparent(block, 1, 0, -1))
			{
				block->lights[0][2] -= LESS;
				block->lights[0][3] -= LESS;
			}
			if (!_block_calculate_is_transparent(block, -1, 0, -1))
			{
				block->lights[0][0] -= LESS;
				block->lights[0][1] -= LESS;
			}
		}
	}
	if (block->visibleFace & BLOCK_FACE_BACK)
	{
		if (block->cz < CHUNK_WIDTH - 1)
		{
			if (!_block_calculate_is_transparent(block, 1, -1, 1))
				block->lights[1][3] -= LESS;
			if (!_block_calculate_is_transparent(block, 0, -1, 1))
			{
				block->lights[1][3] -= LESS;
				block->lights[1][0] -= LESS;
			}
			if (!_block_calculate_is_transparent(block, -1, -1, 1))
				block->lights[1][0] -= LESS;
			if (!_block_calculate_is_transparent(block, -1, 1, 1))
				block->lights[1][1] -= LESS;
			if (!_block_calculate_is_transparent(block, 0, 1, 1))
			{
				block->lights[1][1] -= LESS;
				block->lights[1][2] -= LESS;
			}
			if (!_block_calculate_is_transparent(block, 1, 1, 1))
				block->lights[1][2] -= LESS;
			if (!_block_calculate_is_transparent(block, 1, 0, 1))
			{
				block->lights[1][2] -= LESS;
				block->lights[1][3] -= LESS;
			}
			if (!_block_calculate_is_transparent(block, -1, 0, 1))
			{
				block->lights[1][0] -= LESS;
				block->lights[1][1] -= LESS;
			}
		}
	}
	if (block->visibleFace & BLOCK_FACE_LEFT)
	{
		if (block->cx > 0)
		{
			if (!_block_calculate_is_transparent(block, -1, -1, 1))
				block->lights[2][3] -= LESS;
			if (!_block_calculate_is_transparent(block, -1, -1, 0))
			{
				block->lights[2][3] -= LESS;
				block->lights[2][0] -= LESS;
			}
			if (!_block_calculate_is_transparent(block, -1, -1, -1))
				block->lights[2][0] -= LESS;
			if (!_block_calculate_is_transparent(block, -1, 1, -1))
				block->lights[2][1] -= LESS;
			if (!_block_calculate_is_transparent(block, -1, 1, 0))
			{
				block->lights[2][1] -= LESS;
				block->lights[2][2] -= LESS;
			}
			if (!_block_calculate_is_transparent(block, -1, 1, 1))
				block->lights[2][2] -= LESS;
			if (!_block_calculate_is_transparent(block, -1, 0, 1))
			{
				block->lights[2][2] -= LESS;
				block->lights[2][3] -= LESS;
			}

			if (!_block_calculate_is_transparent(block, -1, 0, -1))
			{
				block->lights[2][0] -= LESS;
				block->lights[2][1] -= LESS;
			}
		}
	}
	if (block->visibleFace & BLOCK_FACE_RIGHT)
	{
		if (block->cx < CHUNK_WIDTH - 1)
		{
			if (!_block_calculate_is_transparent(block, 1, -1, 1))
				block->lights[3][3] -= LESS;
			if (!_block_calculate_is_transparent(block, 1, -1, 0))
			{
				block->lights[3][3] -= LESS;
				block->lights[3][0] -= LESS;
			}
			if (!_block_calculate_is_transparent(block, 1, -1, -1))
				block->lights[3][0] -= LESS;
			if (!_block_calculate_is_transparent(block, 1, 1, -1))
				block->lights[3][1] -= LESS;
			if (!_block_calculate_is_transparent(block, 1, 1, 0))
			{
				block->lights[3][1] -= LESS;
				block->lights[3][2] -= LESS;
			}
			if (!_block_calculate_is_transparent(block, 1, 1, 1))
				block->lights[3][2] -= LESS;
			if (!_block_calculate_is_transparent(block, 1, 0, 1))
			{
				block->lights[3][2] -= LESS;
				block->lights[3][3] -= LESS;
			}

			if (!_block_calculate_is_transparent(block, 1, 0, -1))
			{
				block->lights[3][0] -= LESS;
				block->lights[3][1] -= LESS;
			}
		}
	}
	if (block->visibleFace & BLOCK_FACE_BOTTOM)
	{
		if (block->y > 0)
		{
			if (!_block_calculate_is_transparent(block, -1, -1, -1))
				block->lights[4][1] -= LESS;
			if (!_block_calculate_is_transparent(block, -1, -1, 0))
			{
				block->lights[4][0] -= LESS;
				block->lights[4][1] -= LESS;
			}
			if (!_block_calculate_is_transparent(block, -1, -1, 1))
				block->lights[4][0] -= LESS;
			if (!_block_calculate_is_transparent(block, 1, -1, -1))
				block->lights[4][2] -= LESS;
			if (!_block_calculate_is_transparent(block, 1, -1, 0))
			{
				block->lights[4][2] -= LESS;
				block->lights[4][3] -= LESS;
			}
			if (!_block_calculate_is_transparent(block, 1, -1, 1))
				block->lights[4][3] -= LESS;
			if (!_block_calculate_is_transparent(block, 0, -1, -1))
			{
				block->lights[4][1] -= LESS;
				block->lights[4][2] -= LESS;
			}
			if (!_block_calculate_is_transparent(block, 0, -1, 1))
			{
				block->lights[4][3] -= LESS;
				block->lights[4][0] -= LESS;
			}
		}
	}
	if (block->visibleFace & BLOCK_FACE_TOP)
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
		}
	}
}

void block_calculate_light(t_block *block)
{
	_block_calculate_ambient_occlusion_light(block);
}

static void _add_point(t_block *block, uint8_t light, float x, float y, float z)
{
	double color = light / 255.;
	block->chunk->vao_colors[block->chunk->vao_colors_pos++] = color * block->red;
	block->chunk->vao_colors[block->chunk->vao_colors_pos++] = color * block->green;
	block->chunk->vao_colors[block->chunk->vao_colors_pos++] = color * block->blue;
	block->chunk->vao_colors[block->chunk->vao_colors_pos++] = block->alpha;
	block->chunk->vao_vertex[block->chunk->vao_vertex_pos++] = block->x + x;
	block->chunk->vao_vertex[block->chunk->vao_vertex_pos++] = block->y + y;
	block->chunk->vao_vertex[block->chunk->vao_vertex_pos++] = block->z + z;
}

void block_draw(t_block *block)
{
	if (!block->alpha)
		return;
	///*
	uint8_t reallocSize = 0;
	if (block->visibleFace & BLOCK_FACE_FRONT)
		++reallocSize;
	if (block->visibleFace & BLOCK_FACE_BACK)
		++reallocSize;
	if (block->visibleFace & BLOCK_FACE_LEFT)
		++reallocSize;
	if (block->visibleFace & BLOCK_FACE_RIGHT)
		++reallocSize;
	if (block->visibleFace & BLOCK_FACE_BOTTOM)
		++reallocSize;
	if (block->visibleFace & BLOCK_FACE_TOP)
		++reallocSize;
	if (!reallocSize)
		return;
	block->chunk->vao_colors_size += reallocSize * 6 * 4; // r/g/b/a
	if (!(block->chunk->vao_colors = realloc(block->chunk->vao_colors, block->chunk->vao_colors_size)))
		ERROR("realloc failed");
	block->chunk->vao_vertex_size += reallocSize * 6 * 4 * 3; // x/y/z
	if (!(block->chunk->vao_vertex = realloc(block->chunk->vao_vertex, block->chunk->vao_vertex_size)))
		ERROR("realloc failed");
	//*/
	if (block->visibleFace & BLOCK_FACE_FRONT)
	{
		if (block->lights[0][1] + block->lights[0][3] > block->lights[0][0] + block->lights[0][2])
		{
			///*
			_add_point(block, block->lights[0][1], -BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[0][2], +BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[0][3], +BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[0][3], +BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[0][0], -BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[0][1], -BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			//*/
			/*glColor4ub(block->lights[0][1] / 255. * block->red, block->lights[0][1] / 255. * block->green, block->lights[0][1] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[0][2] / 255. * block->red, block->lights[0][2] / 255. * block->green, block->lights[0][2] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[0][3] / 255. * block->red, block->lights[0][3] / 255. * block->green, block->lights[0][3] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[0][3] / 255. * block->red, block->lights[0][3] / 255. * block->green, block->lights[0][3] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[0][0] / 255. * block->red, block->lights[0][0] / 255. * block->green, block->lights[0][0] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[0][1] / 255. * block->red, block->lights[0][1] / 255. * block->green, block->lights[0][1] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);*/
		}
		else
		{
			///*
			_add_point(block, block->lights[0][0], -BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[0][1], -BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[0][2], +BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[0][3], +BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[0][0], -BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[0][2], +BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			//*/
			/*glColor4ub(block->lights[0][0] / 255. * block->red, block->lights[0][0] / 255. * block->green, block->lights[0][0] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[0][1] / 255. * block->red, block->lights[0][1] / 255. * block->green, block->lights[0][1] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[0][2] / 255. * block->red, block->lights[0][2] / 255. * block->green, block->lights[0][2] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[0][3] / 255. * block->red, block->lights[0][3] / 255. * block->green, block->lights[0][3] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[0][0] / 255. * block->red, block->lights[0][0] / 255. * block->green, block->lights[0][0] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[0][2] / 255. * block->red, block->lights[0][2] / 255. * block->green, block->lights[0][2] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);*/
		}
	}
	if (block->visibleFace & BLOCK_FACE_BACK)
	{
		if (block->lights[1][1] + block->lights[1][3] > block->lights[1][0] + block->lights[1][2])
		{
			///*
			_add_point(block, block->lights[1][1], -BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[1][2], +BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[1][3], +BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[1][3], +BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[1][0], -BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[1][1], -BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			//*/
			/*glColor4ub(block->lights[1][1] / 255. * block->red, block->lights[1][1] / 255. * block->green, block->lights[1][1] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[1][2] / 255. * block->red, block->lights[1][2] / 255. * block->green, block->lights[1][2] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[1][3] / 255. * block->red, block->lights[1][3] / 255. * block->green, block->lights[1][3] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[1][3] / 255. * block->red, block->lights[1][3] / 255. * block->green, block->lights[1][3] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[1][0] / 255. * block->red, block->lights[1][0] / 255. * block->green, block->lights[1][0] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[1][1] / 255. * block->red, block->lights[1][1] / 255. * block->green, block->lights[1][1] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);*/
		}
		else
		{
			///*
			_add_point(block, block->lights[1][0], -BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[1][1], -BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[1][2], +BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[1][3], +BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[1][0], -BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[1][2], +BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			//*/
			/*glColor4ub(block->lights[1][0] / 255. * block->red, block->lights[1][0] / 255. * block->green, block->lights[1][0] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[1][1] / 255. * block->red, block->lights[1][1] / 255. * block->green, block->lights[1][1] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[1][2] / 255. * block->red, block->lights[1][2] / 255. * block->green, block->lights[1][2] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[1][3] / 255. * block->red, block->lights[1][3] / 255. * block->green, block->lights[1][3] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[1][0] / 255. * block->red, block->lights[1][0] / 255. * block->green, block->lights[1][0] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[1][2] / 255. * block->red, block->lights[1][2] / 255. * block->green, block->lights[1][2] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);*/
		}
	}
	if (block->visibleFace & BLOCK_FACE_LEFT)
	{
		if (block->lights[2][1] + block->lights[2][3] > block->lights[2][0] + block->lights[2][2])
		{
			///*
			_add_point(block, block->lights[2][1], -BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[2][2], -BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[2][3], -BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[2][3], -BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[2][0], -BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[2][1], -BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			//*/
			/*glColor4ub(block->lights[2][1] / 255. * block->red, block->lights[2][1] / 255. * block->green, block->lights[2][1] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[2][2] / 255. * block->red, block->lights[2][2] / 255. * block->green, block->lights[2][2] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[2][3] / 255. * block->red, block->lights[2][3] / 255. * block->green, block->lights[2][3] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[2][3] / 255. * block->red, block->lights[2][3] / 255. * block->green, block->lights[2][3] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[2][0] / 255. * block->red, block->lights[2][0] / 255. * block->green, block->lights[2][0] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[2][1] / 255. * block->red, block->lights[2][1] / 255. * block->green, block->lights[2][1] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);*/
		}
		else
		{
			///*
			_add_point(block, block->lights[2][0], -BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[2][1], -BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[2][2], -BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[2][3], -BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[2][0], -BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[2][2], -BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			//*/
			/*glColor4ub(block->lights[2][0] / 255. * block->red, block->lights[2][0] / 255. * block->green, block->lights[2][0] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[2][1] / 255. * block->red, block->lights[2][1] / 255. * block->green, block->lights[2][1] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[2][2] / 255. * block->red, block->lights[2][2] / 255. * block->green, block->lights[2][2] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[2][3] / 255. * block->red, block->lights[2][3] / 255. * block->green, block->lights[2][3] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[2][0] / 255. * block->red, block->lights[2][0] / 255. * block->green, block->lights[2][0] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[2][2] / 255. * block->red, block->lights[2][2] / 255. * block->green, block->lights[2][2] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);*/
		}
	}
	if (block->visibleFace & BLOCK_FACE_RIGHT)
	{
		if (block->lights[3][1] + block->lights[3][3] > block->lights[3][0] + block->lights[3][2])
		{
			///*
			_add_point(block, block->lights[3][1], +BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[3][2], +BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[3][3], +BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[3][3], +BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[3][0], +BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[3][1], +BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			//*/
			/*glColor4ub(block->lights[3][1] / 255. * block->red, block->lights[3][1] / 255. * block->green, block->lights[3][1] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[3][2] / 255. * block->red, block->lights[3][2] / 255. * block->green, block->lights[3][2] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[3][3] / 255. * block->red, block->lights[3][3] / 255. * block->green, block->lights[3][3] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[3][3] / 255. * block->red, block->lights[3][3] / 255. * block->green, block->lights[3][3] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[3][0] / 255. * block->red, block->lights[3][0] / 255. * block->green, block->lights[3][0] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[3][1] / 255. * block->red, block->lights[3][1] / 255. * block->green, block->lights[3][1] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);*/
		}
		else
		{
			///*
			_add_point(block, block->lights[3][0], +BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[3][1], +BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[3][2], +BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[3][3], +BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[3][0], +BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[3][2], +BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			//*/
			/*glColor4ub(block->lights[3][0] / 255. * block->red, block->lights[3][0] / 255. * block->green, block->lights[3][0] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[3][1] / 255. * block->red, block->lights[3][1] / 255. * block->green, block->lights[3][1] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[3][2] / 255. * block->red, block->lights[3][2] / 255. * block->green, block->lights[3][2] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[3][3] / 255. * block->red, block->lights[3][3] / 255. * block->green, block->lights[3][3] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[3][0] / 255. * block->red, block->lights[3][0] / 255. * block->green, block->lights[3][0] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[3][2] / 255. * block->red, block->lights[3][2] / 255. * block->green, block->lights[3][2] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);*/
		}
	}
	if (block->visibleFace & BLOCK_FACE_BOTTOM)
	{
		if (block->lights[4][1] + block->lights[4][3] > block->lights[4][0] + block->lights[4][2])
		{
			///*
			_add_point(block, block->lights[4][1], -BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[4][2], +BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[4][3], +BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[4][3], +BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[4][0], -BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[4][1], -BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			//*/
			/*glColor4ub(block->lights[4][1] / 255. * block->red, block->lights[4][1] / 255. * block->green, block->lights[4][1] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[4][2] / 255. * block->red, block->lights[4][2] / 255. * block->green, block->lights[4][2] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[4][3] / 255. * block->red, block->lights[4][3] / 255. * block->green, block->lights[4][3] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[4][3] / 255. * block->red, block->lights[4][3] / 255. * block->green, block->lights[4][3] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[4][0] / 255. * block->red, block->lights[4][0] / 255. * block->green, block->lights[4][0] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[4][1] / 255. * block->red, block->lights[4][1] / 255. * block->green, block->lights[4][1] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);*/
		}
		else
		{
			///*
			_add_point(block, block->lights[4][0], -BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[4][1], -BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[4][2], +BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[4][3], +BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[4][0], -BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[4][2], +BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			//*/
			/*glColor4ub(block->lights[4][0] / 255. * block->red, block->lights[4][0] / 255. * block->green, block->lights[4][0] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[4][1] / 255. * block->red, block->lights[4][1] / 255. * block->green, block->lights[4][1] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[4][2] / 255. * block->red, block->lights[4][2] / 255. * block->green, block->lights[4][2] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[4][3] / 255. * block->red, block->lights[4][3] / 255. * block->green, block->lights[4][3] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[4][0] / 255. * block->red, block->lights[4][0] / 255. * block->green, block->lights[4][0] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[4][2] / 255. * block->red, block->lights[4][2] / 255. * block->green, block->lights[4][2] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y - BLOCK_WIDTH, block->z - BLOCK_WIDTH);*/
		}
	}
	if (block->visibleFace & BLOCK_FACE_TOP)
	{
		if (block->lights[5][1] + block->lights[5][3] > block->lights[5][0] + block->lights[5][2])
		{
			///*
			_add_point(block, block->lights[5][1], -BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[5][2], +BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[5][3], +BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[5][3], +BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[5][0], -BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[5][1], -BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			//*/
			/*glColor4ub(block->lights[5][1] / 255. * block->red, block->lights[5][1] / 255. * block->green, block->lights[5][1] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[5][2] / 255. * block->red, block->lights[5][2] / 255. * block->green, block->lights[5][2] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[5][3] / 255. * block->red, block->lights[5][3] / 255. * block->green, block->lights[5][3] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[5][3] / 255. * block->red, block->lights[5][3] / 255. * block->green, block->lights[5][3] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[5][0] / 255. * block->red, block->lights[5][0] / 255. * block->green, block->lights[5][0] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[5][1] / 255. * block->red, block->lights[5][1] / 255. * block->green, block->lights[5][1] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);*/
		}
		else
		{
			///*
			_add_point(block, block->lights[5][0], -BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[5][1], -BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[5][2], +BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, block->lights[5][3], +BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[5][0], -BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, block->lights[5][2], +BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			//*/
			/*glColor4ub(block->lights[5][0] / 255. * block->red, block->lights[5][0] / 255. * block->green, block->lights[5][0] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[5][1] / 255. * block->red, block->lights[5][1] / 255. * block->green, block->lights[5][1] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[5][2] / 255. * block->red, block->lights[5][2] / 255. * block->green, block->lights[5][2] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);
			glColor4ub(block->lights[5][3] / 255. * block->red, block->lights[5][3] / 255. * block->green, block->lights[5][3] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[5][0] / 255. * block->red, block->lights[5][0] / 255. * block->green, block->lights[5][0] / 255. * block->blue, block->alpha);
			glVertex3f(block->x - BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z + BLOCK_WIDTH);
			glColor4ub(block->lights[5][2] / 255. * block->red, block->lights[5][2] / 255. * block->green, block->lights[5][2] / 255. * block->blue, block->alpha);
			glVertex3f(block->x + BLOCK_WIDTH, block->y + BLOCK_WIDTH, block->z - BLOCK_WIDTH);*/
		}
	}
}
