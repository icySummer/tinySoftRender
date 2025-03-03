#pragma once
#include "tgaimage.h"
#include "geometry.h"

extern mat<4,4> ModelView;
extern mat<4,4> Viewport;
extern mat<4,4> Projection;
extern const int width;
extern const int height;

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color);
void viewport(int x, int y, int w, int h);
void projection(const double viewDistance); // coeff = -1/c
void lookat(vec3 eye, vec3 center, vec3 up);

struct IShader {
    virtual vec3 vertex(int iface, int nthvert, vec3& worldPos3) = 0;
    virtual bool fragment(vec3 bar, TGAColor& color) = 0;
};

void triangle(vec3* pts, IShader& shader, TGAImage& image, float* zBuffer);