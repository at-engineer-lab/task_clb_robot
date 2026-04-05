### 修改URDF文件

>   对内项目一般urdf文件都是从solidworks的export_to_urdf插件导出的
>
>   为了适应moveit工程，需要固定的对raw urdf做出一些处理，从而使项目更加规范
>
>   步骤有：
>
>   1.   确保命名规范
>   2.   功能包修改为ROS2格式
>   3.   检查urdf文件(check_urdf xxx.urdf)
>   4.   修改关节配置（属性、限制、零点、轴的方向）
>   5.   添加word_joint
>   6.   添加tcp_link（对于夹爪）
一些小建议：

具体的步骤有：

-1.   修改urdf文件名称为具体型号名称（同 robot_name ），功能包名称为粗略类型+_description 

-   注意：功能包中的src只能装“源文件”，另外建立的urdf、launch等文件夹都是和src平级的（功能包中）；

-   这里给出例子 ec66.urdf、<robot name="ec66">、功能包名称为elite_description


-2.   删除`base_link`惯性标签(<inertial>标签)，删除`fixed关节`的<axis xyz="0 0 0" />，更改功能包为ROS2格式（更换CMakelists.txt和package.xml）在right_gripper_joint中添加mimic标签（sw不会自动导出这个属性）：
    <mimic
      joint="left_gripper_joint"
      multiplier="-1"
      offset="0" />
  
惯性标签：<inertial>标签
注意：<inertial>
  <!-- 1. 质心位置（相对链接坐标系） -->   
  <origin
        xyz="-0.05549 -1.2443E-05 0.014047"
        rpy="0 0 0" />
  <!-- 2. 质量 -->    
  <mass
        value="0.34831" />
  <!-- 3. 惯性张量矩阵（惯性矩）<inertia是必须的 --> 
  <inertia
        ixx="0.00037734"
        ixy="-1.9667E-07"
        ixz="-5.7059E-07"
        iyy="0.0011827"
        iyz="9.3336E-14"
        izz="0.0015361" />
    </inertial>   只删除"base_link"的惯性标签；

-3.   检查urdf最基础的是否导出正确，能否正确的按照轴转动，同时关节的坐标系正确，要是不正确让机械重新导出urdf

-4.   `可动`关节（除了rubber橡胶）属性先设置为revolute，范围限制先给上-3.14到3.14   （不是所有关节！！！）
    记住（重要！）：joint的范围限制改为-3.14到3.14，是为了后期`通过msa的pose来得出joint的准确关节限制`服务的，`不是`为了“urdf_visualizer”插件来服务的，
         "urdf_visualizer"插件：用来检测`urdf文件`是否有语法错误，机器人的长度之类的，不管`关节限制`，因此`关节限制不影响urdf_visualizer插件的使用（也就是不影响对urdf文件的检测以及对模型是否异常的检测）；`
    注意：分辨`rubber`：
                   （1）有的名称里有rubber关键词；
                             `或` 
                  （2）有的有<dynamics damping="2.5" friction="0.7"/>标签；
    
-5.    用urdf_visualize插件导出查看是否有问题，校准好零点（让“机器人”竖直，向上延伸）；
   我们需要借用`urdf_visualizer`来调参使机器人`竖直`
   确定了`关节值`之后，最后进入`urdf文件`调对应的参数（rpy的值），使得在`urdf_visualizer`插件显示界面中，
   当`所有`关节值均为`0`时，机器人在`零点`。
    

-6.    使用moveit_setup_assitant中的pose得到各关节的限制。尽可能使关节范围大（通过滑动每个joint的滑动条，观察何时碰撞，得到lower和upper）,
   并更改属性为`准确的关节限制`（前面将joint`关节限制`改为-3.14到3.14，就是为了这一步服务的）；


-   建立tcp_link（作为eef_link）作为几何中心抓取点
    ~~~urdf
-
  <link name="tcp_link">
    <visual>
      <origin xyz="0 0 0" rpy="0 0 0" />
      <geometry>
        <!-- 半径设为 1cm，方便观察 -->
        <sphere radius="0.005" />
      </geometry>
      <material name="green_transparent">
        <color rgba="0 1 0 0.5" />
      </material>
    </visual>
  </link>
  <joint name="tcp_joint" type="fixed">
    <origin xyz="0 0 0.125" rpy="0 0 0" />
    <parent link="link6" />
    <child link="tcp_link" />
  </joint>
    ~~~



-   从sw导出的stl是高精度的，需要减少模型面数  #只做“减面”就可以了；

    将stl转为dae，让urdf中vision使用dae文件

    并降低stl精度