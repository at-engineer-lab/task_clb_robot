#include "arm_solve_server_action/arm_solve_server_action.hpp"
#include <geometry_msgs/msg/detail/pose_stamped__struct.hpp>
#include <memory>
#include <tf2_ros/buffer.hpp>
#include <tf2_ros/transform_listener.hpp>

namespace arm_solve_server_action
{

void ArmSolveServer::initialize(){
  // 开始 初始化

  // 初始化 加载器和模型
  robot_model_loader_ = std::make_shared<robot_model_loader::RobotModelLoader>(shared_from_this(),"robot_description");

  // robot_model_loader一经创建之后，内部已创建robot_model_，robot_model_ = robot_model_loader_->getModel();只是在；领取“使用权”
  robot_model_ = robot_model_loader_->getModel();

  // 初始化 规划场景监测器
  planning_scene_monitor_ = std::make_shared<planning_scene_monitor::PlanningSceneMonitor>(shared_from_this(),robot_model_loader_);

  // 启动 规划场景监视器，必须在构造函数中启动，使它提前并保持持续打开
  if(!planning_scene_monitor_->getPlanningScene()){
  RCLCPP_FATAL(this->get_logger(),"规划场景监测器 启动失败！！！");
}

  else {
  planning_scene_monitor_->startStateMonitor("/joint_states");
  planning_scene_monitor_->startSceneMonitor("/planning_scene");
  planning_scene_monitor_->startWorldGeometryMonitor();

  RCLCPP_INFO(this->get_logger(),"规划场景监测器 启动成功！！！");
}

  // 初始化 关节模型组（joint_model_group_千万不要使用shared_ptr去初始化，因为joint_model_group_的声明周期由robot_model_管理）
  joint_model_group_ = robot_model_->getJointModelGroup("arm");

  // 初始化 规划管道
  planning_pipeline_ = std::make_shared<planning_pipeline::PlanningPipeline>(robot_model_,shared_from_this(),"ompl_interface/OMPLPlanner");
}

ArmSolveServer::ArmSolveServer(const rclcpp::NodeOptions & options): Node("arm_solve_node", options)
{
  // 初始化 tf2转换以及监听对象
  tf2_buffer_ = std::make_shared<tf2_ros::Buffer>(this->get_clock());
  tf2_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf2_buffer_);

  // 固定关节顺序（需与 URDF 一致）
  joint_order_ = {"joint1", "joint2", "joint3", "joint4", "joint5"};

  // 发布给 fake(real)_system
  joint_pub_ = this->create_publisher<sensor_msgs::msg::JointState>(
    "joint_commanding", 10);

  // 创建 Action Server
  action_server_ = rclcpp_action::create_server<MoveToPose>(
    this,
    "move_to_pose",
    std::bind(&ArmSolveServer::handle_goal, this, std::placeholders::_1, std::placeholders::_2),
    std::bind(&ArmSolveServer::handle_cancel, this, std::placeholders::_1),
    std::bind(&ArmSolveServer::handle_accepted, this, std::placeholders::_1)
  );

  RCLCPP_INFO(this->get_logger(), "ArmSolveServer 已启动");
}


/* ================= Action 回调 ================= */

rclcpp_action::GoalResponse ArmSolveServer::handle_goal(
  const rclcpp_action::GoalUUID & /*uuid*/,
  std::shared_ptr<const MoveToPose::Goal> goal)
{
  // 告诉 编译器 ： 这个 参数 我是故意不用的，防止 编译器 报错   
  (void)goal;
  RCLCPP_INFO(this->get_logger(), "收到目标位姿请求");
// 这里可以加入判断语句，如若不满足条件则直接拒绝接收 &*****&
  return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}


rclcpp_action::CancelResponse ArmSolveServer::handle_cancel(
  const std::shared_ptr<GoalHandleMoveToPose> goal_handle)
{
  (void)goal_handle;
  RCLCPP_WARN(this->get_logger(), "收到取消请求");
// 这里也可以添加判断语句，如若不满足条件，则拒绝接收拒绝请求 &*****&
  return rclcpp_action::CancelResponse::ACCEPT;
}


void ArmSolveServer::handle_accepted(
      // goal_handle 是 管理和追踪 目标请求 的 智能指针
      // goal_handle 是 一个 包装器，其内部包含着 目标位姿（还有其他数据）
  const std::shared_ptr<GoalHandleMoveToPose> goal_handle)
{
  // 开新线程执行
  std::thread([this, goal_handle]() {
  this -> play_and_detach(goal_handle);
    }).detach();
}


/* ================= 核心执行函数 ================= */

