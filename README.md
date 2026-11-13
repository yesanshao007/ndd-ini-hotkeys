# Notepad-- INI Section 快捷键移动插件

为 Windows 版 Notepad-- 添加两个命令，用快捷键交换相邻的 INI Section。光标位于 Section 标题或段内任意一行即可操作；移动前折叠的 Section 在移动后保持折叠。

这是第三方插件，**目前仅在 Windows x64 的 Notepad-- 3.9.2 上完成实机验收**。v1.3 已验证连续上移折叠的 Section 时，其他 Section 不会意外展开。其他版本、平台和特殊 INI 方言尚未验证。

## 下载与安装

1. 从 [Releases](https://github.com/yesanshao007/ndd-ini-hotkeys/releases/latest) 下载 `NddIniHotkeys-v1.3-windows-x64.zip`，解压取得 `ndd_ini_hotkeys.dll`。下载后可按发布页列出的 SHA-256 校验文件。
2. 完全退出 Notepad--，找到 `Notepad--.exe` 所在目录。如果 `plugin` 目录中已有同名 DLL，先备份。
3. 将 `ndd_ini_hotkeys.dll` 复制到 `Notepad--.exe` 同级的 `plugin` 目录，重新启动 Notepad--。在 **插件 → INI Section 移动 1.3** 中确认出现“当前 Section 上移/下移”。
4. 先在 INI 文件副本上试用快捷键，确认移动结果和折叠状态符合预期，再处理正式文件。

如需卸载，退出 Notepad-- 后删除 `plugin\ndd_ini_hotkeys.dll`；如需回退版本，退出后恢复备份的 DLL。若启动时提示缺少 `VCRUNTIME140_1.dll` 等运行库，请安装或修复 Microsoft Visual C++ 2015–2022 x64 Redistributable。

## 使用

| 操作 | 快捷键 |
| --- | --- |
| 当前 Section 上移 | `Ctrl+Alt+↑` |
| 当前 Section 下移 | `Ctrl+Alt+↓` |

插件菜单也提供相同的两个命令。光标不必选中整段，操作不经过剪贴板；每次文本移动可按一次 `Ctrl+Z` 撤销。单行移动仍使用 Notepad-- 自身的功能和快捷键，本插件不处理。

如果快捷键没有响应，先从插件菜单尝试同一命令：菜单能用而快捷键不能用时，检查显卡驱动或其他软件是否占用了组合键。可在源码的两个 `setShortcut(...)` 处改键并自行构建。插件**不支持拖动折叠按钮移动 Section**；鼠标点击该按钮仍执行 Notepad-- 原本的展开或折叠操作。

## Section 范围与限制

- 标题行以可选空格或 Tab 开始，随后为非空的 `[名称]`；右方可有空格或 Tab，以及以 `;` 或 `#` 开始的行尾注释。Section 从标题行延伸到下一标题行之前；其中的 Key/Value、注释和空行一起移动。标题**上方**的注释按此规则属于前一个 Section。
- 文件第一个 Section 之前的内容不移动。第一个 Section 无法上移，最后一个无法下移。只读文档、包含 NUL 字节的文档和超过 64 MiB 的文档不会被修改。
- 移动保留原文的 UTF-8 BOM、换行形式（CRLF/LF/CR）及文件末尾是否有换行。插件按移动前各 Section 的折叠状态恢复移动后的折叠状态；折叠状态变化不属于文本撤销记录。
- 特殊 INI 方言，以及把 `[文字]` 当作普通内容的文件，可能与标题识别规则冲突。请先在副本中验证。

## 从源码构建

### GitHub Actions（Mac 也可触发）

登录 GitHub，在仓库的 **Actions → Build Notepad-- INI hotkeys DLL → Run workflow** 手动运行。工作流在 `windows-2022` x64 上安装 Qt 5.15.2 MSVC2019 64-bit，先运行移动逻辑测试，再编译 DLL。成功后在该运行页面的 **Artifacts** 下载 `ndd_ini_hotkeys-windows-x64`。Actions 产物只保留 7 天；日常安装请使用上面的 Release。

Mac 只需浏览器和可运行 Actions 的 GitHub 账号，不需要本地安装 Windows、Visual Studio 或 Qt。Mac 本地不能直接用这套 MSVC/qmake 配置生成 Windows DLL。

### Windows 本地构建

1. 安装 Visual Studio 2019 或 2022 的 C++ 桌面开发组件、Qt 5.15.2 的 **MSVC2019 64-bit** 组件和 Qt Creator。
2. 克隆本仓库，在 Qt Creator 中打开 `ndd_ini_hotkeys.pro`，选择对应的 64-bit Kit 和 **Release** 模式。
3. 在构建环境中设置 `NDD_SDK_ROOT` 为 `third_party/ndd108` 的绝对目录路径，例如 `C:\dev\ndd-ini-hotkeys\third_party\ndd108`。
4. 编译得到 `build\ndd_ini_hotkeys.dll`，再按上方安装步骤复制。仅复制本插件 DLL；不要用开发包中的旧版 Qt 或 qmyedit DLL 替换 Notepad-- 3.9.2 自带文件。

构建配置见 [`ndd_ini_hotkeys.pro`](ndd_ini_hotkeys.pro) 和 [Windows 工作流](.github/workflows/build-windows.yml)。仓库内的最小开发包来自 [Notepad-- 官方插件项目](https://gitee.com/cxasm/ndd-plugin)的固定提交；来源、文件清单及校验值见 [`third_party/ndd108/README.md`](third_party/ndd108/README.md)。

## 验证与反馈

建议在测试 INI 中折叠全部 Section，从最后一个 Section 开始连续按 `Ctrl+Alt+↑` 移到最前面，逐步检查所有 Section 仍折叠；再用 `Ctrl+Z` 检查文本撤销。此场景已由使用者在 Windows / Notepad-- 3.9.2 上验收；不能据此推断所有环境均正常。

遇到问题时，请在 [Issues](https://github.com/yesanshao007/ndd-ini-hotkeys/issues) 中写明 Notepad-- 版本、Windows 架构、操作步骤、期望与实际结果，以及是否通过插件菜单复现。分享 INI 样例前请移除账号、密钥和私人配置。

## 许可证与来源

本项目自有源码按 [GPL-3.0-only](LICENSE) 发布。`third_party/ndd108` 中的材料保留其上游许可证及文件内声明；详见[开发包说明](third_party/ndd108/README.md)和[上游许可证](third_party/ndd108/LICENSE)。Release 同时提供本仓库对应版本的源码归档。

Notepad-- 本体与本项目分别发布。本插件的目标版本为 [Notepad-- 3.9.2](https://gitee.com/cxasm/notepad--/releases/tag/v3.9.2)。
