#include "section_move.hpp"

#include <pluginGl.h>
#include <qsciscintilla.h>

#include <QAction>
#include <QKeySequence>
#include <QMenu>
#include <QStatusBar>
#include <QWidget>

#include <algorithm>
#include <functional>
#include <limits>
#include <string>
#include <vector>

namespace {

void show_status(QWidget* host, const QString& message) {
    if (auto* bar = host->findChild<QStatusBar*>()) bar->showMessage(message, 4000);
}

struct FoldState {
    std::size_t new_offset;
    bool expanded;
};

std::vector<FoldState> capture_folds(QsciScintilla* editor,
                                     const std::string& text,
                                     const ini_move::Plan& plan) {
    std::vector<FoldState> folds;
    const auto headers = ini_move::section_starts(text);
    folds.reserve(headers.size());
    editor->SendScintilla(QsciScintillaBase::SCI_COLOURISE, 0UL, -1L);
    for (const auto old_offset : headers) {
        const long line = editor->SendScintilla(QsciScintillaBase::SCI_LINEFROMPOSITION,
                                                static_cast<unsigned long>(old_offset));
        if (!(editor->SendScintilla(QsciScintillaBase::SCI_GETFOLDLEVEL,
                                    static_cast<unsigned long>(line)) &
              QsciScintillaBase::SC_FOLDLEVELHEADERFLAG)) continue;

        const long last_child = editor->SendScintilla(QsciScintillaBase::SCI_GETLASTCHILD,
                                                      static_cast<unsigned long>(line), -1L);
        // Visibility is the state the user sees. After an earlier text edit,
        // Scintilla's expanded flag can disagree with visible child lines.
        const bool expanded = last_child > line
            ? editor->SendScintilla(QsciScintillaBase::SCI_GETLINEVISIBLE,
                                    static_cast<unsigned long>(line + 1)) != 0
            : editor->SendScintilla(QsciScintillaBase::SCI_GETFOLDEXPANDED,
                                    static_cast<unsigned long>(line)) != 0;

        folds.push_back({ini_move::remap_header(plan, old_offset), expanded});
    }
    std::sort(folds.begin(), folds.end(),
              [](const FoldState& a, const FoldState& b) {
                  return a.new_offset < b.new_offset;
              });
    return folds;
}

void restore_folds(QsciScintilla* editor, const std::vector<FoldState>& folds) {
    editor->SendScintilla(QsciScintillaBase::SCI_COLOURISE, 0UL, -1L);
    for (const auto& fold : folds) {
        const long line = editor->SendScintilla(QsciScintillaBase::SCI_LINEFROMPOSITION,
                                                static_cast<unsigned long>(fold.new_offset));
        if (!(editor->SendScintilla(QsciScintillaBase::SCI_GETFOLDLEVEL,
                                    static_cast<unsigned long>(line)) &
              QsciScintillaBase::SC_FOLDLEVELHEADERFLAG)) continue;
        const bool flag = editor->SendScintilla(QsciScintillaBase::SCI_GETFOLDEXPANDED,
                                                static_cast<unsigned long>(line)) != 0;
        const long last_child = editor->SendScintilla(QsciScintillaBase::SCI_GETLASTCHILD,
                                                      static_cast<unsigned long>(line), -1L);
        const bool visible = last_child > line
            ? editor->SendScintilla(QsciScintillaBase::SCI_GETLINEVISIBLE,
                                    static_cast<unsigned long>(line + 1)) != 0
            : flag;
        // FoldLine updates both the flag and child visibility. The text edit
        // can disturb a later Section even when it lies outside the swap.
        if (flag != fold.expanded || visible != fold.expanded)
            editor->SendScintilla(QsciScintillaBase::SCI_FOLDLINE,
                                  static_cast<unsigned long>(line),
                                  static_cast<long>(fold.expanded
                                      ? QsciScintillaBase::SC_FOLDACTION_EXPAND
                                      : QsciScintillaBase::SC_FOLDACTION_CONTRACT));
    }
}

void move_current_section(QWidget* host,
                          const std::function<QsciScintilla*(QWidget*)>& current_editor,
                          int direction) {
    auto* editor = current_editor(host);
    if (!editor) {
        show_status(host, QStringLiteral("没有可编辑文档"));
        return;
    }
    if (editor->SendScintilla(QsciScintillaBase::SCI_GETREADONLY) != 0) {
        show_status(host, QStringLiteral("当前文档是只读的"));
        return;
    }
    const long length = editor->SendScintilla(QsciScintillaBase::SCI_GETLENGTH);
    constexpr long max_document = 64L * 1024L * 1024L;
    if (length < 0 || length > max_document) {
        show_status(host, QStringLiteral("当前文档超过插件的 64 MiB 安全上限"));
        return;
    }
    std::vector<char> bytes(static_cast<std::size_t>(length) + 1);
    editor->SendScintilla(QsciScintillaBase::SCI_GETTEXT,
                          static_cast<unsigned long>(bytes.size()),
                          static_cast<void*>(bytes.data()));
    const std::string text(bytes.data(), static_cast<std::size_t>(length));
    const long current_pos = editor->SendScintilla(QsciScintillaBase::SCI_GETCURRENTPOS);
    if (current_pos < 0) return;

    ini_move::Plan plan;
    std::string error;
    if (!ini_move::plan_move(text, static_cast<std::size_t>(current_pos),
                             direction, plan, error)) {
        show_status(host, QString::fromStdString(error));
        return;
    }
    if (plan.first > static_cast<std::size_t>(std::numeric_limits<long>::max()) ||
        plan.last > static_cast<std::size_t>(std::numeric_limits<long>::max())) return;

    const auto folds = capture_folds(editor, text, plan);
    editor->SendScintilla(QsciScintillaBase::SCI_BEGINUNDOACTION);
    editor->SendScintilla(QsciScintillaBase::SCI_SETSEL,
                          static_cast<unsigned long>(plan.first),
                          static_cast<long>(plan.last));
    editor->SendScintilla(QsciScintillaBase::SCI_REPLACESEL,
                          plan.replacement.c_str());
    editor->SendScintilla(QsciScintillaBase::SCI_ENDUNDOACTION);
    editor->SendScintilla(QsciScintillaBase::SCI_GOTOPOS,
                          static_cast<unsigned long>(plan.caret));
    restore_folds(editor, folds);
    show_status(host, direction < 0 ? QStringLiteral("Section 已上移")
                                    : QStringLiteral("Section 已下移"));
}

} // namespace

