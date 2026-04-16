#version 450

layout (set = 0, binding = 0) uniform SceneUBO
{
    mat4 viewProjection;
} scene;

layout (push_constant) uniform PushConstants
{
    mat4 model;
} push;

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 inNormal;
layout (location = 2) in vec2 inUv;

layout (location = 0) out vec2 fragUv;
layout (location = 1) out vec3 fragNormal;

void main()
{
    vec3 worldPos = (push.model * vec4(inPosition.xyz, 1.0)).xyz;
    gl_Position = scene.viewProjection * vec4(worldPos, 1.0);

    fragNormal = normalize(mat3(push.model) * inNormal.xyz);
    fragUv = inUv;
}
