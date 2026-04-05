#include "arm5dof_hardware/fake_system.hpp"

namespace arm5dof_hardware {

FakeSystemNode::FakeSystemNode(const rclcpp::NodeOptions & options)
: Node("fake_system_node", options) {
    joint_names_ = {"joint1", "joint2", "joint3", "joint4", "joint5"};
    current_joint_positions_.assign(5, 0.0);

    cmd_sub_ = this->create_subscription<sensor_msgs::msg::JointState>(
        "joint_commanding", 10,
        std::bind(&FakeSystemNode::command_callback, this, std::placeholders::_1));

    joint_state_pub_ = this->create_publisher<sensor_msgs::msg::JointState>("joint_states", 10);

    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(10),
        std::bind(&FakeSystemNode::timer_callback, this));
}

void FakeSystemNode::command_callback(const sensor_msgs::msg::JointState::SharedPtr msg) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    if (msg->position.size() == current_joint_positions_.size()) {
        // 直接 把 msg 容器中的 position 赋值给 current_joint_positions_ 容器
        current_joint_positions_ = msg->position;
    }
}

void FakeSystemNode::timer_callback() {
    auto msg = sensor_msgs::msg::JointState();
    msg.header.stamp = this->get_clock()->now();
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        // 直接 把 joint_names_ 容器中的 关节名 赋值给 msg 容器 中的 name
        msg.name = joint_names_;
        // 亦是 “容器之间的直接赋值”
        msg.position = current_joint_positions_;
    }
    joint_state_pub_->publish(msg);
}

} // namespace arm5dof_hardware

// 直接定义 main 函数
int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<arm5dof_hardware::FakeSystemNode>(rclcpp::NodeOptions());
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}