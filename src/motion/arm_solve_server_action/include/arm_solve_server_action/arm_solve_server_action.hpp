#ifndef ARM_SOLVE_SERVER_ACTION
#define ARM_SOLVE_SERVER_ACTION

#include <memory>
#include <moveit/robot_model/joint_model_group.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <chrono>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp/logging.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include "robot_interfaces/action/move_to_pose.hpp"            // 你的自定义 Action
#include <sensor_msgs/msg/joint_state.hpp>                     // 发布关节状态

#include <moveit/planning_scene_monitor/planning_scene_monitor.h> // PSM 核心
#include <moveit/planning_scene/planning_scene.h>               // 规划场景类
#include <moveit/robot_state/robot_state.h>                     // 机器人状态类
#include <moveit/robot_model/robot_model.h>                     // 机器人模型类
#include <moveit/robot_state/conversions.h>                    // 状态与消息互转 (robotStateToRobotStateMsg)
#include <moveit/robot_model_loader/robot_model_loader.h>       // 模型加载
#include <moveit/planning_interface/planning_interface.h>       // res的类型，必须是：高级对象

#include <moveit/planning_pipeline/planning_pipeline.h>         // 规划管道核心
#include <moveit_msgs/msg/motion_plan_request.hpp>             // 规划请求消息
#include <moveit_msgs/msg/motion_plan_response.hpp>            // 规划响应消息
#include <moveit/kinematic_constraints/utils.h>                // 构建目标约束 (constructGoalConstraints)

#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>             // Pose 消息转换
#include <tf2_eigen/tf2_eigen.hpp>                             // Eigen 与 ROS 消息互转
#include <geometry_msgs/msg/pose_stamped.hpp>                    // 位姿消息定义

namespace arm_solve_server_action
{

// 继承 ： ArmSolveServer 节点类，是 Node 的 子类
class ArmSolveServer : public rclcpp::Node
{
public:
  using MoveToPose = robot_interfaces::action::MoveToPose;
  using GoalHandleMoveToPose = rclcpp_action::ServerGoalHandle<MoveToPose>;

  // 对 ArmSolveServer节点 进行 初始化
  // rclcpp::NodeOptions() 是 构造函数，构造出一个 “节点配置对象” 
  explicit ArmSolveServer(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());

  // 声明初始化函数，因为在构造函数中不允许使用 shared_from_this()，因此，创建一个新函数来初始化就可以使用了
  void initialize(); 

private:
  /* ================= Action Server ================= */

  // 声明 坐标系转换以及监听TF树的tf2对象
  std::shared_ptr<tf2_ros::Buffer> tf2_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf2_listener_;

  rclcpp_action::Server<MoveToPose>::SharedPtr action_server_;

  // 收到 goal（是否接受）
  rclcpp_action::GoalResponse handle_goal(
    const rclcpp_action::GoalUUID & uuid,
    std::shared_ptr<const MoveToPose::Goal> goal);

  // 收到 cancel 请求
  rclcpp_action::CancelResponse handle_cancel(
    const std::shared_ptr<GoalHandleMoveToPose> goal_handle);

  // 接受 goal 后（启动执行线程）
  void handle_accepted(
    const std::shared_ptr<GoalHandleMoveToPose> goal_handle);

  // 真正执行函数（规划 + 发布）
  void play_and_detach(
    const std::shared_ptr<GoalHandleMoveToPose> goal_handle);

  // 创建 机器人模型加载器对象
  std::shared_ptr<robot_model_loader::RobotModelLoader> robot_model_loader_;

  // 创建 机器人模型对象
  std::shared_ptr<moveit::core::RobotModel> robot_model_;
  
  // 创建 规划场景监视器
  std::shared_ptr<planning_scene_monitor::PlanningSceneMonitor> planning_scene_monitor_;

  // 创建 规划管道
  std::shared_ptr<planning_pipeline::PlanningPipeline> planning_pipeline_;

  // 创建 关节模型组（不能使用shared_ptr，其生命周期由模型管理）
  const moveit::core::JointModelGroup* joint_model_group_;  

  /* ================= real(fake)_system 通信 ================= */

  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_pub_;

  // 固定关节顺序（必须与 URDF 一致）
  std::vector<std::string> joint_order_;

  /* ================= 工具结构 ================= */

  // 用于 joint_names → position 映射（解决顺序问题）
  // 哈希表 构建   
  std::unordered_map<std::string, double> joint_map_;

  // 多线程保护
  std::mutex data_mutex_;
};

}  // namespace arm_solve_server_action

#endif