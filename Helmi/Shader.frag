#version 330 core


out vec4 FragColor;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoords;
in mat3 TBN;
in vec4 fragPosLightSpace;

uniform vec3 viewPos;

vec2 poissonDisk[4] = vec2[](
	vec2(-0.94201624, -0.39906216),
	vec2(0.94558609, -0.76890725),
	vec2(-0.094184101, -0.92938870),
	vec2(0.34495938, 0.29387760)
	);

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

	float shininess;

	bool hasDiffuse;
	bool hasSpecular;
	bool hasNormal;

	sampler2D diffuseMap;
	sampler2D specularMap;
	sampler2D normalMap;
};


uniform sampler2D shadowMap;

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
};


struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
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
};

uniform SpotLight spotLight;
uniform DirLight dirLight;
//uniform PointLight pointLight;

uniform Material material;
uniform Light light;


float ShadowCalculation(vec4 fragPosLightSpace, float bias) {
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


vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 viewPos, vec3 fragPos) {

	vec3 lightDir = normalize(light.position - fragPos);
	normal = normalize(normal);

	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = (light.cutOff - light.outerCutOff);
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	vec3 viewDir = normalize(viewPos - fragPos);

	float diff = abs(dot(normal, lightDir));

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

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

	return intensity*attenuation * (ambient + diffuse + specular);


}

vec3 calcDirectionalLight(DirLight light, vec3 inormal, vec3 viewPos) {
	
	vec3 lightDir = normalize(light.direction);
	inormal = normalize(inormal);

	vec3 viewDir = normalize(viewPos - fragPos);

	float diff = max(dot(inormal, lightDir), 0.0);
	
	vec3 reflectDir = reflect(-lightDir, inormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	
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

	float bias = max(0.05 * (1.0 - dot(inormal, lightDir)), 0.005);
	float shadow = ShadowCalculation(fragPosLightSpace, bias);
	//shadow = 0.0;
	return (0.15*ambient + (1-shadow)*(diffuse + specular));
}

vec3 calcPointLight(Light light, vec3 inormal, vec3 fragPos, vec3 viewPos) {

	vec3 lightDir = normalize(light.position - fragPos);

	vec3 viewDir = normalize(viewPos - fragPos);
	inormal = normalize(inormal);
	float diff = abs(dot(inormal, lightDir));

	vec3 reflectDir = reflect(-lightDir, inormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

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
	const float exposure = 0.5;
	vec3 newNormal = normal;
	if (material.hasNormal) {
		newNormal = texture(material.normalMap, texCoords).rgb;
		newNormal = normalize(normal * 2.0 - 1.0);
		newNormal = normalize(TBN * newNormal);
	}

	vec3 color;
	// A lot of room to optimize; calculations done separately for each light
	//color += calcPointLight(light, normal, fragPos, viewPos);
	
	color += 0.5*calcDirectionalLight(dirLight, newNormal, viewPos);
	//color += calcSpotLight(spotLight, normal, viewPos, fragPos);
	vec3 mapped = vec3(1.0)-exp(-color*exposure);
	mapped = pow(mapped, vec3(1.0 / gamma));
    FragColor = vec4(mapped, 1.0);
}