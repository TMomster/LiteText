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
#include <QHash>
#include <algorithm>

Editor::Editor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_zoomBaseFontSize(10)
    , m_tabWidth(4)
    , m_useTabs(false)
    , m_autoCompletionEnabled(true)
    , m_completionAcceptKey(Qt::Key_Tab)
    , m_wordWrapEnabled(true)   // 默认开启自动换行
    , m_currentLanguage("txt")
    , m_commentPrefix("//")
{
    setLineWrapMode(m_wordWrapEnabled ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
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
        updateTabStopWidth(); 
        emit tabWidthChanged(m_tabWidth);
    }
}

int Editor::tabWidth() const { return m_tabWidth; }

void Editor::setUseTabs(bool use)
{
     m_useTabs = use;
     updateTabStopWidth();
     emit tabWidthChanged(m_tabWidth);
}

void Editor::setWordWrapEnabled(bool enabled)
{
    if (m_wordWrapEnabled != enabled) {
        m_wordWrapEnabled = enabled;
        setLineWrapMode(enabled ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
        viewport()->update();
    }
}

void Editor::updateTabStopWidth()
{
    QFontMetricsF fm(font());
    qreal tabWidthInSpaces = fm.horizontalAdvance(QString(m_tabWidth, ' '));
    QTextOption option = document()->defaultTextOption();
    option.setTabStopDistance(tabWidthInSpaces);
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

QString Editor::longestCommonPrefix(const QStringList &strs) {
    if (strs.isEmpty()) return QString();
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

void Editor::buildSuggestion() {
    if (!m_autoCompletionEnabled) {
        clearSuggestion();
        return;
    }
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
    QString prefix = cursor.selectedText();
    if (prefix.isEmpty() || (!prefix.at(0).isLetter() && prefix.at(0) != '_')) {
        clearSuggestion();
        return;
    }

    QString currentLine = cursor.block().text();
    int cursorPosInBlock = cursor.positionInBlock();
    bool isPreprocessor = currentLine.trimmed().startsWith('#');
    bool isInString = false;
    int quoteCount = 0;
    for (int i = 0; i < cursorPosInBlock; ++i) {
        if (currentLine[i] == '\"') quoteCount++;
    }
    if (quoteCount % 2 == 1) isInString = true;

    QSet<QString> allWords;
    if (!isInString) {
        if (isPreprocessor) {
            QStringList preprocessorKeywords = {"include", "define", "ifdef", "ifndef", "endif", "else", "elif", "pragma", "error", "warning"};
            for (const QString &kw : preprocessorKeywords) allWords.insert(kw);
        } else {
            for (const QString &kw : m_keywordList) allWords.insert(kw);
            allWords.unite(m_identifierSet);
        }
    } else {
        allWords.unite(m_identifierSet);
    }
    if (allWords.contains(prefix)) allWords.remove(prefix);

    QStringList candidates;
    for (const QString &word : allWords) {
        if (word.startsWith(prefix, Qt::CaseInsensitive))
            candidates.append(word);
    }
    if (candidates.isEmpty()) {
        clearSuggestion();
        return;
    }

    std::sort(candidates.begin(), candidates.end(), [](const QString &a, const QString &b) {
        if (a.length() != b.length()) return a.length() < b.length();
        return a < b;
    });
    QString bestMatch = candidates.first();
    QString suffix = bestMatch.mid(prefix.length());
    if (suffix.isEmpty()) {
        clearSuggestion();
        return;
    }

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

void Editor::keyPressEvent(QKeyEvent *event)
{
    // Ctrl+/ 切换行注释
    if ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_Slash) {
        toggleCommentSelection();
        event->accept();
        return;
    }

    if (m_autoCompletionEnabled && !m_pendingCompletion.isEmpty() && event->key() == m_completionAcceptKey) {
        acceptSuggestion();
        event->accept();
        return;
    }

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

// ========== 注释功能 ==========
void Editor::setCurrentLanguage(const QString &lang)
{
    m_currentLanguage = lang;
    static const QHash<QString, QString> prefixMap = {
        {"cpp", "//"}, {"c", "//"}, {"h", "//"}, {"hpp", "//"}, {"cc", "//"}, {"cxx", "//"},
        {"java", "//"}, {"js", "//"}, {"css", "//"},
        {"py", "#"},
        {"yaml", "#"}, {"yml", "#"}, {"ini", "#"}, {"properties", "#"}, {"gitignore", "#"}
    };
    m_commentPrefix = prefixMap.value(lang, QString());
}

void Editor::toggleCommentSelection()
{
    if (m_commentPrefix.isEmpty()) {
        emit statusMessage(tr("当前语言不支持行注释"));
        return;
    }

    QTextCursor cursor = textCursor();
    QTextBlock startBlock = document()->findBlock(cursor.selectionStart());
    QTextBlock endBlock = document()->findBlock(cursor.selectionEnd());
    if (!cursor.hasSelection()) {
        startBlock = endBlock = cursor.block();
    }

    cursor.beginEditBlock();

    QTextBlock block = startBlock;
    while (block.isValid()) {
        QString lineText = block.text();
        int firstNonSpace = 0;
        while (firstNonSpace < lineText.length() && lineText[firstNonSpace].isSpace())
            ++firstNonSpace;

        bool hasComment = lineText.mid(firstNonSpace).startsWith(m_commentPrefix);

        QTextCursor lineCursor(block);
        lineCursor.movePosition(QTextCursor::StartOfBlock);
        if (hasComment) {
            // 删除注释前缀
            lineCursor.setPosition(block.position() + firstNonSpace);
            lineCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, m_commentPrefix.length());
            lineCursor.removeSelectedText();
            // 如果后面紧跟一个空格，也删除
            if (lineCursor.block().text().mid(firstNonSpace).startsWith(' ')) {
                lineCursor.setPosition(block.position() + firstNonSpace);
                lineCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 1);
                lineCursor.removeSelectedText();
            }
        } else {
            // 插入注释前缀 + 一个空格
            lineCursor.setPosition(block.position() + firstNonSpace);
            lineCursor.insertText(m_commentPrefix + " ");
        }

        if (block == endBlock) break;
        block = block.next();
    }

    cursor.endEditBlock();
    setTextCursor(cursor);
}
// ================================

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