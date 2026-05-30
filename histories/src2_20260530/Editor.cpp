#include "Editor.h"
#include <QKeyEvent>
#include <QFont>
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QTextOption>
#include <QRegularExpression>
#include <QTimer>
#include <algorithm>

Editor::Editor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_zoomBaseFontSize(10)
    , m_tabWidth(4)
    , m_useTabs(false)
    , m_autoCompletionEnabled(true)
    , m_completionAcceptKey(Qt::Key_Tab)
{
    setLineWrapMode(QPlainTextEdit::NoWrap);
    m_sidebar = new EditorSidebar(this);
    connect(this, &Editor::blockCountChanged, this, &Editor::updateSidebarGeometry);
    connect(this, &Editor::updateRequest, this, &Editor::updateSidebar);
    connect(this, &Editor::cursorPositionChanged, this, &Editor::updateSidebar);
    connect(this, &Editor::cursorPositionChanged, this, &Editor::onCursorPositionChanged);
    updateSidebarGeometry();
    updateTabStopWidth();

    m_suggestionFormat.setForeground(QColor(128, 128, 128));
    m_suggestionFormat.setFontItalic(true);

    connect(document(), &QTextDocument::contentsChanged, this, &Editor::onDocumentContentsChanged);
}

Editor::~Editor() {}

void Editor::setTabWidth(int spaces)
{
    if (spaces >= 1 && spaces <= 8) {
        m_tabWidth = spaces;
        emit tabWidthChanged(m_tabWidth);
    }
}

int Editor::tabWidth() const { return m_tabWidth; }

void Editor::setUseTabs(bool use) { m_useTabs = use; emit tabWidthChanged(m_tabWidth); }

void Editor::updateTabStopWidth()
{
    QFontMetricsF fm(font());
    qreal fourSpacesWidth = fm.horizontalAdvance("    ");
    QTextOption option = document()->defaultTextOption();
    option.setTabStopDistance(fourSpacesWidth);
    document()->setDefaultTextOption(option);
}

void Editor::setFont(const QFont &font)
{
    QPlainTextEdit::setFont(font);
    updateTabStopWidth();
}

void Editor::setAutoCompletionEnabled(bool enabled)
{
    m_autoCompletionEnabled = enabled;
    if (!enabled) clearSuggestion();
}

void Editor::setCompletionAcceptKey(int key) { m_completionAcceptKey = key; }
void Editor::setKeywordList(const QStringList &keywords) { m_keywordList = keywords; }

void Editor::collectIdentifiersFromDocument()
{
    m_identifierSet.clear();
    QTextDocument *doc = document();
    if (!doc) return;

    QRegularExpression identifierRegex("\\b[a-zA-Z_][a-zA-Z0-9_]*\\b");
    QTextBlock block = doc->firstBlock();
    while (block.isValid()) {
        QString text = block.text();
        QRegularExpressionMatchIterator it = identifierRegex.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            m_identifierSet.insert(match.captured(0));
        }
        block = block.next();
    }
}

void Editor::onDocumentContentsChanged()
{
    static QTimer *timer = nullptr;
    if (!timer) {
        timer = new QTimer(this);
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, this, &Editor::collectIdentifiersFromDocument);
    }
    timer->start(150);
    buildSuggestion();
}

void Editor::onCursorPositionChanged()
{
    clearSuggestion();
    buildSuggestion();
}

void Editor::clearSuggestion()
{
    if (!m_pendingCompletion.isEmpty()) {
        m_pendingCompletion.clear();
        viewport()->update();
    }
}

// ------------------------------------------------------------
// 1. 工具函数：计算最长公共前缀 (辅助用于后续的智能化建议)
// ------------------------------------------------------------
QString Editor::longestCommonPrefix(const QStringList &strs) {
    if (strs.isEmpty()) return QString();
    // 找到最小和最大的字符串 (利用排序简化逻辑)[reference:5]
    QString first = strs.first();
    QString last = strs.last();
    int minLen = std::min(first.length(), last.length());
    for (int i = 0; i < minLen; ++i) {
        if (first[i] != last[i]) {
            return first.left(i);
        }
    }
    return first.left(minLen);
}