void ArmSolveServer::play_and_detach(
  const std::shared_ptr<GoalHandleMoveToPose> goal_handle)
{
  RCLCPP_INFO(this->get_logger(), "开始规划轨迹");

  auto target_goal = goal_handle->get_goal();

  // 手动转换坐标系（try catch）
  geometry_msgs::msg::PoseStamped target_pose_to_base;
  try {
    target_pose_to_base = tf2_buffer_->transform(target_goal->target_pose,"base_link",tf2::durationFromSec(5.0));
  } catch (tf2::TransformException &ex) {
    RCLCPP_ERROR(this->get_logger(), "TF 转换失败: %s", ex.what());
    return;
  }

  moveit_msgs::msg::MotionPlanRequest req;
  planning_interface::MotionPlanResponse res;

  {
    planning_scene_monitor::LockedPlanningSceneRO ls(planning_scene_monitor_);
    const moveit::core::RobotState& current_state = ls->getCurrentState();

    // 先给 req 填充数据
    req.group_name = "arm";

    moveit_msgs::msg::RobotState start_state_msg;
    moveit::core::robotStateToRobotStateMsg(current_state, start_state_msg);
    req.start_state = start_state_msg;

    // 创建 种子，为 setFromIK 服务，种子是当前状态
    moveit::core::RobotState seed_state(robot_model_);
    seed_state = ls->getCurrentState();  // 这是拷贝， seed_state 从 ls 中拷贝一份，这样 seed_state 就可以被修改了
    
    // 开始 ik 逆运动学求解
    // 先转换坐标系
    // bool ik_success = seed_state.setFromIK(joint_model_group_,target_goal->target_pose.pose,5.0);
    RCLCPP_INFO(this->get_logger(), "planning frame: %s",
    planning_scene_monitor_->getPlanningScene()->getPlanningFrame().c_str());
    
    bool ik_success = seed_state.setFromIK(
      joint_model_group_,
      target_pose_to_base.pose,
      "tcp_link",
      5.0
    );

    if(!ik_success){
      RCLCPP_FATAL(this->get_logger()," IK 求解失败！！！");
      return;
    }
    RCLCPP_INFO(this->get_logger()," IK 求解成功！！！");  // 将求出的 目标关节角度 存入到 seed_state中

    // 判断求出的目标关节值会不会使机器人发生自碰撞
    bool is_collison = planning_scene_monitor_->getPlanningScene()->isStateColliding(seed_state,"arm");
    if(is_collison){
      RCLCPP_FATAL(this->get_logger(),"虽求出 IK 值，但是该值使机器人自碰撞！！！");
      return;
    }
    else{
      RCLCPP_INFO(this->get_logger()," IK 值不会使机器人发生自碰撞！！！");
    }

    // 接着 向 req 中填充 目标约束
    auto goal_msg = kinematic_constraints::constructGoalConstraints(seed_state,joint_model_group_);
    req.goal_constraints.push_back(goal_msg);

    // 开始 规划
    auto scene_ptr = planning_scene::PlanningSceneConstPtr(planning_scene_monitor_->getPlanningScene());
    planning_pipeline_->generatePlan(scene_ptr,req,res);
  }

  // 可以先转换为 消息，这样和以前的思路一样
  moveit_msgs::msg::RobotTrajectory traj_msg;
  res.trajectory_->getRobotTrajectoryMsg(traj_msg);

  RCLCPP_INFO(this->get_logger(), "规划成功，开始执行（包含发布轨迹点）");

  auto & trajectory = traj_msg;

  // 起始时间
  rclcpp::Time start_time = this->get_clock()->now();

  size_t total_points = trajectory.joint_trajectory.points.size();

  for (size_t idx = 0; idx < total_points; ++idx) {

    // cancel 检查
    if (goal_handle->is_canceling()) {
      RCLCPP_WARN(this->get_logger(), "任务被取消");
       
      // 调用 .action C++ 接口   
      auto result = std::make_shared<MoveToPose::Result>();
      result->success = false;
      result->message = "canceled";
      goal_handle->canceled(result);
      return;
    }

    const auto & point = trajectory.joint_trajectory.points[idx];

    // ===== 时间对齐 =====
    rclcpp::Time target_time = start_time + point.time_from_start;

    while (this->get_clock()->now() < target_time) {
      // 还没到 发布时间 ,则 sleep_for 进行等待，直到到 发布时间   
      std::this_thread::sleep_for(std::chrono::microseconds(200));
    }

    // ===== 构建 joint_map =====
    {
      std::lock_guard<std::mutex> lock(data_mutex_);
      joint_map_.clear();

      for (size_t i = 0; i < trajectory.joint_trajectory.joint_names.size(); ++i) {
        // 先向 哈希表（容器）中 填充数据，关节名 一定和 关节值 一一对应，但是 关节顺序 不一定和 urdf中的顺序一致
        joint_map_[trajectory.joint_trajectory.joint_names[i]] = point.positions[i];
      }
    }

    // ===== 生成 JointState =====
    sensor_msgs::msg::JointState msg;
    msg.header.stamp = this->get_clock()->now();
    
    // 借助 for循环 遍历 joint_order_（urdf中的关节顺序），借助 哈希表（根据 name 找到对应的 position），最后把 position和nane 都填充到 msg 中
    for (const auto & name : joint_order_) {
      msg.name.push_back(name);

      if (joint_map_.find(name) != joint_map_.end()) {
        msg.position.push_back(joint_map_[name]);
      } else {
        msg.position.push_back(0.0); // 容错
      }
    }

    // 发布给 fake(real)_system_node
    joint_pub_->publish(msg);

    // ===== Feedback =====
    // 调用 .action C++ 接口
    auto feedback = std::make_shared<MoveToPose::Feedback>();
    feedback->progress = static_cast<float>(idx + 1) / total_points;
    goal_handle->publish_feedback(feedback);
  }

  // ===== 发布完成 =====
  RCLCPP_INFO(this->get_logger(), "成功发送到 fake(real)_system_node");

  auto result = std::make_shared<MoveToPose::Result>();
  result->success = true;
  result->message = "success";
  // succeed(result) 表示 发布到 fake_system_ndoe 成功  
  goal_handle->succeed(result);
}
} // namespace arm_solve_server_action

/* ================= main ================= */

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<arm_solve_server_action::ArmSolveServer>();

  node->initialize();

  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}

