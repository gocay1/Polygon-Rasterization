#include <Windows.h>
#include <GL/glut.h>
#include <math.h>
#include "ScanConvert.h"
#include "PolygonDrawer.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

/******************************************************************
	Notes:
	Image size is 400 by 400 by default.  You may adjust this if
		you want to.
	You can assume the window will NOT be resized.
	Call clearFramebuffer to clear the entire framebuffer.
	Call setFramebuffer to set a pixel.  This should be the only
		routine you use to set the color (other than clearing the
		entire framebuffer).  drawit() will cause the current
		framebuffer to be displayed.
	As is, your scan conversion should probably be called from
		within the display function.  There is a very short sample
		of code there now.
	You may add code to any of the subroutines here,  You probably
		want to leave the drawit, clearFramebuffer, and
		setFramebuffer commands alone, though.
  *****************************************************************/

//WHAT DO I NEED BRAH?
/*
Vector
	x, y, z
Vertex
	x, y, z
	Normal at Vertex
	Color at Vertex
Face
	x, y, z
	Normal
Point
	x, y
	Pixel Color
	Normal at pixel
Color
	Division
	Multiplication
	Add another color
*/

float framebuffer[ImageH][ImageW][3];
float zBuffer[ImageH][ImageW][1];

vector<vertex> vertices;
vector<face> faces;
vector<vector<vect>> adj_face_normals;
vect light(1,1,-1.0);

// Draws the scene
void drawit(void)
{
   glDrawPixels(ImageW,ImageH,GL_RGB,GL_FLOAT,framebuffer);
   glFlush();
}

// Clears framebuffer to black
void clearFramebuffer()
{
	int i,j;

	for(i=0;i<ImageH;i++) {
		for (j=0;j<ImageW;j++) {
			framebuffer[i][j][0] = 0.0;
			framebuffer[i][j][1] = 0.0;
			framebuffer[i][j][2] = 0.0;
		}
	}
	for (int x = 0; x < ImageH; x++) {
		for (int y = 0; y < ImageW; y++) {
			zBuffer[x][y][0] = 1;
		}
	}
}

// Sets pixel x,y to the color RGB
// I've made a small change to this function to make the pixels match
// those returned by the glutMouseFunc exactly - Scott Schaefer 
void setFramebuffer(int x, int y, float z, float R, float G, float B)
{
	if (zBuffer[y][x][0] > z) {
		zBuffer[y][x][0] = z;
		// changes the origin from the lower-left corner to the upper-left corner
		y = ImageH - 1 - y;
		if (R <= 1.0)
			if (R >= 0.0)
				framebuffer[y][x][0] = R;
			else
				framebuffer[y][x][0] = 0.0;
		else
			framebuffer[y][x][0] = 1.0;
		if (G <= 1.0)
			if (G >= 0.0)
				framebuffer[y][x][1] = G;
			else
				framebuffer[y][x][1] = 0.0;
		else
			framebuffer[y][x][1] = 1.0;
		if (B <= 1.0)
			if (B >= 0.0)
				framebuffer[y][x][2] = B;
			else
				framebuffer[y][x][2] = 0.0;
		else
			framebuffer[y][x][2] = 1.0;
	}
}

void display(void)
{
	drawit();
}

Pt window_coor(float x, float y) {
	Pt ret;
	ret.x = (((float)ImageW - 1.0) / 2.0)*(1.0 + x);
	ret.y = (((float)ImageH - 1.0) / 2.0)*(1.0 - y);
	return ret;
}

vect cross_product(vect vect1, vect vect2) {
	vect ret;

	ret.x = vect1.y*vect2.z - vect1.z*vect2.y;
	ret.y = vect1.z*vect2.x - vect1.x*vect2.z;
	ret.z = vect1.x*vect2.y - vect1.y*vect2.x;

	return ret;
}

float dot_product(vect vect1, vect vect2) {
	float ret;

	ret = vect1.x*vect2.x + vect1.y*vect2.y + vect1.z*vect2.z;
	return ret;
}

vect normalize(vect n) {
	vect ret;

	float div = sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
	ret.x = n.x / div;
	ret.y = n.y / div;
	ret.z = n.z / div;

	return ret;
}

