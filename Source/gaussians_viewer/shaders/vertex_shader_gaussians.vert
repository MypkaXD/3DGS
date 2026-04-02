#version 430 core

layout (location = 0) in vec2 pos_uv; // Spherical coordinates (theta, phi)

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

out vec3 out_color;
out vec3 out_normal;
out vec3 frag_pos;

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

#define SHC_L0_M0 0.2820947918

#define SHC_L1_MN1 0.3454941495
#define SHC_L1_M0 0.4886025119
#define SHC_L1_M1 -0.3454941495

#define SHC_L2_MN2 0.386274202
#define SHC_L2_MN1 0.772548404
#define SHC_L2_M0 0.3153915653
#define SHC_L2_M1 -0.772548404
#define SHC_L2_M2 0.386274202

#define SHC_L3_MN3 0.4172238236
#define SHC_L3_MN2 1.021985476
#define SHC_L3_MN1 0.3231801841
#define SHC_L3_M0 0.3731763326
#define SHC_L3_M1 -0.3231801841
#define SHC_L3_M2 1.021985476
#define SHC_L3_M3 -0.4172238236


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


// SH's L = 0; M = 0
float get_SH_L0_M0(float value) { return SHC_L0_M0 * value; }

// SH's L = 1; M = -1,...,1
float get_SH_L1_MN1(float value) { return SHC_L1_MN1 * value; }
float get_SH_L1_M0(float value) { return SHC_L1_M0 * value; }
float get_SH_L1_M1(float value) { return SHC_L1_M1 * value; }

// SH's L = 2; M = -2,...,2
float get_SH_L2_MN2(float value) { return SHC_L2_MN2 * value; }
float get_SH_L2_MN1(float value) { return SHC_L2_MN1 * value; }
float get_SH_L2_M0(float value) { return SHC_L2_M0 * value; }
float get_SH_L2_M1(float value) { return SHC_L2_M1 * value; }
float get_SH_L2_M2(float value) { return SHC_L2_M2 * value; }

// SH's L = 3; M = -3,...,3
float get_SH_L3_MN3(float value) { return SHC_L3_MN3 * value; }
float get_SH_L3_MN2(float value) { return SHC_L3_MN2 * value; }
float get_SH_L3_MN1(float value) { return SHC_L3_MN1 * value; }
float get_SH_L3_M0(float value) { return SHC_L3_M0 * value; }
float get_SH_L3_M1(float value) { return SHC_L3_M1 * value; }
float get_SH_L3_M2(float value) { return SHC_L3_M2 * value; }
float get_SH_L3_M3(float value) { return SHC_L3_M3 * value; }

