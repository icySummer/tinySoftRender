#include <limits>
#include "our_gl.h"

mat<4, 4> ModelView;
mat<4, 4> Viewport;
mat<4, 4> Projection;

const int DEPTH = 255;

//绘制线段
void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
{
	bool steep = false; //陡峭情况

	int dy = std::abs(y1 - y0);
	int dx = std::abs(x1 - x0);

	if (dy > dx)
	{
		//按x=y这条直线镜像处理线段
		std::swap(x0, y0);
		std::swap(x1, y1);
		std::swap(dx, dy);
		steep = true;
	}

	if (x0 > x1) //左右方向
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}


	int derror2 = std::abs(dy) * 2;
	int error2 = 0;

	int y = y0;

	for (int x = x0; x <= x1; x++)
	{
		if (error2 > dx)
		{
			y += y0 < y1 ? 1 : -1; //上下方向
			error2 -= 2 * dx;
		}

		if (steep)
		{
			image.set(y, x, color);
		}
		else
		{
			image.set(x, y, color);
		}

		error2 += derror2;
	}
}

void viewport(int x, int y, int w, int h)
{
    Viewport = mat<4, 4> { vec4(w / 2, 0, 0, x + w / 2), vec4(0, h / 2, 0, y + h / 2),
        vec4(0, 0, DEPTH / 2, DEPTH / 2), vec4(0, 0, 0, 1) };
}

void projection(const double viewDistance) {
    Projection = { vec4(1.,0.,0.,0.), vec4(0.,1.,0.,0.), vec4(0.,0.,1.,0), vec4(0.,0.,-1. / viewDistance, 1.) };
	//Projection = { vec4(1.,0.,0.,0.), vec4(0.,1.,0.,0.), vec4(0.,0.,1.,0), vec4(0.,0.,0., 1.) };
}

void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    double commonValue = sqrtl(pow(eye.x, 2) + pow(eye.z, 2));
    double cosA = eye.x / commonValue;
    double sinA = eye.z / commonValue;
    double cosB = eye.y / commonValue;
    vec3 z = vec3(-cosA, -cosB, sinA);
    vec3 x = cross(up, z).normalize();
    vec3 y = cross(z, x).normalize();
    mat<4, 4> rotMat = { vec4(x.x, y.x, z.x , 0), vec4(x.y, y.y, z.y, 0),
        vec4(x.z, y.z, z.z, 0), vec4(0,0,0,1) };
    mat<4, 4> moveMat = { vec4(1, 0, 0, -center.x), vec4(0, 1, 0, -center.y),
        vec4(0, 0, 1, 0), vec4(0, 0, 0, 1) };
    ModelView = rotMat * moveMat;
}

vec3 barycentric(vec3* pts, vec2 P) {
    vec3 result = cross(vec3((pts[1] - pts[0]).x, (pts[2] - pts[0]).x, pts[0].x - P.x),
        vec3((pts[1] - pts[0]).y, (pts[2] - pts[0]).y, pts[0].y - P.y));
    if (std::abs(result.z) < 1) //退化的三角形，扔掉
        return vec3(-1, 1, 1);
    return vec3(1 - (result.x + result.y) / result.z, result.x / result.z, result.y / result.z);
}

void triangle(vec3* pts, IShader& shader, TGAImage& image, float* zBuffer) 
{
	//构造一个包围盒
	vec2 leftBottom = vec2(pts[0].x, pts[0].y);
	vec2 rightTop = vec2(0, 0);

	for (int i = 0; i < 3; i++)
	{
		vec3 curPoint = pts[i];
		leftBottom.x = std::min(curPoint.x, leftBottom.x);
		leftBottom.y = std::min(curPoint.y, leftBottom.y);

		rightTop.x = std::max(curPoint.x, rightTop.x);
		rightTop.y = std::max(curPoint.y, rightTop.y);

		//屏幕裁剪三角形
		leftBottom.x = std::max(0., leftBottom.x);
		leftBottom.y = std::max(0., leftBottom.y);

		rightTop.x = std::min(rightTop.x, (double)width);
		rightTop.y = std::min(rightTop.y, (double)height);
	}

	//遍历包围盒的像素，判断每个像素是否在三角形内，是的话就绘制像素
	for (int x = leftBottom.x; x <= rightTop.x; x++)
	{
		for (int y = leftBottom.y; y <= rightTop.y; y++)
		{
			vec3 bar = barycentric(pts, vec2(x, y));
			if (bar.x < 0 || bar.y < 0 || bar.z < 0) continue;

			float z = bar.x * pts[0].z + bar.y * pts[1].z + bar.z * pts[2].z;

			int idx = x + y * width;

			if (zBuffer[idx] < z)
			{
				zBuffer[idx] = z;

				TGAColor color;
				bool result = shader.fragment(bar, color);

				if (result)
				{
					continue;
				}
				else
				{
					image.set(x, y, color);
				}

			}
		}
	}

}

