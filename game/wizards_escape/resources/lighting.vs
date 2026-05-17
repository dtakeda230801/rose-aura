#version 330

// 入力属性
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// 変換行列（Raylibから自動で渡されます）
uniform mat4 mvp;
uniform mat4 matModel;

// フラグメントシェーダーへの出力
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

void main()
{
    // 頂点位置の計算
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    
    // 法線の計算（スケーリングに対応するため正規化）
    fragNormal = normalize(vec3(matModel * vec4(vertexNormal, 0.0)));

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}