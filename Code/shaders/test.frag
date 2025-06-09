#version 420
#extension GL_KHR_vulkan_glsl : enable
layout(location=0)out vec4 RT0;
layout(location=0)in vec4 V_Color;


layout(set = 1, binding = 0) uniform sampler2D texSampler;

void main(){
    vec3 N = normalize(V_Color.rgb);
    N = (N + vec3(1.0)) * 0.5; // Convert to [0, 1] range
    RT0=vec4(N, 1.0);
}