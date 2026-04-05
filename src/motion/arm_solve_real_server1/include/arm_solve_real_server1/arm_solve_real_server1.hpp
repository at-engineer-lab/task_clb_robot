#ifndef ARM_SOLVE_REAL_SERVER1_HPP_
#define ARM_SOLVE_REAL_SERVER1_HPP_

#include <rclcpp/rclcpp.hpp>
#include <vector>
#include <sensor_msgs/msg/joint_state.hpp>
#include <unordered_map>
#include <string>

namespace arm_solve_real_server1 {

class ArmSolveServer : public rclcpp::Node {
public:
    explicit ArmSolveServer(const rclcpp::NodeOptions & options);

    /**
     * @brief 发送单次关节角度测试指令 (例如：设置 joint1 为 90度)
     */
    void send_single_joint_test();

private:
    // 发布给 real_system 驱动的话题发布者
    rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_pub_;
    
    // 定义硬件期望的五轴顺序 (用于构建 position 数组)
    std::vector<std::string> joint_order_;
};

} // namespace arm_solve_server

#endif // ARM_SOLVE_REAL_SERVER1_HPP_