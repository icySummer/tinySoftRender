#include "model.h"
#include "tgaimage.h"
#include "our_gl.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model* model = NULL;
const int width = 800;
const int height = 800;
const int TEX_WIDTH = 1024;
const int TEX_HEIGHT = 1024;
TGAImage diffuseTex(TEX_WIDTH, TEX_HEIGHT, TGAImage::RGB);
TGAImage normalObjTex(TEX_WIDTH, TEX_HEIGHT, TGAImage::RGB);
TGAImage normalTanTex(TEX_WIDTH, TEX_HEIGHT, TGAImage::RGB);
TGAImage specTex(TEX_WIDTH, TEX_HEIGHT, TGAImage::RGB);
vec3 directLight = vec3(0, 0, -1.3).normalize();
vec3 backCullDir = vec3(0, 0, -1);
vec3 eyePoint = vec3(-1, 0.2, 2);
vec3 centerPoint = vec3(0, 0, 0);
vec3 eyeDir = (centerPoint - eyePoint).normalize();
const double M_PI = 3.14159;

struct ShadowMapShader : public IShader {
	mat<4, 4>& MVP_shadowSpace_;

	ShadowMapShader(mat<4, 4>& MVP_shadowSpace) : MVP_shadowSpace_(MVP_shadowSpace) {}

	virtual vec3 vertex(int iface, int nthvert, vec3& projPos3)
	{
		vec3 modelPos3;
		vec4 modelPos4;
		vec4 worldPos4;
		vec4 viewportPos4;
		vec4 projPos4;
		std::vector<int> face = model->face(iface);
		modelPos3 = model->vert(face[nthvert]);
		modelPos4 = vec4(modelPos3.x, modelPos3.y, modelPos3.z, 1.);

		worldPos4 = ModelView * modelPos4;
		projPos4 = Projection * worldPos4;
		projPos4 = projPos4 / projPos4.w;
		viewportPos4 = Viewport * projPos4;
		MVP_shadowSpace_ = Viewport * Projection * ModelView;
		projPos3 = proj<3>(projPos4);

		return vec3(viewportPos4.x, viewportPos4.y, viewportPos4.z);
	}

	virtual bool fragment(vec3 bar, TGAColor& color)
	{
		return true;
	}
};

struct PhongShader : public IShader {
	mat<3,2> varying_tex;
	vec3 modelPos3_[3];
	//vec3 varying_intensity;
	mat<3,3> varying_normal;
	mat<4, 4> MVP_;
	mat<4, 4>& MVP_shadowSpace_;
	mat<4, 3> varying_screenPos;
	float* zBuffer_shadowSpace_;
	double projPos4w = 1.;

	PhongShader(mat<4, 4>& MVP_shadowSpace, float* zBuffer_shadowSpace) : 
		MVP_shadowSpace_(MVP_shadowSpace), zBuffer_shadowSpace_(zBuffer_shadowSpace) {}

	virtual vec3 vertex(int iface, int nthvert, vec3& projPos3)
	{
		vec3 modelPos3;
		vec4 modelPos4;
		vec4 worldPos4;
		vec4 viewportPos4;
		vec4 projPos4;
		std::vector<int> face = model->face(iface);
		modelPos3 = model->vert(face[nthvert]);
		modelPos3_[nthvert] = modelPos3; 
		modelPos4 = vec4(modelPos3.x, modelPos3.y, modelPos3.z, 1.);

		worldPos4 = ModelView * modelPos4;
		projPos4 = Projection * worldPos4;
		projPos4w = projPos4.w;
		projPos4 = projPos4 / projPos4.w;
		viewportPos4 = Viewport * projPos4;
		MVP_ = Viewport * Projection * ModelView;
		projPos3 = proj<3>(projPos4);

		std::vector<int> texIndex = model->texIndex(iface);
		varying_tex[nthvert] = model->uv(texIndex[nthvert]);
		std::vector<int> normalIndex = model->normalIndex(iface);
 		vec3 normal = model->normal(normalIndex[nthvert]).normalize();
		//normal = vec3((normal.x + 1) / 2, (normal.y + 1) / 2, (normal.z + 1) / 2);
		varying_normal.set_col(nthvert, normal);
		//double intensity = normal * directLight * -1;
		//varying_intensity[nthvert] = intensity;

		varying_screenPos.set_col(nthvert, viewportPos4);

		return vec3(viewportPos4.x, viewportPos4.y, viewportPos4.z);
	}

