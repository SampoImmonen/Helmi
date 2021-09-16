#version 330 core

const float PI = 3.14159265359;
const int num_pcf_samples = 64;
const int num_blocker_samples = 16;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoords;
in vec4 fragPosLightSpaceDirLight;
in vec4 fragPosLightSpaceSpotLight;
in mat3 TBN;


uniform float bloomThreshold = 1.0;
uniform vec3 viewPos;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

struct surfaceProperties {
	vec3 albedo;
	float metallic;
	float roughness;
	float ao;
	vec3 emission;
};



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

struct DirLight {
	//inverse direction
	vec3 direction;

	vec3 intensity;

	float size;
	bool castShadows;
	sampler2D shadowMap;
};

struct SpotLight {
	vec3 position;
	vec3 direction;

	float cutOff;
	float outerCutOff;
	
	vec3 intensity;

	float constant;
	float linear;
	float quadratic;

	float size;
	bool castShadows;
	sampler2D shadowMap;
};

const vec3 sampleOffsetDirections[20] = vec3[]
(
	vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
	vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
	vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
	vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
	vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
	);

const vec2 poissonDisk[64] = vec2[](
	vec2(-0.04117257f, -0.1597612f),
	vec2(0.06731031f, -0.4353096f),
	vec2(-0.206701f, -0.4089882f),
	vec2(0.1857469f, -0.2327659f),
	vec2(-0.2757695f, -0.159873f),
	vec2(-0.2301117f, 0.1232693f),
	vec2(0.05028719f, 0.1034883f),
	vec2(0.236303f, 0.03379251f),
	vec2(0.1467563f, 0.364028f),
	vec2(0.516759f, 0.2052845f),
	vec2(0.2962668f, 0.2430771f),
	vec2(0.3650614f, -0.1689287f),
	vec2(0.5764466f, -0.07092822f),
	vec2(-0.5563748f, -0.4662297f),
	vec2(-0.3765517f, -0.5552908f),
	vec2(-0.4642121f, -0.157941f),
	vec2(-0.2322291f, -0.7013807f),
	vec2(-0.05415121f, -0.6379291f),
	vec2(-0.7140947f, -0.6341782f),
	vec2(-0.4819134f, -0.7250231f),
	vec2(-0.7627537f, -0.3445934f),
	vec2(-0.7032605f, -0.13733f),
	vec2(0.8593938f, 0.3171682f),
	vec2(0.5223953f, 0.5575764f),
	vec2(0.7710021f, 0.1543127f),
	vec2(0.6919019f, 0.4536686f),
	vec2(0.3192437f, 0.4512939f),
	vec2(0.1861187f, 0.595188f),
	vec2(0.6516209f, -0.3997115f),
	vec2(0.8065675f, -0.1330092f),
	vec2(0.3163648f, 0.7357415f),
	vec2(0.5485036f, 0.8288581f),
	vec2(-0.2023022f, -0.9551743f),
	vec2(0.165668f, -0.6428169f),
	vec2(0.2866438f, -0.5012833f),
	vec2(-0.5582264f, 0.2904861f),
	vec2(-0.2522391f, 0.401359f),
	vec2(-0.428396f, 0.1072979f),
	vec2(-0.06261792f, 0.3012581f),
	vec2(0.08908027f, -0.8632499f),
	vec2(0.9636437f, 0.05915006f),
	vec2(0.8639213f, -0.309005f),
	vec2(-0.03422072f, 0.6843638f),
	vec2(-0.3734946f, -0.8823979f),
	vec2(-0.3939881f, 0.6955767f),
	vec2(-0.4499089f, 0.4563405f),
	vec2(0.07500362f, 0.9114207f),
	vec2(-0.9658601f, -0.1423837f),
	vec2(-0.7199838f, 0.4981934f),
	vec2(-0.8982374f, 0.2422346f),
	vec2(-0.8048639f, 0.01885651f),
	vec2(-0.8975322f, 0.4377489f),
	vec2(-0.7135055f, 0.1895568f),
	vec2(0.4507209f, -0.3764598f),
	vec2(-0.395958f, -0.3309633f),
	vec2(-0.6084799f, 0.02532744f),
	vec2(-0.2037191f, 0.5817568f),
	vec2(0.4493394f, -0.6441184f),
	vec2(0.3147424f, -0.7852007f),
	vec2(-0.5738106f, 0.6372389f),
	vec2(0.5161195f, -0.8321754f),
	vec2(0.6553722f, -0.6201068f),
	vec2(-0.2554315f, 0.8326268f),
	vec2(-0.5080366f, 0.8539945f)
	);

