#ifndef UNIFORMS_H
#define UNIFORMS_H

namespace Uniforms
{
	// camera settings
	unsigned int camera_pos_loc;
	unsigned int camera_front_loc;
	unsigned int camera_right_loc;
	unsigned int camera_up_loc;
	unsigned int fov_loc;
	unsigned int aspect_loc;

	// compute shader settings
	unsigned int compute_texure;
	
	// gaussians settings
	unsigned int Q_loc;
}

#endif // !UNIFORMS_H
