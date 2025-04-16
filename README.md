# OpenGL Renderer

‌**概要**‌  
OpenGL APIを用いて実装されたレンダラーです。基礎的なライティングレンダリングからパフォーマンス最適化、さらに物理ベースレンダリング（PBR）に至るまでの一連の描画機能を実現しています。

‌**技術スタック**‌

-   ‌**プログラミング言語**‌: C++、GLSL3.3
-   ‌**理論基盤**‌: グラフィックス学、数学、物理
-   ‌**API**‌: glfw3、glad、glm、stb_image、assimp、imgui
-   ‌**パフォーマンス最適化**‌: 背面カリング、インスタンス化、遅延シェーディング、Monte Carloサンプリング、重要性サンプリング
-   ‌**ツール**‌: Vsiual Studio2022、RenderDocなど

‌**プロジェクト構成**‌

-   ‌**Referrence** - 参考文献
-   ‌**Dependency** - 依存ファイル
-   ‌**001CreateWindow** - ウィンドウの作成 
-   ‌**002CreateTriangle** - 三角形の描画
-   ‌**002CreateTriangle_P1** - 塗りつぶしありの三角形の描画
-   ‌**003Shader** - シェーダープログラムの構築
-   ‌**003Shader_P1** - 頂点と色の時間経過に伴う変化をシェーダーで実装
-   ‌**004Texture** - テクスチャのUVマッピング
-   ‌**004Texture_P1** - テクスチャのブレンディング効果
-   ‌**005MatrixTransform** - 変換行列を用いた物体の移動と回転の実装
-   ‌**005MatrixTransform_Practice** - 変換行列を用いた物体のスケーリング
-   ‌**006Coordnate** - 座標系変換による3D化の実装
-   ‌**006Coordnate_Practice** - 座標系変換関連の練習
-   ‌**007Camera** - カメラシステムの導入（移動、回転、FOVのサポート）
-   ‌**007Camera_Practice** - カメラシステム関連の練習
-   ‌**008Color** - 物体の色計算
-   ‌**009BasicLighting** - Phongの照明モデル
-   ‌**009BasicLighting_Practice** - 練習とGouraudシェーディング
-   ‌**010Material** - マテリアルシステム
-   ‌**011LightingMap** - ライトマップ
-   ‌**011LightingMap_Practice** - 練習と放射光マップ
-   ‌**012LightCaster** - 平行光/ポイントライト/スポットライト
-   ‌**013MultipleLights** - 複数種光源の組み合わせシーン
-   ‌**014Assimp** - 3Dモデルインポート用Assimpライブラリの導入
-   ‌**015Mesh** - 頂点データ管理用Meshクラスの導入
-   ‌**016Model** - Assimpによる外部3Dモデルの読み込みとレンダリング
-   ‌**017DepthTest** - 深度テストと深度値可視化
-   ‌**018StencilTest** - ステンシルテストによる輪郭線効果
-   ‌**019Blending** - ブレンディングによる半透明表現
-   ‌**020FaceCulling** - 背面カリングによる描画最適化
-   ‌**021FrameBuffer** - カスタムフレームバッファを用いたポストプロセス（シャープ/エッジ検出/ブラー）
-   ‌**022Cubemap** - キューブマップによるスカイボックスと反射/屈折効果
-   ‌**023AdvancedGLSL** - 高度なGLSL機能（組み込み変数/インターフェースブロック/UBO）
-   ‌**024GeometryShader** - ジオメトリシェーダー応用（法線可視化/爆発エフェクト）
-   ‌**025Instancing** - インスタンス化による大量オブジェクト描画の最適化（10万インスタンス/130fps）
-   ‌**026AntiAliasing** - MSAAアンチエイリアシングの有効化
-   ‌**027AdvancedLighting** - Phong照明モデルからBlinn-Phongモデルへの移行（鏡面反射の品質改善）
-   ‌**028GammaCorrection** - 線形空間に変換→照明計算→ガンマ補正のワークフローによる正確な色表現
-   ‌**029ShadowMapping** - 深度マップを用いた平行光源の影生成
-   ‌**030PointShadow** - 深度キューブマップによるポイントライトの影生成
-   ‌**031NormalMapping** - 接空間計算と法線マッピングの実装
-   ‌**032ParallaxMapping** - 視差マッピング（3種のアルゴリズム比較対応）
-   ‌**033HDR** - Reinhard/ExposureアルゴリズムによるHDRトーンマッピング
-   ‌**034Bloom** - ピンポンバッファ＆ガウスぼかしを用いたブルーム効果
-   ‌**035DeferredShading** - 遅延シェーディングによるGPU負荷軽減（GPU負荷レベル2時：59fps→105fps）
-   ‌**036SSAO** - 遅延シェーディングとSSAOアルゴリズムによるスクリーン空間アンビエントオクルージョンの実装
-   ‌**037PBR** - マイクロサーフェスモデル／エネルギー保存則／Cook-Torrance BRDF に基づく物理ベースレンダリング（直接照明部分）
-   ‌**038PBR_IBL_Diffuse** -マイクロサーフェスモデル／エネルギー保存則／Cook-Torrance BRDF に基づく物理ベースレンダリング（拡散反射部分）
-   ‌**039PBR_IBL_Specular** - マイクロサーフェスモデル／エネルギー保存則／Cook-Torrance BRDF に基づく物理ベースレンダリング（鏡面反射部分）

※ コード中のコメントは一応日本語化していますが、自分用のメモとして中国語のコメントも多く残っています。悪しからずご了承ください。


‌**実行方法**‌  
Visual Studio 2022で該当プロジェクトの`.sln`ファイルを開き、コンパイル後、VS 2022のデバッガーで実行します。

‌**成果画像の例**‌  
<img src="https://github.com/user-attachments/assets/965ede98-de71-40a7-9c90-6b2770fc5a8c" width="1024px" />
<img src="https://github.com/user-attachments/assets/ab5a578d-9328-4e9f-a1a7-95a2ddefa0df" width="1024px" />
<img src="https://github.com/user-attachments/assets/b6c83b43-ae63-406d-ba6a-417c9f45f5c3" width="1024px" />
<img src="https://github.com/user-attachments/assets/fb819302-4bf3-49a7-967f-364b9eccefdd" width="1024px" />
<img src="https://github.com/user-attachments/assets/d7bd3ddd-e98a-4338-88b5-0a9c044a21cd" width="1024px" />
<img src="https://github.com/user-attachments/assets/cdf0da50-a6c3-429f-9061-5bb7713457ab" width="1024px" />
<img src="https://github.com/user-attachments/assets/4f05ce55-a2c5-43c7-97d0-726aed1d770c" width="1024px" />
<img src="https://github.com/user-attachments/assets/3e1e5eed-8e75-434f-996c-44ba92f25f9a" width="1024px" />

‌**連絡先**‌  
koalahjf@gmail.com
