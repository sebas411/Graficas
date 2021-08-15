#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <time.h>
#include <cmath>

#include "obj.h"

using namespace std;

struct Color{
	unsigned char r, g, b;
	bool selected = true;
	};


class V2 {
	public:
	int x, y;
	V2(int x, int y){
		this->x = x;
		this->y = y;
	}
};

class nV2 {
	public:
	float x, y;
	nV2(float x, float y){
		this->x = x;
		this->y = y;
	}
};

/*class V3 {
	public:
	int x, y;
	double z;
	V3(int x, int y, double z){
		this->x = x;
		this->y = y;
		this->z = z;
	}
	V2 toV2(){
		return V2(x, y);
	}
};*/

class nV3 {
	public:
	float x, y;
	double z;
	nV3(float x, float y, double z){
		this->x = x;
		this->y = y;
		this->z = z;
	}
	V2 toV2(){return V2((int) x, (int) y);}
};

class Renderer {
	private:
	int width, height, vpx, vpy, vpw, vph;
	vector<vector<vector<unsigned char>>> framebuffer;
	vector<vector<double>> zbuffer;
	unsigned char currentColor[3] = {255, 255, 255};
	unsigned char clearColor[3] = {0, 0, 0};
	nV3 light = nV3(0, -0.9486832981, 0.316227766);

	void initVector(){
		for (int y = 0; y < height; y++){
			vector<vector<unsigned char>> line;
			vector<double> zline;
			for (int x = 0; x < width; x++){
				vector<unsigned char> color = {0, 0, 0};
				line.push_back(color);
				zline.push_back(-9e200);
			}
			framebuffer.push_back(line);
			zbuffer.push_back(zline);
		}
	}
	
	void insertColor(int x, int y, unsigned char* col) {
		for (int i=0; i<3;i++){
			framebuffer[y][x][i] = col[2-i];
		}
	}
	
	void point(int x, int y, unsigned char* color){
		insertColor(x, y, color);
	}

	void point(int x, int y){
		point(x, y, currentColor);
	}

	void write(string filename){
		ofstream f;
		f.open(filename);
		//header
		static unsigned char header[14];
		
		int size = 14 + 40 + 3 * width * height;
		int offset = 14 + 40;
		int padding = width % 4;
		
		header[0] = (unsigned char)('B');
		header[1] = (unsigned char)('M');
		header[2] = (unsigned char)(size);
		header[3] = (unsigned char)(size >> 8);
		header[4] = (unsigned char)(size >> 16);
		header[5] = (unsigned char)(size >> 24);
		header[2] = (unsigned char)(0);
		header[3] = (unsigned char)(0);
		header[4] = (unsigned char)(0);
		header[5] = (unsigned char)(0);
		header[10] = (unsigned char)(offset);
		header[11] = (unsigned char)(offset >> 8);
		header[12] = (unsigned char)(offset >> 16);
		header[13] = (unsigned char)(offset >> 24);
		
		for (int i = 0; i < (int)(sizeof(header)/sizeof(header[0])); i++) {
			f << header[i];
		}
		
		//infoHeader
		static unsigned char infoHeader[40];
		int infoSize = 40;
		int bitmapSize = 3*width*height;
		
		infoHeader[0] = (unsigned char)(infoSize);
		infoHeader[1] = (unsigned char)(infoSize >> 8);
		infoHeader[2] = (unsigned char)(infoSize >> 16);
		infoHeader[3] = (unsigned char)(infoSize >> 24);
		infoHeader[4] = (unsigned char)(width);
		infoHeader[5] = (unsigned char)(width >> 8);
		infoHeader[6] = (unsigned char)(width >> 16);
		infoHeader[7] = (unsigned char)(width >> 24);
		infoHeader[8] = (unsigned char)(height);
		infoHeader[9] = (unsigned char)(height >> 8);
		infoHeader[10] = (unsigned char)(height >> 16);
		infoHeader[11] = (unsigned char)(height >> 24);
		infoHeader[12] = (unsigned char)(1);
		infoHeader[13] = (unsigned char)(0);
		infoHeader[14] = (unsigned char)(24);
		infoHeader[15] = (unsigned char)(0);
		infoHeader[16] = (unsigned char)(0);
		infoHeader[17] = (unsigned char)(0);
		infoHeader[18] = (unsigned char)(0);
		infoHeader[19] = (unsigned char)(0);
		infoHeader[20] = (unsigned char)(bitmapSize);
		infoHeader[21] = (unsigned char)(bitmapSize >> 8);
		infoHeader[22] = (unsigned char)(bitmapSize >> 16);
		infoHeader[23] = (unsigned char)(bitmapSize >> 24);
		infoHeader[24] = (unsigned char)(0);
		infoHeader[25] = (unsigned char)(0);
		infoHeader[26] = (unsigned char)(0);
		infoHeader[27] = (unsigned char)(0);
		infoHeader[28] = (unsigned char)(0);
		infoHeader[29] = (unsigned char)(0);
		infoHeader[30] = (unsigned char)(0);
		infoHeader[31] = (unsigned char)(0);
		infoHeader[32] = (unsigned char)(0);
		infoHeader[33] = (unsigned char)(0);
		infoHeader[34] = (unsigned char)(0);
		infoHeader[35] = (unsigned char)(0);
		infoHeader[36] = (unsigned char)(0);
		infoHeader[37] = (unsigned char)(0);
		infoHeader[38] = (unsigned char)(0);
		infoHeader[39] = (unsigned char)(0);
		
		for (int i = 0; i < (int)(sizeof(infoHeader)/sizeof(infoHeader[0])); i++) {
			f << infoHeader[i];
		}
		
		
		//bitmap
		for (int y = 0; y < (int)framebuffer.size(); y++) {
			for (int x = 0; x < (int)framebuffer[y].size(); x++) {
				for (int i=0; i<3;i++){
					f << framebuffer[y][x][i];
				}
			}
			for (int j = 0; j < padding; j++) f << (unsigned char) 0;
		}
		f.close();
		
	}
	
