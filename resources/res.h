#pragma once

#include <string>

#define RESOURCES RoseAuraResources::get()

class RoseAuraResources {
public:
/////////////////////////////////
    std::string Res_ConvertPictureF = R"(
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

    float y = texture(texY, fragTexCoord).r;

    float px = fragTexCoord.x * texSizeY.x;
    float ux = floor(px / 2.0);
    float uCoordX = (ux + 0.5) / texSizeU.x;

    vec2 uvCoord = vec2(uCoordX, fragTexCoord.y);

    float u = texture(texU, uvCoord).r;
    float v = texture(texV, uvCoord).r;

    u = u - 0.5;
    v = v - 0.5;

    float r = y + 1.5748 * v;
    float g = y - 0.1873 * u - 0.4681 * v;
    float b = y + 1.8556 * u;

    finalColor = vec4(r, g, b, 1.0);
}
)";

/////////////////////////////////
    std::string Res_ConvertPictureV = R"(
#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

out vec2 fragTexCoord;

uniform mat4 mvp;

void main()
{
    fragTexCoord = vertexTexCoord;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)";

    static void create()
    {
        if (!mIns) {
            mIns = new RoseAuraResources();
        }
    };

    static RoseAuraResources* get() 
    {
        return mIns;
    };

    static void destroy()
    {
        if (mIns) {
            delete mIns;
            mIns = nullptr;
        }
    };

    RoseAuraResources() = default;
private:
    virtual ~RoseAuraResources() = default;

    static RoseAuraResources* mIns;

};
