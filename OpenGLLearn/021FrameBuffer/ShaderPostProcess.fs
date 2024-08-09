#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{   // ԭɫ
    //FragColor = texture(texture_diffuse1, TexCoords);

    // ����
    //FragColor = vec4(vec3(1.0 - texture(texture_diffuse1, TexCoords)), 1.0); 

    // �ڰ�
    //FragColor = texture(texture_diffuse1, TexCoords);
    // ����ƽ�����ķ�ʽ�����ܲ���׼ȷ
    //float average = (FragColor.r + FragColor.g + FragColor.b) / 3.0;
    // ���ۻ����ɫ��������һЩ��������ɫ����ô���У�����Ϊ�˻�ȡ�����ϸ���ȷ��Ч����������Ҫʹ�ü�Ȩ��(Weighted)ͨ��
    //float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;  
    //FragColor = vec4(average, average, average, 1.0); 

    // 

    float offset = 1.0 / 300.0;
    vec2 offsets[9] = vec2[](
    vec2(-offset,  offset), // ����
    vec2( 0.0f,    offset), // ����
    vec2( offset,  offset), // ����
    vec2(-offset,  0.0f),   // ��
    vec2( 0.0f,    0.0f),   // ��
    vec2( offset,  0.0f),   // ��
    vec2(-offset, -offset), // ����
    vec2( 0.0f,   -offset), // ����
    vec2( offset, -offset)  // ����
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

    // ��Ȩ֮�͵���1����ԭ������ɫ�����ڻ�С��1�Ļ���ͼ������/�䰵����������ÿ����Ȩֵ��Ҫ����16.0
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