#ifndef ARM5DOF_REAL_DRIVER__REAL_SYSTEM_HPP_
#define ARM5DOF_REAL_DRIVER__REAL_SYSTEM_HPP_

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"

// 包含底层串口库 (CSerialPort)
#include "CSerialPort/SerialPort.h" 
// 包含 FashionStar 舵机库
#include "FashionStar/UServo/FashionStar_UartServoProtocol.h"
#include "FashionStar/UServo/FashionStar_UartServo.h"

#include <vector>
#include <memory>

using namespace fsuservo;

namespace arm5dof_real_hardware {

class RealServoDriver : public rclcpp::Node {
public:
    explicit RealServoDriver(const rclcpp::NodeOptions & options);
    ~RealServoDriver() override = default;

private:
    /**
     * @brief 订阅来自 arm_solve_server 的关节指令回调
     * 接收弧度值并转换为角度值通过串口下发给 5 个舵机
     */
    //  msg 本身就是 一个容器，能够自动调整长度
    void command_callback(const sensor_msgs::msg::JointState::SharedPtr msg);

    // 串口协议对象 (管理底层 CSerialPort)
    FSUS_Protocol protocol_;
    
    // 舵机对象容器，索引 0-4 分别对应物理 ID 0-4
    // 刚被创建出来的容器 是空的，在 .cpp文件中，会向舵机对象容器中 添加舵机对象
    std::vector<std::unique_ptr<FSUS_Servo>> servos_;

    // ROS 2 订阅者
    rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr cmd_sub_;
    
    // 机械臂关节总数
    const int NUM_JOINTS = 5;
    // 舵机完成动作的预设时间 (ms)
    const uint16_t ACTION_TIME_MS = 2000; 
};

} // namespace arm5dof_hardware

#endif // ARM5DOF_REAL_DRIVER__REAL_SYSTEM_HPP_