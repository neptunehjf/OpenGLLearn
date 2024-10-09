#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

struct Material
{
	sampler2D texture_diffuse1;
    sampler2D texture_diffuse2; // 亮色图
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
        //FragColor = vec4(vec3(1.0 - texture(material.texture_diffuse1, TexCoords)), 1.0); 

        // 黑白
        //FragColor = texture(material.texture_diffuse1, TexCoords);
        // 简单求平均数的方式，可能不够准确
        //float average = (FragColor.r + FragColor.g + FragColor.b) / 3.0;
        // 人眼会对绿色更加敏感一些，而对蓝色不那么敏感，所以为了获取物理上更精确的效果，我们需要使用加权的(Weighted)通道
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

        // 初始化核，不起任何作用
        float kernel[9] = float[](
            0, 0, 0,
            0, 1, 0,
            0, 0, 0
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

        switch (kernel_type)
        {
            case 0:
            {
                //用默认核
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
            // Reinhard色调映射
            mappedColor = color / (color + vec3(1.0));
        }
        else if (iHDRAlgro == 1)
        {
            // 曝光色调映射
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
    float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

    // 当前fragment
    vec3 color = texture(material.texture_diffuse2, TexCoords).rgb * weight[0];

    // 因为Gaussian 波形有x和y维度可分离的特性，所以每次只计算一个维度组合起来，比直接二维计算要效率高
    // 比如 5 x 5 的采样范围，直接二维采样要采样5 * 5 = 25次，分离计算只要 5 + 5 = 10次
    if (horizontal)
    {
        for (int i = 1; i < 5; i++)
        {
            color += texture(material.texture_diffuse2, TexCoords + vec2(sample_offset.x * i, 0.0)).rgb * weight[i];
            color += texture(material.texture_diffuse2, TexCoords - vec2(sample_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for (int i = 1; i < 5; i++)
        {
            color += texture(material.texture_diffuse2, TexCoords + vec2(0.0, sample_offset.y * i)).rgb * weight[i];
            color += texture(material.texture_diffuse2, TexCoords - vec2(0.0, sample_offset.y * i)).rgb * weight[i];
        }
    }

    return color;
}