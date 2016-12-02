#include "cubeworld.h"

#define NUMBEROFSWAPS 400

static t_simplex_noise_grad grads[] = {{1, 1, 0}, {-1, 1, 0}, {1, -1, 0}, {-1, -1, 0}, {1, 0, 1}, {-1, 0, 1}, {1, 0, -1}
	, {1, 0, -1}, {0, 1, 1}, {0, -1, 1}, {0, 1, -1}, {0, -1, -1}};

static short p_supply[] = {151,160,137,91,90,15,
			131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
			190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
			88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
			77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
			102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
			135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
			5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
			223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
			129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
			251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
			49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
			138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180};

static double F2 = 0.36602540378443864676;//0.5 * (sqrt(3.0) - 1.0);
static double G2 = 0.21132486540518711775;//(3.0 - sqrt(3.0)) / 6.0;
static double F3 = 0.33333333333333333333;//1.0 / 3.0;
static double G3 = 0.16666666666666666667;//1.0 / 6.0;

void _simplex_noise_octave_init(t_simplex_noise_octave *octave, uint32_t seed);
double _simplex_noise_octave_get2(t_simplex_noise_octave *octave, double xin, double yin);
double _simplex_noise_octave_get3(t_simplex_noise_octave *octave, double xin, double yin, double zin);

void simplex_noise_init(t_simplex_noise *noise, uint32_t largest_feature, double persistence, int32_t seed)
{
	noise->largest_feature = largest_feature;
	noise->persistence = persistence;
	noise->seed = seed;
	noise->octaves_number = ceil(log10(noise->largest_feature) / log10(2));
	if (!(noise->octaves = malloc(sizeof(*noise->octaves) * noise->octaves_number)))
		ERROR("malloc failed");
	if (!(noise->frequencies = malloc(sizeof(*noise->frequencies) * noise->octaves_number)))
		ERROR("malloc failed");
	if (!(noise->amplitudes = malloc(sizeof(*noise->amplitudes) * noise->octaves_number)))
		ERROR("malloc failed");
	for (uint32_t i = 0; i < noise->octaves_number; ++i)
	{
		_simplex_noise_octave_init(&noise->octaves[i], seed);
		noise->frequencies[i] = pow(2, i);
		noise->amplitudes[i] = pow(noise->persistence, noise->octaves_number - i);
	}
}

double simplex_noise_get2(t_simplex_noise *noise, int32_t x, int32_t y)
{
	double result = 0;
	for (uint32_t i = 0; i < noise->octaves_number; ++i)
	{
		result += _simplex_noise_octave_get2(&noise->octaves[i], x / noise->frequencies[i], y / noise->frequencies[i]) * noise->amplitudes[i];
	}
	return (result);
}

double simplex_noise_get3(t_simplex_noise *noise, int32_t x, int32_t y, int32_t z)
{
	double result = 0;
	for (uint32_t i = 0 ; i < noise->octaves_number; ++i)
	{
		double frequency = pow(2, i);
		double amplitude = pow(noise->persistence, noise->octaves_number - i);
		result += _simplex_noise_octave_get3(&noise->octaves[i], x / frequency, y / frequency, z / frequency) * amplitude;
	}
	return (result);
}

static int fastfloor(double x)
{
	int xi = (int)x;
	return (x < xi ? xi - 1 : xi);
}

static double dot2(t_simplex_noise_grad *g, double x, double y)
{
	return (g->x * x + g->y * y);
}

static double dot3(t_simplex_noise_grad *g, double x, double y, double z)
{
	return (g->x * x + g->y * y + g->z * z);
}


void _simplex_noise_octave_init(t_simplex_noise_octave *octave, uint32_t seed)
{
	short *p = malloc(sizeof(p_supply));
	if (!p)
		ERROR("malloc failed");
	memcpy(p, p_supply, sizeof(p_supply));
	seed = seed * 6364136223846793005LL + 1442695040888963407LL;
	seed = seed * 6364136223846793005LL + 1442695040888963407LL;
	seed = seed * 6364136223846793005LL + 1442695040888963407LL;
	for (uint32_t i = 0; i < NUMBEROFSWAPS; ++i)
	{
		seed = seed * 6364136223846793005LL + 1442695040888963407LL;
		int r = (int)((seed + 31) % (256));
		if (r < 0)
			r += (i + 1);
		short temp = p[r];
		p[r] = p[i % 256];
		p[i % 256] = temp;
	}
	for (uint32_t i = 0; i < 512; ++i)
	{
		octave->perm[i] = p[i & 255];
		octave->permMod12[i] = (short)(octave->perm[i] % 12);
	}

}

