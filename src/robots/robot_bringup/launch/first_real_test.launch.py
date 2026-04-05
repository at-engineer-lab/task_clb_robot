import os
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
from launch.actions import SetEnvironmentVariable

def generate_launch_description():
    # 1. 获取功能包路径 (学习示例中的结构)
    # 获取 arm_solve_real_server1 在 install 目录下的 share 路径
    solve_server_share = get_package_share_directory('arm_solve_real_server1')
    
    # 2. 定位动态库路径 (resources/arm_sdk/lib)
    # 逻辑：从 share/arm_solve_real_server1 向上跳 4 级到达工作空间根目录
    workspace_root = os.path.abspath(os.path.join(solve_server_share, '..', '..', '..', '..'))
    sdk_lib_path = os.path.join(workspace_root, 'resources', 'arm_sdk', 'lib')

    # 3. 设置环境变量 LD_LIBRARY_PATH (确保 real_system 能加载动态库)
    set_ld_library_path = SetEnvironmentVariable(
        name='LD_LIBRARY_PATH',
        value=[sdk_lib_path, ':', os.environ.get('LD_LIBRARY_PATH', '')]
    )

    # 4. 启动实机驱动节点 (real_system)
    # 对应你 CMakeLists.txt 中的可执行文件名 real_system_node
    real_system_node = Node(
        package='arm5dof_real_hardware',
        executable='real_system_node',
        output='screen'
    )

    # 5. 启动指令分发节点 (arm_solve_node)
    # 对应你 CMakeLists.txt 中的可执行文件名 arm_solve_node
    arm_solve_node = Node(
        package='arm_solve_real_server1',
        executable='arm_solve_real_server1_node',
        output='screen'
    )

    # 6. 构建启动描述列表
    return LaunchDescription([
        set_ld_library_path,  # 先注入库路径
        real_system_node,     # 启动驱动
        arm_solve_node        # 发送指令
    ])