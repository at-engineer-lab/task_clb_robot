cmake_minimum_required(VERSION 3.8)
project(arm5dof_hardware)

# 寻找依赖
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)

# 添加可执行文件
add_executable(fake_system_node src/fake_system.cpp)

# 包含头文件目录
target_include_directories(fake_system_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# 链接依赖
ament_target_dependencies(fake_system_node
  rclcpp
  sensor_msgs
)

install(TARGETS fake_system_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()