	virtual bool fragment(vec3 bar, TGAColor& color) 
	{
		float u = bar.x * varying_tex[0].x + bar.y * varying_tex[1].x + bar.z * varying_tex[2].x;
		float v = bar.x * varying_tex[0].y + bar.y * varying_tex[1].y + bar.z * varying_tex[2].y;
		TGAColor texColor = diffuseTex.get(u * TEX_WIDTH, v * TEX_HEIGHT);
		TGAColor normalObjColor = normalObjTex.get(u * TEX_WIDTH, v * TEX_HEIGHT);
		//normalObjColor = TGAColor(0, 0, 255, 255);
		vec3 normalObj = vec3(normalObjColor.bgra[2] / 255.*2-1, normalObjColor.bgra[1] / 255. * 2 - 1, normalObjColor.bgra[0] / 255. * 2 - 1).normalize();
		TGAColor normalTanColor = normalTanTex.get(u * TEX_WIDTH, v * TEX_HEIGHT);
		//normalTanColor = TGAColor(0, 0, 255, 255);
		vec3 normalTan = vec3(normalTanColor.bgra[2]/255.*2-1, normalTanColor.bgra[1] / 255. * 2 - 1, normalTanColor.bgra[0] / 255. * 2 - 1).normalize();

		TGAColor specColor = specTex.get(u * TEX_WIDTH, v * TEX_HEIGHT);

		//modelPos3_[0].z = 0;
		//modelPos3_[1].z = 0;
		//modelPos3_[2].z = 0;

		vec3 e1 = (modelPos3_[1] - modelPos3_[0]);
		vec3 e2 = (modelPos3_[2] - modelPos3_[0]);
		vec3 en = varying_normal * bar;

		mat<3, 3> uvMat = { e1, e2, en };
		uvMat = uvMat.invert();

		vec3 T = (uvMat * vec3(varying_tex[1][0] - varying_tex[0][0], varying_tex[2][0] - varying_tex[0][0], 0));
		vec3 B = (uvMat * vec3(varying_tex[1][1] - varying_tex[0][1], varying_tex[2][1] - varying_tex[0][1], 0) * -1);
		vec3 N = cross(T, B);

		//mat<3, 3> normalTanToObj = { vec3(T.x, T.y, T.z), vec3(B.x, B.y, B.z), vec3(N.x, N.y, N.z)};
		mat<3, 3> normalTanToObj = { T.normalize(), B.normalize(), N.normalize()};
		normalTanToObj = normalTanToObj.transpose();
		//mat<3, 3> invertMat = normalTanToObj.invert();
		
		vec3 normal = (normalTanToObj * normalTan).normalize();
		vec4 normal4 = ModelView.invert_transpose() * vec4(normal.x, normal.y, normal.z, 0);
		normal = vec3(normal4.x, normal4.y, normal4.z);
		//std::cout << "normal " << normal.x << "," << normal.y << ", " << normal.z << std::endl;
		//normal = vec3((normal.x + 1) / 2, (normal.y + 1) / 2, (normal.z + 1) / 2);
		//normal = normalObj.normalize();
		//normal = en.normalize();

		double env = 12;
		double diff = std::max(0., std::min(1., normal * directLight * -1));
		vec3 e = eyeDir.normalize() * -1;
		vec3 l = directLight.normalize() * -1;
		double cosLN = l * normal;
		vec3 r = 2 * cosLN * normal - l;
		const double power = specColor.bgra[0];
		double specular =  std::max(0., std::pow( std::max(0.,(e * r)), power));

		//将正常渲染下的屏幕坐标转到阴影缓冲区的屏幕坐标
		vec4 screenPos = varying_screenPos * bar;
		screenPos = (Viewport.invert() * screenPos) * projPos4w;
		vec4 test = MVP_.invert() * screenPos;
		vec4 screenPos_shadowSpace = MVP_shadowSpace_* (Projection*ModelView).invert() * screenPos;
		screenPos_shadowSpace = screenPos_shadowSpace / screenPos_shadowSpace.w;
		screenPos_shadowSpace.y = std::max(0, std::min(height, (int)screenPos_shadowSpace.y));
		screenPos_shadowSpace.x = std::max(0, std::min(width, (int)screenPos_shadowSpace.x));
		float curZbuffer_shadowSpace = zBuffer_shadowSpace_[(int)screenPos_shadowSpace.y * width + (int)screenPos_shadowSpace.x];
		float curZ = screenPos_shadowSpace.z;
		bool isShadow = curZ < (curZbuffer_shadowSpace - 10);
		double intensity = (isShadow ? 0.2 : (diff + 0.6 * specular));


		//color = TGAColor(
		//	std::min(255., env + texColor.bgra[2] * intensity),
		//	std::min(255., env + texColor.bgra[1] * intensity),
		//	std::min(255., env + texColor.bgra[0] * intensity),
		//	255
		//);
		
		color = TGAColor(
			std::min(255., env + texColor.bgra[2] * intensity),
			std::min(255., env + texColor.bgra[1] * intensity),
			std::min(255., env + texColor.bgra[0] * intensity),
			255
		);
	

		//color = TGAColor(texColor.bgra[2] * diff, texColor.bgra[1] * diff, texColor.bgra[0] * diff, 255);
		//TGAColor debugColor = TGAColor(normalTan.x*255, normalTan.y*255, normalTan.z*255, 255);
		//TGAColor debugColor = TGAColor((normal.x+1)/2 * 255, (normal.y + 1) / 2 * 255, (normal.z + 1) / 2 * 255, 255);
		//debugColor = TGAColor(intensity * 255, intensity * 255, intensity * 255, 255);
		//color = debugColor;
		//color = normalTanColor;

		return false;
	}
};

