#version 420
layout(location=0)in vec4 position;
layout(location=1)in vec4 texcoord;
layout(location=2)in vec4 normal;
layout(location=3)in vec4 tangent;

layout(push_constant)uniform PushConstants {
    mat4 constantVmatrix;
    mat4 constantPmatrix;
}Constants;

layout(binding = 0) uniform VPMatrices {
    mat4 view;
    mat4 projection;
}vpUBO;
layout(binding = 1) uniform ModelMatrix {
    mat4 model;
}objectUBO;

layout(location=0)out vec4 V_Color;
void main(){
    V_Color=normal;
    gl_Position=vpUBO.projection * vpUBO.view * objectUBO.model*position;//ndc
}