#pragma once

using namespace std;
using namespace glm;


#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

#define CHARACTRER_SCALE_DEFAULT 0.1f
#define CHARACTRER_SCALE_OUTLINE 0.103f
#define CUBE_SCALE_DEFAULT 1.0f
#define CUBE_SCALE_OUTLINE 1.05f

//之前1万个小行星帧数144，但是10万个小行星帧数只有20多帧了(配置3080 + i7 10700k + 16G)。
//研究发现，本场景中，片段着色器对帧数影响小，优化空间约等于0；面剔除对帧数的影响也基本为0.
//最后发现几何着色器会把图元拆解成顶点，每个顶点都会吊一次片段着色器，最后又要把顶点装配成图元，这里的开销是很大的。
//删除几何着色器后帧数达到了120多帧，优化成功。
// 
// 以前1万個の小惑星では144フレーム/秒だったが、10万個では20フレームまで低下（RTX3080 + i7 10700k + 16G環境）
// 調査結果　本シーンでは：
// ・フラグメントシェーダーのフレームレートへの影響が小さい → 最適化余地ほぼ無し
// ・フェイスカリングの影響もほぼ無し
// ・ジオメトリシェーダーがプリミティブを頂点に分解する処理が原因：
//   各頂点でフラグメントシェーダーを呼び出し、再び頂点をプリミティブに再構築する際のオーバーヘッドが極大
// 　ジオメトリシェーダー削除後、フレームレート120以上を達成 → 最適化成功
#define ROCK_NUM 1000

#define MSAA_SAMPLE_NUM 4

#define ROTATE_SPEED_PLANET 0.1f
#define ROTATE_SPEED_ROCK 0.1f

double preTime = 0.0;
double deltaTime = 0.0;
double curTime = 0.0;

const vec3 lampPos[4] = { vec3(20.0f, 3.0f, 0.0f) , vec3(20.0f, 3.0f, -10.0f) , vec3(20.0f, 3.0f, -20.0f) , vec3(20.0f, 3.0f, -30.0f) };

// 如果用其他变量接收imgui变量，必须保证两者初始值一致，因为imgui收起的状态是不传值的。
// ImGUI変数を受け取る変数は必ず初期値を一致させる必要（ImGUIの折り畳み状態では値が伝達されないため）
/************** Imgui変数 **************/
//float posValue = 0.0f;
vec3 bkgColor = vec3(0.0f, 0.0f, 0.0f);
vec3 dirLight_direction = vec3(-1.0f, -1.0f, -1.0f);
vec3 dirLight_ambient = vec3(0.0f);
vec3 dirLight_diffuse = vec3(0.0f);
vec3 dirLight_specular = vec3(0.0f);
vec3 pointLight_ambient = vec3(0.2f);
vec3 pointLight_diffuse = vec3(0.8f);
vec3 pointLight_specular = vec3(1.0f);
vec3 spotLight_ambient = vec3(0.0f);
vec3 spotLight_diffuse = vec3(0.0f);
vec3 spotLight_specular = vec3(0.0f);
float spotLight_innerCos = 5.0f;
float spotLight_outerCos = 8.0f;
int item = 0;
int material_shininess = 256;
int postProcessType = 0;
float sampleOffsetBase = 300.0f;
float imgui_speed = 100.0f;
float imgui_camNear = 0.1f;
float imgui_camFar = 500.0f;
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
bool bSkyBox = 1;
bool bMSAA = true;
int iLightModel = 1; //{ "Phong", "Blinn-Phong" }
bool bGammaCorrection = true;
int iAtteFormula = 0;
/************** Imgui変数 **************/

