#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D entryTexture;
uniform int mode;

//Light structure
struct Light {
    vec3 position; 
    vec3 ambient;
    vec3 diffuse; 
    vec3 specular;
};

//Material structure
struct Material {
    vec3 specular;
    float shininess;
};

uniform Light light;
uniform Material material;

uniform vec3 viewPos; // Camera position

void main() {
    if (mode == 1)
        FragColor = vec4(1.0, 1.0, 0.0, 1.0); // Yellow for edges (Wireframe)
    else if (mode == 2)
        FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red for vertices (Points)
    else // mode == 0 (Faces - Filled)
    {		
        vec4 texColor = texture(entryTexture, TexCoord);

        // 1. Ambient Lighting
        vec3 ambient = light.ambient * vec3(texColor); 

        // 2. Diffuse Lighting
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(light.position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light.diffuse * (diff * vec3(texColor));

        // 3. Specular Lighting
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = light.specular * (spec * material.specular);  
        
        // Final Color
        vec3 result = ambient + diffuse + specular;
        FragColor = vec4(result, texColor.a);
    }
}