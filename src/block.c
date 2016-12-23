#include "cubeworld.h"

#define BLOCK_WIDTH (.5)

static uint32_t colors[] = {0
	, (44 << 24) | (130 << 16) | (201 << 8) | 255
	, (247 << 24) | (218 << 16) | (100 << 8) | 255
	, (65 << 24) | (168 << 16) | (95 << 8) | 255
	, (153 << 24) | (51 << 16) | (255 << 8) | 255};

void block_init(t_block *block, t_chunk *chunk, int32_t x, int32_t y, int32_t z, uint8_t type)
{
	memset(block, 0, sizeof(*block));
	block->chunk = chunk;
	(void)x;
	(void)y;
	(void)z;
	block->type = type;
	/*
	if (block->type == 1)
	{
		block->red = 44;
		block->green = 130;
		block->blue = 201;
		block->alpha = 255;
	}
	else if (block->type == 2)
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
	if (block->type == 0)
		block->alpha = 0;
	*/
	block->visibleFace = (colors[block->type] & 0xff) != 0 ? 0xff : 0;
}

void block_free(t_block *block)
{
	free(block);
}

static bool _block_calculate_is_transparent_same_chunk(t_block *block, t_vec3i *pos, int8_t addX, int8_t addY, int8_t addZ)
{
	t_block *tmp_block = chunk_block_get(block->chunk, pos->x - block->chunk->x + addX, pos->y + addY, pos->z - block->chunk->z + addZ);
	return (!tmp_block || (colors[tmp_block->type] & 0xff) != 255);
}

static bool _block_calculate_is_transparent_other_chunk(t_chunk *chunk, int32_t newBlockX, int32_t newBlockY, int32_t newBlockZ)
{
	if (!chunk)
		return (1);
	t_block *tmp_block = chunk_block_get(chunk, newBlockX, newBlockY, newBlockZ);
	return (!tmp_block || (colors[tmp_block->type] & 0xff) != 255);
}

