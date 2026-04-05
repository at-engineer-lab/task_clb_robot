import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import TimerAction
from ament_index_python.packages import get_package_share_directory
from moveit_configs_utils import MoveItConfigsBuilder

def generate_launch_description():

    # 1. MoveIt 配置
    moveit_config = MoveItConfigsBuilder(
        "arm5dof",
        package_name="robot_arm_moveit_config"
    ).to_moveit_configs()

    # 2. robot_state_publisher
    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='both',
        parameters=[moveit_config.robot_description],
    )

    # --- 新增：静态坐标转换发布器 ---
    # 这行代码强行连接 world 和 base_link，消除 "frame does not exist" 或 "time 0.0" 的同步问题
    static_tf_node = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_transform_publisher',
        arguments=['0', '0', '0', '0', '0', '0', 'world', 'base_link']
    )

    # # 3. fake_system_node（发布 joint_states）
    # fake_system_node = Node(
    #     package='arm5dof_hardware',
    #     executable='fake_system_node',
    #     output='screen'
    # )

    real_system_node = Node(
        package='arm5dof_real_hardware',
        executable='real_system_node',
        output='screen'
    )

    # 4. MoveGroup（MoveIt 核心）
    run_move_group_node = Node(
        package='moveit_ros_move_group',
        executable='move_group',
        output='screen',
        parameters=[moveit_config.to_dict()],
    )

    # 5. arm_solve_server_action_node 节点 开启
    arm_solve_server_node = Node(
        package='arm_solve_server_action',
        executable='arm_solve_server_action_node',
        output='screen',
        parameters=[
            moveit_config.to_dict(),
            {"planning_plugin": "ompl_interface/OMPLPlanner"} # 强行指定
                    ],
    )

    # --- 新增：RViz 节点配置 ---
    # 获取 moveit 配置包中的 rviz 配置文件路径（通常是 moveit.rviz）
    # rviz_config_file = os.path.join(
    #     get_package_share_directory("robot_arm_moveit_config"),
    #     "config",
    #     "moveit.rviz"
    # )

    # rviz_node = Node(
    #     package="rviz2",
    #     executable="rviz2",
    #     name="rviz2",
    #     output="log",
    #     arguments=["-d", rviz_config_file],
    #     parameters=[
    #         moveit_config.robot_description,
    #         moveit_config.robot_description_semantic,
    #         moveit_config.planning_pipelines,
    #         moveit_config.robot_description_kinematics,
    #     ],
    # )

    # 6. arm_solve_client（延迟启动）
    arm_solve_client_node = Node(
        package='arm_solve_client',
        executable='arm_solve_client_node',
        output='screen'
    )

    delayed_client = TimerAction(
        period=5.0,   # 延迟 3 秒（可根据情况调整到 5 秒更稳）
        actions=[arm_solve_client_node]
    )
    
    # 可以按照 依赖 来写 launch文件 中的 启动顺序
    return LaunchDescription([
        robot_state_publisher,
        static_tf_node,
        real_system_node,
        # fake_system_node,
        run_move_group_node,
        arm_solve_server_node,
        # rviz_node,
        delayed_client
    ])