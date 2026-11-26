# 医院诊疗信息管理系统

本项目是 Qt 应用程序开发实验三的程序实现，使用 Qt Widgets、Qt SQL、
`QStackedWidget` 和 Model/View 架构完成。

## 功能

- 基于 `User` 表的登录检测，演示账号为 `admin / admin123`
- 登录、退出、工作台及功能页面切换
- 患者信息列表、关键字查询、新增、修改和确认删除
- 性别列自定义代理编辑
- `UUID` 与创建时间自动生成
- 身份证号、手机号、年龄、身高、体重和必填项校验
- 医生、科室及操作记录查询
- SQLite 外键、唯一约束和自动建表

## 在 Qt Creator 中打开

1. 使用 Qt Creator 打开 `HospitalManagementSystem.pro`。
2. 选择 Qt 6.x MinGW 64-bit 套件。
3. 执行“构建”与“运行”。
4. 程序首次运行时会在构建目录生成 `hospital.sqlite`。

## 自动测试

使用 Qt Creator 打开 `tests/HospitalTests.pro`，构建并运行，或在构建目录执行
`HospitalTests.exe -v2`。

测试覆盖登录成功与失败、必填字段、重复身份证号、查询无结果、增删改后的
数据库与模型一致性，以及数据库路径错误。

## 自动截图

打开 `HospitalCapture.pro` 后运行，程序会在构建目录的 `screenshots` 文件夹生成
实验报告所需的界面截图。