// ------------------------------------------------------------
// 2. 核心函数：重构的构建建议逻辑 (支持多候选、语义优先级和上下文感知)
// ------------------------------------------------------------
void Editor::buildSuggestion() {
    if (!m_autoCompletionEnabled) {
        clearSuggestion();
        return;
    }
    QTextCursor cursor = textCursor();
    // 获取光标前的完整单词（标识符前缀）
    cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
    QString prefix = cursor.selectedText();
    if (prefix.isEmpty() || (!prefix.at(0).isLetter() && prefix.at(0) != '_')) {
        clearSuggestion();
        return;
    }

    // ================= 上下文感知 =================
    // 获取当前行文本及光标位置
    QString currentLine = cursor.block().text();
    int cursorPosInBlock = cursor.positionInBlock();
    // 1. 检查是否在预处理指令中 (当前行以 # 开头，且光标在行首附近)
    bool isPreprocessor = currentLine.trimmed().startsWith('#');
    // 2. 检查是否在字符串字面量中 (简化逻辑：检测是否在引号内)
    bool isInString = false;
    int quoteCount = 0;
    for (int i = 0; i < cursorPosInBlock; ++i) {
        if (currentLine[i] == '\"') quoteCount++;
    }
    if (quoteCount % 2 == 1) isInString = true;

    // 构建候选词集合
    QSet<QString> allWords;
    // 根据上下文决定是否包含关键字
    if (!isInString) {
        if (isPreprocessor) {
            // 预处理上下文：仅包含预处理关键字
            QStringList preprocessorKeywords = {"include", "define", "ifdef", "ifndef", "endif", "else", "elif", "pragma", "error", "warning"};
            for (const QString &kw : preprocessorKeywords) allWords.insert(kw);
        } else {
            // 普通代码上下文：关键字 + 文档标识符
            for (const QString &kw : m_keywordList) allWords.insert(kw);
            allWords.unite(m_identifierSet);
        }
    } else {
        // 字符串上下文：只提供文档标识符（可选）
        allWords.unite(m_identifierSet);
    }
    // 排除自身（如果前缀本身就是完整的标识符，不补全）
    if (allWords.contains(prefix)) allWords.remove(prefix);

    // 寻找匹配前缀的候选词
    QStringList candidates;
    for (const QString &word : allWords) {
        if (word.startsWith(prefix, Qt::CaseInsensitive))
            candidates.append(word);
    }
    if (candidates.isEmpty()) {
        clearSuggestion();
        return;
    }

    // ================= 智能选择建议项 =================
    // 排序候选词：长度短的优先（实现“if”优先于“int”）, 长度相同时按字母序
    std::sort(candidates.begin(), candidates.end(), [](const QString &a, const QString &b) {
        if (a.length() != b.length()) return a.length() < b.length();
        return a < b;
    });
    // 取最短匹配作为建议对象
    QString bestMatch = candidates.first();
    // 提取建议的后缀部分
    QString suffix = bestMatch.mid(prefix.length());
    if (suffix.isEmpty()) {
        clearSuggestion();
        return;
    }

    // 设置新的建议并刷新显示
    m_pendingCompletion = suffix;
    viewport()->update();
}

void Editor::acceptSuggestion()
{
    if (m_pendingCompletion.isEmpty()) return;
    QTextCursor cursor = textCursor();
    cursor.insertText(m_pendingCompletion);
    m_pendingCompletion.clear();
    viewport()->update();
}

void Editor::paintEvent(QPaintEvent *event)
{
    QPlainTextEdit::paintEvent(event);
    if (m_pendingCompletion.isEmpty()) return;

    QPainter painter(viewport());
    painter.setFont(font());
    painter.setPen(m_suggestionFormat.foreground().color());
    QFont f = font();
    f.setItalic(true);
    painter.setFont(f);

    QTextCursor cursor = textCursor();
    QRect cursorRect = this->cursorRect(cursor);
    int startX = cursorRect.x();
    int startY = cursorRect.y() + painter.fontMetrics().ascent();
    painter.drawText(startX, startY, m_pendingCompletion);
}

// ---------- 键盘事件 ----------
void Editor::keyPressEvent(QKeyEvent *event)
{
    // 1. 最高优先级：采纳建议
    if (m_autoCompletionEnabled && !m_pendingCompletion.isEmpty() && event->key() == m_completionAcceptKey) {
        acceptSuggestion();
        event->accept();
        return;
    }

    // 2. 原始 Tab 键行为
    if (event->key() == Qt::Key_Tab) {
        if (m_useTabs) insertPlainText("\t");
        else insertPlainText(QString(m_tabWidth, ' '));
        return;
    }

    if (event->key() == Qt::Key_Backtab) {
        QTextCursor cursor = textCursor();
        if (!m_useTabs) {
            cursor.movePosition(QTextCursor::StartOfLine);
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, m_tabWidth);
            if (cursor.selectedText() == QString(m_tabWidth, ' ')) {
                cursor.removeSelectedText();
                return;
            }
        } else {
            cursor.movePosition(QTextCursor::StartOfLine);
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
            if (cursor.selectedText() == "\t") {
                cursor.removeSelectedText();
                return;
            }
        }
        return;
    }

    if ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_X) {
        if (!textCursor().hasSelection()) { cutLine(); return; }
    }
    if ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_C) {
        if (!textCursor().hasSelection()) { copyLine(); return; }
    }
    if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)) {
        insertLineBelowKeepCursor(); return;
    }
    if ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_D) {
        duplicateLine(); return;
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        insertIndentedNewLine(); return;
    }
    if (event->key() == Qt::Key_Backspace) {
        QTextCursor cursor = textCursor();
        if (!cursor.hasSelection() && !cursor.atBlockStart()) {
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
            QString prevChar = cursor.selectedText();
            if (prevChar == " " && !m_useTabs) {
                cursor.clearSelection();
                int spacesBefore = 0;
                int pos = cursor.position() - 1;
                QTextDocument *doc = cursor.document();
                while (pos >= 0 && doc->characterAt(pos) == ' ') { spacesBefore++; pos--; }
                if (spacesBefore >= m_tabWidth) {
                    cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, m_tabWidth);
                    cursor.removeSelectedText();
                    return;
                }
            } else if (prevChar == "\t" && m_useTabs) {
                cursor.clearSelection();
                cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
                if (cursor.selectedText() == "\t") { cursor.removeSelectedText(); return; }
            }
        }
        QPlainTextEdit::keyPressEvent(event);
        buildSuggestion();
        return;
    }

    QPlainTextEdit::keyPressEvent(event);
    buildSuggestion();
}

