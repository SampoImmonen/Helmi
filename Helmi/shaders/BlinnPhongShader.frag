#version 330 core


layout (location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoords;
in mat3 TBN;
in vec4 fragPosLightSpaceDirLight;
in vec4 fragPosLightSpaceSpotLight;

uniform vec3 viewPos;
uniform float bloomThreshold = 1.0;


const int num_blocker_samples = 16;
const float light_size = 0.05;
const int num_pcf_samples = 64;

const vec2 poissonDisk[64] =  vec2[](
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

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	vec3 emission;

	float shininess;

	bool hasDiffuse;
	bool hasSpecular;
	bool hasNormal;
	bool hasEmission;

	sampler2D diffuseMap;
	sampler2D specularMap;
	sampler2D normalMap;
	sampler2D emissionMap;
};


//uniform sampler2D shadowMap;

struct Light {
    vec3 position;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

struct PointLight {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;

	float size;
	bool castShadows;
	sampler2D shadowMap;
};


struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float size;
	bool castShadows;
	sampler2D shadowMap;
};

struct SpotLight {
	vec3 position;
	vec3 direction;

	float cutOff;
	float outerCutOff;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;

	float size;
	bool castShadows;
	sampler2D shadowMap;
};

uniform SpotLight spotLight;
uniform DirLight dirLight;
//uniform PointLight pointLight;

uniform Material material;
uniform Light light;
uniform float exposure;

float ShadowCalculationPoisson(vec4 fragPosLightSpace, float bias, sampler2D shadowMap) {
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	float visibility = 1.0;
	float currentDepth = projCoords.z;
	for (int i = 0; i < 4; i++) {
		if (texture(shadowMap, projCoords.xy + poissonDisk[i] / 700.0).r < currentDepth - bias) {
			visibility -= 0.2;
		}
	}
	//float closestDepth = texture(shadowMap, projCoords.xy).r;
	//float shadow = currentDepth-bias > closestDepth ? 1.0 : 0.5;
	return 1.0-visibility;
}

vec2 blockerSearchDirectionalLight(vec2 texCoords, float zReceiver, float bias, sampler2D shadowMap) {
	float blockers_dist = 0;
	float num_blockers = 0;
	for (int i = 0; i < num_blocker_samples; i++) {
		vec2 stexCoords = texCoords + poissonDisk[i] * dirLight.size;
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
			sum+=1.0;
		}
	}
	return sum / num_pcf_samples;
}


float PCSSDirectionalLight(vec4 fragPosLightSpace, float bias, sampler2D shadowMap) {
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	vec2 ptexCoords = projCoords.xy;
	vec2 blocker_stats = blockerSearchDirectionalLight(ptexCoords, projCoords.z ,bias, shadowMap);
	if (blocker_stats.y < 1) {
		return 0.0f;
	}
	float filterradius = dirLight.size *penumbraSize(projCoords.z, blocker_stats[0]);
	return pcf(ptexCoords, projCoords.z, filterradius, bias, shadowMap);
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 viewPos, vec3 fragPos) {

	vec3 lightDir = normalize(light.position - fragPos);
	normal = normalize(normal);

	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = (light.cutOff - light.outerCutOff);
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	vec3 viewDir = normalize(viewPos - fragPos);

	float diff = max(dot(normal, lightDir), 0.0);

	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

	vec3 diffuse;
	vec3 ambient;
	if (material.hasDiffuse) {

		diffuse = light.diffuse * diff * vec3(texture(material.diffuseMap, texCoords));
		ambient = light.ambient * vec3(texture(material.diffuseMap, texCoords));
	}
	else {

		diffuse = light.diffuse * diff * material.diffuse;
		ambient = light.ambient * material.ambient;
	}

	vec3 specular;
	if (material.hasSpecular) {
		specular = light.specular * spec * vec3(texture(material.specularMap, texCoords));
	}
	else {
		specular = light.specular * (spec * material.specular);
	}

	//float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	//float shadow = ShadowCalculationPoisson(fragPosLightSpace, bias);sds
	float bias = 0.0;
	float shadow = 0.0;
	if (light.castShadows) {
		shadow = PCSSDirectionalLight(fragPosLightSpaceSpotLight, bias, light.shadowMap);
		//shadow = 0.7;
	}
	return intensity*attenuation * (0.15*ambient + (1-shadow)*(diffuse + specular));
}


