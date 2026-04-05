# ROS 2 Control 架构分析

### 图示

![D1FB35619991F3124283554E7054F2DC.png](https://raw.githubusercontent.com/dengyu32/note_images/main/images/20251211150203231.webp)

------

###  纵览流程

1. **Controller Manager**

- 管理所有 `Controller` 的加载、卸载、启动、停止。
- 控制器在加载（load）时**声明需要的接口类型（State / Command）**。
- 然后通过 `Resource Manager` 分配实际接口。

2. **Resource Manager**

- 管理底层硬件资源接口（State/Command）。
- 接收 Controller Manager 的接口 claim 请求。
- 将接口映射到底层的 Sensor、System、Actuator 实体。
- 启动时会**加载 hardware components**（从 plugin 加载，例如 URDF 中 plugin 配置的 hardware_interface）。

------

### 接口类型

| 接口类型          | 功能                      | 权限        | 说明                   |
| ----------------- | ------------------------- | ----------- | ---------------------- |
| State Interface   | 读取当前传感器/执行器状态 | 只读 (read) | 例如位置、速度、温度等 |
| Command Interface | 写入控制指令              | 读写 (rw)   | 例如目标位置、电压等   |

------

### 细节分析

#### Controller（控制器）

- 如：`JointTrajectoryController`、`ForwardCommandController` 等。
- 每个 Controller **通过 Controller Manager 加载**，声明所需接口（如 `/position`, `/velocity` 命令接口）。
- Controller 不直接访问硬件，而是**向 Resource Manager 申请接口**。
- 如果申请的接口不可用（如已被其他控制器占用），启动失败。

#### Sensor / System / Actuator

- 三类 hardware component 类型：

- **Sensor**：提供状态，如温度、IMU 等
- **Actuator**：驱动设备，如电机
- **System**：中间混合层，可包含多个接口（如复合执行器）

- 每个 component 会：

- 实现并**导出一组 StateInterface / CommandInterface**
- Resource Manager 调用其 `export_state_interfaces()` / `export_command_interfaces()` 方法注册接口

####  Transmissions（可选）

- 如果硬件提供的是 joint-level，而 controller 面向 actuator-level，可添加 `Transmission` 做坐标/力转换。
- 例：reduction gear、4轮全向机构等。

#### Hardware Resources

- 最底层的硬件资源。
- 每个 component 可通过 plugin 插入，对应类需继承：

- `hardware_interface::SystemInterface`
- `hardware_interface::SensorInterface`
- `hardware_interface::ActuatorInterface`

- 实现生命周期方法：

- `configure()`, `start()`, `read()`, `write()`, `export_state_interfaces()`, `export_command_interfaces()` 等

------

### 关键流程

| 步骤 | 操作主体           | 描述                                                    |
| ---- | ------------------ | ------------------------------------------------------- |
| 1    | 用户/launch 文件   | 加载 Controller、Hardware Plugin                        |
| 2    | Controller Manager | 请求 Resource Manager 分配接口资源                      |
| 3    | Resource Manager   | 管理并绑定接口至 Sensor/System/Actuator                 |
| 4    | Interface          | 接口向上暴露（Controller 使用），向下映射到实际硬件资源 |
| 5    | Controller         | 调用 read/write 执行控制逻辑                            |

------

### 从属关系

| 名称                                  | 所属模块/层                   | 实现/作用                                  |
| ------------------------------------- | ----------------------------- | ------------------------------------------ |
| `Sensor`, `System`, `Actuator`        | `hardware_interface` 插件实现 | 你写的 plugin，导出接口给 Resource Manager |
| `StateInterface` / `CommandInterface` | 被 Resource Manager 管理      | 从 hardware component 中导出               |
| `Hardware Resources`                  | 被 Resource Manager 管理      | 所有已加载的 hardware plugin 实例          |
| `Resource Manager`                    | ROS 2 control 框架中间层      | 分配接口、管理资源占用                     |
| `Controller Manager`                  | ROS 2 control 框架中间层      | 管理 controller 生命周期                   |

