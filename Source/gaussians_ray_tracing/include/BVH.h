#ifndef BVH_H
#define BVH_H

#include <AABB.h>

#include <vector>

#include <bvh/v2/bvh.h>
#include <bvh/v2/vec.h>
#include <bvh/v2/bbox.h>
#include <bvh/v2/default_builder.h>
#include <bvh/v2/node.h>

using Scalar = float;
using Vec3 = bvh::v2::Vec<Scalar, 3>;
using BBox = bvh::v2::BBox<Scalar, 3>;
using Node = bvh::v2::Node<Scalar, 3>;

void create_bvh(std::vector<AABB>& aabb, std::vector<AABB>& bvh_boxes)
{
	std::vector<BBox> bboxes;
	std::vector<Vec3> centers;

	bboxes.reserve(aabb.size());
	centers.reserve(aabb.size());

	for (std::size_t idx = 0; idx < aabb.size(); ++idx)
	{
		Vec3 min = Vec3(
			aabb[idx].center.x - aabb[idx].extent.x,
			aabb[idx].center.y - aabb[idx].extent.y,
			aabb[idx].center.z - aabb[idx].extent.z
		);

		Vec3 max = Vec3(
			aabb[idx].center.x + aabb[idx].extent.x,
			aabb[idx].center.y + aabb[idx].extent.y,
			aabb[idx].center.z + aabb[idx].extent.z
		);

		BBox bbox = BBox(min, max);

		bboxes.emplace_back(bbox);
		centers.emplace_back(bbox.get_center());
	}

	typename bvh::v2::DefaultBuilder<Node>::Config config;
	config.quality = bvh::v2::DefaultBuilder<Node>::Quality::High;
	config.min_leaf_size = 4;
	config.max_leaf_size = 8;

	auto bvh = bvh::v2::DefaultBuilder<Node>::build(bboxes, centers, config);

	bvh_boxes.clear();
	bvh_boxes.reserve(bvh.nodes.size());

	for (std::size_t idx = 0; idx < bvh.nodes.size(); ++idx)
	{
		auto box = bvh.nodes[idx].get_bbox();

		glm::vec3 center(
			box.min[0] + (box.max[0] - box.min[0]) * 0.5f,
			box.min[1] + (box.max[1] - box.min[1]) * 0.5f,
			box.min[2] + (box.max[2] - box.min[2]) * 0.5f
		);

		glm::vec3 extent(
			(box.max[0] - box.min[0]) * 0.5f,
			(box.max[1] - box.min[1]) * 0.5f,
			(box.max[2] - box.min[2]) * 0.5f
		);

		bvh_boxes.emplace_back(center, extent);
	}
}

#endif // !BVH_H
