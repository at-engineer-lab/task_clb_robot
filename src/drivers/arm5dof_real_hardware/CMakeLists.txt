cmake_minimum_required(VERSION 3.8)
project(arm5dof_real_hardware)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 1. 寻找 ROS 2 基础依赖
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)

# 2. 精准定位你的 SDK 路径 (基于你的 VS Code 截图)
# ${CMAKE_CURRENT_SOURCE_DIR} 是当前 package 的路径
set(ARM_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../resources/arm_sdk")

# 3. 添加头文件搜索路径
include_directories(
  include
  "${ARM_SDK_ROOT}/include"
)

# 4. 添加库文件搜索路径
link_directories("${ARM_SDK_ROOT}/lib")

# 5. 生成可执行文件
add_executable(real_system_node src/real_system.cpp)

# 6. 链接 ROS 2 依赖
ament_target_dependencies(real_system_node
  rclcpp
  sensor_msgs
)

# 7. 链接外部动态库 (根据截图中的文件名去 lib 前缀)
# libcserialport.so -> cserialport
# libfsuartservo.so -> fsuartservo
target_link_libraries(real_system_node
  fsuartservo
  cserialport
)

# 8. 安装设置
install(TARGETS
  real_system_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()