static bool _block_calculate_is_transparent(t_block *block, t_vec3i *pos, int8_t addX, int8_t addY, int8_t addZ)
{
	if (pos->y + addY < 0 || pos->y + addY >= CHUNK_HEIGHT)
		return (1);
	bool different = (addX < 0 && pos->x - block->chunk->x < -addX)
		|| (addX > 0 && pos->x - block->chunk->x + addX >= CHUNK_WIDTH)
		|| (addZ < 0 && pos->z - block->chunk->z < -addZ)
		|| (addZ > 0 && pos->z - block->chunk->z + addZ >= CHUNK_WIDTH);
	if (different)
	{
		int32_t newX = pos->x - block->chunk->x + addX;
		int32_t newY = pos->y + addY;
		int32_t newZ = pos->z - block->chunk->z + addZ;
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
	return (_block_calculate_is_transparent_same_chunk(block, pos, addX, addY, addZ));
}

void block_calculate_visibility(t_block *block, t_vec3i *pos)
{
	if (!(colors[block->type] & 0xff))
		return;
	block->visibleFace = 0;
	if (_block_calculate_is_transparent(block, pos, 0, 0, -1))
		block->visibleFace |= BLOCK_FACE_FRONT;
	if (_block_calculate_is_transparent(block, pos, 0, 0, 1))
		block->visibleFace |= BLOCK_FACE_BACK;
	if (_block_calculate_is_transparent(block, pos, -1, 0, 0))
		block->visibleFace |= BLOCK_FACE_LEFT;
	if (_block_calculate_is_transparent(block, pos, 1, 0, 0))
		block->visibleFace |= BLOCK_FACE_RIGHT;
	if (_block_calculate_is_transparent(block, pos, 0, -1, 0))
		block->visibleFace |= BLOCK_FACE_BOTTOM;
	if (_block_calculate_is_transparent(block, pos, 0, 1, 0))
		block->visibleFace |= BLOCK_FACE_TOP;
}

void _block_calculate_ambient_occlusion_light(t_block *block, t_vec3i *pos)
{
	if (!(colors[block->type] & 0xff))
		return;
	memset(&block->lights, 0xff, sizeof(block->lights));
	#define LESS 1
	if (block->visibleFace & BLOCK_FACE_FRONT)
	{
		if (pos->z - block->chunk->z > 0)
		{
			if (!_block_calculate_is_transparent(block, pos, 1, -1, -1))
				block->lights.f1p4 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, 0, -1, -1))
			{
				block->lights.f1p4 -= LESS;
				block->lights.f1p1 -= LESS;
			}
			if (!_block_calculate_is_transparent(block, pos, -1, -1, -1))
				block->lights.f1p1 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, -1, 1, -1))
				block->lights.f1p2 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, 0, 1, -1))
			{
				block->lights.f1p2 -= LESS;
				block->lights.f1p3 -= LESS;
			}
			if (!_block_calculate_is_transparent(block, pos, 1, 1, -1))
				block->lights.f1p3 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, 1, 0, -1))
			{
				block->lights.f1p3 -= LESS;
				block->lights.f1p4 -= LESS;
			}
			if (!_block_calculate_is_transparent(block, pos, -1, 0, -1))
			{
				block->lights.f1p1 -= LESS;
				block->lights.f1p2 -= LESS;
			}
		}
	}
	if (block->visibleFace & BLOCK_FACE_BACK)
	{
		if (pos->z - block->chunk->z < CHUNK_WIDTH - 1)
		{
			if (!_block_calculate_is_transparent(block, pos, 1, -1, 1))
				block->lights.f2p4 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, 0, -1, 1))
			{
				block->lights.f2p4 -= LESS;
				block->lights.f2p1 -= LESS;
			}
			if (!_block_calculate_is_transparent(block, pos, -1, -1, 1))
				block->lights.f2p1 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, -1, 1, 1))
				block->lights.f2p2 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, 0, 1, 1))
			{
				block->lights.f2p2 -= LESS;
				block->lights.f2p3 -= LESS;
			}
			if (!_block_calculate_is_transparent(block, pos, 1, 1, 1))
				block->lights.f2p3 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, 1, 0, 1))
			{
				block->lights.f2p3 -= LESS;
				block->lights.f2p4 -= LESS;
			}
			if (!_block_calculate_is_transparent(block, pos, -1, 0, 1))
			{
				block->lights.f2p1 -= LESS;
				block->lights.f2p2 -= LESS;
			}
		}
	}
	if (block->visibleFace & BLOCK_FACE_LEFT)
	{
		if (pos->x - block->chunk->x > 0)
		{
			if (!_block_calculate_is_transparent(block, pos, -1, -1, 1))
				block->lights.f3p4 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, -1, -1, 0))
			{
				block->lights.f3p4 -= LESS;
				block->lights.f3p1 -= LESS;
			}
			if (!_block_calculate_is_transparent(block, pos, -1, -1, -1))
				block->lights.f3p1 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, -1, 1, -1))
				block->lights.f3p2 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, -1, 1, 0))
			{
				block->lights.f3p2 -= LESS;
				block->lights.f3p3 -= LESS;
			}
			if (!_block_calculate_is_transparent(block, pos, -1, 1, 1))
				block->lights.f3p3 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, -1, 0, 1))
			{
				block->lights.f3p3 -= LESS;
				block->lights.f3p4 -= LESS;
			}

			if (!_block_calculate_is_transparent(block, pos, -1, 0, -1))
			{
				block->lights.f3p1 -= LESS;
				block->lights.f3p2 -= LESS;
			}
		}
	}
	if (block->visibleFace & BLOCK_FACE_RIGHT)
	{
		if (pos->x - block->chunk->x < CHUNK_WIDTH - 1)
		{
			if (!_block_calculate_is_transparent(block, pos, 1, -1, 1))
				block->lights.f4p4 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, 1, -1, 0))
			{
				block->lights.f4p4 -= LESS;
				block->lights.f4p1 -= LESS;
			}
			if (!_block_calculate_is_transparent(block, pos, 1, -1, -1))
				block->lights.f4p1 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, 1, 1, -1))
				block->lights.f4p2 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, 1, 1, 0))
			{
				block->lights.f4p2 -= LESS;
				block->lights.f4p3 -= LESS;
			}
			if (!_block_calculate_is_transparent(block, pos, 1, 1, 1))
				block->lights.f4p3 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, 1, 0, 1))
			{
				block->lights.f4p3 -= LESS;
				block->lights.f4p4 -= LESS;
			}

			if (!_block_calculate_is_transparent(block, pos, 1, 0, -1))
			{
				block->lights.f4p1 -= LESS;
				block->lights.f4p2 -= LESS;
			}
		}
	}
	if (block->visibleFace & BLOCK_FACE_BOTTOM)
	{
		if (pos->y > 0)
		{
			if (!_block_calculate_is_transparent(block, pos, -1, -1, -1))
				block->lights.f5p2 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, -1, -1, 0))
			{
				block->lights.f5p1 -= LESS;
				block->lights.f5p2 -= LESS;
			}
			if (!_block_calculate_is_transparent(block, pos, -1, -1, 1))
				block->lights.f5p1 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, 1, -1, -1))
				block->lights.f5p3 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, 1, -1, 0))
			{
				block->lights.f5p3 -= LESS;
				block->lights.f5p4 -= LESS;
			}
			if (!_block_calculate_is_transparent(block, pos, 1, -1, 1))
				block->lights.f5p4 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, 0, -1, -1))
			{
				block->lights.f5p2 -= LESS;
				block->lights.f5p3 -= LESS;
			}
			if (!_block_calculate_is_transparent(block, pos, 0, -1, 1))
			{
				block->lights.f5p4 -= LESS;
				block->lights.f5p1 -= LESS;
			}
		}
	}
	if (block->visibleFace & BLOCK_FACE_TOP)
	{
		if (pos->y < CHUNK_HEIGHT - 1)
		{
			if (!_block_calculate_is_transparent(block, pos, -1, 1, 1))
				block->lights.f6p1 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, -1, 1, 0))
			{
				block->lights.f6p1 -= LESS;
				block->lights.f6p2 -= LESS;
			}
			if (!_block_calculate_is_transparent(block, pos, -1, 1, -1))
				block->lights.f6p2 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, 1, 1, -1))
				block->lights.f6p3 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, 1, 1, 0))
			{
				block->lights.f6p3 -= LESS;
				block->lights.f6p4 -= LESS;
			}
			if (!_block_calculate_is_transparent(block, pos, 1, 1, 1))
				block->lights.f6p4 -= LESS;
			if (!_block_calculate_is_transparent(block, pos, 0, 1, -1))
			{
				block->lights.f6p2 -= LESS;
				block->lights.f6p3 -= LESS;
			}
			if (!_block_calculate_is_transparent(block, pos, 0, 1, 1))
			{
				block->lights.f6p4 -= LESS;
				block->lights.f6p1 -= LESS;
			}
		}
	}
}

