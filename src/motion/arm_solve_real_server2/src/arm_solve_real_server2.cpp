#include "arm_solve_real_server2/arm_solve_real_server2.hpp"
#include <thread>
#include <chrono>
#include <unordered_map>

namespace arm_solve_real_server2 {

ArmSolveServer::ArmSolveServer(const rclcpp::NodeOptions & options)
: Node("arm_solve_node", options) {
    // 1. 初始化硬件期望的关节顺序
    joint_order_ = {"joint1", "joint2", "joint3", "joint4", "joint5"};
    
    // 2. 发布话题给实机驱动 (real_system)
    joint_pub_ = this->create_publisher<sensor_msgs::msg::JointState>("fake_joint_commands", 10);
}

void ArmSolveServer::plan_and_dispatch() {
    RCLCPP_INFO(this->get_logger(), "正在初始化 MoveGroup 接口...");
    
    // 初始化 MoveGroup (规划组名称需与你的 SRDF 一致，假设为 "arm")
    move_group_ = std::make_shared<moveit::planning_interface::MoveGroupInterface>(shared_from_this(), "arm");

    // 1. 显式设置目标位姿 (这里你可以根据实际需求修改数值)
    geometry_msgs::msg::Pose target_pose;
    
    // 设置位置 (单位: 米)
    target_pose.position.x = 0.00162208;    
    target_pose.position.y = -0.0534118;    
    target_pose.position.z = 0.29807;   

    // 设置姿态 (四元数) - 示例为夹爪垂直向下
    target_pose.orientation.x = 0.287518;
    target_pose.orientation.y = 0.296375;  
    target_pose.orientation.z = -0.634151;
    target_pose.orientation.w = 0.653718; 
    
    // 设置目标位姿给 MoveIt
    move_group_->setPoseTarget(target_pose, "tcp_link");

    // 2. 调用 MoveIt 进行路径规划
    moveit::planning_interface::MoveGroupInterface::Plan my_plan;
    bool success = (move_group_->plan(my_plan) == moveit::core::MoveItErrorCode::SUCCESS);

    if (success) {
        RCLCPP_INFO(this->get_logger(), "规划成功！开始解析轨迹并下发至硬件...");

        auto & trajectory = my_plan.trajectory_.joint_trajectory;
        rclcpp::Time start_real_time = this->get_clock()->now();

        // 3. 遍历轨迹点，按时间戳平滑下发
        for (const auto & point : trajectory.points) {
            
            // 对齐规划的时间轴
            rclcpp::Duration expected_offset = point.time_from_start;
            rclcpp::Time target_real_time = start_real_time + expected_offset;

            while (this->get_clock()->now() < target_real_time) {
                std::this_thread::sleep_for(std::chrono::microseconds(500));
            }

            // --- 核心分拣逻辑：哈希表对齐 ---
            // A. 将 MoveIt 给出的当前点数据存入哈希表 (无视其原始顺序)
            std::unordered_map<std::string, double> current_joint_map;
            for (size_t i = 0; i < trajectory.joint_names.size(); ++i) {
                current_joint_map[trajectory.joint_names[i]] = point.positions[i];
            }

            // B. 构建 JointState 消息
            sensor_msgs::msg::JointState msg;
            msg.header.stamp = this->get_clock()->now();

            // C. 按照硬件 joint_order_ 列表提取数据，保证发给驱动的数组是有序的
            for (const auto & name : joint_order_) {
                msg.name.push_back(name);
                if (current_joint_map.find(name) != current_joint_map.end()) {
                    msg.position.push_back(current_joint_map[name]);
                } else {
                    msg.position.push_back(0.0); // 容错
                }
            }

            // 4. 发送给 real_system 驱动
            joint_pub_->publish(msg);
        }
        RCLCPP_INFO(this->get_logger(), "实机轨迹执行完毕。");
    } else {
        RCLCPP_ERROR(this->get_logger(), "规划失败，请检查目标点是否在工作空间内且无遮挡！");
    }
}

} // namespace arm_solve_real_server2

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    
    // 使用正确的命名空间
    auto node = std::make_shared<arm_solve_real_server2::ArmSolveServer>(rclcpp::NodeOptions());
    
    // 开启独立线程执行业务逻辑，不阻塞 spin
    std::thread([node]() {
        // 等待 MoveGroup 骨干节点完全启动
        rclcpp::sleep_for(std::chrono::seconds(3)); 
        node->plan_and_dispatch();
    }).detach();

    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}