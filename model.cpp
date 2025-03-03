#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            vec3 v;
            double temp[3];
            for (int i=0;i<3;i++) iss >> temp[i];
            v = vec3(temp[0], temp[1], temp[2]);
            verts_.push_back(v);
        }
        else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            vec2 uv;
            double temp[2];
            for (int i = 0; i < 2; i++) iss >> temp[i];
            uv = vec2(temp[0], temp[1]);
            tex_coord_.push_back({ uv.x, 1 - uv.y });
        }
        else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            vec3 normal;
            double temp[3];
            for (int i = 0; i < 3; i++) iss >> temp[i];
            normal = vec3(temp[0], temp[1], temp[2]);
            normal_.push_back(normal);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;
            std::vector<int> t;
            std::vector<int> n;
            int itrash, idx, texIndex, normalIndex;
            iss >> trash;
            while (iss >> idx >> trash >> texIndex >> trash >> normalIndex) {
                idx--; // in wavefront obj all indices start at 1, not zero
                f.push_back(idx);
                texIndex--;
                t.push_back(texIndex);
                normalIndex--;
                n.push_back(normalIndex);
            }
            faces_.push_back(f);
            tex_index_.push_back(t);
            normal_index_.push_back(n);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

std::vector<int> Model::texIndex(int idx) {
    return tex_index_[idx];
}

std::vector<int> Model::normalIndex(int idx) {
    return normal_index_[idx];
}

vec3 Model::vert(int i) {
    return verts_[i];
}

vec2 Model::uv(int i) {
    return tex_coord_[i];
}

vec3 Model::normal(int i) {
    return normal_[i];
}