void renderBufferToImage(float* buffer, TGAImage& image, int imageWidth = width, int imageHeight = height)
{
	for (int idx = 0; idx < imageWidth * imageHeight; ++idx)
	{
		int x = idx % width;
		int y = idx / width;
		float value = buffer[idx];
		image.set(x, y, TGAColor(value, value, value, 255));
	}
}

double max_elevation_angle(int x, int y, float* zBuffer, double angle)
{
	vec2 dir = vec2(cos(angle), sin(angle)).normalize();
	vec2 center = vec2(x, y);
	double maxAngle = 0;
	for (int i = 1; i < 100; i++)
	{
		vec2 targetPoint = center + dir * i;
		if (targetPoint.x > width || targetPoint.x < 0 || targetPoint.y > height || targetPoint.y < 0) continue;
		double centerZBuffer = zBuffer[(int)center.x + ((int)center.y) * width];
		if (centerZBuffer < 0.01) continue;
		double heightDistance = zBuffer[(int)targetPoint.x + ((int)targetPoint.y) * width] - centerZBuffer;
		double horizontalDistance = (targetPoint - center).norm();
		maxAngle = std::max(maxAngle, atan2(heightDistance, horizontalDistance));
	}
	return maxAngle;
}

int main(int args, char** argv) 
{
	//解析模型文件
	model = new Model("模型存放相对路径");
	

	//读取纹理贴图
	diffuseTex.read_tga_file("纹理贴图存放相对路径");
	//读取模型空间下的法线贴图
	normalObjTex.read_tga_file("法线贴图存放相对路径");
	normalTanTex.read_tga_file("法线贴图存放相对路径");
	specTex.read_tga_file("法线贴图存放相对路径");

	//定义TGA格式的图像
	TGAImage image(width, height, TGAImage::RGB);

	//深度图
	TGAImage zBufferImage(width, height, TGAImage::RGB);

	//画点
	//image.set(400, 400, red);

	//画线
	//左右方向、上下方向、陡峭情况
	//line(0, 0, 80, 40, image, white); 
	//line(80, 40, 0, 0, image, white); 
	//line(0, 0, 40, 80, image, red);   
	//line(40, 80, 0, 0, image, red);   
	//line(0, 40, 80, 0, image, white);

	float* zBuffer_shadowSpace = new float[width * height];
	float cosAngle;
	float viewDistance;
	mat<4, 4> MVP_shadowSpace;

#pragma region 渲染到阴影贴图

//光照着色模型
	cosAngle = 0;

	//初始化zBuffer
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			zBuffer_shadowSpace[i + j * width] = -std::numeric_limits<float>::max();
		}
	}

	//照相机的透视投影距离
	viewDistance = 5.;

	projection(viewDistance);
	lookat(directLight * -1, centerPoint, vec3(0, 1, 0));
	//lookat(vec3(0, 0, 2), vec3(0, 0, 0), vec3(0, 1, 0));
	viewport(0, 0, width, height);

	//vec4 test = viewMat * vec4(0,0,1,1);

	ShadowMapShader shadowMapShader(MVP_shadowSpace);
	for (int i = 0; i < model->nfaces(); i++) {
		vec3 screenPos[3];
		vec3 worldPos3[3];
		for (int j = 0; j < 3; j++)
		{
			screenPos[j] = shadowMapShader.vertex(i, j, worldPos3[j]);
		}

		vec3 line1 = worldPos3[1] - worldPos3[0];
		vec3 line2 = worldPos3[2] - worldPos3[0];
		vec3 normalLine = cross(line2, line1);
		normalLine.normalize();

		cosAngle = normalLine * backCullDir;
		if (cosAngle < 0)
			continue;

		triangle(screenPos, shadowMapShader, image, zBuffer_shadowSpace);
		//break;
	}

	//线框渲染模型
	//for (int i = 0; i < model->nfaces(); i++) {
	//	std::vector<int> face = model->face(i);
	//	for (int j = 0; j < 3; j++) {
	//		vec3 v0 = model->vert(face[j]);
	//		vec3 v1 = model->vert(face[(j + 1) % 3]);
	//		int x0 = (v0.x + 1.f) / 2.f * width;
	//		int y0 = (v0.y + 1.f) / 2.f * height;
	//		int x1 = (v1.x + 1.f) / 2.f * width;
	//		int y1 = (v1.y + 1.f) / 2.f * height;
	//		line(x0, y0, x1, y1, image, white);
	//	}
	//}

	//image.write_tga_file("output.tga");
	//renderBufferToImage(zBuffer_shadowSpace, zBufferImage);
	//zBufferImage.write_tga_file("zBuffer.tga");


