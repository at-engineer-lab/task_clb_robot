# 鱼香 IKFast项目

>   参考: 小鱼 - IKFast运动学插件配置
>
>   https://blog.csdn.net/qq_27865227/article/details/139245523?spm=1001.2014.3001.5502
>
>   https://fishros.org.cn/forum/topic/680/moveit-ikfast%E8%BF%90%E5%8A%A8%E5%AD%A6%E6%8F%92%E4%BB%B6%E9%85%8D%E7%BD%AE-%E6%9C%80%E8%AF%A6%E7%BB%86-%E6%B2%A1%E6%9C%89%E4%B9%8B%E4%B8%80

[TOC]

### 使用 Docker

>   安装 Docker 并拉取项目代码。

```
# 安装 Docker
sudo apt install docker.io

# 克隆项目代码
git clone https://github.com/Elite-Robots/ROS elite_robot
```

------

### 配置代理（网络代理）

>   当 Docker 镜像无法下载时，需要检查并配置 HTTP/HTTPS 代理。

```
sudo mkdir -p /etc/systemd/system/docker.service.d
sudo nano /etc/systemd/system/docker.service.d/proxy.conf
```

内容示例：

```
[Service]
Environment="HTTP_PROXY=http://127.0.0.1:7890"
Environment="HTTPS_PROXY=http://127.0.0.1:7890"
Environment="NO_PROXY=localhost,127.0.0.1"
```

------

### 添加 Allow LAN

>   在 Clash 软件中修改 `.yaml` 文件的 `allow lan`，允许局域网设备访问（包括 Docker 容器联网）。

-   打开相应开关，设置为 `true`