uniform PointLight pointlight;
uniform DirLight dirlight;
uniform SpotLight spotlight;

struct PBRmaterial {
	vec3 albedo;
	float metallic;
	float roughness;
	float ao;
	vec3 emission;

	bool hasAlbedo;
	bool hasRoughness;
	bool hasNormal;
	bool hasEmission;
	bool hasMetalness;

	sampler2D albedoMap;
	sampler2D roughnessMap;
	sampler2D normalMap;
	sampler2D emissionMap;
	sampler2D metalnessMap;
};
uniform PBRmaterial material;

surfaceProperties getSurfaceProperties() {
	//if statements in shaders are bad????
	surfaceProperties properties;
	//albedo
	if (material.hasAlbedo) {
		properties.albedo = pow(texture(material.albedoMap, texCoords).rgb, vec3(2.2));
	}
	else {
		properties.albedo = material.albedo;
	}
	//roughness
	if (material.hasRoughness) {
		properties.roughness = texture(material.roughnessMap, texCoords).r;
	}
	else {
		properties.roughness = material.roughness;
	}
	//emission
	if (material.hasEmission) {
		properties.emission = texture(material.emissionMap, texCoords).rgb;
	}
	else {
		properties.emission = material.emission;
	}
	//metalness
	if (material.hasMetalness) {
		properties.metallic = texture(material.metalnessMap, texCoords).r;
	}
	else {
		properties.metallic = material.metallic;
	}
	return properties;
}

//functions used for cook-terrance BRDF
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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

vec2 blockerSearchDirectionalLight(vec2 texCoords, float zReceiver, float bias, sampler2D shadowMap, float size) {
	float blockers_dist = 0;
	float num_blockers = 0;
	for (int i = 0; i < num_blocker_samples; i++) {
		vec2 stexCoords = texCoords + poissonDisk[i] * size;
		float b_dist = texture(shadowMap, stexCoords).r;
		if (b_dist < zReceiver - bias) {
			num_blockers++;
			blockers_dist += b_dist;
		}
	}
	return vec2(blockers_dist / num_blockers, num_blockers);
}


float penumbraSize(float zReceiver, float avgBlockerDist) {
	return (zReceiver - avgBlockerDist) / zReceiver;
}

float pcf(vec2 texCoords, float zReceiver, float filterRadius, float bias, sampler2D shadowMap) {
	float sum = 0.0;
	for (int i = 0; i < num_pcf_samples; i++) {
		vec2 sTexCoords = texCoords + poissonDisk[i] * filterRadius;
		if (texture(shadowMap, sTexCoords).r < zReceiver - bias) {
			sum += 1.0;
		}
	}
	return sum / num_pcf_samples;
}

