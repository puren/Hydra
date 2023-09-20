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
#pragma once
#include <kimera_pgmo/MeshDelta.h>
#include <kimera_pgmo/utils/CommonStructs.h>
#include <pose_graph_tools/BowQuery.h>
#include <pose_graph_tools/PoseGraph.h>
#include <ros/time.h>

#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "hydra/common/common.h"
#include "hydra/common/dsg_types.h"
#include "hydra/common/input_queue.h"
#include "hydra/common/robot_prefix_config.h"
#include "hydra/loop_closure/registration_solution.h"

namespace hydra {

struct LcdInput {
  using Ptr = std::shared_ptr<LcdInput>;

  uint64_t timestamp_ns;
  NodeIdSet archived_places;
  std::vector<NodeId> new_agent_nodes;
};

struct BackendInput {
  using Ptr = std::shared_ptr<BackendInput>;

  RobotPrefixConfig prefix;
  uint64_t timestamp_ns;
  pose_graph_tools::PoseGraph::ConstPtr deformation_graph;
  std::list<pose_graph_tools::PoseGraph::ConstPtr> pose_graphs;
  pose_graph_tools::PoseGraph::ConstPtr agent_node_measurements;
  kimera_pgmo::MeshDelta::Ptr mesh_update;
};

struct SharedModuleState {
  using Ptr = std::shared_ptr<SharedModuleState>;

  SharedModuleState();

  ~SharedModuleState();

  NodeIdSet latest_places;

  InputQueue<pose_graph_tools::BowQuery::ConstPtr> visual_lcd_queue;
  InputQueue<BackendInput::Ptr> backend_queue;
  InputQueue<LcdInput::Ptr>::Ptr lcd_queue;
  InputQueue<lcd::RegistrationSolution> backend_lcd_queue;
};

// TODO(nathan) switch code style
struct BackendModuleStatus {
  size_t total_loop_closures_;
  size_t new_loop_closures_;
  size_t total_factors_;
  size_t total_values_;
  size_t new_factors_;
  size_t new_graph_factors_;
  size_t trajectory_len_;
  size_t num_merges_undone_;

  void reset();
};

}  // namespace hydra
