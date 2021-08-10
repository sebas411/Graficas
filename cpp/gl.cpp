#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>
#include <vector>

#include "obj.h"

using namespace std;

class V2 {
	public:
	int x, y;
	V2(int x, int y){
		this->x = x;
		this->y = y;
	}
};

class V3 {
	public:
	int x, y, z;
	V3(int x, int y, int z){
		this->x = x;
		this->y = y;
		this->z = z;
	}
};

class Renderer {
	private:
	int width, height, vpx, vpy, vpw, vph;
	vector<vector<vector<unsigned char>>> framebuffer;
	unsigned char currentColor[3] = {255, 255, 255};
	unsigned char clearColor[3] = {0, 0, 0};

	void initVector(){
		for (int y = 0; y < height; y++){
			vector<vector<unsigned char>> line;
			for (int x = 0; x < width; x++){
				vector<unsigned char> color = {0, 0, 0};
				line.push_back(color);
			}
			framebuffer.push_back(line);
		}
	}
	
	void insertColor(int x, int y, unsigned char* col) {
		for (int i=0; i<3;i++){
			framebuffer[y][x][i] = col[2-i];
		}
	}
	
	void point(int x, int y){
		insertColor(x, y, currentColor);
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

	auto cross(V3 v0, V3 v1){
		int cx = v0.y * v1.z - v0.z * v1.y;
		int cy = v0.z * v1.x - v0.x * v1.z;
    int cz = v0.x * v1.y - v0.y * v1.x;

		//NOTA
		struct retVals {int i1, i2, i3;};
		return retVals{cx, cy, cz};
	}

	auto barycentric(V2 A, V2 B, V2 C, V2 P){
		auto [cx, cy, cz] = cross(
			V3(B.x - A.x, C.x - A.x, A.x - P.x),
			V3(B.y - A.y, C.y - A.y, A.y - P.y)
		);
		float u = (float) cx / cz;
		float v = (float) cy / cz;
		float w = 1 - (u + v);
		struct retVals{float f1, f2, f3;};
		return retVals{w, v, u};
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

	void fillTriangle(V2 A, V2 B, V2 C){
		auto [xmin, xmax, ymin, ymax] = this->bbox(A, B, C);

		for(int x = xmin; x <= xmax; x++){
			for(int y = ymin; y <= ymax; y++){
				V2 P(x, y);
				auto [w, v, u] = this->barycentric(A, B, C, P);
				if(w < 0 | v < 0 | u < 0) continue;
				this->point(x, y);
			}
		}
	}

	void loadModel(string filename, float tx, float ty, float sx, float sy){
		Obj model(filename);
		
		for(int i = 0; i < (int)model.faces.size(); i++){
			vector<vector<int>> face = model.faces[i];
			
			for(int j = 0; j < (int)face.size(); j++){
				int f1 = face[j][0];
				int f2 = face[(j+1) % (int)face.size()][0];
				
				vector<float> v1 = model.vertices[f1 - 1];
				vector<float> v2 = model.vertices[f2 - 1];
				
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
	glCreateWindow(1003, 1003);
	glViewPort(0, 0, 1000, 1000);
	glColor(0.5, 0, 0);

	//glLoad("./models/indoor plant_02.obj", 0, -3, 0.25, 0.25);

	ren.wireframeTriangle(V2(10, 70),  V2(50, 160), V2(70, 80));
	ren.wireframeTriangle(V2(180, 50), V2(150, 1),  V2(70, 180));
	ren.wireframeTriangle(V2(180, 150), V2(120, 160), V2(130, 180));

	glColor(0.5, 0, 0.5);
	ren.fillTriangle(V2(10, 70),  V2(50, 160), V2(70, 80));
	//ren.fillTriangle(V2(180, 50), V2(150, 1),  V2(70, 180));
	//ren.fillTriangle(V2(180, 150), V2(120, 160), V2(130, 180));
	//auto [u, v, w] = ren.barycentric(V2(10, 70),  V2(50, 160), V2(70, 80), V2(65, 100));
	//cout<< u << v << w;
	glFinish();
	
	return 0;
}
