#version 420

layout(location=0)out vec4 RT0;
layout(location=0)in vec4 V_Color;


layout(binding = 2) uniform sampler2D texSampler;

void main(){
    RT0=V_Color;
}