#pragma endregion

#pragma region 正常渲染
	image = TGAImage(width, height, TGAImage::RGB);

	//光照着色模型
	cosAngle = 0;
	float* zBuffer = new float[width * height];

	//初始化zBuffer
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			zBuffer[i + j * width] = -std::numeric_limits<float>::max();
		}
	}

	//照相机的透视投影距离
	viewDistance = 5.;

	projection(viewDistance);
	lookat(eyePoint, centerPoint, vec3(0, 1, 0));
	//lookat(vec3(0, 0, 2), vec3(0, 0, 0), vec3(0, 1, 0));
	viewport(0, 0, width, height);

	//vec4 test = viewMat * vec4(0,0,1,1);

	PhongShader gouraudShader(MVP_shadowSpace, zBuffer_shadowSpace);
	for (int i = 0; i < model->nfaces(); i++) {
		vec3 screenPos[3];
		vec3 worldPos3[3];
		for (int j = 0; j < 3; j++)
		{
			screenPos[j] = gouraudShader.vertex(i, j, worldPos3[j]);
		}

		vec3 line1 = worldPos3[1] - worldPos3[0];
		vec3 line2 = worldPos3[2] - worldPos3[0];
		vec3 normalLine = cross(line2, line1);
		normalLine.normalize();

		cosAngle = normalLine * backCullDir;
		if (cosAngle < 0)
			continue;

		triangle(screenPos, gouraudShader, image, zBuffer);
		//break;
	}

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			double sumAngle = 0.;
			for (double angle = 0; angle < 2 * M_PI + 0.001; angle += M_PI / 4)
			{
				sumAngle = max_elevation_angle(i, j, zBuffer, angle);
			}
			double averAngle = sumAngle / 8;
			double occPercent = 1 - averAngle / (M_PI / 2);  //越遮蔽，occPercent越小
			occPercent = pow(occPercent, 2);
			//occPercent = 1.;
			double occColor = occPercent * 255;
			zBufferImage.set(i, j, TGAColor(occColor, occColor, occColor, 255));
			TGAColor noEnvColor = image.get(i, j);
			TGAColor envColor = TGAColor(
				std::max(0., std::min(255., noEnvColor.bgra[2] - (255 - occColor))),
				std::max(0., std::min(255., noEnvColor.bgra[1] - (255 - occColor))),
				std::max(0., std::min(255., noEnvColor.bgra[0] - (255 - occColor))),
				255);
			image.set(i, j, envColor);
		}
	}

	//线框渲染模型
	//for (int i = 0; i < model->nfaces(); i++) {
	//	std::vector<int> face = model->face(i);
	//	for (int j = 0; j < 3; j++) {
	//		vec3 v0 = model->vert(face[j]);
	//		vec3 v1 = model->vert(face[(j + 1) % 3]);
	//		int x0 = (v0.x + 1.f) / 2.f * width;
	//		int y0 = (v0.y + 1.f) / 2.f * height;
	//		int x1 = (v1.x + 1.f) / 2.f * width;
	//		int y1 = (v1.y + 1.f) / 2.f * height;
	//		line(x0, y0, x1, y1, image, white);
	//	}
	//}

	//renderBufferToImage(zBuffer, zBufferImage);
	zBufferImage.write_tga_file("zBuffer.tga");
	image.write_tga_file("output.tga");

 	delete[] zBuffer;
#pragma endregion


	delete[] zBuffer_shadowSpace;
	delete model;
	return 0;
}