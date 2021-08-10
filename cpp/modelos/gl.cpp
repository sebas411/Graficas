#include <iostream>
#include <fstream>
#include <cstdint>
#include <stdlib.h>
#include <algorithm>

#include "obj.h"

using namespace std;



unsigned char WHITE[3] = {255, 255, 255}, BLACK[3] = {0, 0, 0}, RED[3] = {255, 0, 0}, BLUE[3] = {0, 0, 255}, GREEN[3] = {0, 255, 0}, PURPLE[3] = {255, 0, 255}, CYAN[3] = {0, 255, 255};
unsigned char*** framebufferPtr; 
int framebufferWidth;
int framebufferHeight;
bool isFramebufferSet = false;

template <int frameWidth = 1024, int frameHeight = 768>
class Renderer {
	private:
	const int static width=frameWidth, height=frameHeight;
	int vpx, vpy, vpw, vph;
	unsigned char framebuffer[height][width][3];
	unsigned char currentColor[3] = {255, 255, 255};
	unsigned char clearColor[3] = {0, 0, 0};
	
	
	void insertColor(int x, int y, unsigned char* col) {
		for (int i=0; i<3;i++){
			framebuffer[y][x][i] = col[2-i];
		}
	}
	
	void write(string filename){
		ofstream f;
		f.open(filename);
		//header
		static unsigned char header[14];
		
		int size = 14 + 40 + 3 * width * height;
		int offset = 14 + 40;
		
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
		for (int y = 0; y < (int)(sizeof(framebuffer)/sizeof(framebuffer[0])); y++) {
			for (int x = 0; x < (int)(sizeof(framebuffer[y])/sizeof(framebuffer[y][0])); x++) {
				for (int i=0; i<3;i++){
					f << framebuffer[y][x][i];
				}
			}
		}
		f.close();
		
	}
	
	
	public:
	
	Renderer() {
		clear();
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
		for (int y = 0; y < (int)(sizeof(framebuffer)/sizeof(framebuffer[0])); y++) {
			for (int x = 0; x < (int)(sizeof(framebuffer[y])/sizeof(framebuffer[y][0])); x++) {
				insertColor(x, y, clearColor);
			}
		}
	}
	
	void drawVertex(float x, float y){
		int wx = (int)((x + 1)/2 * vpw + vpx);
		int wy = (int)((y + 1)/2 * vph + vpy);
		point(wx, wy);
	}
	
	void drawLine(float nx0, float ny0, float nx1, float ny1){
		int x0 = (int)((nx0 + 1)/2 * vpw + vpx);
		int x1 = (int)((nx1 + 1)/2 * vpw + vpx);
		int y0 = (int)((ny0 + 1)/2 * vph + vpy);
		int y1 = (int)((ny1 + 1)/2 * vph + vpy);
		
		int dy = abs(y1 - y0);
		int dx = abs(x1 - x0);
		bool steep = dy > dx;
		
		if(steep){
			swap(x0, y0);
			swap(x1, y1);
			swap(dx, dy);
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
	
	void point(int x, int y){
		insertColor(x, y, currentColor);
	}
};

Renderer<1000, 1000> ren;

void glInit(){
	
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
	glViewPort(0, 0, 1000, 1000);
	glColor(0, 1, 0);
	/*for (float i = -1; i <= 1; i += 0.001){
		glVertex(i, -1);
		glVertex(i, 1);
		glVertex(-1, i);
		glVertex(1, i);
	}
	glLine(-1, -1, 1, 1);
	glLine(-1, -1, 0, 1);
	glLine(-1, -1, 1, 0);*/
	glLoad("./models/indoor plant_02.obj", 0, -3, 0.25, 0.25);
	glFinish();
	
	
	return 0;
}