![Clash allow lan](https://cdn.jsdelivr.net/gh/dengyu32/note_images/images/20251211155522949.png)

------

### 启动项目

```
xhost + 
sudo docker run -it \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    --device /dev/dri \
    --device /dev/snd \
    -e DISPLAY=unix$DISPLAY \
    -v `pwd`:`pwd` \
    -w `pwd` \
    --network=host \
    fishros2/openrave
```

------

### 删除清华源（系统源）

#### 清理 apt（系统包管理器）中的清华源

```
# 查看原配置文件
ls /etc/apt/sources.list /etc/apt/sources.list.d/

# 查找 tsinghua
grep -R "tsinghua" /etc/apt/sources.list /etc/apt/sources.list.d/

# 删除相关文件
sudo rm -f /etc/apt/sources.list.d/ros-fish.list

# 清理缓存并更新
sudo apt-get clean
sudo apt-get update
```

#### 清理 pip（Python 包管理器）中的清华源

```
# 查看 pip 配置
pip3 config list

# 更换为中科大源
pip3 config set global.index-url https://mirrors.ustc.edu.cn/pypi/web/simple

# 配置全局选项
mkdir -p ~/.config/pip
nano ~/.config/pip/pip.conf
```

内容示例：

```
[global]
index-url = https://mirrors.ustc.edu.cn/pypi/web/simple
trusted-host = mirrors.ustc.edu.cn
```

------

### 添加 apt 源（ROS 源）

>   一键安装 ROS1 noetic 并添加中科大源。

```
wget http://fishros.com/install -O fishros && .fishros
```

------

### 添加 rosdep

>   手动创建 rosdep 源列表文件，并使用中科大镜像源。

```
sudo mkdir -p /etc/ros/rosdep/sources.list.d
sudo tee /etc/ros/rosdep/sources.list.d/20-default.list > /dev/null <<EOF
yaml https://mirrors.ustc.edu.cn/rosdistro/rosdep/base.yaml
yaml https://mirrors.ustc.edu.cn/rosdistro/rosdep/python.yaml
yaml https://mirrors.ustc.edu.cn/rosdistro/rosdep/ruby.yaml
gbpdistro https://mirrors.ustc.edu.cn/rosdistro/releases/fuerte.yaml fuerte
EOF

# 更新 rosdep（如需，切换手机热点）
sudo rosdep update
```

------

### 下载缺失依赖

```
# 安装 pip3
sudo apt install python3-pip

# 使用 pip 下载依赖
sudo pip3 install --index-url https://pypi.org/simple elirobots transforms3d pytest rosdepc

# 使用 apt-get 下载 ROS 依赖
sudo apt-get update
sudo apt-get install -y ros-noetic-pcl-conversions
sudo apt-get install -y ros-noetic-joint-trajectory-controller
sudo apt-get install -y ros-noetic-moveit-visual-tools
sudo apt-get install -y ros-noetic-rviz-visual-tools
sudo apt-get install -y ros-noetic-pcl-ros
```

------

### 编译项目

```
cd elite_robot
catkin_make
source devel/setup.bash
```

------

### 生成机械臂 DAE 描述文件

```
# URDF 转 DAE
rosrun collada_urdf urdf_to_collada ec66_description.urdf ec66_description.dae

# 可手动注册包路径
export ROS_PACKAGE_PATH=$ROS_PACKAGE_PATH:/home/wrj/code/MoveitProject/ws_my_robot_clb/src

# 四舍五入浮点数（小数点后 4 位）
rosrun moveit_kinematics round_collada_numbers.py ec66_description.dae ec66_description.dae 5

# 可视化
openrave ec66_description.dae

# 查看 link 和 joint
openrave-robot.py ec66_description.dae --info links
```

------

### 配置 IKFast

```
# 生成 ikfast.cpp
python `openrave-config --python-dir`/openravepy/_openravepy_/ikfast.py \
    --robot=ec66_description.dae \
    --iktype=transform6d \
    --baselink=1 \
    --eelink=8 \
    --savefile=$(pwd)/ikfastec66.cpp

# 编译测试
cp /usr/local/lib/python2.7/dist-packages/openravepy/_openravepy_/ikfast.h .
g++ ikfastec66.cpp -o ikfast-ec66 -llapack -std=c++11
./ikfast-ec66
```

------

### 生成 ikfast_plugin 功能包

```
rosrun moveit_kinematics create_ikfast_moveit_plugin.py \
    ec66 manipulator elite_moveit_ikfast_plugin_ec66 \
    "world" "flan" ../elite_description/urdf/ikfastec66.cpp
```

>   后续小鱼项目均为 ROS1，host 不适合 ROS2 Humble 环境，自定义 IKFast 在后续步骤实现。

------

### 自定义 IKFast

>   流程思路：

-   host 提供 URDF 文件
-   在 Docker 的 ROS1 noetic 环境生成 DAE 和对应 ikfast.cpp
-   返回 host 使用 ROS2 创建 ikfast_moveit_plugin 功能包
-   修改 kinematics.yaml 注册逆解算器

```
# 启动 Docker
cd 目标工作区目录
xhost + && sudo docker run -it \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    --device /dev/dri \
    --device /dev/snd \
    -e DISPLAY=unix$DISPLAY \
    -v `pwd`:`pwd` \
    -w `pwd` \
    --network=host fishros2/openrave 

# 转 DAE
cd 目标urdf目录
rosrun collada_urdf urdf_to_collada 目标urdf 目标dae
rosrun moveit_kinematics round_collada_numbers.py 目标dae 目标dae 5

# 查看 link 信息
openrave-robot.py 目标dae --info links

# 生成 ikfast.cpp
python `openrave-config --python-dir`/openravepy/_openravepy_/ikfast.py \
    --robot=目标dae \
    --iktype=transform6d \
    --baselink=0 \
    --eelink=6 \
    --savefile=$(pwd)/目标ikfast.cpp

# 编译测试
cp /usr/local/lib/python2.7/dist-packages/openravepy/_openravepy_/ikfast.h .
g++ 目标ikfast.cpp -o 目标可执行ikfast -llapack -std=c++11
./目标可执行ikfast
exit

# 创建 IKFast MoveIt 功能包
ros2 run moveit_kinematics create_ikfast_moveit_plugin.py \
  目标机器人名称 \
  目标规划组名称 \
  目标moveit_ikfast_plugin名称 \
  运动链的起始link \
  运动链的结束link \
  ../目标description功能包/urdf/目标ikfast.cpp
```

示例（带绝对路径）：

```
ros2 run moveit_kinematics create_ikfast_moveit_plugin.py \
  clb clb_arm clb_moveit_ikfast_plugin base_link link5 \
  /home/wrj/code/MoveitProject/ws_my_robot_clb/src/clb_robot_description/urdf/ik.cpp
```

------

### 修改 MoveIt 配置并测试

>   修改插件名称与 kinematics.yaml 文件，编译运行 demo.launch.py。

-   XML 插件名称示例：`clb_clb_arm/IKFastKinematicsPlugin`

![MoveIt 插件](https://cdn.jsdelivr.net/gh/dengyu32/note_images/images/20251211155529569.png)

![kinematics.yaml](https://cdn.jsdelivr.net/gh/dengyu32/note_images/images/20251211155531477.png)

#### 常见报错

```
[move_group-3] /opt/ros/humble/lib/moveit_ros_move_group/move_group: symbol lookup error: /home/wrj/code/MoveitProject/ws_my_robot_clb/install/clb_moveit_ikfast_plugin/lib/libclb_clb_arm_moveit_ikfast_plugin.so: undefined symbol: _ZN11clb_clb_arm17GetFreeParametersEv
```

>   解决方法：在 `solver.cpp` 中添加

```
// 在 moveit_ikfast_plugin 功能包生成的 solver.cpp 中四百行左右 加上
IKFAST_API int* GetFreeParameters() { return NULL; }
```

重新编译后，即可成功加载 IKFast 插件。
