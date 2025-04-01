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
    // view空间，摄像机移动x的话，相当于物体反方向移动x。
    // 但是这里的view转化矩阵忽略了位移分量，这样摄像机和物体之间就不会有相对位移了
    // 这样一来，相当于原点有个摄像机，被天空盒包裹起来，摄像机离天空盒的各个面的距离始终都是1（注意这里的1和NDC标准化设备坐标无关），
    // 因此视锥的near应该远小于1 far应该远大于1，否则天空盒的某部分可能会落在视锥之外，被裁剪掉
    // 总结：摄像机被包裹在一个很小的天空盒(2x2x2)内，只是没有相对位移产生了盒子很大的错觉

    // スカイボックスはローカル→ワールド変換を適用しないため、ワールド座標＝ローカル座標
    // つまり原点(0,0,0)を中心とした2x2x2立方体として定義される
    // ビュー空間における座標変換原理：カメラがX量移動する場合、オブジェクトは逆方向にX量移動した座標変換が適用される
    // ただし本実装ではビュー変換行列の並進成分を無視しているため、カメラと物体の相対位置変化が発生しない
    // 結果として、原点のカメラを包むスカイボックスは常に各面までの距離1を維持（NDC正規化座標とは無関係）
    // 視錐体設定：ニアクリップ面≪1、ファークリップ面≫1 必須（設定不備でスカイボックスクリッピング発生リスク）
    // 要約：カメラは小さなスカイボックス(2x2x2)内に包まれ、相対位置変化のないレンダリングにより大規模空間の錯覚を生成
    mat4 view = mat4(mat3(uni_view));
    vec4 pos = uni_projection * view * vec4(aPos, 1.0);  
    TexCoords = aPos;
    gl_Position = pos.xyww;
}