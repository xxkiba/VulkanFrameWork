#version 420
#extension GL_KHR_vulkan_glsl : enable

layout(location=0)in vec4 V_Texcoord;
layout(location=1)in vec4 V_NormalWS;
layout(location=2)in vec4 V_PositionWS;
layout(location=3)flat in uint V_instanceID;

layout(location=0)out vec4 RT0;

layout(set = 0, binding = 2) uniform samplerCube skyboxSampler;
layout(set = 1, binding = 0) uniform sampler2D texSampler[3];

void main(){
    // Sample the skybox texture
    vec3 N = normalize(V_NormalWS.xyz);
    vec3 V = normalize(V_PositionWS.xyz);
    vec3 R = reflect(V, N);
    vec3 color = texture(skyboxSampler, R).rgb;
    //vec3 color = texture(texSampler[V_instanceID], V_Texcoord.xy).rgb;
    RT0=vec4(color, 1.0);
}