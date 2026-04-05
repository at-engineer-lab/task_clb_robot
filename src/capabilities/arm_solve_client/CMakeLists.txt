cmake_minimum_required(VERSION 3.8)
project(arm_solve_client)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(robot_interfaces REQUIRED)  # 你的 MoveToPose.action 包
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

# 命名规范：可执行文件命名 加后缀 _node ，方便让 可执行文件 和 功能包 区分开来
add_executable(arm_solve_client_node src/arm_solve_client.cpp)

target_include_directories(arm_solve_client_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

ament_target_dependencies(arm_solve_client_node
  rclcpp 
  rclcpp_action 
  robot_interfaces 
  geometry_msgs
  tf2
  tf2_ros
  tf2_geometry_msgs
  )

install(TARGETS
  arm_solve_client_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
