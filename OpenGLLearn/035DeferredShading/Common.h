#pragma once

using namespace std;
using namespace glm;


#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

#define CHARACTRER_SCALE_DEFAULT 0.1f
#define CHARACTRER_SCALE_OUTLINE 0.103f
#define CUBE_SCALE_DEFAULT 1.0f
#define CUBE_SCALE_OUTLINE 1.05f

//֮ǰ1���С����֡��144������10���С����֡��ֻ��20��֡��(3080 + i7 10700k + 16G)��
//�о����֣��������У�Ƭ����ɫ����֡��Ӱ��С���Ż��ռ�Լ����0�����޳���֡����Ӱ��Ҳ����Ϊ0.
//����ּ�����ɫ�����ͼԪ���ɶ��㣬ÿ�����㶼���һ��Ƭ����ɫ���������Ҫ�Ѷ���װ���ͼԪ������Ŀ����Ǻܴ�ġ�
//ɾ��������ɫ����֡���ﵽ��120��֡���Ż��ɹ���
#define ROCK_NUM 1000

#define MSAA_SAMPLE_NUM 4

#define ROTATE_SPEED_PLANET 0.1f
#define ROTATE_SPEED_ROCK 0.1f

#define SHADOW_RESOLUTION_WIDTH 1024
#define SHADOW_RESOLUTION_HEIGHT 1024

double preTime = 0.0;
double deltaTime = 0.0;
double curTime = 0.0;

const vec3 lampPos[4] = { vec3(20.0f, 3.0f, 0.0f) , vec3(20.0f, 3.0f, -10.0f) , vec3(20.0f, 3.0f, -20.0f) , vec3(20.0f, 3.0f, -30.0f) };

float windowWidth = WINDOW_WIDTH;
float windowHeight = WINDOW_HEIGHT;

/************** Imgui���� **************/
//float posValue = 0.0f;
vec3 bkgColor = vec3(0.0f, 0.0f, 0.0f);
vec3 dirLight_direction = vec3(-5.0f, -5.0f, -5.0f);
vec3 dirLight_ambient = vec3(0.0f);
vec3 dirLight_diffuse = vec3(0.0f);
vec3 dirLight_specular = vec3(0.0f);
vec3 pointLight_ambient = vec3(0.0f);
vec3 pointLight_diffuse = vec3(30.0f);
vec3 pointLight_specular = vec3(0.0f);
vec3 spotLight_ambient = vec3(0.0f);
vec3 spotLight_diffuse = vec3(0.0f);
vec3 spotLight_specular = vec3(0.0f);
float spotLight_innerCos = 5.0f;
float spotLight_outerCos = 8.0f;
int item = 0;
int material_shininess = 256;
int postProcessType = 0;
float sampleOffsetBase = 300.0f;
float imgui_speed = 10.0f;
float imgui_camNear = 0.1f;
float imgui_camFar = 500.0f;
float pointSize = 1.0f;
bool bSplitScreen = 0;
bool bGMTest = 0;
float explodeMag = 0.0;
bool bBlending = true;
bool bFaceCulling = false;
float normalLen = 0.1;
bool bInstanceTest = 0;
bool bSkyBox = 0;
bool bMSAA = true;
int iLightModel = 1; //{ "Phong", "Blinn-Phong" }
bool bGammaCorrection = true;
int iAtteFormula = 0;
bool bShadow = true;
bool bDisDepthmap = false;
bool bFrontFaceCulling = false;
vec3 lampWithShadowPos = { 0.0f, 3.0f, 0.0f };
bool bDepthCubemapDebug = false;
float fBiasDirShadow = 0.005f;
float fBiasPtShadow = 0.5f;
float fFarPlanePt = 20.0f;
bool bEnableNormalMap = true;
bool bEnableParallaxMap = true;
bool bDebug = false;
float height_scale = 0.1f;
int iParaAlgo = 2; // { "Parallax Mapping", "Steep Parallax Mapping", "Parallax Occlusion Mapping" }
bool bHDR = true;
float fExposure = 0.3;
int iHDRAlgro = 1; // { "reinhard tone mapping", "exposure tone mapping" }
bool bBloom = false;
bool bDeferred = true;
/************** Imgui���� **************/