vec3 get_color(vec3 sphere_point, in Ellipsoid ellipsoid)
{
    vec3 sh0_color = vec3(
        clamp(get_SH_L0_M0(ellipsoid.sh_dc[0]) + 0.5, 0, 1), 
        clamp(get_SH_L0_M0(ellipsoid.sh_dc[1]) + 0.5, 0, 1), 
        clamp(get_SH_L0_M0(ellipsoid.sh_dc[2]) + 0.5, 0, 1)
    );

    float x = sphere_point.x; // sin(theta)*cos(phi)
    float y = sphere_point.y; // sin(theta)*sin(phi)
    float z = sphere_point.z; // cos(theta)

    vec3 sh1_color = vec3(
        clamp(ellipsoid.sh_rest[0] * get_SH_L1_MN1(x) + ellipsoid.sh_rest[1] * get_SH_L1_M0(z) + ellipsoid.sh_rest[2] * get_SH_L1_M1(x), 0, 1),
        clamp(ellipsoid.sh_rest[3] * get_SH_L1_MN1(x) + ellipsoid.sh_rest[4] * get_SH_L1_M0(z) + ellipsoid.sh_rest[5] * get_SH_L1_M1(x), 0, 1),
        clamp(ellipsoid.sh_rest[6] * get_SH_L1_MN1(x) + ellipsoid.sh_rest[7] * get_SH_L1_M0(z) + ellipsoid.sh_rest[8] * get_SH_L1_M1(x), 0, 1)
    );

    vec3 sh2_color = vec3(
        clamp(
            ellipsoid.sh_rest[9] * get_SH_L2_MN2(x*x - y*y) + 
            ellipsoid.sh_rest[10] * get_SH_L2_MN1(x * z) + 
            ellipsoid.sh_rest[11] * get_SH_L2_M0(z * z - 1) +
            ellipsoid.sh_rest[12] * get_SH_L2_M1(x * z) +
            ellipsoid.sh_rest[13] * get_SH_L2_M2(x*x - y*y), 0, 1),
        clamp(
            ellipsoid.sh_rest[14] * get_SH_L2_MN2(x*x - y*y) + 
            ellipsoid.sh_rest[15] * get_SH_L2_MN1(x * z) + 
            ellipsoid.sh_rest[16] * get_SH_L2_M0(z * z - 1) +
            ellipsoid.sh_rest[17] * get_SH_L2_M1(x * z) +
            ellipsoid.sh_rest[18] * get_SH_L2_M2(x*x - y*y), 0, 1),
        clamp(
            ellipsoid.sh_rest[19] * get_SH_L2_MN2(x*x - y*y) + 
            ellipsoid.sh_rest[20] * get_SH_L2_MN1(x * z) + 
            ellipsoid.sh_rest[21] * get_SH_L2_M0(z * z - 1) +
            ellipsoid.sh_rest[22] * get_SH_L2_M1(x * z) +
            ellipsoid.sh_rest[23] * get_SH_L2_M2(x*x - y*y), 0, 1)
    );

    vec3 sh3_color = vec3(
        clamp(
            ellipsoid.sh_rest[24] * get_SH_L3_MN3(x*(x*x-y*y)) + 
            ellipsoid.sh_rest[25] * get_SH_L3_MN2((x*x+y*y)*z) + 
            ellipsoid.sh_rest[26] * get_SH_L3_MN1(x*(5*z*z-1)) +
            ellipsoid.sh_rest[27] * get_SH_L3_M0(5*z*z*z-3*z) +
            ellipsoid.sh_rest[28] * get_SH_L3_M1(x*(5*z*z-1)) +
            ellipsoid.sh_rest[29] * get_SH_L3_M2((x*x-y*y)*z) +
            ellipsoid.sh_rest[30] * get_SH_L3_M3(x*(x*x-3*y*y)), 0, 1),
        clamp(
            ellipsoid.sh_rest[31] * get_SH_L3_MN3(x*(x*x-y*y)) + 
            ellipsoid.sh_rest[32] * get_SH_L3_MN2((x*x+y*y)*z) + 
            ellipsoid.sh_rest[33] * get_SH_L3_MN1(x*(5*z*z-1)) +
            ellipsoid.sh_rest[34] * get_SH_L3_M0(5*z*z*z-3*z) +
            ellipsoid.sh_rest[35] * get_SH_L3_M1(x*(5*z*z-1)) +
            ellipsoid.sh_rest[36] * get_SH_L3_M2((x*x-y*y)*z) +
            ellipsoid.sh_rest[37] * get_SH_L3_M3(x*(x*x-3*y*y)), 0, 1),
        clamp(
            ellipsoid.sh_rest[38] * get_SH_L3_MN3(x*(x*x-y*y)) + 
            ellipsoid.sh_rest[39] * get_SH_L3_MN2((x*x+y*y)*z) + 
            ellipsoid.sh_rest[40] * get_SH_L3_MN1(x*(5*z*z-1)) +
            ellipsoid.sh_rest[41] * get_SH_L3_M0(5*z*z*z-3*z) +
            ellipsoid.sh_rest[42] * get_SH_L3_M1(x*(5*z*z-1)) +
            ellipsoid.sh_rest[43] * get_SH_L3_M2((x*x-y*y)*z) +
            ellipsoid.sh_rest[44] * get_SH_L3_M3(x*(x*x-3*y*y)), 0, 1)
    );

    // return sh0_color;
    return sh0_color + sh1_color + sh2_color + sh3_color;
}

void main()
{

    const Ellipsoid ellipsoid = ellipsoids[gl_InstanceID];
    
    vec3 mu = vec3(ellipsoid.mu[0], ellipsoid.mu[1], ellipsoid.mu[2]);
    vec3 sigma = vec3(ellipsoid.sigma[0], ellipsoid.sigma[1], ellipsoid.sigma[2]);

    // Convert spherical coordinates
    vec3 sphere_point = vec3(sin(pos_uv.y) * cos(pos_uv.x), sin(pos_uv.y) * sin(pos_uv.x), cos(pos_uv.y)); // x ~ phi = [0, 2PI], y ~ theta = [0, PI]

    // Scale the point by the ellipsoid's sigma
    vec3 scaled_point = sphere_point * sigma;

    // Rotate the point using the ellipsoid's quaternion
    mat3 rotation = quaternion_to_matrix(vec4(ellipsoid.quaternion[0], ellipsoid.quaternion[1], ellipsoid.quaternion[2], ellipsoid.quaternion[3]));
    vec3 rotated_point = rotation * scaled_point;

    // Translate the point by the ellipsoid's mu
    vec3 pos = mu + ellipsoid.Q * rotated_point;
    
    // Calculate the color based on the ellipsoid's SH coefficients
    out_color = get_color(sphere_point, ellipsoid);

    // Calculate the normal
    out_normal = normalize((sphere_point / sigma) * transpose(rotation));

    // Calculate fragment pos by interpolation
    frag_pos = vec3(model * vec4(pos, 1.0));

    gl_Position = projection * view * model * vec4(pos, 1.0);
}