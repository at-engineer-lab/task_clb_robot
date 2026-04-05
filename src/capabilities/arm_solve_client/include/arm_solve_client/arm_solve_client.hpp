#ifndef ARM_SOLVE_CLIENT_HPP_
#define ARM_SOLVE_CLIENT_HPP_

#include <memory>
#include <chrono>
#include <functional>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "robot_interfaces/action/move_to_pose.hpp"

namespace arm_solve_client{

using MoveToPose = robot_interfaces::action::MoveToPose;

// 创建 节点类 ，继承 Node类
// 在 .hpp 中声明 class ，就不要在 .cpp 中再写一遍 class 了
class ArmSolveClient : public rclcpp::Node
{
public:
    // 构造函数
    // explicit ： 禁止 隐式转换，建议写在 构造函数 前
    explicit ArmSolveClient(const std::string & node_name = "arm_solve_client");

    // 发送目标位姿
    void send_goal(const geometry_msgs::msg::PoseStamped & target_pose);

private:
    // Action Client
    rclcpp_action::Client<MoveToPose>::SharedPtr client_;

    // 回调函数
    void goal_response_callback(
        rclcpp_action::ClientGoalHandle<MoveToPose>::SharedPtr goal_handle);

    void feedback_callback(
        rclcpp_action::ClientGoalHandle<MoveToPose>::SharedPtr,
        const std::shared_ptr<const MoveToPose::Feedback> feedback);

    void result_callback(
        const rclcpp_action::ClientGoalHandle<MoveToPose>::WrappedResult & result);
};
}

#endif // ARM_SOLVE_CLIENT_HPP_