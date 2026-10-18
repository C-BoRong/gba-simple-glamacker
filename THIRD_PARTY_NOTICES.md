# 依赖与素材来源

## 本仓库内容

C 引擎、Python 工具和 JSON 示例来自本项目开发成果。示例中文剧情为项目自有示例，不是商业游戏原文。assets/images/ 中所有 PNG 由 scripts/generate_demo_assets.py 使用 Pillow 基础几何图形和内置字体生成；未复制原游戏图片、按钮、角色或音乐。示例不包含音频文件。

本次未选择许可证；本文件是来源说明，不构成授权声明，也不将第三方代码或素材重新许可。

## 外部依赖（需自行安装）

| 依赖 | 用途 | 官方来源 |
| --- | --- | --- |
| Python | 运行转换和校验脚本 | https://www.python.org/ |
| Pillow | 图片处理与文字栅格化 | https://python-pillow.github.io/ |
| devkitPro / devkitARM / GNU Arm tools | GBA 交叉编译、链接、ROM 工具 | https://devkitpro.org/ |
| libgba | GBA 硬件接口 | https://github.com/devkitPro/libgba |
| Maxmod / mmutil | 音频库和音频资源打包 | https://github.com/devkitPro/maxmod |
| mGBA | 试玩与 SDL 回归验证 | https://mgba.io/ |

这些依赖的源代码、安装包和可执行文件不在发布包内。各依赖适用其自己的许可证，以官方发行版随附条款为准。构建 ROM 会链接运行库；再发布 ROM 时还应查看所使用运行库的条款。

GBA_FONT 指向使用者本机的字体。项目未附带 Windows 字体或任何提取出的字库。构建会将剧情所需字形写入生成资源；如发布游戏，请选择适合你的分发方式的字体授权。
