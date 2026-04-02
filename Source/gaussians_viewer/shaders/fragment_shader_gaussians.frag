#version 430 core

uniform float ambient_strength;
uniform vec3 ambient_color;
uniform vec3 diffuse_pos;
uniform vec3 diffuse_color;
uniform vec3 camera_pos;

in vec3 out_color;
in vec3 out_normal;
in vec3 frag_pos;

out vec4 FragColor;

void main()
{
    vec3 normal = normalize(out_normal);
    
    // ambient
    vec3 ambient = ambient_strength * ambient_color;
    
    // diffuse
    vec3 lightDir = normalize(diffuse_pos - frag_pos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * diffuse_color;
    
    // specular (Blinn-Phong)
    float specularStrength = 0.5;
    vec3 viewDir = normalize(camera_pos - frag_pos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * diffuse_color;
    
    // specular (Phong)
    // float specular_strength = 0.5;
    // vec3 view_dir = normalize(camera_pos - frag_pos);
    // vec3 reflect_dir = reflect(-lightDir, normal);
    // float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    // vec3 specular = specular_strength * spec * diffuse_color;


    FragColor = vec4((ambient + diffuse + specular) * out_color, 1.0);
}