#version 450

layout (set = 1, binding = 0) uniform sampler2D texSampler;

layout (location = 0) in vec2 fragUv;
layout (location = 1) in vec3 fragNormal;

layout (location = 0) out vec4 outColor;

void main()
{
    outColor = texture(texSampler, fragUv);
}
