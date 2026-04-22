#version 330

in vec2 fragTexCoord;

uniform sampler2D texY;
uniform sampler2D texU;
uniform sampler2D texV;

out vec4 finalColor;

void main()
{
    vec2 texSizeY = textureSize(texY, 0);
    vec2 texSizeU = textureSize(texU, 0);

    // ★ 0～255に戻す
    float y = texture(texY, fragTexCoord).r * 255.0;

    float px = fragTexCoord.x * texSizeY.x;
    float ux = floor(px / 2.0);
    float uCoordX = (ux + 0.5) / texSizeU.x;

    vec2 uvCoord = vec2(uCoordX, fragTexCoord.y);

    float u = texture(texU, uvCoord).r * 255.0;
    float v = texture(texV, uvCoord).r * 255.0;

    // ★ Octaveと完全一致
    u = (u - 128.0) / 128.0;
    v = (v - 128.0) / 128.0;

    // ★ Yも255スケールで使う
    float r = y + 1.5748 * v * 255.0;
    float g = y - 0.1873 * u * 255.0 - 0.4681 * v * 255.0;
    float b = y + 1.8556 * u * 255.0;

    // ★ 最後に戻す
    finalColor = vec4(r/255.0, g/255.0, b/255.0, 1.0);
}