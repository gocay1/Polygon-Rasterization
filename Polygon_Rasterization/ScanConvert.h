#ifndef SCAN_CONVERT_H
#define SCAN_CONVERT_H

#include <vector>
#define ImageW 400
#define ImageH 400

struct color {
	float r, g, b;		// Color (R,G,B values)
	color(){}
	color(float r1, float g1, float b1) {
		r = r1;
		g = g1;
		b = b1;
	}
};
class vect {
public:
	float x, y, z;
	vect(float nx, float ny, float nz)
	{
		x = nx;
		y = ny;
		z = nz;
	}
	vect()
	{
		x = 0;
		y = 0;
		z = 0;
	}
};
class vertex {
public:
	float x, y, z;
	vect normal;
	color c;
	int key;
};
class face {
public:
	vertex v1, v2, v3;
	vect norm;
	bool operator < (const face &f)
	{
		float least_z = 0;
		if (v1.z <= v2.z && v1.z <= v3.z) { least_z == v1.z; }
		else if (v2.z <= v1.z && v2.z <= v3.z) { least_z == v2.z; }
		else { least_z == v3.z; }

		float least_zf = 0;
		if (f.v1.z <= f.v2.z && f.v1.z <= f.v3.z) { least_zf == f.v1.z; }
		else if (f.v2.z <= f.v1.z && f.v2.z <= f.v3.z) { least_zf == f.v2.z; }
		else { least_zf == f.v3.z; }

		return least_z < least_zf;
	}
};

// Clears framebuffer to black
void clearFramebuffer();

// Sets pixel x,y to the color RGB
void setFramebuffer(int x, int y, float z, float R, float G, float B);

color find_color(vect normal);

#endif