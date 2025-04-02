#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "common.h"

class Camera
{
public:
	Camera() {};
	Camera(const vec3& camPos, const vec3& camFront, const vec3& camUp);

	float camSpeed;
	float camNear;
	float camFar;
	vec3 camPos;
	vec3 camFront;
	vec3 camUp;

	float deltaTime; // 当前帧与上一帧的时间差
	float lastFrame; // 上一帧的时间
	float currentFrame; //当前帧时间

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
	camSpeed = imgui_speed;
	camNear = imgui_camNear;
	camFar = imgui_camFar;

	deltaTime = 0.0f; 
	lastFrame = 0.0f;  
	currentFrame = 0.0f;  

	lastX = 0.0f;
	lastY = 0.0f;
	pitchValue = -45.0f;
	yawValue = -87.0f; // 默认镜头朝向X正方向，所以向左转90度校正
	 
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
	vec3 front;
	front.x = cos(radians(yawValue)) * cos(radians(pitchValue)); // 因为视角默认朝向X轴正方向，所以应该用与X轴正方向的角度计算偏移
	front.y = sin(radians(pitchValue));
	front.z = sin(radians(yawValue)) * cos(radians(pitchValue)); // 因为视角默认朝向X轴正方向，所以应该用与X轴正方向的角度计算偏移

	camFront = normalize(front);
}

