#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

struct Material
{
	sampler2D texture_diffuse1;
    sampler2D texture_diffuse2; // ブルーム用高輝度マップ
};

uniform Material material;

uniform float sample_offset_base; 
uniform int kernel_type;
uniform float window_width;
uniform float window_height;
uniform bool split_flag;
uniform bool bHDR;
uniform float fExposure;
uniform int iHDRAlgro;
uniform bool bBloom;
uniform bool horizontal;

float[9] CopyKernel(float src[9]);
vec3 ToneMapping(vec3 color);
vec3 BloomBlur();

void main()
{   
    // 原色
    vec3 originColor = texture(material.texture_diffuse1, TexCoords).rgb;

    vec3 color = originColor;

    if (bHDR || bBloom)
    {
        // Bloom 要在HDR Tone Mapping 之前，因为HDR下 才能brightness > 1.0，对亮度有更好的控制
        if (bBloom)
        {
           vec3 bloomColor = BloomBlur();
           color += bloomColor;
        }
        // Reinhard Tone Mapping
        color = ToneMapping(color);
    
        FragColor = vec4(color, 1.0);
    }
    else
    {
        // PostProcess 要在 Tone Mapping之后，因为后期是基于LDR的

        // 反相
        // 色反転
        //FragColor = vec4(vec3(1.0 - texture(material.texture_diffuse1, TexCoords)), 1.0); 

        // 黑白
    // グレースケール変換
        //FragColor = texture(material.texture_diffuse1, TexCoords);
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
            sampleTex[i] = vec3(texture(material.texture_diffuse1, TexCoords.st + sample[i]));
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

vec3 BloomBlur()
{
    // Gaussian blur
    vec2 sample_offset = 1.0 / textureSize(material.texture_diffuse2, 0);
    float weight[9] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216, 0.006216, 0.003216, 0.001216, 0.000216);

    // 中心サンプル点
    vec3 color = texture(material.texture_diffuse2, TexCoords).rgb * weight[0];

    // 因为Gaussian 波形有x和y维度可分离的特性，所以每次只计算一个维度组合起来，比直接二维计算要效率高
    // 比如 5 x 5 的采样范围，直接二维采样要采样5 X 5 = 25次，分离计算只要 5 + 5 = 10次

    // ガウシアン関数の分離可能特性を活用：
    // x軸とy軸を個別に処理することで計算効率向上
    // （例）5x5サンプルの場合：
    // 通常   -> 25回サンプリング
    // 分離処理 -> 10回サンプリング (5+5)

    // 以下用x y轴交替采样的方式，采样数是(1 + 2 X 8) + (1 + 2 X 8) = 17 + 17 = 34
    // 現在の処理方向に応じたサンプリング（合計34サンプル）

    if (horizontal)
    {
        for (int i = 1; i < 9; i++)
        {
            color += texture(material.texture_diffuse2, TexCoords + vec2(sample_offset.x * i, 0.0)).rgb * weight[i];
            color += texture(material.texture_diffuse2, TexCoords - vec2(sample_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for (int i = 1; i < 9; i++)
        {
            color += texture(material.texture_diffuse2, TexCoords + vec2(0.0, sample_offset.y * i)).rgb * weight[i];
            color += texture(material.texture_diffuse2, TexCoords - vec2(0.0, sample_offset.y * i)).rgb * weight[i];
        }
    }

    return color;
}