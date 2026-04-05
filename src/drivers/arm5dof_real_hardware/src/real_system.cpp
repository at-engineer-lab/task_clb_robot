#include "arm5dof_real_hardware/real_system.hpp"
#include <cmath>

namespace arm5dof_real_hardware {

RealServoDriver::RealServoDriver(const rclcpp::NodeOptions & options)
: Node("real_servo_driver", options),
  // 初始化协议，指定串口设备和波特率
  protocol_("/dev/ttyUSB0", FSUS_DEFAULT_BAUDRATE) 
{
    RCLCPP_INFO(this->get_logger(), "正在初始化 5-DOF 舵机实机驱动...");

    // 批量初始化 5 个舵机对象
    for (int i = 0; i < NUM_JOINTS; ++i) {
        // 将循环索引 i 作为舵机的物理 ID, i 既是 容器中的索引 ， 又是 舵机的id
        // 为每个 舵机 都创建 相应的对象，各自的对象后期可控制各自的舵机
        servos_.push_back(std::make_unique<FSUS_Servo>(i, &protocol_));
    }

    // 订阅 arm_solve_server 发布的消息
    cmd_sub_ = this->create_subscription<sensor_msgs::msg::JointState>(
        "joint_commanding", 10,
        std::bind(&RealServoDriver::command_callback, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "实机驱动已启动。串口: /dev/ttyUSB0, 话题: fake_joint_commands");
}

void RealServoDriver::command_callback(const sensor_msgs::msg::JointState::SharedPtr msg) {
    // 获取接收到的位置数组长度
    size_t received_size = msg->position.size();
    if (received_size == 0) {
        RCLCPP_WARN(this->get_logger(), "收到空的关节位置数据，忽略。");
        return;
    }

    // 循环遍历 5 个舵机进行角度更新
    for (int i = 0; i < NUM_JOINTS; ++i) {
        float target_angle_deg = 0.0f;

        if (i < static_cast<int>(received_size)) {
            // ROS 2 弧度 (rad) -> 舵机库角度 (deg)
            target_angle_deg = static_cast<float>(msg->position[i] * 180.0 / M_PI);
        } else {
            // 如果收到的数组长度不足 5，其余关节保持在 0 度（或你可以设置为维持当前值）
            target_angle_deg = 0.0f;
        }

        // 下发指令给第 i 个舵机
        // 虽然是在循环内，但指令发送间隔极短，肉眼观察 5 个关节是同步动作的
        servos_[i]->setAngle(target_angle_deg, ACTION_TIME_MS);
    }
}

} // namespace arm5dof_real_hardware

// 标准 ROS 2 节点入口
int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<arm5dof_real_hardware::RealServoDriver>(rclcpp::NodeOptions());
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}