
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

	GLuint vertexShader = 0;
	GLuint fragmentShader[2];
	GLuint shaderProgram[2];

	// 编译vertex shader代码
	// ‌バーテックスシェーダコードのコンパイル処理
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

	// 编译fragment shader 1代码
	// ‌フラグメントシェーダコード1のコンパイル処理
	fragmentShader[0] = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader[0], 1, &fragmentShaderSource1, NULL);
	glCompileShader(fragmentShader[0]);

	glGetShaderiv(fragmentShader[0], GL_COMPILE_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetShaderInfoLog(fragmentShader[0], sizeof(infoLog), NULL, infoLog);
		cout << "Fragment shader1 compile failed\n" << infoLog << endl;
		glfwTerminate();
		return -1;
	}

	// 编译fragment shader 2代码
	// ‌フラグメントシェーダコード2のコンパイル処理
	fragmentShader[1] = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader[1], 1, &fragmentShaderSource2, NULL);
	glCompileShader(fragmentShader[1]);

	glGetShaderiv(fragmentShader[1], GL_COMPILE_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetShaderInfoLog(fragmentShader[1], sizeof(infoLog), NULL, infoLog);
		cout << "Fragment shader2 compile failed\n" << infoLog << endl;
		glfwTerminate();
		return -1;
	}

	// 链接着色器程序1
	// ‌シェーダープログラム1のリンク処理
	shaderProgram[0] = glCreateProgram();
	glAttachShader(shaderProgram[0], vertexShader);
	glAttachShader(shaderProgram[0], fragmentShader[0]);
	glLinkProgram(shaderProgram[0]);

	glGetProgramiv(shaderProgram[0], GL_LINK_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetProgramInfoLog(shaderProgram[0], sizeof(infoLog), NULL, infoLog);
		cout << "Shader program link failed\n" << infoLog << endl;
		glfwTerminate();
		return -1;
	}

	// 链接着色器程序2
	// ‌シェーダープログラム2のリンク処理
	shaderProgram[1] = glCreateProgram();
	glAttachShader(shaderProgram[1], vertexShader);
	glAttachShader(shaderProgram[1], fragmentShader[1]);
	glLinkProgram(shaderProgram[1]);

	glGetProgramiv(shaderProgram[1], GL_LINK_STATUS, &success);
	if (!success)
	{
		memset(infoLog, 0, sizeof(infoLog));

		glGetProgramInfoLog(shaderProgram[1], sizeof(infoLog), NULL, infoLog);
		cout << "Shader program link failed\n" << infoLog << endl;
		glfwTerminate();
		return -1;
	}

	// 删除中间着色器对象
	// ‌中間シェーダーオブジェクトの削除処理
	glDeleteShader(vertexShader);
	glDeleteShader(shaderProgram[0]);
	glDeleteShader(shaderProgram[1]);

	// 参照 Referrence/opengl vertex management.png
	// 用VAO来管理 shader的顶点属性
	// VAOを使用してシェーダーの頂点属性を管理
	GLuint VAO[2];
	glGenVertexArrays(2, VAO);
	GLuint VBO[2];
	glGenBuffers(2, VBO);

	/************ 一つ目の三角形 ***********/
	// 绑定VAO上下文后,再创建VBO EBO，都是在当前VAO下创建的了
	// VAOがアクティブな状態で生成したバッファオブジェクト(VBO EBO)は、暗黙的にそのVAOの管理対象となる
	glBindVertexArray(VAO[0]);

	// 存储顶点数据到VBO
	// 頂点データをVBOに格納	
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex1), vertex1, GL_STATIC_DRAW);

	// VBO数据关联到shader的顶点属性
	// VBOデータとシェーダーの頂点属性を関連付け
	glVertexAttribPointer(0, ATTR_LENGTH, GL_FLOAT, GL_FALSE, ATTR_LENGTH * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(0);

	// 解绑 关闭上下文 这里也可以不解绑VAO，因为下面很快就绑定到另一个VAO了
	// コンテキストを閉じ バインド解除。ここではVAOバインド解除を省略可能、直後に別VAOへ再バインドするため
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	/************ 一つ目の三角形 ***********/

	/************ 二つ目の三角形 ***********/
	// 绑定VAO上下文后,再创建VBO EBO，都是在当前VAO下创建的了
	// VAOがアクティブな状態で生成したバッファオブジェクト(VBO EBO)は、暗黙的にそのVAOの管理対象となる
	glBindVertexArray(VAO[1]); // VBO glVertexAttribPointer 操作向VAO上下文写

	// 存储顶点数据到VBO
	// 頂点データをVBOに格納
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex2), vertex2, GL_STATIC_DRAW);

	// VBO数据关联到shader的顶点属性
	// VBOデータとシェーダーの頂点属性を関連付け
	glVertexAttribPointer(0, ATTR_LENGTH, GL_FLOAT, GL_FALSE, ATTR_LENGTH * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(0);

	// 解绑 关闭上下文
	// コンテキストを閉じ バインド解除
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	/************ 二つ目の三角形 ***********/

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//渲染循环
	// レンダリングループ
	while (!glfwWindowShouldClose(window))
	{
		//入力
		glfwPollEvents();
		processInput(window);

		//渲染之前清空窗口
		//レンダリング前にウィンドウをクリアする
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// 绘制三角形
		// 三角形を描画する

		// 运行shader1
		// シェーダー1を実行する
		glUseProgram(shaderProgram[0]);
		// 绑定VAO上下文以读取顶点数据
		// 頂点データを読み取るためにVAOをバインド
		glBindVertexArray(VAO[0]); 
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);

		// 运行shader2
		// シェーダー1を実行する
		glUseProgram(shaderProgram[1]);
		// 頂点データを読み取るためにVAOをバインド
		glBindVertexArray(VAO[1]); 
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);

		//缓冲区交换
		//バッファ交換 
		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(2, VAO);
	glDeleteBuffers(2, VBO);
	glDeleteProgram(shaderProgram[0]);
	glDeleteProgram(shaderProgram[1]);

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