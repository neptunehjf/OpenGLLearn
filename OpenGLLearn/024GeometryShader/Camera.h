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

	float deltaTime; // ��ǰ֡����һ֡��ʱ���
	float lastFrame; // ��һ֡��ʱ��
	float currentFrame; //��ǰ֡ʱ��

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
	camNear = 0.1f;
	camFar = 100.0f;

	deltaTime = 0.0f; 
	lastFrame = 0.0f;  
	currentFrame = 0.0f;  

	lastX = 0.0f;
	lastY = 0.0f;
	pitchValue = -15.0f;
	yawValue = -110.0f; // Ĭ�Ͼ�ͷ����X��������������ת90��У��

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
	front.x = cos(radians(yawValue)) * cos(radians(pitchValue)); // ��Ϊ�ӽ�Ĭ�ϳ���X������������Ӧ������X��������ĽǶȼ���ƫ��
	front.y = sin(radians(pitchValue));
	front.z = sin(radians(yawValue)) * cos(radians(pitchValue)); // ��Ϊ�ӽ�Ĭ�ϳ���X������������Ӧ������X��������ĽǶȼ���ƫ��

	camFront = normalize(front);
}

