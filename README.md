### 快速入手

>   注意命名规范性，将整个工作区push到github的组织中

/resources中含有创乐博机械臂的模型文件和舵机资料



#### 配置基本环境

>   对于每一个新项目而言，配置基本环境的流程是固定的，需要规范

1.   修改urdf
2.   配置_description功能包（_description功能包中包含着"meshes"和"urdf"两个有用的目录，并修改改功能包的CMakeLists.txt和package.xml）；
3.   msa配置_moveit_config功能包（直接让msa生成，CMakeLists.txt和package.xml也让msa生成，生成之后直接使用，不需要更改）；
4.   写一个启动launch（可以看学长在github中发布的项目，只要改其中的部分内容就可以了）
      前提 ： 创建一个 robot_bringup 功能包，里面装config目录和launch目录；
5.   （可选）配置ikfast，生成_ikfast_plugin功能包



>   可实现最简单机械臂任务
>
>      机械臂控制方式分为两种
>
>     		1.  通过 plan + execute 但是需要控制器controller和ros2_control架构 (强ros依赖)
>     		2.  通过 plan + 拆分轨迹发布轨迹点(自定义cmd) 实现，需要自己编写 fake_system

1.   理解两种方式，并编写相应底层 (hardware_interface) (fake_system)
2.   简单的定点规划，在rviz中移动

#### 控制实机

>   resources 中有控制舵机的sdk,可通过调用sdk中的函数，来控制舵机

1.   通过调用sdk来控制舵机
2.   将目标任务移到实机执行