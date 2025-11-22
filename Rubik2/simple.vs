#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
// NEW: Attribute for the vertex normal
layout (location = 2) in vec3 aNormal; 

out vec2 TexCoord;
// NEW: Output variables for lighting calculations in the fragment shader
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    
    // Transform position and normal to world space and pass them on
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal; // Normal transformation
}