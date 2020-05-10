#version 330 core


out vec4 FragColor;

in vec3 normal;
in vec3 fragPos;


uniform vec3 viewPos;

float ambientStrength = 0.4f;
float specularStrength = 0.5f;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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


    float diff = max(dot(norm, T2L), 0.0);
    vec3 diffuse = light.diffuse*(diff * material.diffuse);

    vec3 specular = light.specular*(spec * material.specular);
    

    vec3 color = ambient + diffuse + specular;

    FragColor = vec4(color, 1.0f);
}