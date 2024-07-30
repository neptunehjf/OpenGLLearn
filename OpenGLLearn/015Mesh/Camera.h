#ifndef CAMERA_H  
#define CAMERA_H

#include "glm/glm.hpp"
#include "glm//gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"


class Camera
{
public:

	Camera(const glm::vec3& camPos, const glm::vec3& camFront, const glm::vec3& camUp);

	float camSpeed;
	glm::vec3 camPos;
	glm::vec3 camFront;
	glm::vec3 camUp;

	float deltaTime; // ��ǰ֡����һ֡��ʱ���
	float lastFrame; // ��һ֡��ʱ��
	float currentFrame; //��ǰ֡ʱ��

	float lastX;
	float lastY;
	float pitchValue;
	float yawValue;

	float fov;

	void setCamView();
	glm::mat4 getCamView();

	void setCamFov(float fov);
	float getCamFov();

	void setCamFront();

private:
	glm::mat4 view;
};

Camera::Camera(const glm::vec3& camPos, const glm::vec3& camFront, const glm::vec3& camUp)
{
	camSpeed = 5.0f;

	deltaTime = 0.0f; 
	lastFrame = 0.0f;  
	currentFrame = 0.0f;  

	lastX = 0.0f;
	lastY = 0.0f;
	pitchValue = -35.0f;
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

glm::mat4 Camera::getCamView()
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
	glm::vec3 front;
	front.x = cos(glm::radians(yawValue)) * cos(glm::radians(pitchValue)); // ��Ϊ�ӽ�Ĭ�ϳ���X������������Ӧ������X��������ĽǶȼ���ƫ��
	front.y = sin(glm::radians(pitchValue));
	front.z = sin(glm::radians(yawValue)) * cos(glm::radians(pitchValue)); // ��Ϊ�ӽ�Ĭ�ϳ���X������������Ӧ������X��������ĽǶȼ���ƫ��

	camFront = normalize(front);
}

#endif // !CAMERA_H

