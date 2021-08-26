#version 330 core

const float PI = 3.14159265359;

out vec4 FragColor;
in vec3 normal;
in vec3 fragPos;
in vec3 texCoords;

uniform vec3 viewPos;


struct PointLight {
	vec3 position;
	vec3 intensity;

	float constant;
	float linear;
	float quadratic;
	
	float size;
	bool castShadows;
	samplerCube shadowMap;
	float far_plane;
};

const vec3 sampleOffsetDirections[20] = vec3[]
(
	vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
	vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
	vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
	vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
	vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
	);

uniform PointLight pointlight;

struct PBRmaterial {
	vec3 albedo;
	float metallic;
	float roughness;
	float ao;
};
uniform PBRmaterial material;

vec3 albedo = vec3(0.3, 0.5, 0.0);
float metallic = 1.0;
float roughness = 0.15;
float ao = 1.0;

//functions used for cook-terrance BRDF
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

float pcfPointLight(PointLight light, float bias) {
	float size = 0.01;
	vec3 fragToLight = fragPos - light.position;
	float currentDepth = length(fragToLight);
	float shadow = 0.0;
	for (int i = 0; i < 20; ++i) {
		float closestDepth = texture(light.shadowMap, fragToLight + sampleOffsetDirections[i] * size).r;
		closestDepth *= light.far_plane;
		if (currentDepth - bias > closestDepth) {
			shadow += 1.0;
		}
	}
	return shadow / 20.0;
}

float PointLightShadowCalculation(PointLight light, float bias) {
	vec3 fragToLight = fragPos - light.position;
	float closestDepth = texture(light.shadowMap, fragToLight).r;
	closestDepth *= light.far_plane;
	float currentDepth = length(fragToLight);
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	return shadow;
}


vec3 CalcPointLight(PointLight light, vec3 N, vec3 V, vec3 F0) {
	vec3 L = normalize(light.position - fragPos);
	vec3 H = normalize(V + L);
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
	vec3 radiance = light.intensity * attenuation;

	vec3 F = fresnelSchlick(max(dot(H, V), 0), F0);

	float NDF = DistributionGGX(N, H, material.roughness);
	float G = GeometrySmith(N, V, L, material.roughness);

	vec3 numerator = NDF * G * F;
	float denomirator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = numerator / max(denomirator, 0.001);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - material.metallic;

	float NdotL = max(dot(N, L), 0.0);
	
	//shadow calculations here
	float shadow = 0.0;
	if (light.castShadows) {
		shadow = pcfPointLight(light, 0.05);
		//shadow = PointLightShadowCalculation(light, 0.05);
	}

	return (kD * material.albedo / PI + specular) * radiance * NdotL*(1-shadow);

}

void main() {
	vec3 N = normalize(normal);
	vec3 V = normalize(viewPos - fragPos);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, material.albedo, material.metallic);

	vec3 L0 = vec3(0.0);

	vec3 ambient = vec3(0.03) * material.albedo * material.ao;
	vec3 color = ambient + CalcPointLight(pointlight, N, V, F0);
	FragColor = vec4(color, 1.0);

}