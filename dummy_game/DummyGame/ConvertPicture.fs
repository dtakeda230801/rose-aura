#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texY;
uniform sampler2D texU;
uniform sampler2D texV;

void main()
{
    // Y（フル解像度）
    float y = texture(texY, fragTexCoord).r;

    // 422：横だけ1/2
    vec2 uvUV = vec2(fragTexCoord.x * 0.5, fragTexCoord.y);

    float u = texture(texU, uvUV).r - 0.5;
    float v = texture(texV, uvUV).r - 0.5;

    // BT.601（とりあえずこれでOK）
    float r = y + 1.402 * v;
    float g = y - 0.344 * u - 0.714 * v;
    float b = y + 1.772 * u;

    finalColor = vec4(r, g, b, 1.0);
}