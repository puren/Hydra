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
#include <gtest/gtest.h>
#include <hydra/reconstruction/camera.h>

#include <optional>
#include <set>

#include "hydra_test/resources.h"

namespace hydra {

namespace {

template <typename T>
std::string toString(const Eigen::Quaternion<T>& q) {
  std::stringstream ss;
  ss << "{w: " << q.w() << ", x: " << q.x() << ", y: " << q.y() << ", z: " << q.z()
     << "}";
  return ss.str();
}

}  // namespace

TEST(SensorExtrinsics, TestConstructor) {
  // mostly for coverage
  const SensorExtrinsics identity;
  EXPECT_TRUE(identity.body_R_sensor.isApprox(Eigen::Quaterniond::Identity()));
  EXPECT_TRUE(identity.body_p_sensor.isApprox(Eigen::Vector3d::Zero()));

  const SensorExtrinsics translation_only(Eigen::Vector3d(1.0, 2.0, 3.0));
  EXPECT_TRUE(translation_only.body_R_sensor.isApprox(Eigen::Quaterniond::Identity()));
  EXPECT_TRUE(translation_only.body_p_sensor.isApprox(Eigen::Vector3d(1.0, 2.0, 3.0)));

  const SensorExtrinsics rotation_only(Eigen::Quaterniond(0.0, 1.0, 0.0, 0.0));
  EXPECT_TRUE(
      rotation_only.body_R_sensor.isApprox(Eigen::Quaterniond(0.0, 1.0, 0.0, 0.0)));
  EXPECT_TRUE(rotation_only.body_p_sensor.isApprox(Eigen::Vector3d::Zero()));
}

TEST(SensorExtrinsics, KimeraExtrinsicsParsing) {
  const auto filepath =
      test::get_resource_path("reconstruction/kimera_extrinsics.yaml");
  KimeraSensorExtrinsics::Config config;
  config.sensor_filepath = filepath;
  KimeraSensorExtrinsics tf(config);
  EXPECT_TRUE(tf.body_R_sensor.isApprox(Eigen::Quaterniond(-0.5, 0.5, -0.5, 0.5)))
      << toString(tf.body_R_sensor);
  EXPECT_TRUE(tf.body_p_sensor.isApprox(Eigen::Vector3d(1.0, 2.0, 3.0)));
}

}  // namespace hydra