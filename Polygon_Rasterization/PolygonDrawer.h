#ifndef POLYGON_DRAWER_H
#define POLYGON_DRAWER_H

#include <vector>
#include "ScanConvert.h"

using namespace std;

class Pt
{
public:
	int x, y;
	float z;
	color c;
	vect normal;
	
	Pt ( void )
	{
		x = y = 0;
	}

	Pt ( int nX, int nY )
	{
		x = nX;
		y = nY;
	}
};

void drawPolygon ( vector<Pt> points );
void setRGB(color c);
void setShaderType(int x);

#endif