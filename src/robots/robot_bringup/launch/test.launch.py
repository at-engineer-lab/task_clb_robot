import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    TimerAction,
    ExecuteProcess,
    RegisterEventHandler,
)
from launch.event_handlers import OnProcessExit
from moveit_configs_utils import MoveItConfigsBuilder
from launch_ros.actions import Node

def generate_launch_description():
    # 核心配置
    robot_name = "arm5dof"
    config_package = 'robot_arm_moveit_config'

    # 路径配置
    config_pkg_dir = get_package_share_directory(config_package)
    ros2_controllers_path = os.path.join(config_pkg_dir, "config", "ros2_controllers.yaml")
    urdf_xacro_path = os.path.join(config_pkg_dir, "config", "arm5dof.urdf.xacro")
    rviz_config_file = os.path.join(config_pkg_dir, "config", "moveit.rviz")
    moveit_controllers_path = os.path.join(config_pkg_dir, "config", "moveit_controllers.yaml")

    # MoveItConfigsBuilder 配置
    moveit_config = (
        MoveItConfigsBuilder(
            robot_name=robot_name,
            package_name=config_package
        )
        .robot_description(file_path=urdf_xacro_path)
        .planning_pipelines(pipelines=["ompl"])
        .to_moveit_configs()
    )

    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        output="both",
        parameters=[moveit_config.robot_description],
        respawn=False,
    )

    ros2_control_node = Node(
    package="controller_manager",
    executable="ros2_control_node",
    output="screen",
    parameters=[
        moveit_config.robot_description,
        ros2_controllers_path,   # 原有的 YAML 文件
        {   # 原有的硬件接口配置
            "hardware_interface": {
                "arm5dof_hardware/FakeSystemHardware": {
                    "plugin": "arm5dof_hardware/FakeSystemHardware",
                    "initial_state": {
                        "joint1": 0.0,
                        "joint2": -1.0,
                        "joint3": 1.3,
                        "joint4": 0.0,
                        "joint5": 0.0,
                        "left_gripper_joint": 0.04
                    }
                }
            }
        }
    ],
    
    remappings=[("/controller_manager/robot_description", "/robot_description")],
    respawn=False,
)

    joint_state_broadcaster = TimerAction(
        period=2.0,
        actions=[
            Node(
                package='controller_manager',
                executable='spawner',
                arguments=["joint_state_broadcaster", "-c", "/controller_manager"],
                output="screen",
            )
        ]
    )

    # 机械臂控制器
    arm_controller = TimerAction(
        period=3.0,
        actions=[
            Node(
                package='controller_manager',
                executable='spawner',
                arguments=["arm5dof_arm", "-c", "/controller_manager"],
                output="screen",
            )
        ]
    )
    
    # 手爪控制器
    hand_controller = TimerAction(
        period=4.0,
        actions=[
            Node(
                package='controller_manager',
                executable='spawner',
                arguments=["arm5dof_hand", "-c", "/controller_manager"],
                output="screen",
            )
        ]
    )

    # MoveGroup节点
    move_group_node = TimerAction(
        period=5.0,
        actions=[
            Node(
                package="moveit_ros_move_group",
                executable="move_group",
                output="screen",
                parameters=[
                    moveit_config.to_dict(),
                    moveit_controllers_path,
                    {"planning_scene_monitor_options.publish_planning_scene": True},
                    {"planning_scene_monitor_options.publish_geometry_updates": True},
                    {"moveit_controller_manager": "moveit_simple_controller_manager/MoveItSimpleControllerManager"}
                ],
                arguments=["--ros-args", "--log-level", "info"],
                respawn=False,
            )
        ]
    )

    # RViz节点（单独定义，方便监听）
    rviz_node_action = Node(
        package="rviz2",
        executable="rviz2",
        output="log",
        arguments=["-d", rviz_config_file],
        parameters=[
            moveit_config.robot_description,
            moveit_config.robot_description_semantic,
            moveit_config.robot_description_kinematics,
        ],
        respawn=False,
    )
    rviz_node = TimerAction(
        period=6.0,
        actions=[rviz_node_action]
    )

    # -------------------------- 组装LaunchDescription --------------------------
    return LaunchDescription([
        # 原有节点逻辑
        robot_state_publisher_node,
        ros2_control_node,
        joint_state_broadcaster,
        arm_controller,
        hand_controller,
        move_group_node,
        rviz_node,
    ])