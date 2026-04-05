import os
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
from moveit_configs_utils import MoveItConfigsBuilder

def generate_launch_description():
    # 1. 自动构建 MoveIt 配置 (替换为你的 MoveIt 配置包名)
    # 如果你的配置包叫 'engineer_moveit_config'
    moveit_config = MoveItConfigsBuilder("arm5dof", package_name="robot_arm_moveit_config").to_moveit_configs()

    # 2. 启动 arm_solve_server 节点
    # 注意：它必须携带 robot_description 和规划组参数，否则 MoveGroupInterface 无法初始化
    arm_solve_node = Node(
        package='arm_solve_server',
        executable='arm_solve_server_node',
        output='screen',
        parameters=[
            moveit_config.robot_description,
            moveit_config.robot_description_semantic,
            moveit_config.robot_description_kinematics,
            moveit_config.planning_pipelines,
            moveit_config.joint_limits,
        ],
    )

    # 3. 启动 fake_system_node 节点
    fake_system_node = Node(
        package='arm5dof_hardware',
        executable='fake_system_node',
        output='screen',
    )

    # 4. 启动官方的 robot_state_publisher (发布 TF 树)
    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='both',
        parameters=[moveit_config.robot_description],
    )

    # 5. 启动 RViz2
    # 找 moveit.rviz 文件
    rviz_config_file = os.path.join(
        get_package_share_directory('robot_arm_moveit_config'), 'config', 'moveit.rviz')
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='log',
        arguments=['-d', rviz_config_file],
        parameters=[
            moveit_config.robot_description,
            moveit_config.robot_description_semantic,
            moveit_config.robot_description_kinematics,
            moveit_config.planning_pipelines,
        ],
    )

    # 6. 启动 MoveGroup 核心节点 (处理规划请求)
    run_move_group_node = Node(
        package='moveit_ros_move_group',
        executable='move_group',
        output='screen',
        parameters=[moveit_config.to_dict(),],
    )

    return LaunchDescription([
        robot_state_publisher,
        run_move_group_node,
        fake_system_node,
        arm_solve_node,
        rviz_node,
    ])