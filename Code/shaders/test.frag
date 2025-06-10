#version 420
#extension GL_KHR_vulkan_glsl : enable

layout(location=0)in vec4 V_Texcoord;
layout(location=1)in vec4 V_NormalWS;
layout(location=2)flat in uint V_instanceID;

layout(location=0)out vec4 RT0;

layout(set = 1, binding = 0) uniform sampler2D texSampler[3];

void main(){
    vec3 color = texture(texSampler[V_instanceID], V_Texcoord.xy).rgb;
    RT0=vec4(color, 1.0);
}