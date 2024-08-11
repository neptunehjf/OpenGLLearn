#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uni_view;
uniform mat4 uni_projection;
uniform mat4 uni_model;

out vec3 TexCoords;

void main()
{
    // 天空盒没有进行local->world的转换，因此world坐标就是local坐标，也就是原点为(0,0,0)，2x2x2的立方体
    // view空间是以摄像机为原点的，因此假设物体离远点距离X的话，就相当于把物体从其所在世界的位置推离X。
    // 但是这里的view转化矩阵忽略了w分量，这样摄像机和物体之间就不会有相对位移了
    // 这样一来，相当于原点有个摄像机，被天空盒包裹起来，摄像机离天空盒的各个面的距离始终都是1（注意这里的1和NDC标准化设备坐标无关），
    // 因此视锥的near应该远小于1 far应该远大于1，否则天空盒的某部分可能会落在视锥之外，被裁剪掉
    // 总结：摄像机被包裹在一个很小的天空盒(2x2x2)内，只是没有相对位移产生了盒子很大的错觉
    mat4 view = mat4(mat3(uni_view));
    gl_Position = uni_projection * view * vec4(aPos, 1.0);  
    TexCoords = aPos;
}