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
#include <KimeraRPGO/SolverParams.h>
#include <kimera_pgmo/KimeraPgmoInterface.h>

#include "hydra/common/dsg_types.h"
#include "hydra/rooms/room_finder_config.h"

namespace hydra {

// supplementary pgmo config
struct HydraPgmoConfig : public kimera_pgmo::KimeraPgmoConfig {
  // covariance
  double place_mesh_variance;
  double place_edge_variance;
  double place_merge_variance;
  double object_merge_variance;
  double sg_loop_closure_variance;
  // rpgo
  bool gnc_fix_prev_inliers = true;
  KimeraRPGO::Verbosity rpgo_verbosity = KimeraRPGO::Verbosity::UPDATE;
  KimeraRPGO::Solver rpgo_solver = KimeraRPGO::Solver::LM;
};

struct BackendConfig {
  float angle_step = 10.0f;
  bool visualize_place_factors = true;
  SemanticNodeAttributes::ColorVector building_color{169, 8, 194};  // purple
  SemanticNodeAttributes::Label building_semantic_label = 22u;

  bool enable_rooms = true;
  RoomFinderConfig room_finder;

  HydraPgmoConfig pgmo;

  // dsg
  bool add_places_to_deformation_graph = true;
  bool optimize_on_lc = true;
  bool enable_node_merging = true;
  bool use_mesh_subscribers = false;
  std::map<LayerId, bool> merge_update_map{{DsgLayers::OBJECTS, false},
                                           {DsgLayers::PLACES, true},
                                           {DsgLayers::ROOMS, false},
                                           {DsgLayers::BUILDINGS, false}};
  bool merge_update_dynamic = true;
  double places_merge_pos_threshold_m = 0.4;
  double places_merge_distance_tolerance_m = 0.3;
  bool enable_merge_undos = false;
  bool use_active_flag_for_updates = true;
  size_t num_neighbors_to_find_for_merge = 1;
  std::string zmq_send_url = "tcp://127.0.0.1:8001";
  std::string zmq_recv_url = "tcp://127.0.0.1:8002";
  bool use_zmq_interface = false;
  size_t zmq_num_threads = 2;
  size_t zmq_poll_time_ms = 10;
};

void declare_config(HydraPgmoConfig& conf);
void declare_config(BackendConfig& conf);

}  // namespace hydra

namespace kimera_pgmo {

void declare_config(KimeraPgmoConfig& conf);

}  // namespace kimera_pgmo