vect find_normal(face f) {
	
	vect ret;

	vect vect1, vect2;

	vect1.x = f.v3.x - f.v1.x;
	vect1.y = f.v3.y - f.v1.y;
	vect1.z = f.v3.z - f.v1.z;

	vect2.x = f.v2.x - f.v1.x;
	vect2.y = f.v2.y - f.v1.y;
	vect2.z = f.v2.z - f.v1.z;

	ret = cross_product(vect1, vect2);

	return ret;
}

vect vertex_norm(vertex v1, vertex v2, vertex v3) {
	
	vect ret;

	vect vect1, vect2;

	vect1.x = v1.x - v3.x;
	vect1.y = v1.y - v3.y;
	vect1.z = v1.z - v3.z;

	vect2.x = v1.x - v2.x;
	vect2.y = v1.y - v2.y;
	vect2.z = v1.z - v2.z;

	ret = cross_product(vect1, vect2);

	return ret;
}

float angle_vect(vect vect1, vect vect2) {
	float dot = dot_product(vect1, vect2);
	float nrm = sqrt(vect1.x*vect1.x + vect1.y*vect1.y + vect1.z*vect1.z)*
				sqrt(vect2.x*vect2.x + vect2.y*vect2.y + vect2.z*vect2.z);
	float angle = acos(dot / nrm);

	return angle;
}

color find_color(vect normal) {

	color ret;

	float multiplier = 2.0*dot_product(light, normal);
	vect r = normal;
	r.x = r.x*multiplier - light.x;
	r.y = r.y*multiplier - light.y;
	r.z = r.z*multiplier - light.z;

	//ANGULAR WAY

	float alpha = angle_vect(r, vect(0.0, 0.0, -1.0));
	float angle = angle_vect(normal, light);

	ret.r = 0.5*.1 + (0.7*cos(angle) + 0.5*pow(cos(alpha), 5));
	ret.g = 0.5*.1 + (0.5*pow(cos(alpha), 5));
	ret.b = 0.5*.1 + (0.5*pow(cos(alpha), 5));

	//DOT PRODUCT WAY

	//float alpha = dot_product(r, vect(0.0, 0.0, -1.0));
	//float angle = dot_product(normal, light);

	//ret.r = 0.5*.1 + (0.7*angle + 0.5*pow(alpha, 5));
	//ret.g = 0.5*.1 + (0.5*pow(angle, 5));
	//ret.b = 0.5*.1 + (0.5*pow(angle, 5));

	return ret;
}

void flat_shading() {

	setShaderType(1);
	clearFramebuffer();

	for (face v : faces) {

		Pt one, two, three;
		one = window_coor(v.v1.x, v.v1.y);
		two = window_coor(v.v2.x, v.v2.y);
		three = window_coor(v.v3.x, v.v3.y);

		one.z = v.v1.z;
		two.z = v.v2.z;
		three.z = v.v3.z;

		color c = find_color(v.norm);
		setRGB(c);
		vector<Pt> pts;

		pts.push_back(one);
		pts.push_back(two);
		pts.push_back(three);

		if (v.norm.z <= 0) {
			drawPolygon(pts);
		}
	}
}

void gouraud_shading() {

	setShaderType(2);
	clearFramebuffer();

	for (face v : faces) {

		vector<Pt> pts;

		Pt one, two, three;
		one = window_coor(v.v1.x, v.v1.y);
		two = window_coor(v.v2.x, v.v2.y);
		three = window_coor(v.v3.x, v.v3.y);

		one.z = v.v1.z;
		two.z = v.v2.z;
		three.z = v.v3.z;

		one.c = vertices[v.v1.key].c;
		two.c = vertices[v.v2.key].c;
		three.c = vertices[v.v3.key].c;

		//one.c = color(1,0,0);
		//two.c = color(0,1,0);
		//three.c = color(0, 0, 1);

		pts.push_back(one);
		pts.push_back(two);
		pts.push_back(three);

		if (v.norm.z <= 0) {
			drawPolygon(pts);
		}
	}
}

