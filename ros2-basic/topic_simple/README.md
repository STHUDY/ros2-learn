## 介绍 
这是Helloworld例程

## 依赖

```
sudo apt install colcon python3-colcon-common-extensions
```

## 编译

编译代码

```
colcon build
```

低内存设备（从下面命令当中选择一个）

```
colcon build --executor sequential
```

```
colcon build --parallel-workers 1 
```

超低内存编译命令

```
MAKEFLAGS="-j1" colcon build --executor sequential
```

## 运行

```
ros2 run node_helloworld node_helloworld
```