vec3 calcDirectionalLight(DirLight light, vec3 inormal, vec3 viewPos) {
	
	vec3 lightDir = normalize(light.direction);
	inormal = normalize(inormal);

	vec3 viewDir = normalize(viewPos - fragPos);

	float diff = max(dot(inormal, lightDir), 0.0);
	
	vec3 reflectDir = reflect(-lightDir, inormal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(inormal, halfwayDir), 0.0), material.shininess);
	
	vec3 diffuse;
	vec3 ambient;
	if (material.hasDiffuse) {

		diffuse = light.diffuse * diff * vec3(texture(material.diffuseMap, texCoords));
		ambient = light.ambient * vec3(texture(material.diffuseMap, texCoords));
	}
	else {

		diffuse = light.diffuse * diff * material.diffuse;
		ambient = light.ambient * material.ambient;
	}

	vec3 specular;
	if (material.hasSpecular) {
		specular = light.specular * spec * vec3(texture(material.specularMap, texCoords));
	}
	else {
		specular = light.specular * (spec * material.specular);
	}

	
	//bias = 0.005f;
	//float shadow = ShadowCalculationPoisson(fragPosLightSpace, bias);
	float shadow = 0.0;
	if (light.castShadows){
		float bias = max(0.05 * (1.0 - dot(inormal, lightDir)), 0.005);
		bias = 
		//bias = 0.0002;
		shadow = PCSSDirectionalLight(fragPosLightSpaceDirLight, bias, light.shadowMap);
	}
	//shadow = 0.0;
	return (0.15*ambient + (1-shadow)*(diffuse + specular));
}

vec3 calcPointLight(Light light, vec3 inormal, vec3 fragPos, vec3 viewPos) {

	vec3 lightDir = normalize(light.position - fragPos);
	vec3 viewDir = normalize(viewPos - fragPos);
	inormal = normalize(inormal);
	float diff = abs(dot(inormal, lightDir));

	vec3 reflectDir = reflect(-lightDir, inormal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(inormal, halfwayDir), 0.0), material.shininess);

	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

	vec3 diffuse;
	vec3 ambient;
	if (material.hasDiffuse) {

		diffuse = light.diffuse * diff * vec3(texture(material.diffuseMap, texCoords));
		ambient = light.ambient * vec3(texture(material.diffuseMap, texCoords));
	}
	else {

		diffuse = light.diffuse * diff * material.diffuse;
		ambient = light.ambient * material.ambient;
	}

	vec3 specular;
	if (material.hasSpecular) {
		specular = light.specular * spec * vec3(texture(material.specularMap, texCoords));
	}
	else {
		specular = light.specular * (spec * material.specular);
	}

	
	return attenuation * (ambient + diffuse + specular);

}





void main()
{
	const float gamma = 2.2;
	//const float exposure = 1.5;
	vec3 newNormal = normal;
	if (material.hasNormal) {
		newNormal = texture(material.normalMap, texCoords).rgb;
		newNormal = normalize(newNormal * 2.0 - 1.0);
		newNormal = normalize(TBN * newNormal);
	}
	vec3 emission = material.emission;
	if (material.hasEmission) {
		emission = texture(material.emissionMap, texCoords).rgb;
	}


	vec3 color;
	// A lot of room to optimize; calculations done separately for each light
	//color += calcPointLight(light, normal, fragPos, viewPos);
	
	color += calcDirectionalLight(dirLight, newNormal, viewPos);
	color += calcSpotLight(spotLight, normal, viewPos, fragPos);
	color += 1.3*emission;
	//vec3 mapped = vec3(1.0)-exp(-color*exposure);
	//mapped = pow(mapped, vec3(1.0 / gamma));
    FragColor = vec4(color, 1.0);
	float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
	if (brightness > bloomThreshold) {
		BrightColor = vec4(FragColor.rgb, 1.0);
	}
	else {
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
	}

}