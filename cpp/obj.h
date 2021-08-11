#ifndef OBJ_H
#define OBJ_H

#include <iostream>
#include <fstream>
#include <vector>
#include <bits/stdc++.h>

using namespace std;

class Obj {
	public:
	vector<string> lines;
	vector<vector<double>> vertices;
	vector<vector<vector<int>>> faces;
	//vector<vector<char>> faces;
	
	//init
	Obj(string filename){
		ifstream input(filename);
		
		for(string line; getline(input, line); ){
			lines.push_back(line);
		}
		
		read();
	}
	
	void read(){
		for(int i = 0; i < (int)lines.size(); i++){
			
			if(lines[i][0] == 'v' and lines[i][1] == ' '){
				stringstream line(lines[i]);
				string v, x, y, z;
				vector<double> vertex;
				//store line in the variables
				line >> v >> x >> y >> z;
				//push variables to vector
				vertex.push_back(stod(x));
				vertex.push_back(stod(y));
				vertex.push_back(stod(z));
				//push vector to vertices list
				vertices.push_back(vertex);
				
			} else if(lines[i][0] == 'f'){
				stringstream line(lines[i]);
				string word;
				vector<vector<int>> face;
				string del = "/";
				
				while (line >> word){
					//get every trio of coords
					if (word == "f" or word[0] == '/'){
						continue;
					}
					vector<int> subface;
					int start = 0;
					int end = word.find(del);
					//for each trio break into coords 
					while (end != -1) {
						subface.push_back(stoi(word.substr(start, end - start)));
						start = end + del.size();
						end = word.find(del, start);
					}
					subface.push_back(stoi(word.substr(start, end - start)));
					face.push_back(subface);
				}
				faces.push_back(face);
			}
		}
	}
	void printVertices(){
		for(int i = 0; i < (int)vertices.size(); i++){
			for(int j = 0; j < (int)vertices[i].size(); j++){
				cout << vertices[i][j] << " ";
			}
			cout << endl;
		}
	}
	
	void printFaces(){
		for(int i = 0; i < (int)faces.size(); i++){
			for(int j = 0; j < (int)faces[i].size(); j++){
				cout << "[";
				for(int k = 0; k < (int)faces[i][j].size(); k++){
					cout << faces[i][j][k] << " ";
				}
				cout << "]";
			}
			cout << endl;
		}
	}
};

#endif