double _simplex_noise_octave_get2(t_simplex_noise_octave *octave, double xin, double yin)
{
		double n0;
		double n1;
		double n2;
		double s = (xin + yin) * F2;
		int i = fastfloor(xin + s);
		int j = fastfloor(yin + s);
		double t = (i + j) * G2;
		double X0 = i - t;
		double Y0 = j - t;
		double x0 = xin - X0;
		double y0 = yin - Y0;
		int i1;
		int j1;
		if (x0 > y0)
		{
			i1 = 1;
			j1 = 0;
		}
		else
		{
			i1 = 0;
			j1 = 1;
		}
		double x1 = x0 - i1 + G2;
		double y1 = y0 - j1 + G2;
		double x2 = x0 - 1.0 + 2.0 * G2;
		double y2 = y0 - 1.0 + 2.0 * G2;
		int ii = i & 255;
		int jj = j & 255;
		int gi0 = octave->permMod12[ii + octave->perm[jj]];
		int gi1 = octave->permMod12[ii + i1 + octave->perm[jj + j1]];
		int gi2 = octave->permMod12[ii + 1 + octave->perm[jj + 1]];
		double t0 = 0.5 - x0 * x0 - y0 * y0;
		if (t0 < 0)
		{
			n0 = 0.0;
		}
		else
		{
			t0 *= t0;
			n0 = t0 * t0 * dot2(&grads[gi0], x0, y0);
		}
		double t1 = 0.5 - x1 * x1 - y1 * y1;
		if (t1 < 0)
		{
			n1 = 0.0;
		}
		else
		{
			t1 *= t1;
			n1 = t1 * t1 * dot2(&grads[gi1], x1, y1);
		}
		double t2 = 0.5 - x2 * x2 - y2 * y2;
		if (t2 < 0)
		{
			n2 = 0.0;
		}
		else
		{
			t2 *= t2;
			n2 = t2 * t2 * dot2(&grads[gi2], x2, y2);
		}
		return (70.0 * (n0 + n1 + n2));
	}


double _simplex_noise_octave_get3(t_simplex_noise_octave *octave, double xin, double yin, double zin)
{
	double n0;
	double n1;
	double n2;
	double n3;
	double s = (xin + yin + zin) * F3;
	int i = fastfloor(xin + s);
	int j = fastfloor(yin + s);
	int k = fastfloor(zin + s);
	double t = (i + j + k) * G3;
	double X0 = i - t;
	double Y0 = j - t;
	double Z0 = k - t;
	double x0 = xin - X0;
	double y0 = yin - Y0;
	double z0 = zin - Z0;
	int i1;
	int j1;
	int k1;
	int i2;
	int j2;
	int k2;
	if (x0 >= y0)
	{
		if (y0 >= z0)
		{
			i1 = 1;
			j1 = 0;
			k1 = 0;
			i2 = 1;
			j2 = 1;
			k2 = 0;
		}
		else if (x0 >= z0)
		{
			i1 = 1;
			j1 = 0;
			k1 = 0;
			i2 = 1;
			j2 = 0;
			k2 = 1;
		}
		else
		{
			i1 = 0;
			j1 = 0;
			k1 = 1;
			i2 = 1;
			j2 = 0;
			k2 = 1;
		}
	}
	else
	{
		if (y0 < z0)
		{
			i1 = 0;
			j1 = 0;
			k1 = 1;
			i2 = 0;
			j2 = 1;
			k2 = 1;
		}
		else if (x0 < z0)
		{
			i1 = 0;
			j1 = 1;
			k1 = 0;
			i2 = 0;
			j2 = 1;
			k2 = 1;
		}
		else
		{
			i1 = 0;
			j1 = 1;
			k1 = 0;
			i2 = 1;
			j2 = 1;
			k2 = 0;
		}
	}
	double x1 = x0 - i1 + G3;
	double y1 = y0 - j1 + G3;
	double z1 = z0 - k1 + G3;
	double x2 = x0 - i2 + 2.0 * G3;
	double y2 = y0 - j2 + 2.0 * G3;
	double z2 = z0 - k2 + 2.0 * G3;
	double x3 = x0 - 1.0 + 3.0 * G3;
	double y3 = y0 - 1.0 + 3.0 * G3;
	double z3 = z0 - 1.0 + 3.0 * G3;
	int ii = i & 255;
	int jj = j & 255;
	int kk = k & 255;
	int gi0 = octave->permMod12[ii + octave->perm[jj + octave->perm[kk]]];
	int gi1 = octave->permMod12[ii + i1 + octave->perm[jj + j1 + octave->perm[kk + k1]]];
	int gi2 = octave->permMod12[ii + i2 + octave->perm[jj + j2 + octave->perm[kk + k2]]];
	int gi3 = octave->permMod12[ii + 1 + octave->perm[jj + 1 + octave->perm[kk + 1]]];
	double t0 = 0.6 - x0 * x0 - y0 * y0 - z0 * z0;
	if (t0 < 0)
	{
		n0 = 0.0;
	}
	else
	{
		t0 *= t0;
		n0 = t0 * t0 * dot3(&grads[gi0], x0, y0, z0);
	}
	double t1 = 0.6 - x1 * x1 - y1 * y1 - z1 * z1;
	if (t1 < 0)
	{
		n1 = 0.0;
	}
	else
	{
		t1 *= t1;
		n1 = t1 * t1 * dot3(&grads[gi1], x1, y1, z1);
	}
	double t2 = 0.6 - x2 * x2 - y2 * y2 - z2 * z2;
	if (t2 < 0)
	{
		n2 = 0.0;
	}
	else
	{
		t2 *= t2;
		n2 = t2 * t2 * dot3(&grads[gi2], x2, y2, z2);
	}
	double t3 = 0.6 - x3 * x3 - y3 * y3 - z3 * z3;
	if (t3 < 0)
	{
		n3 = 0.0;
	}
	else
	{
		t3 *= t3;
		n3 = t3 * t3 * dot3(&grads[gi3], x3, y3, z3);
	}
	return (32.0 * (n0 + n1 + n2 + n3));
}
