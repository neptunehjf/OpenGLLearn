#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{   // 原色
    //FragColor = texture(texture_diffuse1, TexCoords);

    // 反相
    //FragColor = vec4(vec3(1.0 - texture(texture_diffuse1, TexCoords)), 1.0); 

    // 黑白
    //FragColor = texture(texture_diffuse1, TexCoords);
    // 简单求平均数的方式，可能不够准确
    //float average = (FragColor.r + FragColor.g + FragColor.b) / 3.0;
    // 人眼会对绿色更加敏感一些，而对蓝色不那么敏感，所以为了获取物理上更精确的效果，我们需要使用加权的(Weighted)通道
    //float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;  
    //FragColor = vec4(average, average, average, 1.0); 

    // 

    float offset = 1.0 / 300.0;
    vec2 offsets[9] = vec2[](
    vec2(-offset,  offset), // 左上
    vec2( 0.0f,    offset), // 正上
    vec2( offset,  offset), // 右上
    vec2(-offset,  0.0f),   // 左
    vec2( 0.0f,    0.0f),   // 中
    vec2( offset,  0.0f),   // 右
    vec2(-offset, -offset), // 左下
    vec2( 0.0f,   -offset), // 正下
    vec2( offset, -offset)  // 右下
    );

    float sharpen[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );

    float edgeDetect[9] = float[](
         1,  1,  1,
         1, -8,  1,
         1,  1,  1
    );

    // 加权之和等于1才是原来的颜色，大于或小于1的话，图像会变亮/变暗。所以这里每个加权值都要除以16.0
    float Blur[9] = float[](
         1.0 / 16.0,  2.0 / 16.0,  1.0 / 16.0,
         2.0 / 16.0,  4.0 / 16.0,  2.0 / 16.0,
         1.0 / 16.0,  2.0 / 16.0,  1.0 / 16.0
    );

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(texture_diffuse1, TexCoords.st + offsets[i]));
    }
    vec3 color = vec3(0.0);
    for(int i = 0; i < 9; i++)
        color += sampleTex[i] * Blur[i];

    FragColor = vec4(color, 1.0);

   
}