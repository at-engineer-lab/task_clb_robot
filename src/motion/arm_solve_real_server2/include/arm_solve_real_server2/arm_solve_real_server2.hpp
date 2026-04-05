#ifndef ARM_SOLVE_REAL_SERVER2_HPP_
#define ARM_SOLVE_REAL_SERVER2_HPP_

#include <rclcpp/rclcpp.hpp>
#include <moveit/move_group_interface/move_group_interface.h>
#include <sensor_msgs/msg/joint_state.hpp>
#include <trajectory_msgs/msg/joint_trajectory.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <unordered_map>
#include <vector>
#include <string>

namespace arm_solve_real_server2 {

class ArmSolveServer : public rclcpp::Node {
public:
    explicit ArmSolveServer(const rclcpp::NodeOptions & options);
    // 执行规划与分发的主函数
    void plan_and_dispatch();

private:
    rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_pub_;
    
    // 1. 定义硬件期望的五轴顺序
    std::vector<std::string> joint_order_;
    
    // 2. 内部使用的哈希表容器
    std::unordered_map<std::string, double> joint_map_;

    // MoveIt 相关接口
    std::shared_ptr<moveit::planning_interface::MoveGroupInterface> move_group_;

    // 定义一个目标位姿
    geometry_msgs::msg::Pose pose;
};

} // namespace arm_solve_real_server2

#endif // ARM_SOLVE_REAL_SERVER2_HPP_
