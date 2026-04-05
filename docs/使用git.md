#### 使用 Git

>   常用 Git 指令整理，按场景分类，便于实际操作

------

**配置 Git**

注册用户信息

```
# 设置用户名和邮箱
git config --global user.name "John Doe"
git config --global user.email johndoe@example.com
```

配置代理

```
git config --global http.proxy http://127.0.0.1:7890
git config --global https.proxy http://127.0.0.1:7890
```

查看所有配置

```
git config --list 
```

配置 SSH Key（最后在目录 ~/.ssh 中）

[参考链接](https://blog.csdn.net/weixin_42310154/article/details/118340458)

------

**创建一个新项目**

1.  在 GitHub 新建仓库
2.  初始化本地仓库并添加 README

```
echo "# " > README.md
git init 
git add README.md   # 或 git add .
git commit -m "first commit"
git branch -M main
```

1.  添加远程仓库并推送

```
git remote add origin https://github.com/xxxx/xxxx.git
# 或使用 SSH
git remote add origin git@github.com:${user}/${repository}.git
git remote set-url origin git@github.com:${user}/${repository}.git  # 更新权限
git push -u origin main
```

------

**其他 Git 命令**

```
git checkout -b -h           # 创建新分支并跟踪远程分支
git restore                   # 恢复文件
git log                       # 查看提交日志
git branch                     # 查看本地分支
git checkout <commit_hash>     # 切换到指定提交
git switch -c <new_branch>     # 创建分支并保留当前状态
git switch <branch_name>       # 切换分支
git pull origin <branch_name>  # 拉取远程分支
git reset --hard <commit_hash> # 强制回退
git push -f origin twice        # 本地与远程不一致时强推
git push origin <local>:<remote> # 指定分支推送
git branch -d <local_branch>    # 删除本地分支
git push origin --delete <remote_branch> # 删除远程分支
```

------

**多分支开发场景**

```
# 创建本地分支
git checkout -b ros-control
git checkout -b fake-control

# 推送到远程
git push origin ros-control
git push origin fake-control

# 查看分支状态
git branch -r        # 远程分支
git branch -vv       # 本地分支与远程对应关系

# 建立跟踪关系
git checkout ros-control
git branch --set-upstream-to=origin/ros-control

git checkout fake-control
git branch --set-upstream-to=origin/fake-control

# 分支操作
git merge main       # 合并分支
git stash            # 暂存当前改动
```

------

**协作开发场景**

```
# 成为协作者后的操作流程
mkdir project
cd project
git init
git checkout -b main
git remote add origin <仓库地址>
git remote set-url origin <仓库地址>
git remote -v

# 拉取远程仓库并创建新功能分支
git pull origin master
git branch -vv
git checkout -b feature/wrj
git push -u origin feature/wrj
git branch --set-upstream-to=origin/feature/wrj feature/wrj  # 建立跟踪关系
```

---

**多远程推送**

>   一个仓库可以同时设置多个远程
>
>   origin → 组织仓库
>   personal → 个人仓库

~~~bash
git remote add personal git@github.com:xxx/xxx.git
git remote add origin  git@github.com:xxx/xxx.git
git remote set-url personal git@github.com:xxx/xxx.git
git remote set-url origin git@github.com:xxx/xxx.git

git config remote.pushDefault origin
git config remote.origin.push refs/heads/*
git config remote.personal.push refs/heads/*

git remote -vv

git push origin
git push personal
~~~



**报错 / 注意事项**

1.  **Git 指令无法使用，报错 "another git process running"**

>   原因：Git 检测到进程未正常结束，锁定仓库防止数据损坏

解决方法：

```
ls .git | grep index
rm -f .git/index.lock
```

2.   **不要提交缓存文件（如 install/ log/ build/）**

>   提交后发现不该提交的缓存文件

解决方法：

```
# 添加 .gitignore
git rm -r --cached
git commit -m "Remove install/log/build from repo and update .gitignore"
git push
```

3.   **检查 SSH 是否可用**

```
ssh -T git@github.com
```

------

**参考 / 链接**

-   [Git SSH 配置教程](https://blog.csdn.net/weixin_42310154/article/details/118340458)