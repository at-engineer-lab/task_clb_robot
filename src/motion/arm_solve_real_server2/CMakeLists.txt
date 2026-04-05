cmake_minimum_required(VERSION 3.8)
project(arm_solve_real_server2)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(trajectory_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)

add_executable(arm_solve_real_server2_node src/arm_solve_real_server2.cpp)
target_include_directories(arm_solve_real_server2_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
ament_target_dependencies(arm_solve_real_server2_node
  rclcpp
  moveit_ros_planning_interface
  sensor_msgs
  trajectory_msgs
)

install(TARGETS arm_solve_real_server2_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()