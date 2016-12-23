#include "cubeworld.h"

void frustum_update(t_world *world)
{
	float proj[16];
	float modl[16];
	float clip[16];
	float t;

	glGetFloatv(GL_PROJECTION_MATRIX, proj);
	glGetFloatv(GL_MODELVIEW_MATRIX, modl);

	clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
	clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
	clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
	clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];
	clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
	clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
	clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
	clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];
	clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
	clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
	clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
	clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];
	clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
	clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
	clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
	clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];
	// Right
	world->frustum[0][0] = clip[3] - clip[0];
	world->frustum[0][1] = clip[7] - clip[4];
	world->frustum[0][2] = clip[11] - clip[8];
	world->frustum[0][3] = clip[15] - clip[12];
	t = sqrt(world->frustum[0][0] * world->frustum[0][0] + world->frustum[0][1] * world->frustum[0][1] + world->frustum[0][2] * world->frustum[0][2]);
	world->frustum[0][0] /= t;
	world->frustum[0][1] /= t;
	world->frustum[0][2] /= t;
	world->frustum[0][3] /= t;
	//Left
	world->frustum[1][0] = clip[3] + clip[0];
	world->frustum[1][1] = clip[7] + clip[4];
	world->frustum[1][2] = clip[11] + clip[8];
	world->frustum[1][3] = clip[15] + clip[12];
	t = sqrt(world->frustum[1][0] * world->frustum[1][0] + world->frustum[1][1] * world->frustum[1][1] + world->frustum[1][2] * world->frustum[1][2]);
	world->frustum[1][0] /= t;
	world->frustum[1][1] /= t;
	world->frustum[1][2] /= t;
	world->frustum[1][3] /= t;
	//Bottom
	world->frustum[2][0] = clip[3] + clip[1];
	world->frustum[2][1] = clip[7] + clip[5];
	world->frustum[2][2] = clip[11] + clip[9];
	world->frustum[2][3] = clip[15] + clip[13];
	t = sqrt(world->frustum[2][0] * world->frustum[2][0] + world->frustum[2][1] * world->frustum[2][1] + world->frustum[2][2] * world->frustum[2][2]);
	world->frustum[2][0] /= t;
	world->frustum[2][1] /= t;
	world->frustum[2][2] /= t;
	world->frustum[2][3] /= t;
	//Top
	world->frustum[3][0] = clip[3] - clip[1];
	world->frustum[3][1] = clip[7] - clip[5];
	world->frustum[3][2] = clip[11] - clip[ 9];
	world->frustum[3][3] = clip[15] - clip[13];
	t = sqrt(world->frustum[3][0] * world->frustum[3][0] + world->frustum[3][1] * world->frustum[3][1] + world->frustum[3][2] * world->frustum[3][2]);
	world->frustum[3][0] /= t;
	world->frustum[3][1] /= t;
	world->frustum[3][2] /= t;
	world->frustum[3][3] /= t;
	//Back
	world->frustum[4][0] = clip[3] - clip[2];
	world->frustum[4][1] = clip[7] - clip[6];
	world->frustum[4][2] = clip[11] - clip[10];
	world->frustum[4][3] = clip[15] - clip[14];
	t = sqrt(world->frustum[4][0] * world->frustum[4][0] + world->frustum[4][1] * world->frustum[4][1] + world->frustum[4][2] * world->frustum[4][2]);
	world->frustum[4][0] /= t;
	world->frustum[4][1] /= t;
	world->frustum[4][2] /= t;
	world->frustum[4][3] /= t;
	//Front
	world->frustum[5][0] = clip[3] + clip[2];
	world->frustum[5][1] = clip[7] + clip[6];
	world->frustum[5][2] = clip[11] + clip[10];
	world->frustum[5][3] = clip[15] + clip[14];
	t = sqrt(world->frustum[5][0] * world->frustum[5][0] + world->frustum[5][1] * world->frustum[5][1] + world->frustum[5][2] * world->frustum[5][2]);
	world->frustum[5][0] /= t;
	world->frustum[5][1] /= t;
	world->frustum[5][2] /= t;
	world->frustum[5][3] /= t;
}

bool frustum_cube(t_world *world, float x1, float y1, float z1, float x2, float y2, float z2)
{
	for (uint8_t i = 0; i < 6; ++i)
	{
		if (world->frustum[i][0] * x1 + world->frustum[i][1] * y1 + world->frustum[i][2] * z1 + world->frustum[i][3] > 0)
			continue;
		if (world->frustum[i][0] * x2 + world->frustum[i][1] * y1 + world->frustum[i][2] * z1 + world->frustum[i][3] > 0)
			continue;
		if (world->frustum[i][0] * x1 + world->frustum[i][1] * y2 + world->frustum[i][2] * z1 + world->frustum[i][3] > 0)
			continue;
		if (world->frustum[i][0] * x2 + world->frustum[i][1] * y2 + world->frustum[i][2] * z1 + world->frustum[i][3] > 0)
			continue;
		if (world->frustum[i][0] * x1 + world->frustum[i][1] * y1 + world->frustum[i][2] * z2 + world->frustum[i][3] > 0)
			continue;
		if (world->frustum[i][0] * x2 + world->frustum[i][1] * y1 + world->frustum[i][2] * z2 + world->frustum[i][3] > 0)
			continue;
		if (world->frustum[i][0] * x1 + world->frustum[i][1] * y2 + world->frustum[i][2] * z2 + world->frustum[i][3] > 0)
			continue;
		if (world->frustum[i][0] * x2 + world->frustum[i][1] * y2 + world->frustum[i][2] * z2 + world->frustum[i][3] > 0)
			continue;
		return (false);
	}
	return (true);
}
