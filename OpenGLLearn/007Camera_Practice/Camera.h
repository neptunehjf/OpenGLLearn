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

	// 对于yaw，camera坐标系的+Z从+X开始逆时针旋转计算的，所以要旋转-90度校正到从+Z开始
	// ヨー角において、カメラ座標系の+Z軸は+X軸基準の反時計回りで計算されるため、+Z軸基準に合わせるには-90度回転で補正が必要
	// 参照Referrence/camera rotate.jpg Referrence/Euler Angle.png
	yawValue = -90.0f; 
	
	fov = 45.0f;

	this->camPos = camPos;
	this->camFront = camFront;
	this->camUp = camUp;
}

void Camera::setCamView()
{
	camPos.y = 0; // 只允许在xz平面移动
	              // xz平面内でのみ移動が許可される

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

	// 旋转矩阵
	// 回転行列
	mat4 rotate = mat4(
		camX.x, camX.y, camX.z, 0.0f,
		camY.x, camY.y, camY.z, 0.0f,
		camZ.x, camZ.y, camZ.z, 0.0f,
		  0.0f,   0.0f,   0.0f, 1.0f
	);
	
	// 位移矩阵
	// 平行移動行列
	//　参照Referrence/Transformation.jpg
	mat4 translate = mat4(
		1.0f, 0.0f, 0.0f, -camPos.x,
		0.0f, 1.0f, 0.0f, -camPos.y,
		0.0f, 0.0f, 1.0f, -camPos.z,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// opengl是用列主序存储矩阵的，要么在构造矩阵时按列主序构造，要么调用transpose把行主序矩阵转成列主序！！！
	// OpenGLは行列を列優先順で保存します。行列を構築する際に列優先順で作成するか、transposeを呼び出して行優先順の行列を列優先順に変換すること！！！
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

