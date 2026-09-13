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

// As little graph surface as a node can be configured to have: parameter
// services, the parameter-event publisher and the /rosout publisher are all
// off. What rclcpp still puts on the graph for it - the type-description
// service and a /parameter_events subscription - is what keeps it a node the
// gateway must list, so this fixture is the control on discovery's rule that a
// name the graph attributes no endpoint to is not an App. That rule may only
// remove names whose endpoints are gone, never a node that is merely quiet.
//
// `advertise:=true` adds a publisher, for a case that wants one endpoint of
// its own rather than only the ones rclcpp creates.

#include <memory>
#include <string>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include "ros2_medkit_integration_tests/demo_node_main.hpp"

class SilentNode : public rclcpp::Node {
 public:
  explicit SilentNode(const rclcpp::NodeOptions & options) : Node("silent_node", options) {
    if (declare_parameter<bool>("advertise", false)) {
      heartbeat_pub_ = create_publisher<std_msgs::msg::String>("~/heartbeat", 10);
    }
  }

  ~SilentNode() override {
    heartbeat_pub_.reset();
  }

  SilentNode(const SilentNode &) = delete;
  SilentNode & operator=(const SilentNode &) = delete;
  SilentNode(SilentNode &&) = delete;
  SilentNode & operator=(SilentNode &&) = delete;

 private:
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr heartbeat_pub_;
};

int main(int argc, char ** argv) {
  return ros2_medkit_integration_tests::run_demo_node(argc, argv, [] {
    rclcpp::NodeOptions options;
    options.start_parameter_services(false);
    options.start_parameter_event_publisher(false);
    options.enable_rosout(false);
    return std::make_shared<SilentNode>(options);
  });
}