float PCSSshadows(vec4 fragPosLightSpace, float bias, sampler2D shadowMap, float size) {
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	vec2 ptexCoords = projCoords.xy;
	vec2 blocker_stats = blockerSearchDirectionalLight(ptexCoords, projCoords.z, bias, shadowMap, size);
	if (blocker_stats.y < 1) {
		return 0.0f;
	}
	float filterradius = size * penumbraSize(projCoords.z, blocker_stats[0]);
	return pcf(ptexCoords, projCoords.z, filterradius, bias, shadowMap);
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


vec3 CalcPointLight(PointLight light, vec3 N, vec3 V, vec3 F0, surfaceProperties props) {
	vec3 L = normalize(light.position - fragPos);
	vec3 H = normalize(V + L);
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
	vec3 radiance = light.intensity * attenuation;

	vec3 F = fresnelSchlick(max(dot(H, V), 0), F0);

	float NDF = DistributionGGX(N, H, props.roughness);
	float G = GeometrySmith(N, V, L, props.roughness);

	vec3 numerator = NDF * G * F;
	float denomirator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = numerator / max(denomirator, 0.001);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - props.metallic;

	float NdotL = max(dot(N, L), 0.0);
	
	//shadow calculations here
	float shadow = 0.0;
	if (light.castShadows) {
		shadow = pcfPointLight(light, 0.05);
		//shadow = PointLightShadowCalculation(light, 0.05);
	}

	return (kD * props.albedo / PI + specular) * radiance * NdotL*(1-shadow);
}

vec3 CalcDirectionalLight(DirLight light, vec3 N, vec3 V, vec3 F0, surfaceProperties props) {
	vec3 L = normalize(light.direction);
	vec3 H = normalize(V + L);
	//float distance = length(light.position - fragPos);
	vec3 radiance = light.intensity;

	vec3 F = fresnelSchlick(max(dot(H, V), 0), F0);

	float NDF = DistributionGGX(N, H, props.roughness);
	float G = GeometrySmith(N, V, L, props.roughness);

	vec3 numerator = NDF * G * F;
	float denomirator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = numerator / max(denomirator, 0.001);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - props.metallic;
	float shadow = 0.0;
	if (light.castShadows) {
		//float bias = max(0.05 * (1.0 - dot(N, lightDir)), 0.005);
		//bias = 0.0002;
		shadow = PCSSshadows(fragPosLightSpaceDirLight, 0.05, light.shadowMap, light.size);
	}

	float NdotL = max(dot(N, L), 0.0);
	return (kD * props.albedo / PI + specular) * radiance * NdotL*(1-shadow);
}


vec3 CalcSpotLight(SpotLight light, vec3 N, vec3 V, vec3 F0, surfaceProperties props) {
	vec3 L = normalize(light.position - fragPos);
	vec3 H = normalize(V + L);
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
	
	float theta = dot(L, normalize(-light.direction));
	float epsilon = (light.cutOff - light.outerCutOff);
	float spot_intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	
	vec3 radiance = light.intensity * attenuation*spot_intensity;
	vec3 F = fresnelSchlick(max(dot(H, V), 0), F0);

	float NDF = DistributionGGX(N, H, props.roughness);
	float G = GeometrySmith(N, V, L, props.roughness);

	vec3 numerator = NDF * G * F;
	float denomirator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = numerator / max(denomirator, 0.001);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - props.metallic;

	float NdotL = max(dot(N, L), 0.0);

	//shadow calculations here
	float shadow = 0.0;
	if (light.castShadows) {
		float bias = 0.00;
		shadow = PCSSshadows(fragPosLightSpaceSpotLight, 0.00, light.shadowMap, light.size);
		//shadow = 0.7;
	}

	return (kD * props.albedo / PI + specular) * radiance * NdotL * (1 - shadow);
}



void main() {

	surfaceProperties props = getSurfaceProperties();

	vec3 N = normalize(normal);
	if (material.hasNormal) {
		N = texture(material.normalMap, texCoords).rgb;
		N = normalize(N * 2.0 - 1.0);
		N = normalize(TBN * N);
	}
	
	//N = normalize(normal);
	


	vec3 V = normalize(viewPos - fragPos);
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, props.albedo, props.metallic);
	vec3 L0 = vec3(0.0);

	//indirect ambient
	vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, props.roughness);
	vec3 kD = 1.0 - kS;
	vec3 irradiance = texture(irradianceMap, N).rgb;
	vec3 diffuse = irradiance * props.albedo;

	//indirect specular
	vec3 R = reflect(-V, N);
	const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefiltercolor = textureLod(prefilterMap, R, props.roughness * MAX_REFLECTION_LOD).rgb;
	vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, props.roughness);
	vec2 envBRDF = texture(brdfLUT, vec2(max(dot(N, V), 0.0), props.roughness)).rg;
	vec3 specular = prefiltercolor * (F * envBRDF.x + envBRDF.y);

	vec3 ambient = (kD * diffuse + specular) * material.ao;

	vec3 color = ambient;
	color += CalcPointLight(pointlight, N, V, F0, props);
	color += CalcDirectionalLight(dirlight, N, V, F0, props);
	color += CalcSpotLight(spotlight, N, V, F0, props);
	color += props.emission * 200.0;
	FragColor = vec4(color, 1.0);

	float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
	if (brightness > bloomThreshold) {
		BrightColor = vec4(FragColor.rgb, 1.0);
	}
	else {
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
	}

}