	void denormalizedLine(int x0, int y0, int x1, int y1){
		int dy = abs(y1 - y0);
		int dx = abs(x1 - x0);
		bool steep = dy > dx;
		
		if(steep){
			swap(x0, y0);
			swap(x1, y1);
			swap(dx, dy);
		}

		if(x0 > x1){
			swap(x0, x1);
			swap(y0, y1);
		}
		
		int offset = 0;
		int threshold = dx;
		int y = y0;		
		
		for(int x = x0; x <= x1; x++){
			if(steep){point(y, x);}
			else {point(x, y);}
			
			offset += 2 * dy;
			
			if(offset >= threshold){
				if(y0 < y1){y ++;}
				else {y--;}
				threshold += 2 * dx;
			}
		}
	}

	void vertexLine(V2 A, V2 B){
		this->denormalizedLine(A.x, A.y, B.x, B.y);
	}

	auto bbox(V2 A, V2 B, V2 C){
		int xmin = A.x;
		int xmax = A.x;
		if (B.x < xmin) xmin = B.x;
    if (C.x < xmin) xmin = C.x;

    if (B.x > xmax) xmax = B.x;
    if (C.x > xmax) xmax = C.x;

    int ymin = A.y;
    int ymax = A.y;
    if (B.y < ymin) ymin = B.y;
    if (C.y < ymin) ymin = C.y;

    if (B.y > ymax) ymax = B.y;
    if (C.y > ymax) ymax = C.y;

		struct retVals {int i1, i2, i3, i4;};
    return retVals{xmin, xmax, ymin, ymax};
	}

	auto cross(nV3 v0, nV3 v1){
		double cx = v0.y * v1.z - v0.z * v1.y;
		double cy = v0.z * v1.x - v0.x * v1.z;
    double cz = v0.x * v1.y - v0.y * v1.x;

		struct retVals {double i1, i2, i3;};
		return retVals{cx, cy, cz};
	}

	auto barycentric(V2 A, V2 B, V2 C, V2 P){
		auto [cx, cy, cz] = cross(
			nV3(B.x - A.x, C.x - A.x, A.x - P.x),
			nV3(B.y - A.y, C.y - A.y, A.y - P.y)
		);
		struct retVals{double f1, f2, f3;};

		if(cz == 0) return retVals{-1, -1, -1};

		double u = (double) cx / cz;
		double v = (double) cy / cz;
		double w = 1 - (u + v);
		
		return retVals{w, v, u};
	}

	nV3 sub(nV3 v0, nV3 v1){
		return nV3(
      v0.x - v1.x,
      v0.y - v1.y,
      v0.z - v1.z
    );
	}

	double length(nV3 v0) {
		return pow(pow(v0.x, 2) + pow(v0.y, 2) + pow(v0.z, 2), 0.5);
	}

