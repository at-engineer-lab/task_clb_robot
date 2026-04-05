#include "arm_solve_client/arm_solve_client.hpp"

namespace arm_solve_client{

using MoveToPose = robot_interfaces::action::MoveToPose;
    // 在 .hpp 中是 构造函数的声明     在 .cpp 中是 构造函数的实现，在构造函数实现体中 创建 client_ 客户端
    ArmSolveClient::ArmSolveClient(const std::string & /*node_name*/):Node("arm_solve_client")
    {
        client_ = rclcpp_action::create_client<MoveToPose>(this, "move_to_pose");
        RCLCPP_INFO(this->get_logger(), "ArmSolveClient initialized");
    }

    void ArmSolveClient::send_goal(const geometry_msgs::msg::PoseStamped & target_pose)
    {
        if (!client_->wait_for_action_server(std::chrono::seconds(5))) {
            RCLCPP_ERROR(this->get_logger(), "Action server not available");
            return;
        }

        auto goal_msg = MoveToPose::Goal();
        goal_msg.target_pose = target_pose;
        
        // send_goal_options ： 提供 事件注册机制，当 Server 一满足某种条件，Client 就可以调用哪个回调
        auto send_goal_options = rclcpp_action::Client<MoveToPose>::SendGoalOptions();
        
        // goal_response_callback ： 对应 handle_goal 函数
        send_goal_options.goal_response_callback =
            std::bind(&ArmSolveClient::goal_response_callback, this, std::placeholders::_1);
        
        // feedback_callback ： 对应 publish_feedback 函数
        send_goal_options.feedback_callback =
            std::bind(&ArmSolveClient::feedback_callback, this,
                      std::placeholders::_1, std::placeholders::_2);
        
        // result_callback : 对应 Server 完成
        send_goal_options.result_callback =
            std::bind(&ArmSolveClient::result_callback, this, std::placeholders::_1);

        client_->async_send_goal(goal_msg, send_goal_options);
    }

    rclcpp_action::Client<MoveToPose>::SharedPtr client_;

    void ArmSolveClient::goal_response_callback(
    rclcpp_action::ClientGoalHandle<MoveToPose>::SharedPtr goal_handle)
    {
      if (!goal_handle) {
          RCLCPP_ERROR(this->get_logger(), "Goal was rejected by server");
      }   else {
            RCLCPP_INFO(this->get_logger(), "Goal accepted by server, executing...");
      }
    }

    void ArmSolveClient::feedback_callback(
        rclcpp_action::ClientGoalHandle<MoveToPose>::SharedPtr,
        // 调用 .action C++ 接口
        const std::shared_ptr<const MoveToPose::Feedback> feedback)
    {
        RCLCPP_INFO(this->get_logger(), "Feedback: %f", feedback->progress);
    }

    void ArmSolveClient::result_callback(
        
        // WrappedResult ： 是一个 结构体，里面有 result.code （状态码） result.result （具体结果数据 message 啥的）
        const rclcpp_action::ClientGoalHandle<MoveToPose>::WrappedResult & result)
    {
        switch (result.code) {
            case rclcpp_action::ResultCode::SUCCEEDED:
                RCLCPP_INFO(this->get_logger(), "Goal succeeded");
                break;
            case rclcpp_action::ResultCode::ABORTED:
                RCLCPP_ERROR(this->get_logger(), "Goal was aborted: %s",
                             result.result->message.c_str());
                break;
            case rclcpp_action::ResultCode::CANCELED:
                RCLCPP_WARN(this->get_logger(), "Goal was canceled");
                break;
            default:
                RCLCPP_ERROR(this->get_logger(), "Unknown result code");
                break;
        }
    }
};


int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<arm_solve_client::ArmSolveClient>();

    geometry_msgs::msg::PoseStamped target_pose;
    target_pose.header.frame_id = "world";
    target_pose.pose.position.x = -0.16282;
    target_pose.pose.position.y = 9.2687e-06;
    target_pose.pose.position.z = 0.28499;
    target_pose.pose.orientation.w = -3.98908e-05;
    target_pose.pose.orientation.x = -0.4254;
    target_pose.pose.orientation.y = 4.2966e-05;
    target_pose.pose.orientation.z = 0.905006;

    geometry_msgs::msg::PoseStamped target_base;
    

    node->send_goal(target_pose);
    RCLCPP_INFO(rclcpp::get_logger("logger"),"target_pose成功发送！！！");

    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}