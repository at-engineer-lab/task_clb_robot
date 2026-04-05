import os
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
from moveit_configs_utils import MoveItConfigsBuilder
from launch.actions import SetEnvironmentVariable

def generate_launch_description():
    # 1. 环境准备
    solve_server_share = get_package_share_directory('arm_solve_real_server2')
    # 向上回溯到工作空间根目录定位 SDK
    workspace_root = os.path.abspath(os.path.join(solve_server_share, '..', '..', '..', '..'))
    sdk_lib_path = os.path.join(workspace_root, 'resources', 'arm_sdk', 'lib')

    set_ld_library_path = SetEnvironmentVariable(
        name='LD_LIBRARY_PATH',
        value=[sdk_lib_path, ':', os.environ.get('LD_LIBRARY_PATH', '')]
    )

    # 2. 自动构建 MoveIt 配置
    # 这里的 package_name 必须是你存放 URDF/SRDF 的那个配置包
    moveit_config = MoveItConfigsBuilder("arm5dof", package_name="robot_arm_moveit_config").to_moveit_configs()

    # 3. 核心节点定义
    
    # 机器人状态发布者 (TF 树)
    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='both',
        parameters=[moveit_config.robot_description, {'use_sim_time': False}],
    )

    # MoveGroup 核心节点 (处理规划)
    run_move_group_node = Node(
        package='moveit_ros_move_group',
        executable='move_group',
        output='screen',
        parameters=[moveit_config.to_dict(), {'use_sim_time': False}],
    )

    # 实机串口驱动节点
    real_system_node = Node(
        package='arm5dof_real_hardware',
        executable='real_system_node',
        output='screen'
    )

    # 业务逻辑节点 (你的控制代码)
    arm_solve_node2 = Node(
        package='arm_solve_real_server2',
        executable='arm_solve_real_server2_node', # 请确认 CMake 里的 target 名
        output='screen',
        parameters=[
            moveit_config.to_dict(), # 传递全部 MoveIt 参数，包含 description 和 kinematics
            {'use_sim_time': False}
        ],
    )

    return LaunchDescription([
        set_ld_library_path,
        robot_state_publisher,
        run_move_group_node,
        real_system_node,
        arm_solve_node2
    ])