#version 330 core


out vec4 FragColor;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoords;


uniform vec3 viewPos;


struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

	float shininess;

	bool hasDiffuse;
	bool hasSpecular;

	sampler2D diffuseMap;
	sampler2D specularMap;
    
};

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



vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 viewPos, vec3 fragPos) {

	
	
	
	vec3 lightDir = normalize(light.position - fragPos);
	normal = normalize(normal);

	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = (light.cutOff - light.outerCutOff);
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	vec3 viewDir = normalize(viewPos - fragPos);

	float diff = max(dot(normal, lightDir), 0.0);

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

vec3 calcDirectionalLight(DirLight light, vec3 normal, vec3 viewPos) {
	
	vec3 lightDir = normalize(light.direction);
	normal = normalize(normal);

	vec3 viewDir = normalize(viewPos - fragPos);

	float diff = max(dot(normal, lightDir), 0.0);
	
	vec3 reflectDir = reflect(-lightDir, normal);
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

	return (ambient + diffuse + specular);
}

vec3 calcPointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewPos) {

	vec3 lightDir = normalize(light.position - fragPos);

	vec3 viewDir = normalize(viewPos - fragPos);
	normal = normalize(normal);
	float diff = max(dot(normal, lightDir), 0.0);

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

	return attenuation * (ambient + diffuse + specular);

}




void main()
{
	vec3 color;
	// A lot of room to optimize; calculations done separately for each light
	color += calcPointLight(light, normal, fragPos, viewPos);
	color += calcDirectionalLight(dirLight, normal, viewPos);
	color += calcSpotLight(spotLight, normal, viewPos, fragPos);
    FragColor = vec4(color, 1.0f);
}