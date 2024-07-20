#ifndef CAMERA_H  
#define CAMERA_H

#include "glm/glm.hpp"
#include "glm//gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"


using namespace std;
using namespace glm;

class Camera
{
public:

	Camera(const vec3& camPos, const vec3& camFront, const vec3& camUp);

	float camSpeed;
	vec3 camPos;
	vec3 camFront;
	vec3 camUp;

	float deltaTime; // ��ǰ֡����һ֡��ʱ���
	float lastFrame; // ��һ֡��ʱ��
	float currentFrame; //��ǰ֡ʱ��

	float lastX;
	float lastY;
	bool  isFirst;
	float pitchValue;
	float yawValue;

	float fov;

	void setCamView();
	mat4 getCamView();

	void setCamFov(float fov);
	float getCamFov();

private:
	mat4 view;
};

Camera::Camera(const vec3& camPos, const vec3& camFront, const vec3& camUp)
{
	camSpeed = 5.0f;

	deltaTime = 0.0f; 
	lastFrame = 0.0f;  
	currentFrame = 0.0f;  

	lastX = 0.0f;
	lastY = 0.0f;
	isFirst = true;
	pitchValue = 0.0f;
	yawValue = -90.0f; // Ĭ�Ͼ�ͷ����X��������������ת90��У��

	fov = 45.0f;

	this->camPos = camPos;
	this->camFront = camFront;
	this->camUp = camUp;
}

void Camera::setCamView()
{
	camPos.y = 0; // ֻ������xzƽ���ƶ�

	vec3 camZ = normalize(-camFront);
	vec3 camX = normalize(cross(normalize(camUp), camZ));
	vec3 camY = normalize(cross(camZ, camX));

	//mat4 rotate = mat4(
	//	camX.x, camY.x, camZ.x, 0.0f,
	//	camX.y, camY.y, camZ.y, 0.0f,
	//	camX.z, camY.z, camZ.z, 0.0f,
	//	0.0f, 0.0f, 0.0f, 1.0f
	//);
	//mat4 translate = mat4(
	//	1.0f, 0.0f, 0.0f, 0.0f,
	//	0.0f, 1.0f, 0.0f, 0.0f,
	//	0.0f, 0.0f, 1.0f, 0.0f,
	//	-camPos.x, -camPos.y, -camPos.z, 1.0f
	//);

	mat4 rotate = mat4(
		camX.x, camX.y, camX.z, 0.0f,
		camY.x, camY.y, camY.z, 0.0f,
		camZ.x, camZ.y, camZ.z, 0.0f,
		  0.0f,   0.0f,   0.0f, 1.0f
	);
	mat4 translate = mat4(
		1.0f, 0.0f, 0.0f, -camPos.x,
		0.0f, 1.0f, 0.0f, -camPos.y,
		0.0f, 0.0f, 1.0f, -camPos.z,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// glm��opengl������������洢����ģ�Ҫô�ڹ������ʱ���������죬Ҫô����transpose�����������ת�������򣡣���
	rotate = transpose(rotate);
	translate = transpose(translate);

	view = rotate * translate;

	//view = lookAt(camPos, camPos + camFront, camUp);
	//for (int row = 0; row < 4; row++) {
	//	for (int col = 0; col < 4; col++) {
	//		cout << view[row][col] << " ";
	//	}
	//	std::cout << std::endl;
	//}

	//cout << "1" << endl;
}

mat4 Camera::getCamView()
{
	return view;
}

void Camera::setCamFov(float fov)
{
	this->fov = fov;
}

float Camera::getCamFov()
{
	return fov;
}
#endif // !CAMERA_H

