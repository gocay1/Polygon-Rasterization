#include "PolygonDrawer.h"
#include "ScanConvert.h"
#include <algorithm>
#include <math.h>
#include "ScanConvert.h"
#include <iostream>

using namespace std;

class Edge
{
public:
	float slopeRecip;
	float maxY;
	float currentX;
	color currentF;
	color fIncr;
	vect currentN;
	vect nIncr;
	float currentZ;
	float zIncr;

	bool operator < ( const Edge &e )
	{
		if ( currentX == e.currentX )
		{
			return slopeRecip < e.slopeRecip;
		}
		else
		{
			return currentX < e.currentX;
		}
	}
};

vector<vector<Edge> > activeEdgeTable;
vector<Edge> activeEdgeList;
float r, g, b;
int shaderType;

void setShaderType(int x) {
	shaderType = x;
}

void print_color(color c) {
	cout << c.r << "      " << c.g << "      " << c.b << endl;
	cout << "----------------------" << endl;
}

void buildActiveEdgeTable ( vector<Pt> &points )
{
	int i;

	activeEdgeTable.clear ( );

	// add rows equal to height of image to active edge table
	for ( i = 0; i < ImageH; i++ )
	{
		vector<Edge> row;

		activeEdgeTable.push_back ( row );
	}

	for ( i = 0; i < points.size ( ); i++ )
	{
		Edge e;
		int next = ( i + 1 ) % points.size ( );

		// ignore horizontal lines
		if ( points [ i ].y == points [ next ].y )
		{
			continue;
		}
		e.maxY = max ( points [ i ].y, points [ next ].y );
		e.slopeRecip = ( points [ i ].x - points [ next ].x ) / (float)( points [ i ].y - points [ next ].y );

		e.fIncr.r = (points[i].c.r - points[next].c.r) / (float)(points[i].y - points[next].y);
		e.fIncr.g = (points[i].c.g - points[next].c.g) / (float)(points[i].y - points[next].y);
		e.fIncr.b = (points[i].c.b - points[next].c.b) / (float)(points[i].y - points[next].y);

		e.nIncr.x = (points[i].normal.x - points[next].normal.x) / (float)(points[i].y - points[next].y);
		e.nIncr.y = (points[i].normal.y - points[next].normal.y) / (float)(points[i].y - points[next].y);
		e.nIncr.z = (points[i].normal.z - points[next].normal.z) / (float)(points[i].y - points[next].y);

		e.zIncr = (points[i].z - points[next].z) / (float)(points[i].y - points[next].y);

		if ( points [ i ].y == e.maxY )
		{
			e.currentX = points[ next ].x;
			e.currentF = points[ next ].c;
			e.currentN = points[ next ].normal;
			e.currentZ = points[ next ].z;
			activeEdgeTable [ points [ next ].y ].push_back ( e );
		}
		else
		{
			e.currentX = points [ i ].x;
			e.currentF = points[ i ].c;
			e.currentN = points[ i ].normal;
			e.currentZ = points[ i ].z;
			activeEdgeTable [ points [ i ].y ].push_back ( e );
		}
		//cout << e.currentF.r << "       " << e.currentF.g << "      " << e.currentF.b << endl;
	}

}

void setRGB(color c) {
	r = c.r;
	g = c.g;
	b = c.b;
}

// assumes all vertices are within window!!!
void drawPolygon ( vector<Pt> points)
{
	int x, y, i;
	color dF;
	vect dN;
	float dZ;

	activeEdgeList.clear ( );
	buildActiveEdgeTable ( points );

	for ( y = 0; y < ImageH; y++ )
	{
		// add edges into active Edge List
		for ( i = 0; i < activeEdgeTable [ y ].size ( ); i++ )
		{
			activeEdgeList.push_back ( activeEdgeTable [ y ] [ i ] );
		}

		// delete edges from active Edge List
		for ( i = 0; i < activeEdgeList.size ( ); i++ )
		{
			if ( activeEdgeList [ i ].maxY <= y )
			{
				//cout << activeEdgeList[i].currentF.r << "      " << activeEdgeList[i].currentF.g << "      " << activeEdgeList[i].currentF.b << endl;
				activeEdgeList.erase ( activeEdgeList.begin ( ) + i );
				i--;
			}
		}

		// sort according to x value... a little expensive since not always necessary
		sort ( activeEdgeList.begin ( ), activeEdgeList.end ( ) );

		// draw scan line
		for ( i = 0; i < activeEdgeList.size ( ); i += 2 )
		{
			
			float xRange = activeEdgeList[i + 1].currentX - activeEdgeList[i].currentX;
			color set = activeEdgeList[i].currentF;
			dF.r = (activeEdgeList[i + 1].currentF.r - activeEdgeList[i].currentF.r) / xRange;
			dF.g = (activeEdgeList[i + 1].currentF.g - activeEdgeList[i].currentF.g) / xRange;
			dF.b = (activeEdgeList[i + 1].currentF.b - activeEdgeList[i].currentF.b) / xRange;
			
			vect setn = activeEdgeList[i].currentN;
			dN.x = (activeEdgeList[i + 1].currentN.x - activeEdgeList[i].currentN.x) / xRange;
			dN.y = (activeEdgeList[i + 1].currentN.y - activeEdgeList[i].currentN.y) / xRange;
			dN.z = (activeEdgeList[i + 1].currentN.z - activeEdgeList[i].currentN.z) / xRange;

			float setz = activeEdgeList[i].currentZ;
			dZ = (activeEdgeList[i + 1].currentZ - activeEdgeList[i].currentZ) / xRange;

			for ( x = (int)ceil ( activeEdgeList [ i ].currentX ); x < activeEdgeList [ i + 1 ].currentX; x++ )
			{
				if (shaderType == 2) {
					float multiplier = x - activeEdgeList[i].currentX;
					color use;
					use.r = set.r + dF.r*multiplier;
					use.g = set.g + dF.g*multiplier;
					use.b = set.b + dF.b*multiplier;
					setRGB(use);
				}
				if (shaderType == 3) {
					float multiplier = x - activeEdgeList[i].currentX;
					vect use;
					use.x = setn.x + dN.x*multiplier;
					use.y = setn.y + dN.y*multiplier;
					use.z = setn.z + dN.z*multiplier;
					setRGB(find_color(use));
				}
				float multiplier = x - activeEdgeList[i].currentX;
				float z = setz + dZ*multiplier;

				setFramebuffer ( x, y, z, r, g, b );
			}
		}
		

		// update edges in active edge list
		for ( i = 0; i < activeEdgeList.size ( ); i++ )
		{
			activeEdgeList [ i ].currentX += activeEdgeList [ i ].slopeRecip;
			activeEdgeList [ i ].currentF.r += activeEdgeList[i].fIncr.r;
			activeEdgeList [ i ].currentF.g += activeEdgeList[i].fIncr.g;
			activeEdgeList [ i ].currentF.b += activeEdgeList[i].fIncr.b;
			activeEdgeList[i].currentN.x += activeEdgeList[i].nIncr.x;
			activeEdgeList[i].currentN.y += activeEdgeList[i].nIncr.y;
			activeEdgeList[i].currentN.z += activeEdgeList[i].nIncr.z;
			activeEdgeList[i].currentZ += activeEdgeList[i].zIncr;
		}
	}
}