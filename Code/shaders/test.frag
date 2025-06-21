#version 420
#extension GL_KHR_vulkan_glsl : enable

layout(location=0)in vec4 V_Texcoord;
layout(location=1)in vec4 V_NormalWS;
layout(location=2)in vec4 V_PositionWS;
layout(location=3)flat in uint V_instanceID;

layout(location=0)out vec4 FragColor;

layout(set = 0, binding = 2) uniform samplerCube skyboxSampler;
layout(set = 0, binding = 3) uniform cameraParameters{
    vec4 CameraWorldPosition;
};
layout(set = 1, binding = 0) uniform sampler2D texSampler[3];

const float PI = 3.14159265359;
vec3 F(vec3 inF0, vec3 inH, vec3 inV){// Schlick Fresnel equation
    // inF0: Fresnel reflectance at normal incidence
    float HDotV = max(dot(inH, inV), 0.0);
    return inF0 + (vec3(1.0) - inF0) * pow(1.0 - HDotV, 5.0);
}

float NDF(vec3 inN, vec3 inH, float inRoughness){
    // Normal Distribution Function (NDF) for microfacet model
    float NdotH = max(dot(inN, inH), 0.0);
    float alpha = inRoughness * inRoughness;
    float denom = (NdotH * NdotH) * (alpha * alpha - 1.0) + 1.0;
    return alpha * alpha / (PI * denom * denom);
}

float G1(float inNdotX, float inRoughness){
    // Geometry function for shadowing/masking
    float k = pow(inRoughness + 1.0, 2.0) / 8.0; // k is a constant based on roughness
    return inNdotX / (inNdotX * (1.0 - k) + k);
}

float G(vec3 inN, vec3 inV, vec3 inL, float inRoughness){
    // Combined geometry function
    float NdotV = max(dot(inN, inV), 0.0);
    float NdotL = max(dot(inN, inL), 0.0);
    return G1(NdotV, inRoughness) * G1(NdotL, inRoughness);
}


void main(){
    
    vec3 L = normalize(vec3(1.0,1.0, 1.0)); // Light direction
    vec3 N = normalize(V_NormalWS.xyz);
    vec3 V = normalize(CameraWorldPosition.xyz - V_PositionWS.xyz); // surface -> eye
    vec3 H = normalize(L + V); // Halfway vector

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float roughness = 0.1; // Example roughness value, can be replaced with a varying input

    vec3 F0 = vec3(0.04); // Fresnel reflectance at normal incidence for dielectrics
    vec3 albedo = vec3(1.0,0.0,0.0); // Example albedo color, can be replaced with a varying input
    vec3 FinalColor = vec3(0.0);
    float eps = 0.01;

    float metallic = 0.6; // use metallic 
    F0 = mix(F0, albedo, metallic); // Adjust F0 based on metallic property, linear interpolation between F0 and albedo


    // Cook-Torrance BRDF model
    // direct light 
    {
        roughness = max(roughness, eps);
        vec3 Fresnel = F(F0, H, V);
        float D = NDF(N, H, roughness);
        float GFactor = G(N, V, L, roughness);
        vec3 specularColor = Fresnel * D * GFactor / (4.0 * NdotL * NdotV + eps);// Specular reflection term
        FinalColor += specularColor;

        vec3 kd = vec3(1.0) - Fresnel; // Diffuse reflectance
        kd *= 1.0 - metallic; // Adjust diffuse color based on metallic property
        vec3 diffuseColor = kd * albedo / PI; // Lambertian diffuse term

        FinalColor += diffuseColor;
        FinalColor *= NdotL * vec3(1.0,1.0,1.0); // Apply light contribution
    }
    
    //ibl : ambient color
    {
    }

    FragColor = vec4(FinalColor, 1.0);
    //FragColor = vec4(vec3(NdotL), 1.0);
}