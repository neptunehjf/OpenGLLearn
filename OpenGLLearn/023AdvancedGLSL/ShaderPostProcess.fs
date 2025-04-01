#version 330 core

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform float sample_offset_base; 
uniform int kernel_type;
uniform float window_width;
uniform float window_height;
uniform bool split_flag;

out vec4 FragColor;

float[9] CopyKernel(float src[9]);

void main()
{   // ԭɫ
    vec4 originColor = texture(texture_diffuse1, TexCoords);

    // ����
    // ɫ��ܞ
    //FragColor = vec4(vec3(1.0 - texture(texture_diffuse1, TexCoords)), 1.0); 

    // �ڰ�
    // ����`�����`���Q
    //FragColor = texture(texture_diffuse1, TexCoords);
    // ����ƽ�����ķ�ʽ�����ܲ���׼ȷ
    // �g����ƽ����������������Ȳ���ο����Ԥ��꣩
    //float average = (FragColor.r + FragColor.g + FragColor.b) / 3.0;
    // ���ۻ����ɫ��������һЩ��������ɫ����ô���У�����Ϊ�˻�ȡ�����ϸ���ȷ��Ч����������Ҫʹ�ü�Ȩ��(Weighted)ͨ��
    // ���g��ҕҙ���Ԥ򿼑]��������ƽ�����v�ɷ֤���ҕ��
    //float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;  
    //FragColor = vec4(average, average, average, 1.0); 

    float sample_offset = 1.0 / sample_offset_base;

    vec2 sample[9] = vec2[](
    vec2(-sample_offset,  sample_offset), // ����
    vec2( 0.0f,    sample_offset), // ����
    vec2( sample_offset,  sample_offset), // ����
    vec2(-sample_offset,  0.0f),   // ��
    vec2( 0.0f,    0.0f),   // ��
    vec2( sample_offset,  0.0f),   // ��
    vec2(-sample_offset, -sample_offset), // ����
    vec2( 0.0f,   -sample_offset), // ����
    vec2( sample_offset, -sample_offset)  // ����
    );

    // ���`�ͥ���ڻ����o��Q��
    float kernel[9] = float[](
        0, 0, 0,
        0, 1, 0,
        0, 0, 0
    );

    // ����`�ץ��å��`�ͥ�
    float sharpen[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );

    // ���å��ʳ��å��`�ͥ�
    float edgeDetect[9] = float[](
         1,  1,  1,
         1, -8,  1,
         1,  1,  1
    );

    // ��Ȩ֮�͵���1����ԭ������ɫ�����ڻ�С��1�Ļ���ͼ������/�䰵����������ÿ����Ȩֵ��Ҫ����16.0
    // �ܤ����å��`�ͥ루�t�ͤ�1�ˤʤ�褦16�ǳ��㣩
    float Blur[9] = float[](
         1.0 / 16.0,  2.0 / 16.0,  1.0 / 16.0,
         2.0 / 16.0,  4.0 / 16.0,  2.0 / 16.0,
         1.0 / 16.0,  2.0 / 16.0,  1.0 / 16.0
    );

    switch (kernel_type)
    {
        case 0:
        {
            // �ǥե���ȥ��`�ͥ��ʹ��
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
    }
    vec3 color = vec3(0.0);
    for(int i = 0; i < 9; i++)
        color += sampleTex[i] * kernel[i];

    //gl_FragCoord ��Ӧ�����ӿ����꣨���أ� 
    if ((gl_FragCoord.x < window_width / 2.0) && split_flag)
    {
        FragColor = originColor;
    }
    else if ((gl_FragCoord.x < window_width / 2.0 + 1.0) && split_flag) // gl_FragCoord.x == window_width / 2.0 ��Ϊ����ԭ���㲻����
    {
        FragColor = vec4(0.0, 255.0, 0.0, 1.0);
    }
    else
    {
        FragColor = vec4(color, 1.0);
    }
}

// ע��GLSL��֧��ָ����������Կ��ﵱȻ���ܷ����ڴ��ˣ���ֻ��ͨ������ֵ�ķ�ʽ��������
// ���`�ͥ����Х��ԩ`�v����GLSL�ϥݥ��󥿲������ɤΤ��ႎ�ɤ��ǌg�F��
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