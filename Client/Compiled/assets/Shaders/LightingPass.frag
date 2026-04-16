#version 450

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 UV;

layout (set = 0, binding = 0) uniform sampler2D GColor;
layout (set = 0, binding = 1) uniform sampler2D Gmrs;
layout (set = 0, binding = 2) uniform sampler2D GNormal;
layout (set = 0, binding = 3) uniform sampler2D GPosition;

layout (set = 0, binding = 4) uniform MiscsBuffer {
    vec3 viewPos;
} Miscs;

//temporarily hardcoding them values
vec3 LightDir = normalize(vec3(1.0, -1.0, 1.0));
vec3 LightColor = vec3(1.0, 1.0, 1.0);

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a  = roughness * roughness;
    float a2 = a * a;

    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    return NdotV / (NdotV * (1.0 - k) + k);
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
vec3 DiffuseBurley(vec3 albedo, float roughness, float NdotV, float NdotL, float LdotH) {
    float FD90 = 0.5 + 2.0 * LdotH * LdotH * roughness;

    float lightScatter = 1.0 + (FD90 - 1.0) * pow(1.0 - NdotL, 5.0);
    float viewScatter  = 1.0 + (FD90 - 1.0) * pow(1.0 - NdotV, 5.0);

    return albedo * lightScatter * viewScatter / PI;
}

void main() {
    //decoding all values from GBuffer
    vec3 Albedo = texture(GColor, UV).rgb;
    vec3 MRS = texture(Gmrs, UV).rgb;
    vec3 Normal = normalize(texture(GNormal, UV).xyz * 2.0 - 1.0);
    vec3 Position = texture(GPosition, UV).rgb;

    float Metalness = MRS.r;
    float Roughness = clamp(MRS.g, 0.04, 1.0);
    float Specular = MRS.b;

    //actual shading
    vec3 V = normalize(Miscs.viewPos - Position);
    vec3 L = normalize(-LightDir);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(Normal, L), 0.0);
    float NdotV = max(dot(Normal, V), 0.0);
    float LdotH = max(dot(L, H), 0.0);

    vec3 F0 = mix(vec3(0.04), Albedo, Metalness);

    float NDF = DistributionGGX(Normal, H, Roughness);
    float G   = GeometrySmith(Normal, V, L, Roughness);
    vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 specular = (NDF * G * F) / max(4.0 * NdotV * NdotL, 0.0001);

    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - Metalness);

    vec3 diffuse = kD * DiffuseBurley(Albedo, Roughness, NdotV, NdotL, LdotH);

    vec3 lightColor = LightColor;

    vec3 color = (diffuse + specular) * lightColor * NdotL;

    vec3 ambient = 0.03 * Albedo;

    color += ambient;

    FragColor = vec4(color, 1.0);
}