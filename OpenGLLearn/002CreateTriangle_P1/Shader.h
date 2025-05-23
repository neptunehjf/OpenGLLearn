#pragma once

const char* vertexShaderSource = "#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";


const char* fragmentShaderSource1 = "#version 330 core\n"
"out vec4 fragColor;\n"
"void main()\n"
"{\n"
"fragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";

const char* fragmentShaderSource2 = "#version 330 core\n"
"out vec4 fragColor;\n"
"void main()\n"
"{\n"
"fragColor = vec4(0.0f, 0.3f, 0.1f, 1.0f);\n"
"}\0";