void phong_shading() {

	setShaderType(3);
	clearFramebuffer();

	for (face v : faces) {

		vector<Pt> pts;

		Pt one, two, three;
		one = window_coor(v.v1.x, v.v1.y);
		two = window_coor(v.v2.x, v.v2.y);
		three = window_coor(v.v3.x, v.v3.y);

		one.z = v.v1.z;
		two.z = v.v2.z;
		three.z = v.v3.z;

		one.normal = vertices[v.v1.key].normal;
		two.normal = vertices[v.v2.key].normal;
		three.normal = vertices[v.v3.key].normal;

		//one.c = color(1,0,0);
		//two.c = color(0,1,0);
		//three.c = color(0, 0, 1);

		pts.push_back(one);
		pts.push_back(two);
		pts.push_back(three);

		if (v.norm.z <= 0) {
			drawPolygon(pts);
		}
	}
}

void keyboard(unsigned char key, int x, int y)
{
	glClear(GL_COLOR_BUFFER_BIT);

	if (key == 'c') {
		clearFramebuffer();
	}
	if(key == '1') {
		flat_shading();
	}
	if (key == '2') {
		gouraud_shading();
	}
	if (key == '3') {
		phong_shading();
	}
	glutPostRedisplay();
}

void init(void)
{
	clearFramebuffer();
	light = normalize(light);
	for (int x = 0; x < ImageH; x++) {
		for (int y = 0; y < ImageW; y++) {
			zBuffer[x][y][0] = 1;
		}
	}
	// test clipping and drawing
	//vector<Pt> pts;

	/*pts.push_back ( Pt ( 10, 10 ) );
	pts.push_back ( Pt ( 10, 100 ) );
	pts.push_back ( Pt ( 100, 200 ) );
	pts.push_back ( Pt ( 200, 100 ) );
	pts.push_back ( Pt ( 250, 100 ) );
	pts.push_back ( Pt ( 300, 200 ) );
	pts.push_back ( Pt ( 300, 10 ) );

	drawPolygon ( pts );*/
}

int main(int argc, char** argv)
{

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
	glutInitWindowSize(ImageW,ImageH);
	glutInitWindowPosition(100,100);
	glutCreateWindow("Tunc Gocay - Homework 4");
	init();
	if (argv[1]) {
		ifstream myfile;
		myfile.open(argv[1], ios_base::in);
		if (myfile.is_open()) {
			while (!myfile.eof()) {
				string line;
				getline(myfile, line);
				if (line[0] == 'v') {
					stringstream ss(line);
					char c;    ss >> c;    // read "v"
					float d1; ss >> d1;   // read 1
					float d2; ss >> d2;   // read 2
					float d3; ss >> d3;   // read 3
					vertex n;
					n.x = d1;
					n.y = d2;
					n.z = d3;
					n.key = vertices.size();
					vertices.push_back(n);
					vector<vect> x;
					adj_face_normals.push_back(x);
				}
				else if (line[0] == 'f') {
					stringstream ss(line);
					char c;    ss >> c;    // read "v"
					int d1; ss >> d1;   // read 1
					int d2; ss >> d2;   // read 2
					int d3; ss >> d3;   // read 3
					face f;
					
					f.v1 = vertices[d1 - 1];
					f.v2 = vertices[d2 - 1];
					f.v3 = vertices[d3 - 1];

					f.norm = find_normal(f);
					f.norm = normalize(f.norm);

					adj_face_normals[d1 - 1].push_back(f.norm);
					adj_face_normals[d2 - 1].push_back(f.norm);
					adj_face_normals[d3 - 1].push_back(f.norm);

					faces.push_back(f);
				}
			}
			myfile.close();
			clearFramebuffer();
			flat_shading();

			//Calculate Vertex Normals and color 
			for (int i = 0; i < adj_face_normals.size(); i++) {
				vect vertex_norm;
				float counter = 0;
				for (int r = 0; r < adj_face_normals[i].size(); r++) {
					vertex_norm.x += adj_face_normals[i][r].x;
					vertex_norm.y += adj_face_normals[i][r].y;
					vertex_norm.z += adj_face_normals[i][r].z;
					counter++;
				}
				vertex_norm.x = vertex_norm.x / counter;
				vertex_norm.y = vertex_norm.y / counter;
				vertex_norm.z = vertex_norm.z / counter;
				vertices[i].normal = vertex_norm;
				vertices[i].c = find_color(vertices[i].normal);
			}
			//---------------------------------
		}
		else {
			printf("ERROR: can't open file:");
		}

	}


	glutKeyboardFunc(keyboard);
	glutDisplayFunc(display);

	glutMainLoop();
	return 0;
}
