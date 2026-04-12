#ifndef BVH_H
#define BVH_H

#include <AABB.h>

#include <stack>
#include <vector>

#ifdef DEBUG
#include <fstream>
#endif // DEBUG


#include <bvh/v2/bvh.h>
#include <bvh/v2/vec.h>
#include <bvh/v2/bbox.h>
#include <bvh/v2/default_builder.h>
#include <bvh/v2/node.h>

using Scalar = float;
using Vec3 = bvh::v2::Vec<Scalar, 3>;
using BBox = bvh::v2::BBox<Scalar, 3>;
using Node = bvh::v2::Node<Scalar, 3>;

struct BVHNodeGPU
{
	float bounds[6];

	int index_left;
	int index_right;

	int primitive_counts;
	int primitive_index;
};

struct BVHGPU
{
	std::vector<BVHNodeGPU> nodes;
	std::vector<unsigned int> prim_ids;
};

void create_gpu_bvh(const auto& bvh, BVHGPU& bvh_gpu)
{
	bvh_gpu.prim_ids.resize(bvh.prim_ids.size());
	for (std::size_t idx = 0; idx < bvh.prim_ids.size(); ++idx)
	{
		bvh_gpu.prim_ids[idx] = static_cast<unsigned int>(bvh.prim_ids[idx]);
	}

	bvh_gpu.nodes.reserve(bvh.nodes.size());

	for (std::size_t idx = 0; idx < bvh.nodes.size(); ++idx)
	{
		BVHNodeGPU node_gpu;
		const auto& node = bvh.nodes[idx];

		//  опируем bounds (6 float'ов)
		node_gpu.bounds[0] = node.bounds[0];
		node_gpu.bounds[1] = node.bounds[1];
		node_gpu.bounds[2] = node.bounds[2];
		node_gpu.bounds[3] = node.bounds[3];
		node_gpu.bounds[4] = node.bounds[4];
		node_gpu.bounds[5] = node.bounds[5];

		// »спользуем методы Index дл€ определени€ типа и получени€ данных
		if (node.index.is_leaf())
		{
			// Ћист: получаем начало и количество примитивов
			node_gpu.primitive_index = node.index.first_id();   // метод, не поле!
			node_gpu.primitive_counts = node.index.prim_count(); // метод, не поле!

			// ƒл€ листьев дети не используютс€
			node_gpu.index_left = -1;
			node_gpu.index_right = -1;
		}
		else  // внутренний узел
		{
			// ¬нутренний узел: first_id указывает на первого ребенка
			int first_child = node.index.first_id();

			node_gpu.index_left = first_child;
			node_gpu.index_right = first_child + 1;  // дл€ бинарного дерева
			// ѕри branching factor = 4 нужно хранить 4 ребенка

			node_gpu.primitive_index = -1;
			node_gpu.primitive_counts = 0;
		}

		bvh_gpu.nodes.emplace_back(node_gpu);
	}
}

int max_depth = 0;

void get_depth_iterative(const bvh::v2::Bvh<Node>& bvh, int start_index = 0)
{
	std::stack<std::pair<unsigned int, int>> stack;
	stack.push({ start_index, 0 });

	while (!stack.empty())
	{
		auto [idx, depth] = stack.top();
		stack.pop();

		if (depth > max_depth)
			max_depth = depth;

		const auto& node = bvh.nodes[idx];
		if (!node.is_leaf())
		{
			stack.push({ node.index.first_id(), depth + 1 });
			stack.push({ node.index.first_id() + 1, depth + 1 });
		}
	}
}

void create_bvh(std::vector<AABB>& aabb, std::vector<AABB>& bvh_boxes, BVHGPU& bvh_gpu)
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

	get_depth_iterative(bvh);
	std::cout << "Max depth of BVH: " << max_depth << std::endl;

	create_gpu_bvh(bvh, bvh_gpu);

	bvh_boxes.clear();
	bvh_boxes.reserve(bvh.nodes.size());
#ifdef DEBUG
	std::ofstream file("bvh.txt");
#endif // DEBUG

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

#ifdef DEBUG
		if (file.is_open())
		{
			file << "lines: BVH_" << idx << std::endl;

			float minX = bvh.nodes[idx].bounds[0];
			float maxX = bvh.nodes[idx].bounds[1];
			float minY = bvh.nodes[idx].bounds[2];
			float maxY = bvh.nodes[idx].bounds[3];
			float minZ = bvh.nodes[idx].bounds[4];
			float maxZ = bvh.nodes[idx].bounds[5];

			file << "(" << minX << "," << minY << "," << minZ << ")(" << maxX << "," << minY << "," << minZ << ")(1,0,0)" << std::endl;
			file << "(" << maxX << "," << minY << "," << minZ << ")(" << maxX << "," << maxY << "," << minZ << ")(1,0,0)" << std::endl;
			file << "(" << maxX << "," << maxY << "," << minZ << ")(" << minX << "," << maxY << "," << minZ << ")(1,0,0)" << std::endl;
			file << "(" << minX << "," << maxY << "," << minZ << ")(" << minX << "," << minY << "," << minZ << ")(1,0,0)" << std::endl;

			file << "(" << minX << "," << minY << "," << maxZ << ")(" << maxX << "," << minY << "," << maxZ << ")(1,0,0)" << std::endl;
			file << "(" << maxX << "," << minY << "," << maxZ << ")(" << maxX << "," << maxY << "," << maxZ << ")(1,0,0)" << std::endl;
			file << "(" << maxX << "," << maxY << "," << maxZ << ")(" << minX << "," << maxY << "," << maxZ << ")(1,0,0)" << std::endl;
			file << "(" << minX << "," << maxY << "," << maxZ << ")(" << minX << "," << minY << "," << maxZ << ")(1,0,0)" << std::endl;

			file << "(" << minX << "," << minY << "," << minZ << ")(" << minX << "," << minY << "," << maxZ << ")(1,0,0)" << std::endl;
			file << "(" << maxX << "," << minY << "," << minZ << ")(" << maxX << "," << minY << "," << maxZ << ")(1,0,0)" << std::endl;
			file << "(" << maxX << "," << maxY << "," << minZ << ")(" << maxX << "," << maxY << "," << maxZ << ")(1,0,0)" << std::endl;
			file << "(" << minX << "," << maxY << "," << minZ << ")(" << minX << "," << maxY << "," << maxZ << ")(1,0,0)" << std::endl;
		}
#endif // DEBUG

		bvh_boxes.emplace_back(center, extent);
	}

#ifdef DEBUG
	file.close();
#endif // DEBUG

}

#endif // !BVH_H
