# ament_package()
cmake_minimum_required(VERSION 3.8)
project(arm_solve_server_action)

# 编译标准
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 1. 查找基础依赖
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_eigen REQUIRED)
find_package(Eigen3 REQUIRED)

# 2. 查找自定义接口
find_package(robot_interfaces REQUIRED)

# 3. 查找 MoveIt 组件
# 注意：有些版本的 MoveIt 也可以直接 find_package(moveit_ros_planning)
find_package(moveit_core REQUIRED)
find_package(moveit_ros_planning REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)
find_package(moveit_msgs REQUIRED)

# 4. 添加可执行文件
# 假设你的源文件名为 src/arm_solve_server_node.cpp
add_executable(arm_solve_server_action_node
  src/arm_solve_server_action.cpp
)

# 5. 包含头文件路径
target_include_directories(arm_solve_server_action_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${EIGEN3_INCLUDE_DIRS}
)

# 6. 关联依赖 (这行会解决你所有的 "No such file or directory" 报错)
ament_target_dependencies(arm_solve_server_action_node
  rclcpp
  rclcpp_action
  geometry_msgs
  sensor_msgs
  tf2_geometry_msgs
  tf2_eigen
  robot_interfaces
  moveit_core
  moveit_ros_planning
  moveit_ros_planning_interface
  moveit_msgs
)

# 7. 静态检查与安装 (可选但推荐)
install(TARGETS arm_solve_server_action_node
  DESTINATION lib/${PROJECT_NAME})

ament_package()