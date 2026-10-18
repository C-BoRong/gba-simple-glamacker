# 贡献说明

当前尚未指定项目许可证。提交代码前请先与维护者确认贡献的授权方式。

修改引擎时请保持示例剧情和固定 fixture 能独立构建。无需提交 generated/、build/、dist/、存档、本机配置、第三方二进制或受限素材。新增依赖或素材时，补充来源与适用许可说明。

建议验证顺序：

1. 安装 requirements.txt，运行 python scripts/test_schema.py。
2. 运行 build.bat，检查生成的资源容量报告。
3. 运行 test.bat；模拟器 SDL 由 GBA_EMULATOR 指定。
4. 试玩修改涉及的路线、存档或画面。

提交说明应描述触发条件、修改后的行为和实际完成的测试。报告问题时附 Python/devkitPro/mGBA 版本、可复现步骤与相关错误，避免附带个人存档或无授权素材。
