# GBA Galgame Framework

一个轻量的中文 GBA 视觉小说模板。使用 JSON 编写剧情、登记背景和人物、设置分支与声音，再编译为可在 GBA 模拟器或掌机上运行的 ROM。

发布目录仅包含引擎、素材转换工具、原创示例、测试和文档。示例为《潮声里的约定》，使用程序绘制的港口背景和机器人占位角色，不依赖商业游戏文件。

你可以用它制作原创短篇，也可以让 AI 协助把已有 Galgame 适配为 GBA 游戏。**它不是能直接打开任意游戏 EXE 的通用转换器**：不同作品的脚本格式、立绘分层、分支逻辑需要分别解析，原版视频和复杂演出也需要适配。

## 从哪里开始

| 你的目标 | 入口 |
| --- | --- |
| 先确认工具能跑 | 按下方“快速开始”编译随包示例 |
| 不熟悉代码，想让 AI 帮忙 | 阅读“让 AI 引导移植”，将工程目录交给 AI |
| 手动写自己的故事 | “开始制作”及 [开发指南](开发指南.md) |
| 使用单独下载的素材包 | “加载独立素材包” |
| 修改引擎或提交修复 | [CONTRIBUTING.md](CONTRIBUTING.md) |

## 功能

- 240×160 画面、中文对白、逐字显示、快进、自动播放和历史记录。
- 每页一张背景、一张透明立绘，支持位置、翻转和淡入/滑动转场。
- 每页最多两个选项，支持变量、条件、赋值、增减和自动控制节点。
- 两个普通结局、一个真结局；可配置真结局解锁条件。
- 继续游戏、三个手动存档槽、已读状态、设置及双份 SRAM 存档校验。
- PCM WAV 音乐与人声；构建时转换图片、字体、音频并检查资源容量。

本目录不包含可视化编辑器。日常创作修改 assets/story.json、assets/audio.json 和引用素材；更换标题菜单时修改 assets/ui.json。

## 快速开始（Windows）

需要 Python 3.10+、Pillow，以及安装 gba-dev 工具链的 devkitPro（devkitARM、libgba、Maxmod、mmutil）。试玩和模拟器测试需另行安装 mGBA。本项目不附带工具链、模拟器、BIOS 或字体文件。

