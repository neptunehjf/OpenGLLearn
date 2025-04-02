#version 330 core

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform float sample_offset_base; 
uniform int kernel_type;
uniform float window_width;
uniform float window_height;
uniform bool split_flag;
uniform bool bHDR;
uniform float fExposure;
uniform int iHDRAlgro;

out vec4 FragColor;

float[9] CopyKernel(float src[9]);
vec3 ToneMapping(vec3 color);

void main()
{   
    // 原色
    vec3 originColor = texture(texture_diffuse1, TexCoords).rgb;

    // Reinhard Tone Mapping
    originColor = ToneMapping(originColor);

    // 反相
    // 色反転
    //FragColor = vec4(vec3(1.0 - texture(texture_diffuse1, TexCoords)), 1.0); 

    // 黑白
    // グレースケール変換
    //FragColor = texture(texture_diffuse1, TexCoords);
    // 简单求平均数的方式，可能不够准确
    // 単純な平均値算出方法（精度不足の可能性あり）
    //float average = (FragColor.r + FragColor.g + FragColor.b) / 3.0;
    // 人眼会对绿色更加敏感一些，而对蓝色不那么敏感，所以为了获取物理上更精确的效果，我们需要使用加权的(Weighted)通道
    // 人間の視覚特性を考慮した加重平均（緑成分を重視）
    //float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;  
    //FragColor = vec4(average, average, average, 1.0); 

    float sample_offset = 1.0 / sample_offset_base;

    vec2 sample[9] = vec2[](
    vec2(-sample_offset,  sample_offset), // 左上
    vec2( 0.0f,    sample_offset), // 正上
    vec2( sample_offset,  sample_offset), // 右上
    vec2(-sample_offset,  0.0f),   // 左
    vec2( 0.0f,    0.0f),   // 中
    vec2( sample_offset,  0.0f),   // 右
    vec2(-sample_offset, -sample_offset), // 左下
    vec2( 0.0f,   -sample_offset), // 正下
    vec2( sample_offset, -sample_offset)  // 右下
    );

    // カーネル初期化（無変換）
    float kernel[9] = float[](
        0, 0, 0,
        0, 1, 0,
        0, 0, 0
    );

    // シャープン用カーネル
    float sharpen[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );

    // エッジ検出用カーネル
    float edgeDetect[9] = float[](
         1,  1,  1,
         1, -8,  1,
         1,  1,  1
    );

    // 加权之和等于1才是原来的颜色，大于或小于1的话，图像会变亮/变暗。所以这里每个加权值都要除以16.0
    // ぼかし用カーネル（総和が1になるよう16で除算）
    float Blur[9] = float[](
         1.0 / 16.0,  2.0 / 16.0,  1.0 / 16.0,
         2.0 / 16.0,  4.0 / 16.0,  2.0 / 16.0,
         1.0 / 16.0,  2.0 / 16.0,  1.0 / 16.0
    );

    switch (kernel_type)
    {
        case 0:
        {
            // デフォルトカーネルを使用
            break;
        }
        case 1:
        {
            kernel = CopyKernel(sharpen);
            break;
        }
        case 2:
        {
             kernel = CopyKernel(edgeDetect);
            break;
        }
        case 3:
        {
             kernel = CopyKernel(Blur);
            break;
        }
        default:
        {
            break;
        }
    }

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(texture_diffuse1, TexCoords.st + sample[i]));
        sampleTex[i] = ToneMapping(sampleTex[i]);
    }
    vec3 color = vec3(0.0);
    for(int i = 0; i < 9; i++)
        color += sampleTex[i] * kernel[i];

    //gl_FragCoord 对应的是视口坐标（像素） 
    if ((gl_FragCoord.x < window_width / 2.0) && split_flag)
    {
        FragColor = vec4(originColor, 1.0);
    }
    else if ((gl_FragCoord.x < window_width / 2.0 + 1.0) && split_flag) // gl_FragCoord.x == window_width / 2.0 因为误差的原因算不出来
    {
        FragColor = vec4(0.0, 255.0, 0.0, 1.0);
    }
    else
    {
        FragColor = vec4(color, 1.0);
    }
}

// 注意GLSL不支持指针操作（在显卡里当然不能访问内存了），只能通过返回值的方式返回数组
// カーネル配列コピー関数（GLSLはポインタ操作不可のため値渡しで実現）
float[9] CopyKernel(float src[9])
{
    float dst[9] = float[](
        0, 0, 0,
        0, 1, 0,
        0, 0, 0
    );

    for(int i = 0; i < 9; i++)
        dst[i] = src[i];

    return dst;
}

vec3 ToneMapping(vec3 color)
{
    if (bHDR)
    {
        vec3 mappedColor;
        if (iHDRAlgro == 0)
        {
            // Reinhard Tone Mapping
            mappedColor = color / (color + vec3(1.0));
        }
        else if (iHDRAlgro == 1)
        {
            // Exposure Tone Mapping
            mappedColor = vec3(1.0) - exp(-color * fExposure); //vec3(1.0) 减去 自然常数e的-color * fExposure次方
        }

        return mappedColor;
    }
    else
        return color;

}