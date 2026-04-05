cmake_minimum_required(VERSION 3.8)
project(arm_solve_real_server1)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)

add_executable(arm_solve_real_server1_node src/arm_solve_real_server1.cpp)
target_include_directories(arm_solve_real_server1_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
ament_target_dependencies(arm_solve_real_server1_node
  rclcpp
  sensor_msgs
)

install(TARGETS arm_solve_real_server1_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()