#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "namespace.h"

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
	float pitchValue;
	float yawValue;

	float fov;

	void setCamView();
	mat4 getCamView();

	void setCamFov(float fov);
	float getCamFov();

	void setCamFront();

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
	pitchValue = -35.0f;
	// 对于yaw，camera坐标系的+Z从+X开始逆时针旋转计算的，所以要旋转-90度校正到从+Z开始
	// ヨー角において、カメラ座標系の+Z軸は+X軸基準の反時計回りで計算されるため、+Z軸基準に合わせるには-90度回転で補正が必要
	// 参照Referrence/camera rotate.jpg Referrence/Euler Angle.png
	yawValue = -110.0f;

	fov = 45.0f;

	this->camPos = camPos;
	this->camFront = camFront;
	this->camUp = camUp;
}

void Camera::setCamView()
{
	setCamFront();
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

void Camera::setCamFront()
{
	// 相机旋转
	// カメラ回転
	// 对于yaw，设camera坐标系的+Z从+X开始逆时针旋转计算
	// ヨー角計算時、カメラ座標系の+Z方向は+X軸を起点とする反時計回り回転として定義されます
	// 参照Referrence/camera rotate.jpg Referrence/Euler Angle.png
	vec3 front;
	front.x = cos(radians(yawValue)) * cos(radians(pitchValue)); // 因为视角默认朝向X轴正方向，所以应该用与X轴正方向的角度计算偏移
	front.y = sin(radians(pitchValue));
	front.z = sin(radians(yawValue)) * cos(radians(pitchValue)); // 因为视角默认朝向X轴正方向，所以应该用与X轴正方向的角度计算偏移

	camFront = normalize(front);
}

