// 简单版 话题通信 + 显式设置位姿

#include "arm_solve_server/arm_solve_server.hpp"
#include <thread>
#include <chrono>

namespace arm_solve_server {

ArmSolveServer::ArmSolveServer(const rclcpp::NodeOptions & options)
: Node("arm_solve_node", options) {
    // 初始化五轴机械臂关节名顺序（需与 URDF 一致）
    joint_order_ = {"joint1", "joint2", "joint3", "joint4", "joint5"};
    
    // 发布给 fake_system 的话题
    joint_pub_ = this->create_publisher<sensor_msgs::msg::JointState>("fake_joint_commands", 10);
}

void ArmSolveServer::plan_and_dispatch() {
    // 初始化 MoveGroup
    move_group_ = std::make_shared<moveit::planning_interface::MoveGroupInterface>(shared_from_this(), "arm");

    // 1. 设置目标姿态
    geometry_msgs::msg::Pose pose;

    pose.position.x = -0.16282;    
    pose.position.y = 9.2687e-06;    
    pose.position.z = 0.28499;   

    // 设置姿态 (四元数) - 示例为夹爪垂直向下
    pose.orientation.x = -0.4254;
    pose.orientation.y = 4.2966e-05;  
    pose.orientation.z = 0.905006;
    pose.orientation.w = -3.98908e-05; 
    
    move_group_->setPoseTarget(pose, "tcp_link");

    // 2. 调用 MoveIt 规划
    moveit::planning_interface::MoveGroupInterface::Plan my_plan;
    bool success = (move_group_->plan(my_plan) == moveit::core::MoveItErrorCode::SUCCESS);

    if (success) {
        RCLCPP_INFO(this->get_logger(), "规划成功，开始按时间戳平滑发布轨迹...");

        auto & trajectory = my_plan.trajectory_.joint_trajectory;
        
        // --- A. 记录轨迹开始运行的基准时间 ---
        rclcpp::Time start_real_time = this->get_clock()->now();

        for (const auto & point : trajectory.points) {
            
            // --- B. 动态等待逻辑：对齐 MoveIt 规划的时间轴 ---
            // 获取当前点相对于轨迹起点应有的时间偏移
            rclcpp::Duration expected_offset = point.time_from_start;
            rclcpp::Time target_real_time = start_real_time + expected_offset;

            // 阻塞等待，直到系统时钟到达该点对应的时刻
            while (this->get_clock()->now() < target_real_time) {
                // 微秒级休眠，保证 CPU 占用率低且检查频率高
                std::this_thread::sleep_for(std::chrono::microseconds(200));
            }

            // --- C. 构建并发送 JointState 消息 ---
            // 每次循环清空 Map，确保数据新鲜
            joint_map_.clear();
            for (size_t i = 0; i < trajectory.joint_names.size(); ++i) {
                joint_map_[trajectory.joint_names[i]] = point.positions[i];
            }

            sensor_msgs::msg::JointState msg;
            msg.header.stamp = this->get_clock()->now();

            // 按照预设的硬件顺序 (joint_order_) 提取关节值
            for (const auto & name : joint_order_) {
                msg.name.push_back(name);
                if (joint_map_.find(name) != joint_map_.end()) {
                   msg.position.push_back(joint_map_[name]);
                } else {
                    msg.position.push_back(0.0); // 容错处理
                }
            }

            // 把每个 轨迹点 的 关节组的关节值 发送给 fake_system
            joint_pub_->publish(msg);
        }
        RCLCPP_INFO(this->get_logger(), "轨迹执行完毕，已到达目标位姿。");
    } else {
        RCLCPP_ERROR(this->get_logger(), "规划失败，请检查目标点是否在工作空间内！");
    }
}

} // namespace arm_solve_server

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<arm_solve_server::ArmSolveServer>(rclcpp::NodeOptions());
    
    // 在独立线程中执行，防止阻塞主循环的 spin
    std::thread([node]() {
        // 等待系统各节点（如 RViz, MoveGroup）初始化完成
        rclcpp::sleep_for(std::chrono::seconds(3)); 
        node->plan_and_dispatch();
    }).detach();

    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}

