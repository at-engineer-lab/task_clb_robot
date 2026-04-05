## 创建项目（ROS2 + MoveIt2 + 通信上位机）

>   本文记录从 URDF → MoveIt2 配置 → ros2_control → 硬件接口 → RViz 验证的完整流程，以便后续可扩展到上位机通信系统。

------

### 创建 moveit_config 包

#### 开始

>   使用 MoveIt Setup Assistant 快速生成环境，但后续修改复杂，因此需详细记录流程。

------

#### **1. 通过 SolidWorks 插件导出 URDF**

>   SW 插件需提前设置各项参数，确保导出的 URDF 满足 MoveIt2 和 ros2_control 要求。

关键注意点：

-   导出前确保 **名称准确一致**
-   删除 base_link 的 inertial（惯性）标签
-   修改 CMakeLists.txt 与 package.xml（内容如下）
-   所有关节类型必须设为 **revolute（旋转关节）**
-   关节限制暂时设置为 **-3.14 到 3.14**（后续 MoveIt 会再调整）
-   使用插件检查导出问题，并校准零点

CMakeLists 示例：

```
cmake_minimum_required(VERSION 3.8)
project(engineer_robot_description)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

find_package(ament_cmake REQUIRED)

install(
  DIRECTORY urdf meshes config
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
```

package.xml：

```
<?xml version="1.0"?>
<?xml-model href="http://download.ros.org/schema/package_format3.xsd" schematypens="http://www.w3.org/2001/XMLSchema"?>
<package format="3">
  <name>engineer_robot_description</name>
  <version>0.0.0</version>
  <description>TODO: Package description</description>
  <maintainer email="2281904682@qq.com">wrj</maintainer>
  <license>TODO: License declaration</license>

  <buildtool_depend>ament_cmake</buildtool_depend>

  <build_depend>xacro</build_depend>
  <build_depend>urdf</build_depend>

  <exec_depend>xacro</exec_depend>
  <exec_depend>urdf</exec_depend>

  <test_depend>ament_lint_auto</test_depend>
  <test_depend>ament_lint_common</test_depend>

  <export>
    <build_type>ament_cmake</build_type>
  </export>
</package>
```

------

#### **2. 使用 MoveIt Setup Assistant 创建 moveit_config 包**

>   需确保已经 `colcon build` 并 `source install/setup.bash`。

操作要点：

-   添加 URDF

-   配置 SRDF

-   规划分组

-   添加虚拟关节

-   校正 Pose 时可找到更准确关节 limit

-   修改 URDF 关节 limit（以 MoveIt 为准）

-   visual使用dae,collision使用降低精度的stl (注意，修改urdf后不用重新创建moveit_config包更新，找的直接就是修改的urdf，可支持动态修改)

    ![image-20251220165049572](https://cdn.jsdelivr.net/gh/dengyu32/note_images/images/config_urdf.png)

------

#### **3. moveit_controllers.yaml 中加入配置**

```
action_ns: follow_joint_trajectory
default: true
```

------

#### **4. 修改 joint_limits.yaml（必须为浮点数）**

示例（原图保留）：

![image-20251213213749273](https://cdn.jsdelivr.net/gh/dengyu32/note_images/images/temple_moveit2_joint_limits_yaml)

------

#### **5. initial_position.yaml 中设置初始角度**

>   保证机器人在 RViz 打开时姿态正确。

---

#### **6. 添加ompl_planning.yaml **

添加

~~~
default_planning_pipeline: ompl
planning_pipelines: [ompl]   # ← 很关键，明确只用 ompl
ompl:
  planning_plugin: ompl_interface/OMPLPlanner
  request_adapters: >-
    default_planner_request_adapters/FixWorkspaceBounds
    default_planner_request_adapters/FixStartStateBounds
    default_planner_request_adapters/FixStartStateCollision
    default_planner_request_adapters/FixStartStatePathConstraints
    default_planner_request_adapters/AddTimeParameterization
  start_state_max_bounds_error: 0.1

planner_configs:
  RRTConnectkConfigDefault:
    type: geometric::RRTConnect
    range: 0.2

group_planner_configs:
  clb_arm:
    - RRTConnectkConfigDefault




~~~

------

#### **7. ros2_control.xacro 中，load_yaml 前必须添加 `xacro.`**

>   否则不会正确加载配置。

------

#### **8. 再次使用 MoveIt Setup Assistant 时注意：**

-   关闭 perception 功能
-   重选 ros2_control
-   **重新执行 2–6 步**

因为重新生成配置会覆盖部分文件。

------

#### **9. RViz 配置阶段**

添加插件：

-   RobotModel（设置 topic：/robot_description）
-   TF

并保存配置文件。

------

#### **10. 使用 ros2_control 底层架构**

>   MoveIt2 可直接使用该配置，额外需要自定义 hardware_interface。

流程：

-   下载 rosTeamWorkspace（可快速生成模板）
-   创建包：

```
ros2 pkg create hardware_interface
colcon build
source install/setup.bash
```

-   一键生成模板：

```
ros2_control_setup-hardware-interface-package hardware_interface HardWare
```

要求：
 需与 moveit_config 中 command_interface / state_interface 完全一致。

---

#### **11. 添加IKFast**

见笔记 IKFast

------

#### **12. 编写 hardware_interface 逻辑**

需要修改：

-   类成员变量
-   on_init()
-   on_configure()
-   read()
-   write()

另外：

-   移植舵机 SDK 或封装串口通信逻辑
-   处理数据解析与同步

------

#### **13. 修改 clb_moveit_config 中 ros2_control 配置**

将 hardware 字段替换成自定义的 hardware 类：

![image-20251026171829922](../../assets/image-20251026171829922.png)

![image-20251026171849750](../../assets/image-20251026171849750.png)

按第一个图示例填写 class 标签即可。

------

#### **14. 验证完整链路**

测试流程：

-   启动系统
-   在 RViz 中逐个转动关节
-   若出现反转：

修改 URDF 中对应关节的 axis 标签：

-   1 → -1
-   -1 → 1

并同步修改初始 pose。

------

### 参考

[https://blog.csdn.net/weixin_58234807/article/details/142097741?ops_request_misc=&request_id=&biz_id=102&utm_term=requesting%20initial%20scene%20faile&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduweb~default-2-142097741.142^v102^pc_search_result_base6&spm=1018.2226.3001.4187](https://blog.csdn.net/weixin_58234807/article/details/142097741?ops_request_misc=&request_id=&biz_id=102&utm_term=requesting initial scene faile&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduweb~default-2-142097741.142^v102^pc_search_result_base6&spm=1018.2226.3001.4187)

------

### 创建 launch_bringup

>   用于集中管理 launch 文件、脚本、上位机流程。

launch 目录可包含：

-   系统总启动文件
-   MoveIt2
-   ros2_control
-   上位机节点
-   Gazebo 仿真（可选）

------

### 创建 base_interfaces 消息包

### 总体流程

>   ROS2 自定义消息的标准流程。

find_package() → rosidl_generate_interface() →
 使用 rosidl_target_interfaces()（或 rosidl_get_typesupport_target + target_link_libraries）
