#ifndef FAKE_SYSTEM_NODE_HPP_
#define FAKE_SYSTEM_NODE_HPP_

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <vector>
#include <string>
#include <mutex>

namespace arm5dof_hardware {

class FakeSystemNode : public rclcpp::Node {
public:
    explicit FakeSystemNode(const rclcpp::NodeOptions & options);

private:
    // 订阅来自 arm_solve_node 的指令
    void command_callback(const sensor_msgs::msg::JointState::SharedPtr msg);
    // 定时器回调：持续向 RViz 发布当前状态
    void timer_callback();

    rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr cmd_sub_;
    rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_state_pub_;
    rclcpp::TimerBase::SharedPtr timer_;

    // 核心容器：存储当前机械臂的关节位置
    std::vector<double> current_joint_positions_;
    std::vector<std::string> joint_names_;
    
    // 互斥锁：防止计时器读取时，订阅回调正在写入数据（多线程安全）
    std::mutex data_mutex_;
};

} // namespace arm5dof_hardware

#endif // FAKE_SYSTEM_NODE_HPP_