# Notepad-- 插件开发包最小构建文件

来源：[cxasm/ndd-plugin](https://gitee.com/cxasm/ndd-plugin)，固定提交 `810fab590bbfaf44816570c6e22b632ffd5169e6`。2026-11-13 从该提交的 `ndd108` 目录原样复制以下文件，供 Windows x64 构建本插件使用：

- `include/pluginGl.h`
- `qscint/src/Qsci/*.h`
- `x64/Release/qmyedit_qt5.lib`

未复制开发包中的运行时 DLL、示例插件或其他平台文件。`LICENSE` 是上游仓库同一提交的 GPLv3 许可证文件。QScintilla 头文件还包含 Riverbank Computing 的版权及 GPLv3/商业许可声明，使用时应保留这些文件内声明。完整开发包和更新记录请从上游仓库获取。

关键文件 SHA-256：

| 文件 | SHA-256 |
| --- | --- |
| `include/pluginGl.h` | `2154f5333d74da33c2833191c076c3a3ee3c903bbe41f4c65138b1c966d8d6e9` |
| `qscint/src/Qsci/qsciscintilla.h` | `895fe1dd1ea87fe933c4629dc18dd64d70d4f29acbb4a9d811f13611282b39eb` |
| `x64/Release/qmyedit_qt5.lib` | `588a801c6a01ec29557176af76360a4a10b89988448b6c6076915c3fec07bc1e` |
