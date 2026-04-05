import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import TimerAction
from launch_ros.actions import Node
from moveit_configs_utils import MoveItConfigsBuilder


def generate_launch_description():

    robot_name = "arm5dof"
    config_package = "robot_arm_moveit_config"

    config_pkg_dir = get_package_share_directory(config_package)
    urdf_xacro_path = os.path.join(
        config_pkg_dir, "config", "arm5dof.urdf.xacro"
    )
    rviz_config_file = os.path.join(
        config_pkg_dir, "config", "moveit.rviz"
    )

    # ================= MoveIt 配置 =================
    moveit_config = (
        MoveItConfigsBuilder(
            robot_name=robot_name,
            package_name=config_package,
        )
        .robot_description(file_path=urdf_xacro_path)
        .planning_pipelines(pipelines=["ompl"])
        .trajectory_execution(
            file_path="config/moveit_controllers.yaml"
        )
        .to_moveit_configs()
    )

    # ================= fake 执行节点 =================
    fake_system_node = Node(
        package="arm5dof_hardware",
        executable="fake_system_node",
        output="screen",
    )

    # ================= robot_state_publisher =================
    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        output="both",
        parameters=[moveit_config.robot_description],
    )

    # ================= move_group =================
    move_group_node = TimerAction(
        period=2.0,
        actions=[
            Node(
                package="moveit_ros_move_group",
                executable="move_group",
                output="screen",
                parameters=[
                    moveit_config.to_dict(),
                    {
                        "moveit_controller_manager":
                        "moveit_simple_controller_manager/MoveItSimpleControllerManager"
                    },
                ],
                arguments=["--ros-args", "--log-level", "info"],
            )
        ],
    )

    # ================= RViz =================
    rviz_node = TimerAction(
        period=3.0,
        actions=[
            Node(
                package="rviz2",
                executable="rviz2",
                output="log",
                arguments=["-d", rviz_config_file],
                parameters=[
                    moveit_config.robot_description,
                    moveit_config.robot_description_semantic,
                    moveit_config.robot_description_kinematics,
                ],
            )
        ],
    )

    # ================= Launch =================
    return LaunchDescription(
        [
            fake_system_node,
            # joint_state_broadcaster,
            robot_state_publisher_node,
            move_group_node,
            rviz_node,
        ]
    )