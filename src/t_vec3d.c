#include "cubeworld.h"

void vec3d_rotate_x(t_vec3d *vector, double angle)
{
	double		temp_y;
	double		temp_z;

	temp_y = vector->y;
	temp_z = vector->z;
	vector->y = temp_y * cos(angle) - temp_z * sin(angle);
	vector->z = temp_y * sin(angle) + temp_z * cos(angle);
}

void vec3d_rotate_y(t_vec3d *vector, double angle)
{
	double		temp_x;
	double		temp_z;

	temp_x = vector->x;
	temp_z = vector->z;
	vector->z = temp_z * cos(angle) - temp_x * sin(angle);
	vector->x = temp_z * sin(angle) + temp_x * cos(angle);
}

void vec3d_rotate_z(t_vec3d *vector, double angle)
{
	double		temp_x;
	double		temp_y;

	temp_x = vector->x;
	temp_y = vector->y;
	vector->x = temp_x * cos(angle) - temp_y * sin(angle);
	vector->y = temp_x * sin(angle) + temp_y * cos(angle);
}

void vec3d_rotate(t_vec3d *vector, t_vec3d *rotation)
{
	vec3d_rotate_x(vector, TO_RADIANS(rotation->x));
	vec3d_rotate_y(vector, TO_RADIANS(rotation->y));
	vec3d_rotate_z(vector, TO_RADIANS(rotation->z));
}

void vec3d_unrotate(t_vec3d *vector, t_vec3d *rotation)
{
	vec3d_rotate_z(vector, -(TO_RADIANS(rotation->z)));
	vec3d_rotate_y(vector, -(TO_RADIANS(rotation->y)));
	vec3d_rotate_x(vector, -(TO_RADIANS(rotation->x)));
}

double vec3d_angle(t_vec3d *v1, t_vec3d *v2)
{
	double	angle;

	angle = acos(vec3d_dot(v1, v2) / (vec3d_size(v1) * vec3d_size(v2)));
	return (angle);
}

double vec3d_dot(t_vec3d *v1, t_vec3d *v2)
{
	return (v1->x * v2->x + v1->y * v2->y + v1->z * v2->z);
}

double vec3d_size(t_vec3d *vector)
{
	double	dx;
	double	dy;
	double	dz;

	dx = vector->x * vector->x;
	dy = vector->y * vector->y;
	dz = vector->z * vector->z;
	return (sqrt(dx + dy + dz));
}
