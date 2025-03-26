#include <glad/glad.h>
#include <glfw/glfw3.h>


#include <iostream>

#include "VertextData.h"
#include "Shader.h"

using namespace std;

#define WIDTH 800
#define HEIGHT 600
#define LOG_LENGTH 512

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);


int main()
{
	int success = 0;
	char infoLog[LOG_LENGTH] = "\0";

	// 初始化
	// 初期化
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 创建窗口
	// ウィンドウを作成する
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenglWindow", NULL, NULL);

	if (window == NULL)
	{
		cout << "Failed to create window." << endl;
		glfwTerminate();
		return -1;
	}
	
	// 窗口的所有像素可以看作在一个framebuffer内。窗口移动，大小等变化时，调用framebuffer_size_callback。
	// 「ウィンドウの全画素はフレームバッファ内に配置されているとみなされます。
	// ウィンドウの移動・サイズ変更などの状態変化が発生した際、framebuffer_size_callback が自動的に呼び出されます。」
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// 绘制是基于窗口的，要先设置上下文到窗口(glfwMakeContextCurrent)才能调用gladLoadGLLoader，
	// 否则GLAD会初始化失败(无法正确加载opengl函数地址)
	// 「レンダリングはウィンドウベースで行われるため、gladLoadGLLoaderを呼び出す前に、
	//  必ずウィンドウコンテキストを設定（glfwMakeContextCurrent）する必要があります。
	//  これを怠るとGLADの初期化が失敗します(OpenGL関数のアドレスを正しく取得できません)。」
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialze GLAD." << endl;
		glfwTerminate();
		return -1;
	}

	// 编译vertex shader代码
	// ‌バーテックスシェーダコードのコンパイル処理
	GLuint vertexShader = 0;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetShaderInfoLog(vertexShader, sizeof(infoLog), NULL, infoLog);
		cout << "Vertex shader compile failed\n" << infoLog << endl;
		glfwTerminate();
		return -1;
	}

	// 编译fragment shader代码
	// ‌フラグメントシェーダコードのコンパイル処理
	GLuint fragmentShader = 0;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetShaderInfoLog(fragmentShader, sizeof(infoLog), NULL, infoLog);
		cout << "Fragment shader compile failed\n" << infoLog << endl;
		glfwTerminate();
		return -1;
	}

	// 链接着色器程序
	// ‌シェーダープログラムのリンク処理
	GLuint shaderProgram = 0;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
		cout << "Shader program link failed\n" << infoLog << endl;
		glfwTerminate();
		return -1;
	}

	// 删除中间着色器对象
	// ‌中間シェーダーオブジェクトの削除処理
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// 参照 Referrence/opengl vertex management.png
	// 
	// 用VAO来管理 shader的顶点属性
    // VAOを使用してシェーダーの頂点属性を管理
	GLuint VAO = 0;
	glGenVertexArrays(1, &VAO);
	
	// 绑定VAO上下文后,再创建VBO EBO，都是在当前VAO下创建的了
	// VAOがアクティブな状態で生成したバッファオブジェクト(VBO EBO)は、暗黙的にそのVAOの管理対象となる
	glBindVertexArray(VAO);

	// 存储顶点数据到VBO
	// 頂点データをVBOに格納
	GLuint VBO = 0;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	// 存储下标数据到EBO
	// インデックスデータをEBOに格納
	GLuint EBO = 0;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);
	
	// VBO数据关联到shader的顶点属性
	// VBOデータとシェーダーの頂点属性を関連付け

	//void glVertexAttribPointer(
	//	GLuint index,          // 顶点属性位置（对应着色器中的 layout(location=N)）
	//						   // 頂点属性ロケーション（シェーダーの layout(location=N) に対応）
	//	GLint size,            // 每个顶点的分量数量（1-4）
	//						   // 頂点あたりの成分数（1-4）
	//	GLenum type,           // 数据类型（如 GL_FLOAT, GL_INT 等）
	//						   // データ型（例: GL_FLOAT, GL_INT など）
	//	GLboolean normalized,  // 是否归一化
	//                         // 正規化フラグ
	//	GLsizei stride,        // 顶点之间的字节跨度
	//						   // 頂点間のバイトストライド
	//	const void* pointer    // 数据起始位置的偏移量
	//                         // データ開始オフセット
	//);

	glVertexAttribPointer(0, ATTR_LENGTH, GL_FLOAT, GL_FALSE, ATTR_LENGTH * sizeof(GL_FLOAT), (void *)0);
	glEnableVertexAttribArray(0);

	// 注意 EBO必须在VAO之后解绑，否则VAO就没有EBO配置了，保险起见，一直确保VAO第一个解绑
	// 「EBOはVAOの後に必ずアンバインドする。さもないとVAOがEBO設定を保持できません。安全のため、常にVAOを最初にアンバインドする」
	// 
	// 解绑 关闭VAO上下文
	// VAOコンテキストの解放
	glBindVertexArray(0);

	// 解绑 关闭VBO上下文
	// VBOコンテキストの解放
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	// 解绑 关闭EBO上下文
	// EBOコンテキストの解放
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	// 运行shader
	// シェーダーを実行する
	glUseProgram(shaderProgram);

	// 线框模式表示
	// ワイヤーフレームモード表示
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//　渲染循环
	// レンダリングループ
	while (!glfwWindowShouldClose(window))
	{
		//输入
		//入力
		processInput(window);

		//渲染之前清空窗口
		//レンダリング前にウィンドウをクリアする
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// 绑定VAO上下文以读取顶点数据
		// 頂点データを読み取るためにVAOをバインド
		glBindVertexArray(VAO); 
		// 绘制三角形
		// 三角形を描画する
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		//缓冲区交换 轮询事件
		//バッファ交換 イベントポーリング 
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	glfwTerminate();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}