void Editor::insertIndentedNewLine()
{
    QTextCursor cursor = textCursor();
    QTextBlock currentBlock = cursor.block();
    QString currentLine = currentBlock.text();
    QString indent;
    int pos = 0;
    while (pos < currentLine.length()) {
        QChar ch = currentLine[pos];
        if (ch == ' ' || ch == '\t') indent.append(ch);
        else break;
        pos++;
    }
    cursor.beginEditBlock();
    cursor.insertBlock();
    cursor.insertText(indent);
    cursor.endEditBlock();
    setTextCursor(cursor);
    buildSuggestion();
}

void Editor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    updateSidebarGeometry();
}

void Editor::zoomIn()
{
    QFont f = font();
    int newSize = f.pointSize() + 1;
    if (newSize <= 72) { f.setPointSize(newSize); setFont(f); updateSidebarGeometry(); }
}

void Editor::zoomOut()
{
    QFont f = font();
    int newSize = f.pointSize() - 1;
    if (newSize >= 6) { f.setPointSize(newSize); setFont(f); updateSidebarGeometry(); }
}

void Editor::setZoomBaseFontSize(int size)
{
    m_zoomBaseFontSize = size;
    QFont f = font();
    f.setPointSize(size);
    setFont(f);
    updateSidebarGeometry();
}

QTextBlock Editor::firstVisibleBlockPublic() const { return firstVisibleBlock(); }
QRectF Editor::blockBoundingGeometryPublic(const QTextBlock &block) const { return blockBoundingGeometry(block); }
QPointF Editor::contentOffsetPublic() const { return contentOffset(); }
QRectF Editor::blockBoundingRectPublic(const QTextBlock &block) const { return blockBoundingRect(block); }

void Editor::updateSidebarGeometry()
{
    setViewportMargins(m_sidebar->sizeHint().width(), 0, 0, 0);
    m_sidebar->setGeometry(0, 0, m_sidebar->sizeHint().width(), height());
}
void Editor::updateSidebar() { m_sidebar->update(); }

void Editor::copyLine()
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection()) {
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
        QString lineText = cursor.selectedText();
        QApplication::clipboard()->setText(lineText + "\n");
        emit statusMessage("已复制");
    }
}
void Editor::cutLine()
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection()) {
        cursor.beginEditBlock();
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
        QString lineText = cursor.selectedText();
        cursor.removeSelectedText();
        if (!cursor.atEnd()) cursor.deleteChar();
        cursor.endEditBlock();
        QApplication::clipboard()->setText(lineText + "\n");
        emit statusMessage("已剪切");
    }
}
void Editor::insertLineBelowKeepCursor()
{
    QTextCursor cursor = textCursor();
    QTextCursor insertCursor = cursor;
    insertCursor.movePosition(QTextCursor::EndOfLine);
    insertCursor.insertBlock();
    setTextCursor(cursor);
    emit statusMessage("已插入新行");
}
void Editor::duplicateLine()
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    QString lineText = cursor.selectedText();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::EndOfLine);
    cursor.insertBlock();
    cursor.insertText(lineText);
    cursor.endEditBlock();
    cursor.movePosition(QTextCursor::StartOfLine);
    setTextCursor(cursor);
    emit statusMessage("已复制行");
}

EditorSidebar::EditorSidebar(Editor *editor) : QWidget(editor), m_editor(editor) {}
QSize EditorSidebar::sizeHint() const
{
    int digits = QString::number(m_editor->blockCount()).length();
    int space = 8 + m_editor->fontMetrics().horizontalAdvance(QLatin1Char('9')) * (digits + 1);
    return QSize(space, 0);
}
void EditorSidebar::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(event->rect(), QColor(40,40,40));
    painter.setFont(m_editor->font());
    painter.setPen(Qt::white);
    QTextBlock block = m_editor->firstVisibleBlockPublic();
    int blockNumber = block.blockNumber();
    qreal top = m_editor->blockBoundingGeometryPublic(block).translated(m_editor->contentOffsetPublic()).top();
    qreal bottom = top + m_editor->blockBoundingRectPublic(block).height();
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            painter.drawText(0, (int)top, width()-4, m_editor->fontMetrics().height(), Qt::AlignRight, QString::number(blockNumber+1));
        }
        block = block.next();
        top = bottom;
        bottom = top + m_editor->blockBoundingRectPublic(block).height();
        ++blockNumber;
    }
}

