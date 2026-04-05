cmake_minimum_required(VERSION 3.8)
project(robot_arm_description)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
# 1. 查找依赖包
# package.xml中有哪些“依赖”，CMakeLists.txt中就找哪些包；
find_package(ament_cmake REQUIRED)

# 6. 安装 launch 和 config 文件夹 (如果有的话)
# 因为--build-type 是ament_cmake类型，所以，根本不需要resource目录
# 又src是空目录，因此，也不需要install src目录（因为没意义）；
install(DIRECTORY
  urdf
  meshes
  DESTINATION share/${PROJECT_NAME}
)
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

ament_package()
