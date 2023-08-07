/* -----------------------------------------------------------------------------
 * Copyright 2022 Massachusetts Institute of Technology.
 * All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Research was sponsored by the United States Air Force Research Laboratory and
 * the United States Air Force Artificial Intelligence Accelerator and was
 * accomplished under Cooperative Agreement Number FA8750-19-2-1000. The views
 * and conclusions contained in this document are those of the authors and should
 * not be interpreted as representing the official policies, either expressed or
 * implied, of the United States Air Force or the U.S. Government. The U.S.
 * Government is authorized to reproduce and distribute reprints for Government
 * purposes notwithstanding any copyright notation herein.
 * -------------------------------------------------------------------------- */
#include "hydra/eval/place_metrics.h"

#include <nanoflann.hpp>

#include <glog/logging.h>

namespace hydra::eval {

using voxblox::Layer;
using places::GvdVoxel;
using nanoflann::KDTreeSingleIndexAdaptor;
using nanoflann::L2_Simple_Adaptor;

void fillGvdPositions(const Layer<GvdVoxel>& layer,
                      size_t min_gvd_basis,
                      voxblox::AlignedVector<Eigen::Vector3d>& result) {
  result.clear();

  voxblox::BlockIndexList blocks;
  layer.getAllAllocatedBlocks(&blocks);

  for (const auto& idx : blocks) {
    const auto& block = layer.getBlockByIndex(idx);
    for (size_t i = 0; i < block.num_voxels(); ++i) {
      const auto& voxel = block.getVoxelByLinearIndex(i);
      if (!voxel.observed || voxel.num_extra_basis < min_gvd_basis) {
        continue;
      }

      result.push_back(block.computeCoordinatesFromLinearIndex(i).cast<double>());
    }
  }
}

struct VoxelKdTreeAdaptor {
  VoxelKdTreeAdaptor(const voxblox::AlignedVector<Eigen::Vector3d>& positions)
      : positions(positions) {}

  inline size_t kdtree_get_point_count() const { return positions.size(); }

  inline double kdtree_get_pt(const size_t idx, const size_t dim) const {
    return positions[idx](dim);
  }

  template <class T>
  bool kdtree_get_bbox(T&) const {
    return false;
  }

  voxblox::AlignedVector<Eigen::Vector3d> positions;
};

struct DistanceFinder {
  using Dist = L2_Simple_Adaptor<double, VoxelKdTreeAdaptor>;
  using KDTree = KDTreeSingleIndexAdaptor<Dist, VoxelKdTreeAdaptor, 3, size_t>;

  DistanceFinder(voxblox::AlignedVector<Eigen::Vector3d>& positions)
      : adaptor(positions) {
    kdtree.reset(new KDTree(3, adaptor));
    kdtree->buildIndex();
  }

  double distance(const Eigen::Vector3d& pos) const {
    size_t idx;
    double dist;
    size_t num_found = kdtree->knnSearch(pos.data(), 1, &idx, &dist);
    return num_found ? std::sqrt(dist) : std::numeric_limits<double>::quiet_NaN();
  }

  VoxelKdTreeAdaptor adaptor;
  std::unique_ptr<KDTree> kdtree;
};

PlaceMetrics scorePlaces(const SceneGraphLayer& places,
                         const Layer<GvdVoxel>& gvd,
                         size_t min_gvd_basis) {
  PlaceMetrics metrics;
  metrics.is_valid = true;

  voxblox::AlignedVector<Eigen::Vector3d> gvd_positions;
  fillGvdPositions(gvd, min_gvd_basis, gvd_positions);
  const DistanceFinder finder(gvd_positions);

  for (auto&& [node_id, node] : places.nodes()) {
    const auto& attrs = node->attributes<PlaceNodeAttributes>();
    metrics.node_order.push_back(node_id);

    const auto pos = attrs.position;
    metrics.node_gvd_distances.push_back(finder.distance(pos));

    const auto* voxel = gvd.getVoxelPtrByCoordinates(pos.cast<float>());
    if (!voxel) {
      metrics.num_missing++;
      continue;
    }

    if (!voxel->observed) {
      metrics.num_unobserved++;
      continue;
    }

    metrics.num_valid++;
    metrics.gvd_distance_errors.push_back(std::abs(voxel->distance - attrs.distance));
  }

  return metrics;
}

/*
nlohmann::json json_results = {
    {"missing", missing},
    {"valid", valid},
    {"dist_errors", dist_errors},
    {"closest_dists", dist_to_closest},
    {"total", places.numNodes()},
    {"mean", mean},
    {"nodes", node_order},
    {"min", valid ? *min : std::numeric_limits<double>::quiet_NaN()},
    {"max", valid ? *max : std::numeric_limits<double>::quiet_NaN()},
};
std::cout << json_results << std::endl;
}
*/

}  // namespace hydra::eval