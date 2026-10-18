# 示例素材

本目录中的背景、机器人占位立绘、标题图和按钮全部由 scripts/generate_demo_assets.py 生成。没有附带商业游戏原图、音频、剧情或字体文件。

- story.json：18 页示例剧情，包括两条普通路线和真结局。
- audio.json：空的音乐、人声注册表。
- ui.json：标题背景、按钮图集与切片坐标。
- images/bg/：9 张 240×160 几何背景。
- images/fg/guide.png：64×160 透明机器人。
- images/ui/：两张标题背景和七行两列的按钮图集。

把自己的素材放在 assets 下，通过相对路径注册。PNG/BMP 图片和整数 PCM WAV 由构建脚本转换；只复制文件不会自动启用。测试 fixture 使用本目录内的示例图片，请勿随意删除。

重新生成示例的命令：python scripts/generate_demo_assets.py。该操作同时重置示例 JSON；请先保留自己的创作副本。
