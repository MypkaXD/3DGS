#version 330 core

layout (location = 0) in vec3 mu;
layout (location = 1) in vec3 sigma;
layout (location = 2) in vec4 quat;
layout (location = 3) in float q;
layout (location = 4) in float opacity;
layout (location = 5) in vec2 pos_uv;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

mat3 quaternion_to_matrix(vec4 q) {
    float x = q.x, y = q.y, z = q.z, w = q.w;
    
    float xx = x * x;
    float xy = x * y;
    float xz = x * z;
    float xw = x * w;
    
    float yy = y * y;
    float yz = y * z;
    float yw = y * w;
    
    float zz = z * z;
    float zw = z * w;
    
    return mat3(
        1.0 - 2.0 * (yy + zz), 2.0 * (xy - zw), 2.0 * (xz + yw),
        2.0 * (xy + zw), 1.0 - 2.0 * (xx + zz), 2.0 * (yz - xw),
        2.0 * (xz - yw), 2.0 * (yz + xw), 1.0 - 2.0 * (xx + yy)
    );
}

void main()
{

    vec3 sphere_point = vec3(
        sin(pos_uv.y) * cos(pos_uv.x),
        sin(pos_uv.y) * sin(pos_uv.x),
        cos(pos_uv.y)
    );

    vec3 scaled_point = sphere_point * sigma;

    mat3 rotation = quaternion_to_matrix(quat);
    vec3 rotated_point = rotation * scaled_point;

    vec3 pos = mu + q * rotated_point;
    
    gl_Position = projection * view * model * vec4(pos, 1.0);
}