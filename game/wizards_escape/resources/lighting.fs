#version 330

// 頂点シェーダーからの入力
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// 入力ユニフォーム（テクスチャ）
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// ライティング用のユニフォーム
uniform vec3 lightDir;   // 光の方向
uniform vec4 lightColor; // 光の色
uniform vec4 ambient;    // 環境光（暗部が真っ黒にならないようにする）

// 出力色
out vec4 finalColor;

void main()
{
    // テクスチャとマテリアルカラーの結合
    vec4 texelColor = texture(texture0, fragTexCoord) * colDiffuse * fragColor;
    
    // 法線の正規化
    vec3 normal = normalize(fragNormal);
    
    // ディフューズ（拡散光）の計算：光の方向と法線の内積
    // ※lightDirは「光が当たる方向（光源への方向）」として計算
    float dotProduct = max(dot(normal, normalize(lightDir)), 0.0);
    
    // 最終的な光の計算 (環境光 + 拡散光)
    vec4 light = ambient + (lightColor * dotProduct);
    
    finalColor = texelColor * light;
}