1. 安装 [Python 3.10+](https://www.python.org/downloads/)，确认安装了 pip，在此目录打开 PowerShell，运行 python -m pip install -r requirements.txt。
2. 安装 [devkitPro](https://devkitpro.org/wiki/Getting_Started)，确认工具链及其 MSYS2 可用。
3. 在本目录的 PowerShell 中运行：

~~~powershell
$env:DEVKITPRO = 'C:/devkitPro'
$env:GBA_PYTHON = (Get-Command python.exe).Source
# 默认读取 Windows 宋体；也可指定本机具有中文字符的 TrueType 字体。
$env:GBA_FONT = 'C:/Windows/Fonts/simsun.ttc'
./build.ps1
~~~

构建成功后，ROM 位于 dist/galgame-framework.gba，容量报告位于 generated/resource-report.md。仅需构建时可双击 build.bat。建议使用不含空格的工程路径；当前直接 Makefile 构建不保证支持空格路径。

上述环境变量仅对当前终端及其子进程生效。下次打开终端需重新设置；自定义环境下建议始终从这个终端构建，不直接双击 BAT。DEVKITPRO 可以省略，默认 C:/devkitPro；GBA_PYTHON 不设置时使用 PATH 中的 python.exe。

~~~powershell
$env:GBA_EMULATOR_GUI = 'D:/tools/mGBA/mGBA.exe'
./run.ps1
~~~

run.ps1 会先构建再启动模拟器。也可手动用 mGBA 打开 ROM。A 推进/确认，B 快进，方向键选择，Start 打开菜单，Select 切换自动；键盘映射由模拟器设置决定。

当前提供并验证的构建入口面向 Windows。尚未将 Linux/macOS 构建作为正式支持目标。

## 让 AI 引导移植

在支持读写本地文件的 AI 编程助手中打开本工程，要求它先读 [AGENTS.md](AGENTS.md)。该文件面向 AI，定义了检查源文件、保留原文/分支、处理身体与表情、构建测试和交付的流程。普通用户不必先看懂 C 或 JSON。

你只需先准备这些信息：

- 原作文件所在目录。若已提取，请保留原文件名、子目录、脚本和图片元数据，不要先把素材全部缩小。
- 需要移植的语言、全篇或指定章节，以及你重视的声音/画面效果。
- Windows 环境是否已安装 Python、devkitPro 和 mGBA。不清楚时让 AI 检查。

可直接复制以下请求，把示例路径改成自己的实际目录：

~~~text
请先阅读当前工程的 AGENTS.md 和 README.md，再引导我移植一部 Galgame。
原作目录：D:/games/my-game-source
目标语言：中文
范围：完整剧情，不删减对白、旁白、选项、分支或结局。
请保留原文件，另建工作副本；先检查脚本、素材分层和工具环境，
给我具体适配清单，再逐步完成导入、完整性核对、构建与试玩验证。
身体与表情要按原坐标及同一尺寸组合；不支持的效果明确列出。
需要我提供的信息请集中说明，不要让我手工逐句转换 JSON。
~~~

AI 应先检查真实文件，再制作本作导入脚本。只支持聊天、不能访问文件的 AI 无法直接完成本地转换。请勿把“已生成 JSON”或“框架测试通过”当作原作全部移植通过，应查看逐段原文核对、路线覆盖和运行验证结果。

## 加载独立素材包

源码包与原作素材包分开管理。源码包已有可运行的几何示例，不下载素材包也能编译。单独提供的 ATRI 素材包是本项目当前移植成果，不是任意游戏都通用的资源包。

1. 将框架复制到自己的工作目录，例如 D:/gba/my-game，先成功编译示例。
2. 备份当前整个 assets/ 目录。素材包配置会整体替换剧情、声音和标题配置，**不会自动合并你已经写好的故事**。
3. 解压素材包，将其中 galgame-framework/ 下的文件复制到你的工程同级目录，按包内说明确认覆盖。不要再嵌套出 galgame-framework/galgame-framework/。
4. 确认 assets/story.json、audio.json、ui.json 已更新，assets/atri/ 存在。保留源码包自带的几何示例图片，固定测试仍会引用。
5. 执行 ./build.ps1，打开新生成的 dist/galgame-framework.gba。不要误打开之前的示例 ROM；旧存档先另行备份。

已转换素材包足以本地构建，不要求相邻存在原始 atri/ 或安装 ffmpeg。它不附带从原作资源重新导入的工具；从原始资源重做转换属于另外的导入流程。

带原作文本/图像/音乐的素材包应按各自权利和用途管理，不随框架源码公开上传。具体完整性和视听适配范围以素材包 README 为准。

## 开始制作

1. 复制本目录作为自己的工程，保留 source、include、scripts、tests 和 Makefile。
2. 编辑 assets/story.json：修改标题、场景、人物、对白和去向。完整小型模板见 docs/examples/story-template.json。
3. 将自己的 PNG/BMP 图片放入 assets，并在 JSON 中登记路径。背景推荐 240×160；立绘应预先缩放为偶数宽，并保证坐标加尺寸不超出屏幕。
4. 在 assets/audio.json 登记整数 PCM WAV，以节点 music/voice 引用；示例默认无声音。
5. 重新构建并试玩每条路线。详细字段及存档行为见 [开发指南](开发指南.md)。

标题背景和按钮已经画好，修改 JSON 的 title 不会重绘图片中文字。菜单的七个按钮顺序固定；ui.json 用于图片切片和布局。

每页的 scene、speaker、text 需要明确填写；next 表示下一页，choices 表示选项。素材 file 路径从 assets/ 开始计算。完整可替换的 JSON 模板位于 [docs/examples/story-template.json](docs/examples/story-template.json)，字段、变量、表情切换和转场示例位于 [开发指南](开发指南.md)。

当前引擎固定为“两普通结局 + 一真结局”。如果原作结局数、选项数或变量数量与下方上限不符，应让 AI 先扩展引擎并补验证；不能为套模板删掉原作路线。默认任意一个普通结局通关即可解锁真线，true_unlock_endings 可指定允许解锁的普通结局 ID；它不是“全部通关”的 AND 条件。

图片会转为 GBA 调色板资源，半透明立绘会二值化。音频构建会转为 15,768 Hz 单声道 16 位 PCM，完整语音通常超出 32 MiB ROM，需事先确定声音适配方案。不要按 PC 原游戏目录大小估计 GBA 成品容量。

## 测试

~~~powershell
# 固定 fixture 的校验回归，不需要编译器或模拟器。
python scripts/test_schema.py
# 校验你当前的 story.json，并生成剧情控制代码；不编译 ROM。
python scripts/prepare_story.py
# 构建并运行完整回归（需要 devkitPro 和 mGBA SDL）。
$env:GBA_EMULATOR = 'D:/tools/mGBA/mgba-sdl.exe'
./test.ps1
# 无模拟器时仍编译测试 ROM。
./test.ps1 -SkipEmulator
~~~

测试使用固定 fixture 和隔离输出目录，不会改写你的正式剧情。固定示例图片同时也是回归依赖，请保留；脚本 scripts/generate_demo_assets.py 可重新生成示例图片，**也会重置示例 story.json、audio.json、ui.json**，请勿直接用于已有创作。

prepare_story.py 主要检查剧情结构；图片尺寸、文字排版、音频和容量在完整构建中检查。test.ps1 验证框架行为，不会逐句核对你导入的原作。移植完成还需本作的文本与分支对照、关键立绘抽查、各结局及存读档试玩。

## 常见问题

| 现象 | 处理方式 |
| --- | --- |
| python 版本过旧、找不到 PIL | 使用 python --version 确认 >=3.10；用同一个解释器执行 python -m pip install -r requirements.txt，并设置 GBA_PYTHON |
| 找不到 bash、编译器、mmutil | 检查 devkitPro 的 gba-dev 安装是否完整，以及 DEVKITPRO 是否为安装根目录 |
| PowerShell 禁止执行脚本 | 在已设置环境的终端中运行 build.bat；它用单次进程策略启动，不需要修改系统全局策略 |
| 字体不存在或中文缺字 | 设置 GBA_FONT 为本机包含所需汉字的 TrueType/TTC 字体；字体不随包分发 |
| 正文超过三行 | 分为多个节点并接好 next，保留全部文字；不要直接截断 |
| 立绘眼睛、嘴错位 | 核查部件原坐标、姿势和尺寸目录，身体与全部表情统一尺寸后合成，再缩放 |
| Missing asset 或 unknown target | 检查 assets 内相对路径、大小写、素材 ID 和跳转 ID；只放入文件不会自动登记 |
| Unreachable nodes | 检查是否有草稿未接入 start/true_start；不要为了过校验盲目删除原作节点 |
| 构建失败，输出文件被占用 | 关闭正在打开同一 ROM 的模拟器后重试，保留自己的 .sav；不要删除存档来解决编译问题 |
| run.ps1 找不到模拟器 | 设置 GBA_EMULATOR_GUI 指向图形版 mGBA.exe；GBA_EMULATOR 是另一个变量，用于 SDL 测试程序 |
| 标题还是示例文字 | 替换 ui.json 引用的标题图片；JSON title 不会修改图片里的字 |
| 旧存档位置重置 | 改节点 ID、顺序或变量结构可能使旧位置与已读记录失效；先备份存档，并用独立存档测试首次启动 |
| ROM 或 RAM 超限 | 查看 generated/resource-report.md，让 AI 检查重复素材、音频时长和运行内存；不要直接删剧情 |

环境问题请附最先出现的错误、Python 版本、devkitPro 位置与执行的命令。更详细的字段解释见开发指南，不必从头重装或重新导入整个工程。

## 目录

| 路径 | 内容 |
| --- | --- |
| source/、include/ | C 运行时：剧情、画面、动画、音频、存档 |
| AGENTS.md | 给 AI 的移植工作流程和正确性要求 |
| assets/ | 示例 JSON、原创几何图片及自己的素材 |
| scripts/ | 校验、转换、构建辅助与回归测试 |
| tests/fixtures/ | 固定测试剧情 |
| docs/ | 最小故事模板、依赖及发布说明 |
| generated/、build/、dist/ | 构建生成，默认忽略 |

上限为 16,384 节点、16 个变量、每页两个选项、三行正文、32 MiB ROM。场景和立绘使用 16 位引用；实际可用数量受 ROM 与运行内存限制。静态 RAM 容量报告不包含全部运行时堆栈。

## 输出与分享

本地试玩成品是 dist/galgame-framework.gba；.elf 用于调试，.sav 是个人进度，generated/ 与 build/ 可重新生成。源码分享不应夹带本机配置、存档、字体、工具链或原作商业素材。发布自己的游戏前确认图片、音乐、剧本、字体与链接库的适用条款。

整理版示例曾独立构建为 709,328 字节 ROM，并通过21项主机校验、754项剧情/存档/菜单检查、12项音频检查；这不是对后续移植内容的验证，也未替代实体 GBA 测试。

## 发布状态与许可证

这是用于公开源码的整理版本。**按作者当前选择，尚未指定开源许可证，也未添加 LICENSE。** 公开可见不等于自动授予复制、修改、商用或再分发许可；如希望以标准开源项目发布，后续需由作者选定许可证。

依赖及示例来源见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)，贡献说明见 [CONTRIBUTING.md](CONTRIBUTING.md)。不包含原商业游戏的素材、剧情、音频、转换结果或 ROM。