	nV3 norm(nV3 v0) {
		double l = length(v0);
		if (l == 0) return nV3(0, 0, 0);

		return nV3(
			v0.x/l,
			v0.y/l,
			v0.z/l
		);
	}

	double dot(nV3 v0, nV3 v1){
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
	}

	public:
	
	void createWindow(int width, int height){
		this->width=width;
		this->height=height;
		initVector();
	}
	
	void setViewPort(int x, int y, int width, int height){
		this->vpx = x;
		this->vpy = y;
		this->vpw = width;
		this->vph = height;
		
	}
	
	void setClearColor(unsigned char clearColor[3]){
		for (int i=0; i<3;i++){
			this->clearColor[i] = clearColor[i];
		}
	}
	
	void setCurrentColor(unsigned char color[3]){
		for (int i=0; i<3;i++){
			this->currentColor[i] = color[i];
		}
	}
	
	void clear(){
		for (int y = 0; y < (int)framebuffer.size(); y++) {
			for (int x = 0; x < (int)framebuffer[y].size(); x++) {
				insertColor(x, y, clearColor);
			}
		}
	}
	
	void drawVertex(float x, float y){
		int wx = (int)((x + 1)/2 * vpw + vpx);
		int wy = (int)((y + 1)/2 * vph + vpy);
		if(wx == vpw) wx -= 1;
		if(wy == vph) wy -= 1;
		point(wx, wy);
	}

	void drawLine(float nx0, float ny0, float nx1, float ny1){
		int x0 = (int)((nx0 + 1)/2 * vpw + vpx);
		int x1 = (int)((nx1 + 1)/2 * vpw + vpx);
		int y0 = (int)((ny0 + 1)/2 * vph + vpy);
		int y1 = (int)((ny1 + 1)/2 * vph + vpy);
		
		this->denormalizedLine(x0, y0, x1, y1);
		
	}
	
	void wireframeTriangle(V2 A, V2 B, V2 C){
		this->vertexLine(A, B);
		this->vertexLine(B, C);
		this->vertexLine(C, A);
	}

	void fillTriangle(nV3 nA, nV3 nB, nV3 nC, Color color = Color{0, 0, 0, false}){
		nV3 A((int)((nA.x + 1)/2 * vpw + vpx), (int)((nA.y + 1)/2 * vph + vpy), nA.z);
		nV3 B((int)((nB.x + 1)/2 * vpw + vpx), (int)((nB.y + 1)/2 * vph + vpy), nA.z);
		nV3 C((int)((nC.x + 1)/2 * vpw + vpx), (int)((nC.y + 1)/2 * vph + vpy), nA.z);
		
		auto [xmin, xmax, ymin, ymax] = this->bbox(A.toV2(), B.toV2(), C.toV2());
		//if(xmin < vpx | xmax > (vpx + vpw) | ymin < vpy | ymax > (vpy +vph)) return;

		for(int x = xmin; x <= xmax; x++){
			for(int y = ymin; y <= ymax; y++){
				V2 P(x, y);
				auto [w, v, u] = this->barycentric(A.toV2(), B.toV2(), C.toV2(), P);
				if(w < 0 | v < 0 | u < 0) continue;
				if(x < vpx | x > (vpx + vpw) | y < vpy | y > (vpy +vph)) continue;
				float z = A.z * w + B.z * v + C.z * u;
				if(z > zbuffer[y][x]){
					if(color.selected){
						unsigned char newCol[3] = {color.r, color.g, color.b}; 
						this->point(x, y, newCol);
					}
					else this->point(x, y);
					zbuffer[y][x] = z;
				}
			}
		}
	}

	void loadTrian(nV3 A, nV3 B, nV3 C, nV3 dA, nV3 dB, nV3 dC){
		auto [a, b, c] = this->cross(sub(B, A), sub(C, A));
		nV3 normal = norm(nV3(a, b, c));
		double intensity = dot(normal, light);
		//cout << intensity;
		//printf("\ncross: x=%f y=%f z=%f", a, b, c);
		//printf("\nnormal: x=%f y=%f z=%f\n", normal.x, normal.y, normal.z);
		unsigned char grey = (unsigned char)(255 * intensity);
		//cout << (int) grey << endl;

		if (intensity >= 0) 
		this->fillTriangle(dA, dB, dC, Color{0, (unsigned char)((int)grey*0.75), 0});
	}