extern "C" NDD_EXPORT bool NDD_PROC_IDENTIFY(NDD_PROC_DATA* data) {
    if (!data) return false;
    data->m_strPlugName = QStringLiteral("INI Section 移动 1.3");
    data->m_strComment = QStringLiteral("Ctrl+Alt+上/下移动光标所在的完整 Section");
    data->m_version = QStringLiteral("1.3");
    data->m_auther = QStringLiteral("Codex");
    data->m_menuType = 1;
    return true;
}

extern "C" NDD_EXPORT int NDD_PROC_MAIN(
    QWidget* host, const QString&,
    std::function<QsciScintilla*(QWidget*)> current_editor,
    std::function<bool(QWidget*, int, void*)>, NDD_PROC_DATA* data) {
    if (!host || !data || !data->m_rootMenu || !current_editor) return -1;

    auto* up = data->m_rootMenu->addAction(QStringLiteral("当前 Section 上移"));
    up->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Up));
    up->setShortcutContext(Qt::WindowShortcut);
    up->setAutoRepeat(false);
    QObject::connect(up, &QAction::triggered, host,
                     [host, current_editor] { move_current_section(host, current_editor, -1); });

    auto* down = data->m_rootMenu->addAction(QStringLiteral("当前 Section 下移"));
    down->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Down));
    down->setShortcutContext(Qt::WindowShortcut);
    down->setAutoRepeat(false);
    QObject::connect(down, &QAction::triggered, host,
                     [host, current_editor] { move_current_section(host, current_editor, 1); });
    return 0;
}
