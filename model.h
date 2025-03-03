#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<vec3> verts_;
	std::vector<std::vector<int> > faces_;
	std::vector<std::vector<int> > tex_index_;
	std::vector<std::vector<int> > normal_index_;
	std::vector<vec2> tex_coord_; // per-vertex array of tex coords
	std::vector<vec3> normal_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	vec3 vert(int i);
	vec2 uv(int i);
	vec3 normal(int i);
	std::vector<int> face(int idx);
	std::vector<int> texIndex(int idx);
	std::vector<int> normalIndex(int idx);
};

#endif //__MODEL_H__
