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

	float deltaTime; // 当前帧与上一帧的时间差
					 // フレーム間時間差
	float lastFrame; // 上一帧的时间
					 // 前フレーム時刻
	float currentFrame; //当前帧时间
						//現在フレーム時刻
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

	// 默认镜头朝向X正方向，所以向左转90度校正 (朝向-Z)
	// デフォルトのカメラ方向が+X軸方向を向いているため、左回りに90度補正 (-Zに向かう)
	// 参照 Referrence/Euler Angle.png
	yawValue = -90.0f; 
	
	fov = 45.0f;

	this->camPos = camPos;
	this->camFront = camFront;
	this->camUp = camUp;
}

void Camera::setCamView()
{
	view = lookAt(camPos, camPos + camFront, camUp);
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