	void loadModel(string filename, float tx, float ty, float sx, float sy){
		Obj model(filename);
		
		
		for(int i = 0; i < (int)model.faces.size(); i++){
			vector<vector<int>> face = model.faces[i];
			if (face.size() == 3){
				int f1 = face[0][0] - 1;
				int f2 = face[1][0] - 1;
				int f3 = face[2][0] - 1;

				nV3 A (model.vertices[f1][0], model.vertices[f1][1], model.vertices[f1][2]);
				nV3 B (model.vertices[f2][0], model.vertices[f2][1], model.vertices[f2][2]);
				nV3 C (model.vertices[f3][0], model.vertices[f3][1], model.vertices[f3][2]);

				nV3 dA ((A.x + tx)*sx, (A.y + ty)*sy, A.z * 1e10);
				nV3 dB ((B.x + tx)*sx, (B.y + ty)*sy, B.z * 1e10);
				nV3 dC ((C.x + tx)*sx, (C.y + ty)*sy, C.z * 1e10);

				this->loadTrian(A, B, C, dA, dB, dC);

			} else if (face.size() == 4){
				int f1 = face[0][0] - 1;
				int f2 = face[1][0] - 1;
				int f3 = face[2][0] - 1;
				int f4 = face[3][0] - 1;

				nV3 A (model.vertices[f1][0], model.vertices[f1][1], model.vertices[f1][2]);
				nV3 B (model.vertices[f2][0], model.vertices[f2][1], model.vertices[f2][2]);
				nV3 C (model.vertices[f3][0], model.vertices[f3][1], model.vertices[f3][2]);
				nV3 D (model.vertices[f4][0], model.vertices[f4][1], model.vertices[f4][2]);

				nV3 dA ((A.x + tx)*sx, (A.y + ty)*sy, A.z * 1e10);
				nV3 dB ((B.x + tx)*sx, (B.y + ty)*sy, B.z * 1e10);
				nV3 dC ((C.x + tx)*sx, (C.y + ty)*sy, C.z * 1e10);
				nV3 dD ((D.x + tx)*sx, (D.y + ty)*sy, D.z * 1e10);

				this->loadTrian(A, B, C, dA, dB, dC);
				this->loadTrian(A, C, D, dA, dC, dD);

			} else {
				for(int j = 0; j < (int)face.size(); j++){
					int f1 = face[j][0];
					int f2 = face[(j+1) % (int)face.size()][0];
					
					vector<double> v1 = model.vertices[f1 - 1];
					vector<double> v2 = model.vertices[f2 - 1];
					
					float x1 = (v1[0] + tx) * sx;
					float y1 = (v1[1] + ty) * sy;
					float x2 = (v2[0] + tx) * sx;
					float y2 = (v2[1] + ty) * sy;
					if((x1 >= -1 && y1 >= -1 && x2 >= -1 && y2 >= -1) && (x1 <= 1 && y1 <= 1 && x2 <= 1 && y2 <= 1)){
						drawLine(x1, y1, x2, y2);
					}
				}
			}
		}
	}
	
	void render(){
		write("image.bmp");
	}
	
};

Renderer ren;

void glInit(){}

void glCreateWindow(int width, int height){
	ren.createWindow(width, height);
}

void glViewPort(int x, int y, int width, int height){
	ren.setViewPort(x, y, width, height);
}

void glClear() {ren.clear();}

void glClearColor(float r, float g, float b) {
	unsigned char color[] = {(unsigned char)(r*255), (unsigned char)(g*255), (unsigned char)(b*255)};
	ren.setClearColor(color);
}

void glVertex(float x, float y){
	ren.drawVertex(x, y);
}

void glColor(float r, float g, float b) {
	unsigned char color[] = {(unsigned char)(r*255), (unsigned char)(g*255), (unsigned char)(b*255)};
	ren.setCurrentColor(color);
}

void glLine(float x0, float y0, float x1, float y1){
	ren.drawLine(x0, y0, x1, y1);
}

void glLoad(string filename, float tx, float ty, float sx, float sy){
	ren.loadModel(filename, tx, ty, sx, sy);
}

void glFinish() {ren.render();}

int main() {
	srand(time(NULL));
	glCreateWindow(1001, 1001);
	glViewPort(0, 0, 1000, 1000);
	glColor(0.5, 0.5, 0);

	//glLoad("./models/model.obj", 0, 0, 0.85, 0.85);
	//glLoad("./models/face.obj", 0, -10, 0.05, 0.05);
	glLoad("./models/plantita.obj", 0, -3, 0.25, 0.25);
	glFinish();
	//prueba
	return 0;
}
