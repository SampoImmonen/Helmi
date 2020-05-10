#version 330 core


out vec4 FragColor;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoords;


uniform vec3 viewPos;

float ambientStrength = 0.4f;
float specularStrength = 0.5f;

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


uniform Material material;
uniform Light light;




void main()
{
    

    vec3 ambient = light.ambient * material.ambient;

    vec3 T2V = normalize(viewPos - fragPos);
    vec3 norm = normalize(normal);
    vec3 T2L = normalize(light.position - fragPos);

    vec3 reflectDir = reflect(-T2L, norm);

    float spec = pow(max(dot(T2V, reflectDir), 0.0), material.shininess);

	vec3 diffuse;
    float diff = max(dot(norm, T2L), 0.0);
	if (material.hasDiffuse) {
		
		diffuse = light.diffuse * diff * vec3(texture(material.diffuseMap, texCoords));
		ambient = light.ambient * vec3(texture(material.diffuseMap, texCoords));
	}
	else {
		
		diffuse = light.diffuse * diff * material.diffuse;
	}

	vec3 specular;
	if (material.hasSpecular) {
		specular = light.specular * spec * vec3(texture(material.specularMap, texCoords));
	}
	else {
		specular = light.specular * (spec * material.specular);
	}

	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
	
    vec3 color = (ambient + diffuse + specular)*attenuation;
	
    FragColor = vec4(color, 1.0f);
}