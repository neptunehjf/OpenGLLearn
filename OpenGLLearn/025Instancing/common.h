#pragma once

using namespace std;
using namespace glm;


#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

//#define WINDOW_WIDTH 2560
//#define WINDOW_HEIGHT 1335

#define CHARACTRER_SCALE_DEFAULT 0.1f
#define CHARACTRER_SCALE_OUTLINE 0.103f
#define CUBE_SCALE_DEFAULT 1.0f
#define CUBE_SCALE_OUTLINE 1.05f

/************** Imgui变量 **************/
float posValue = 0.0f;
vec3 bkgColor = vec3(0.0f, 0.0f, 0.0f);
vec3 dirLight_direction = vec3(-1.0f, -1.0f, -1.0f);
vec3 dirLight_ambient = vec3(0.2f);
vec3 dirLight_diffuse = vec3(0.8f);
vec3 dirLight_specular = vec3(1.0f);
vec3 pointLight_ambient = vec3(0.2f);
vec3 pointLight_diffuse = vec3(0.8f);
vec3 pointLight_specular = vec3(1.0f);
vec3 spotLight_ambient = vec3(0.2f);
vec3 spotLight_diffuse = vec3(0.8f);
vec3 spotLight_specular = vec3(1.0f);
float spotLight_innerCos = 5.0f;
float spotLight_outerCos = 8.0f;
int item = 0;
int material_shininess = 32;
int postProcessType = 0;
float sampleOffsetBase = 300.0f;
float imgui_speed = 5.0f;
float imgui_camNear = 0.1f;
float imgui_camFar = 100.0f;
float pointSize = 1.0f;
bool bSplitScreen = 0;
float windowWidth = WINDOW_WIDTH;
float windowHeight = WINDOW_HEIGHT;
bool bGMTest = 0;
float explodeMag = 0.0;
bool bBlending = 1;
bool bFaceCulling = 0;
float normalLen = 0.1;
bool bInstanceTest = 0;
/************** Imgui变量 **************/

