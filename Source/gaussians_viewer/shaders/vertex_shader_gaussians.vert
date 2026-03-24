#version 430 core

layout (location = 0) in vec2 pos_uv; // Spherical coordinates (theta, phi)

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

out vec3 out_color;

struct Ellipsoid
{
	float mu[3];
	float sigma[3];
	float quaternion[4];
	float sh_dc[3];
	float sh_rest[45];

	float opacity;
	float Q;
};

layout(std430, binding = 0) buffer Ellipsoids
{
    Ellipsoid ellipsoids[];
};

#define C0 0.28209479177387814
#define C1 0.4886025119029199
#define C2 1.0925484305920792

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

    const Ellipsoid ellipsoid = ellipsoids[gl_InstanceID];

    // Convert spherical coordinates
    vec3 sphere_point = vec3(sin(pos_uv.y) * cos(pos_uv.x), sin(pos_uv.y) * sin(pos_uv.x), cos(pos_uv.y));

    // Scale the point by the ellipsoid's sigma
    vec3 scaled_point = vec3(sphere_point[0] * ellipsoid.sigma[0], sphere_point[1] * ellipsoid.sigma[1], sphere_point[2] * ellipsoid.sigma[2]);

    // Rotate the point using the ellipsoid's quaternion
    mat3 rotation = quaternion_to_matrix(vec4(ellipsoid.quaternion[0], ellipsoid.quaternion[1], ellipsoid.quaternion[2], ellipsoid.quaternion[3]));
    vec3 rotated_point = rotation * scaled_point;

    // Translate the point by the ellipsoid's mu
    vec3 mu = vec3(ellipsoid.mu[0], ellipsoid.mu[1], ellipsoid.mu[2]);
    vec3 pos = mu + ellipsoid.Q * rotated_point;
    
    // Calculate the color based on the ellipsoid's SH coefficients

    vec3 sh0_color = vec3(clamp(ellipsoid.sh_dc[0] * C0 + 0.5, 0, 1), clamp(ellipsoid.sh_dc[1] * C0 + 0.5, 0, 1), clamp(ellipsoid.sh_dc[2] * C0 + 0.5, 0, 1));

    vec3 sh1_color = vec3(
        C1 * (ellipsoid.sh_rest[0] * sphere_point.y + ellipsoid.sh_rest[1] * sphere_point.z + ellipsoid.sh_rest[2] * sphere_point.x),
        C1 * (ellipsoid.sh_rest[3] * sphere_point.y + ellipsoid.sh_rest[4] * sphere_point.z + ellipsoid.sh_rest[5] * sphere_point.x),
        C1 * (ellipsoid.sh_rest[6] * sphere_point.y + ellipsoid.sh_rest[7] * sphere_point.z + ellipsoid.sh_rest[8] * sphere_point.x)
    );

    float x = sphere_point.x;
    float y = sphere_point.y;
    float z = sphere_point.z;

    vec3 sh2_color = vec3(0.0, 0.0, 0.0);

    sh2_color.r += C2 * ellipsoid.sh_rest[9]  * (x*x - y*y);
    sh2_color.g += C2 * ellipsoid.sh_rest[14] * (x*x - y*y);
    sh2_color.b += C2 * ellipsoid.sh_rest[19] * (x*x - y*y);

    sh2_color.r += C2 * ellipsoid.sh_rest[10] * (y*z);
    sh2_color.g += C2 * ellipsoid.sh_rest[15] * (y*z);
    sh2_color.b += C2 * ellipsoid.sh_rest[20] * (y*z);

    sh2_color.r += C2 * ellipsoid.sh_rest[11] * (3.0*z*z - 1.0);
    sh2_color.g += C2 * ellipsoid.sh_rest[16] * (3.0*z*z - 1.0);
    sh2_color.b += C2 * ellipsoid.sh_rest[21] * (3.0*z*z - 1.0);

    sh2_color.r += C2 * ellipsoid.sh_rest[12] * (x*z);
    sh2_color.g += C2 * ellipsoid.sh_rest[17] * (x*z);
    sh2_color.b += C2 * ellipsoid.sh_rest[22] * (x*z);

    sh2_color.r += C2 * ellipsoid.sh_rest[13] * (x*y);
    sh2_color.g += C2 * ellipsoid.sh_rest[18] * (x*y);
    sh2_color.b += C2 * ellipsoid.sh_rest[23] * (x*y);

    out_color = sh0_color + sh1_color + sh2_color;

    gl_Position = projection * view * model * vec4(pos, 1.0);
}