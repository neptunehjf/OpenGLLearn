#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uni_model;

layout (std140) uniform Matrix
{
	mat4 uni_view;
	mat4 uni_projection;	
};

out VS_OUT
{
	vec4 normal;
} vs_out;

void main()
{
   gl_Position = uni_projection * uni_view * uni_model * vec4(aPos, 1.0);  // 注意矩阵变换的顺序是从右向左

   // 1 mat3(transpose(inverse(uni_view * uni_model))) 是法线矩阵，用于去除因为不等比缩放导致的法线方向错误，
   // 2 又因法线只是方向与位置无关，故用mat3去除齐次分量w
   // 3 因为 model矩阵涉及scale rotation translation，而view矩阵需要对model矩阵后的结果进一步计算，用于3D空间的变换
   // 4 而projection只用于裁剪，透视等，实际上可以看作对 view * model * pos的后期处理，用于决定在2D屏幕上的显示效果
   // 5 所以法线矩阵只能对view model空间进行3D修正，和projection没关系，强行修正projection会出错
   // 6 因为这里要在view空间操作法线，所以transpose(inverse(uni_view * uni_model)
   // 7 其他情况，比如光照计算，是在model空间，所以只要transpose(inverse(uni_model)
   vs_out.normal = uni_projection * vec4(mat3(transpose(inverse(uni_view * uni_model))) * aNormal, 0.0);
}