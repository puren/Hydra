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
#include "hydra/common/hydra_pipeline.h"

#include <config_utilities/settings.h>

namespace hydra {

HydraPipeline::HydraPipeline(int robot_id, const LogSetup::Ptr& log_setup)
    : prefix_(robot_id), log_setup_(log_setup) {
  // TODO(nathan) parse label space config
  // TODO(nathan) parse and use at some point
  const LayerId mesh_layer_id = 1;
  const std::map<LayerId, char> layer_id_map{{DsgLayers::OBJECTS, 'o'},
                                             {DsgLayers::PLACES, 'p'},
                                             {DsgLayers::ROOMS, 'r'},
                                             {DsgLayers::BUILDINGS, 'b'}};

  frontend_dsg_.reset(new SharedDsgInfo(layer_id_map, mesh_layer_id));
  backend_dsg_.reset(new SharedDsgInfo(layer_id_map, mesh_layer_id));
  shared_state_.reset(new SharedModuleState());
}

HydraPipeline::~HydraPipeline() {}

void HydraPipeline::start() {
  const auto print_width = config::Settings().print_width;
  for (auto&& [name, module] : modules_) {
    std::stringstream ss;
    ss << std::string(print_width, '*') << std::endl;
    const auto name_size = name.size() + 3;
    const auto spacing = name_size >= print_width ? 0 : print_width - name_size;
    ss << "* " << name << (spacing ? std::string(spacing, ' ') + "*" : "") << std::endl;
    ss << std::string(print_width, '*') << std::endl;
    if (!module) {
      ss << "UNITIALIZED MODULE!" << std::endl;
    } else {
      const auto info = module->printInfo();
      if (!info.empty()) {
        ss << info << std::endl;
      }
    }
    ss << std::string(print_width, '*') << std::endl;

    VLOG(1) << std::endl << ss.str();
  }

  for (auto&& [name, module] : modules_) {
    if (!module) {
      LOG(FATAL) << "Found unitialized module: " << name;
      continue;
    }

    module->start();
  }
}

void HydraPipeline::stop() {
  for (auto&& [name, module] : modules_) {
    if (!module) {
      LOG(FATAL) << "Found unitialized module: " << name;
      continue;
    }

    module->stop();
  }
}

void HydraPipeline::save(const LogSetup& logs) {
  if (!logs.valid()) {
    return;
  }

  for (auto&& [name, module] : modules_) {
    if (!module) {
      LOG(FATAL) << "Found unitialized module: " << name;
      continue;
    }

    module->save(logs);
  }
}

}  // namespace hydra