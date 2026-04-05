#include "arm_solve_real_server1/arm_solve_real_server1.hpp"
#include <thread>
#include <unordered_map>
#include <chrono>

namespace arm_solve_real_server1 {

ArmSolveServer::ArmSolveServer(const rclcpp::NodeOptions & options)
: Node("arm_solve_node", options) {
    // 关节名顺序必须与 real_system 中的索引顺序一致
    joint_order_ = {"joint1", "joint2", "joint3", "joint4", "joint5"};
    
    // 发布话题给 real_system 驱动
    joint_pub_ = this->create_publisher<sensor_msgs::msg::JointState>("fake_joint_commands", 10);
}

void ArmSolveServer::send_single_joint_test() {
    // 1. 创建临时的哈希表，存储“你想让哪个关节动到多少度”
    std::unordered_map<std::string, double> target_values;
    
    // 初始化所有关节为 0
    for(const auto& name : joint_order_) {
        target_values[name] = 0.0;
    }

    // 2. 赋值：你想改哪个关节，直接根据名字改，不用管顺序
    target_values["joint1"] = 1.5708; // 90度
    // 比如以后你想让 joint3 动，直接写：target_values["joint3"] = 0.5;

    // 3. 构建要发布的 JointState 消息
    sensor_msgs::msg::JointState msg;
    msg.header.stamp = this->get_clock()->now();

    // 4. 【核心步骤】按照 joint_order_ 定义的物理顺序提取数据
    for (const auto & name : joint_order_) {
        msg.name.push_back(name);
        // 根据名字从哈希表里取值，这样存进数组里的顺序就永远是 1, 2, 3, 4, 5
        msg.position.push_back(target_values[name]);
    }

    // 5. 发布
    joint_pub_->publish(msg);
    RCLCPP_INFO(this->get_logger(), "已通过哈希表对齐顺序，发送了 5 轴同步指令。");
}

} // namespace arm_solve_real_server1

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<arm_solve_real_server1::ArmSolveServer>(rclcpp::NodeOptions());
    
    // 在独立线程中执行单次测试逻辑
    std::thread([node]() {
        // 等待 2 秒，确保 real_system 驱动节点已经启动并准备好接收
        rclcpp::sleep_for(std::chrono::seconds(2)); 
        
        // 执行测试：设置 90 度
        node->send_single_joint_test();
    }).detach();

    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}