void block_calculate_light(t_block *block, t_vec3i *pos)
{
	_block_calculate_ambient_occlusion_light(block, pos);
}

static void _add_point(t_block *block, t_vec3i *pos, uint8_t light, float x, float y, float z)
{
	double color = light / 16.;
	block->chunk->vao_colors[block->chunk->vao_colors_pos++] = color * ((colors[block->type] >> 24) & 0xff);
	block->chunk->vao_colors[block->chunk->vao_colors_pos++] = color * ((colors[block->type] >> 16) & 0xff);
	block->chunk->vao_colors[block->chunk->vao_colors_pos++] = color * ((colors[block->type] >> 8) & 0xff);
	block->chunk->vao_colors[block->chunk->vao_colors_pos++] = colors[block->type] & 0xff;
	block->chunk->vao_vertex[block->chunk->vao_vertex_pos++] = pos->x + x;
	block->chunk->vao_vertex[block->chunk->vao_vertex_pos++] = pos->y + y;
	block->chunk->vao_vertex[block->chunk->vao_vertex_pos++] = pos->z + z;
}

void block_draw(t_block *block, t_vec3i *pos)
{
	if (!(colors[block->type] & 0xff))
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
		if (block->lights.f1p2 + block->lights.f1p4 > block->lights.f1p1 + block->lights.f1p3)
		{
			///*
			_add_point(block, pos, block->lights.f1p2, -BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f1p3, +BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f1p4, +BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f1p4, +BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f1p1, -BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f1p2, -BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
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
			_add_point(block, pos, block->lights.f1p1, -BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f1p2, -BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f1p3, +BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f1p4, +BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f1p1, -BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f1p3, +BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
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
		if (block->lights.f2p2 + block->lights.f2p4 > block->lights.f2p1 + block->lights.f2p3)
		{
			///*
			_add_point(block, pos, block->lights.f2p2, -BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f2p3, +BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f2p4, +BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f2p4, +BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f2p1, -BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f2p2, -BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
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
			_add_point(block, pos, block->lights.f2p1, -BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f2p2, -BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f2p3, +BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f2p4, +BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f2p1, -BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f2p3, +BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
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
		if (block->lights.f3p2 + block->lights.f3p4 > block->lights.f3p1 + block->lights.f3p3)
		{
			///*
			_add_point(block, pos, block->lights.f3p2, -BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f3p3, -BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f3p4, -BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f3p4, -BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f3p1, -BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f3p2, -BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
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
			_add_point(block, pos, block->lights.f3p1, -BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f3p2, -BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f3p3, -BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f3p4, -BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f3p1, -BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f3p4, -BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
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
		if (block->lights.f4p2 + block->lights.f4p4 > block->lights.f4p1 + block->lights.f4p3)
		{
			///*
			_add_point(block, pos, block->lights.f4p2, +BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f4p3, +BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f4p4, +BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f4p4, +BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f4p1, +BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f4p2, +BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
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
			_add_point(block, pos, block->lights.f4p1, +BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f4p2, +BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f4p3, +BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f4p4, +BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f4p1, +BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f4p3, +BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
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
		if (block->lights.f5p2 + block->lights.f5p4 > block->lights.f5p1 + block->lights.f5p3)
		{
			///*
			_add_point(block, pos, block->lights.f5p2, -BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f5p3, +BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f5p4, +BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f5p4, +BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f5p1, -BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f5p2, -BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
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
			_add_point(block, pos, block->lights.f5p1, -BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f5p2, -BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f5p3, +BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f5p4, +BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f5p1, -BLOCK_WIDTH, -BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f5p3, +BLOCK_WIDTH, -BLOCK_WIDTH, -BLOCK_WIDTH);
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
		if (block->lights.f6p2 + block->lights.f6p4 > block->lights.f6p1 + block->lights.f6p3)
		{
			///*
			_add_point(block, pos, block->lights.f6p2, -BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f6p3, +BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f6p4, +BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f6p4, +BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f6p1, -BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f6p2, -BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
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
			_add_point(block, pos, block->lights.f6p1, -BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f6p2, -BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f6p3, +BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f6p4, +BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f6p1, -BLOCK_WIDTH, +BLOCK_WIDTH, +BLOCK_WIDTH);
			_add_point(block, pos, block->lights.f6p3, +BLOCK_WIDTH, +BLOCK_WIDTH, -BLOCK_WIDTH);
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
