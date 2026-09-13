// Copyright 2026 bburda
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
#include <memory>

#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>

#include "ros2_medkit_gateway/ros2/status/ros2_lifecycle_state_reader.hpp"
#include "ros2_medkit_gateway/ros2/transports/ros2_fault_service_transport.hpp"

namespace {

using ros2_medkit_gateway::Ros2LifecycleStateReader;
using ros2_medkit_gateway::ros2::Ros2FaultServiceTransport;

// The gateway's helper classes each own a private rclcpp::Node whose first use
// of the ROS graph can land after rclcpp::shutdown() has stopped the context's
// GraphListener - a shutdown signal while a service wait is in flight is the
// ordinary case. Each test therefore runs a complete init/shutdown cycle of the
// default context and exercises the helper across that boundary.
class GraphListenerShutdownSafetyTest : public ::testing::Test {
 protected:
  void SetUp() override {
    rclcpp::init(0, nullptr);
  }

  void TearDown() override {
    if (rclcpp::ok()) {
      rclcpp::shutdown();
    }
  }
};

TEST_F(GraphListenerShutdownSafetyTest, FaultTransportSurvivesAServiceWaitAfterShutdown) {
  auto host = std::make_shared<rclcpp::Node>("fault_transport_host");
  // The gateway node takes a graph event while it is being built, which is what
  // brings the context's GraphListener into existence and starts its thread.
  // rclcpp::shutdown() then stops that listener rather than leaving the helper
  // node to create a fresh one.
  auto host_event = host->get_graph_event();
  auto transport = std::make_unique<Ros2FaultServiceTransport>(host.get());

  rclcpp::shutdown();

  // No fault manager can be reached on a shut-down context, so the wait reports
  // the services as unavailable rather than raising.
  EXPECT_NO_THROW({ EXPECT_FALSE(transport->wait_for_services(std::chrono::duration<double>(0.2))); });

  // The private node is registered with the graph listener, so ~NodeGraph finds
  // it and removes it. An unregistered node aborts the process here instead:
  // NodeNotFoundError escapes a noexcept destructor.
  transport.reset();
  host.reset();
}

TEST_F(GraphListenerShutdownSafetyTest, LifecycleReaderSurvivesAStateReadAfterShutdown) {
  auto host = std::make_shared<rclcpp::Node>("lifecycle_reader_host");
  auto host_event = host->get_graph_event();
  auto reader = std::make_unique<Ros2LifecycleStateReader>(host.get(), std::chrono::duration<double>(0.2));

  rclcpp::shutdown();

  EXPECT_NO_THROW({ EXPECT_FALSE(reader->get_state("/absent_lifecycle_node/get_state").has_value()); });

  reader.reset();
  host.reset();
}

}  // namespace
