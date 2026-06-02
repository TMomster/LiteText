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
    , m_wordWrapEnabled(true)
    , m_currentLanguage("txt")
    , m_commentPrefix("//")
    , m_currentCandidateIndex(-1)
    , m_needsFullCollect(true)
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

    // 初始化当前行高亮
    updateCurrentLineHighlight();
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

// ========== 增量收集实现 ==========
void Editor::collectIdentifiersFromDocument()
{
    m_identifierSet.clear();
    m_identifierFrequency.clear();
    m_blockWords.clear();

    QTextDocument *doc = document();
    if (!doc) return;

    QRegularExpression identifierRegex("\\b[a-zA-Z_][a-zA-Z0-9_]*\\b");
    QTextBlock block = doc->firstBlock();
    while (block.isValid()) {
        QString text = block.text();
        QRegularExpressionMatchIterator it = identifierRegex.globalMatch(text);
        QSet<QString> wordsInBlock;
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            QString word = match.captured(0);
            m_identifierSet.insert(word);
            m_identifierFrequency[word] = m_identifierFrequency.value(word, 0) + 1;
            wordsInBlock.insert(word);
        }
        if (!wordsInBlock.isEmpty())
            m_blockWords[block.blockNumber()] = wordsInBlock;
        block = block.next();
    }
    m_needsFullCollect = false;
}

void Editor::updateIdentifiersForBlock(const QTextBlock &block, bool removeOld)
{
    if (!block.isValid()) return;
    int blockNum = block.blockNumber();
    QString text = block.text();

    QRegularExpression identifierRegex("\\b[a-zA-Z_][a-zA-Z0-9_]*\\b");
    QRegularExpressionMatchIterator it = identifierRegex.globalMatch(text);
    QSet<QString> newWords;
    while (it.hasNext()) {
        newWords.insert(it.next().captured(0));
    }

    if (removeOld && m_blockWords.contains(blockNum)) {
        const QSet<QString> &oldWords = m_blockWords[blockNum];
        for (const QString &word : oldWords) {
            int freq = m_identifierFrequency.value(word, 0);
            if (freq <= 1)
                m_identifierFrequency.remove(word);
            else
                m_identifierFrequency[word] = freq - 1;
            m_identifierSet.remove(word);
        }
        m_blockWords.remove(blockNum);
    }

    if (!newWords.isEmpty()) {
        m_blockWords[blockNum] = newWords;
        for (const QString &word : newWords) {
            m_identifierSet.insert(word);
            m_identifierFrequency[word] = m_identifierFrequency.value(word, 0) + 1;
        }
    }
}

void Editor::clearBlockWordsForBlock(int blockNumber)
{
    if (m_blockWords.contains(blockNumber)) {
        const QSet<QString> &words = m_blockWords[blockNumber];
        for (const QString &word : words) {
            int freq = m_identifierFrequency.value(word, 0);
            if (freq <= 1)
                m_identifierFrequency.remove(word);
            else
                m_identifierFrequency[word] = freq - 1;
            m_identifierSet.remove(word);
        }
        m_blockWords.remove(blockNumber);
    }
}

void Editor::onDocumentContentsChanged()
{
    if (!m_autoCompletionEnabled) return;

    QTextCursor cursor = textCursor();
    QTextBlock currentBlock = cursor.block();
    if (currentBlock.isValid()) {
        int blockNum = currentBlock.blockNumber();
        if (m_needsFullCollect) {
            static QTimer *fullTimer = nullptr;
            if (!fullTimer) {
                fullTimer = new QTimer(this);
                fullTimer->setSingleShot(true);
                connect(fullTimer, &QTimer::timeout, this, &Editor::collectIdentifiersFromDocument);
            }
            fullTimer->start(300);
            m_needsFullCollect = false;
        } else {
            updateIdentifiersForBlock(currentBlock);
        }
    }

    buildSuggestion();
}

void Editor::onCursorPositionChanged()
{
    clearSuggestion();
    buildSuggestion();
    updateCurrentLineHighlight();   // 光标移动时更新当前行高亮
}

void Editor::clearSuggestion()
{
    if (!m_pendingCompletion.isEmpty()) {
        m_pendingCompletion.clear();
        m_currentCandidates.clear();
        m_currentCandidateIndex = -1;
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

void Editor::updateIdentifierFrequency(const QString &word)
{
    if (word.isEmpty() || (!word[0].isLetter() && word[0] != '_')) return;
    m_identifierFrequency[word] = m_identifierFrequency.value(word, 0) + 1;
    m_identifierSet.insert(word);
}

void Editor::rebuildCandidates(const QString &prefix)
{
    m_currentCandidates.clear();
    m_currentCandidateIndex = -1;
    if (prefix.isEmpty()) return;

    QTextCursor cursor = textCursor();
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

    for (const QString &word : allWords) {
        if (word.startsWith(prefix, Qt::CaseInsensitive))
            m_currentCandidates.append(word);
    }
    if (m_currentCandidates.isEmpty()) return;

    std::sort(m_currentCandidates.begin(), m_currentCandidates.end(),
        [this](const QString &a, const QString &b) {
            int freqA = m_identifierFrequency.value(a, 0);
            int freqB = m_identifierFrequency.value(b, 0);
            if (freqA != freqB) return freqA > freqB;
            if (a.length() != b.length()) return a.length() < b.length();
            return a < b;
        });

    m_currentCandidateIndex = 0;
}

void Editor::selectNextCandidate()
{
    if (m_currentCandidates.isEmpty() || m_currentCandidateIndex < 0) return;
    m_currentCandidateIndex = (m_currentCandidateIndex + 1) % m_currentCandidates.size();
    updateSuggestionFromCurrentCandidate();
}

void Editor::selectPrevCandidate()
{
    if (m_currentCandidates.isEmpty() || m_currentCandidateIndex < 0) return;
    m_currentCandidateIndex = (m_currentCandidateIndex - 1 + m_currentCandidates.size()) % m_currentCandidates.size();
    updateSuggestionFromCurrentCandidate();
}

void Editor::updateSuggestionFromCurrentCandidate()
{
    if (m_currentCandidateIndex < 0 || m_currentCandidateIndex >= m_currentCandidates.size()) {
        m_pendingCompletion.clear();
        viewport()->update();
        return;
    }
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
    QString prefix = cursor.selectedText();
    QString bestMatch = m_currentCandidates[m_currentCandidateIndex];
    m_pendingCompletion = bestMatch.mid(prefix.length());
    viewport()->update();
}

void Editor::buildSuggestion() {
    if (!m_autoCompletionEnabled) {
        clearSuggestion();
        return;
    }

    QTextCursor cursor = textCursor();
    int pos = cursor.position();
    QTextDocument *doc = document();

    // 检查光标后是否有非空白字符（单词内部禁止联想）
    if (pos < doc->characterCount()) {   // 未到文档结尾
        QChar nextChar = doc->characterAt(pos);
        if (!nextChar.isNull() && !nextChar.isSpace()) {
            clearSuggestion();
            return;
        }
    }

    cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
    QString prefix = cursor.selectedText();
    if (prefix.isEmpty() || (!prefix.at(0).isLetter() && prefix.at(0) != '_')) {
        clearSuggestion();
        return;
    }

    rebuildCandidates(prefix);
    updateSuggestionFromCurrentCandidate();
}

void Editor::acceptSuggestion()
{
    if (m_pendingCompletion.isEmpty()) return;
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.insertText(m_pendingCompletion);
    cursor.endEditBlock();

    cursor.movePosition(QTextCursor::StartOfWord);
    cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    QString completedWord = cursor.selectedText();
    updateIdentifierFrequency(completedWord);

    m_pendingCompletion.clear();
    m_currentCandidates.clear();
    m_currentCandidateIndex = -1;
    viewport()->update();
}

// 当前行高亮实现
void Editor::updateCurrentLineHighlight()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(45, 45, 60); // 行高亮颜色
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
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

    int textWidth = painter.fontMetrics().horizontalAdvance(m_pendingCompletion);
    int viewportWidth = viewport()->width();
    if (startX + textWidth > viewportWidth) {
        startX = viewportWidth - textWidth - 2;
        if (startX < 0) startX = 0;
    }

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

    // Control+Up/Down 切换候选
    if (event->modifiers() == Qt::ControlModifier) {
        if (event->key() == Qt::Key_Down) {
            selectNextCandidate();
            event->accept();
            return;
        } else if (event->key() == Qt::Key_Up) {
            selectPrevCandidate();
            event->accept();
            return;
        }
    }

    // 采纳建议
    if (m_autoCompletionEnabled && !m_pendingCompletion.isEmpty() && event->key() == m_completionAcceptKey) {
        acceptSuggestion();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Tab) {
        if (textCursor().hasSelection()) {
            indentSelection();
        } else {
            if (m_useTabs) insertPlainText("\t");
            else insertPlainText(QString(m_tabWidth, ' '));
        }
        return;
    }
    if (event->key() == Qt::Key_Backtab) {
        if (textCursor().hasSelection()) {
            unindentSelection();
        } else {
            QTextCursor cursor = textCursor();
            if (!m_useTabs) {
                cursor.movePosition(QTextCursor::StartOfLine);
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, m_tabWidth);
                if (cursor.selectedText() == QString(m_tabWidth, ' ')) {
                    cursor.removeSelectedText();
                }
            } else {
                cursor.movePosition(QTextCursor::StartOfLine);
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
                if (cursor.selectedText() == "\t") {
                    cursor.removeSelectedText();
                }
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
        if (cursor.hasSelection()) {
            QPlainTextEdit::keyPressEvent(event);
            buildSuggestion();
            return;
        }
        if (cursor.atBlockStart()) {
            QPlainTextEdit::keyPressEvent(event);
            buildSuggestion();
            return;
        }

        QTextBlock block = cursor.block();
        QString lineText = block.text();
        int posInBlock = cursor.positionInBlock();
        bool onlyWhitespaceBefore = true;
        for (int i = 0; i < posInBlock; ++i) {
            if (!lineText[i].isSpace()) {
                onlyWhitespaceBefore = false;
                break;
            }
        }

        if (onlyWhitespaceBefore) {
            if (!m_useTabs) {
                int spacesBefore = 0;
                int pos = posInBlock - 1;
                while (pos >= 0 && lineText[pos] == ' ') {
                    spacesBefore++;
                    pos--;
                }
                if (spacesBefore >= m_tabWidth) {
                    cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, m_tabWidth);
                    cursor.removeSelectedText();
                } else if (spacesBefore > 0) {
                    cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, spacesBefore);
                    cursor.removeSelectedText();
                } else {
                    QPlainTextEdit::keyPressEvent(event);
                }
            } else {
                cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
                if (cursor.selectedText() == "\t") {
                    cursor.removeSelectedText();
                } else {
                    QPlainTextEdit::keyPressEvent(event);
                }
            }
        } else {
            QPlainTextEdit::keyPressEvent(event);
        }
        buildSuggestion();
        return;
    }

    QPlainTextEdit::keyPressEvent(event);

    QString text = event->text();
    if (text.length() == 1 && (text[0].isSpace() || text[0] == '.' || text[0] == ',' || text[0] == ';' || text[0] == ':' ||
                               text[0] == '?' || text[0] == '!' || text[0] == ')' || text[0] == ']' || text[0] == '}')) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
        cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
        QString word = cursor.selectedText();
        if (!word.isEmpty() && (word[0].isLetter() || word[0] == '_'))
            updateIdentifierFrequency(word);
    }

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
            lineCursor.setPosition(block.position() + firstNonSpace);
            lineCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, m_commentPrefix.length());
            lineCursor.removeSelectedText();
            if (lineCursor.block().text().mid(firstNonSpace).startsWith(' ')) {
                lineCursor.setPosition(block.position() + firstNonSpace);
                lineCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 1);
                lineCursor.removeSelectedText();
            }
        } else {
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

void Editor::indentSelection()
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection()) return;

    int startPos = cursor.selectionStart();
    int endPos = cursor.selectionEnd();
    QTextDocument *doc = document();

    QTextBlock startBlock = doc->findBlock(startPos);
    QTextBlock endBlock = doc->findBlock(endPos);
    if (endPos == endBlock.position() && endPos != startPos) {
        endBlock = endBlock.previous();
    }

    cursor.beginEditBlock();
    QTextBlock block = startBlock;
    while (block.isValid() && block.blockNumber() <= endBlock.blockNumber()) {
        QTextCursor lineCursor(block);
        lineCursor.movePosition(QTextCursor::StartOfBlock);
        if (m_useTabs) {
            lineCursor.insertText("\t");
        } else {
            lineCursor.insertText(QString(m_tabWidth, ' '));
        }
        block = block.next();
    }
    cursor.endEditBlock();

    QTextCursor newCursor = textCursor();
    newCursor.setPosition(startBlock.position());
    newCursor.setPosition(endBlock.position() + endBlock.length() - 1, QTextCursor::KeepAnchor);
    setTextCursor(newCursor);
}

void Editor::unindentSelection()
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection()) return;

    int startPos = cursor.selectionStart();
    int endPos = cursor.selectionEnd();
    QTextDocument *doc = document();

    QTextBlock startBlock = doc->findBlock(startPos);
    QTextBlock endBlock = doc->findBlock(endPos);
    if (endPos == endBlock.position() && endPos != startPos) {
        endBlock = endBlock.previous();
    }

    cursor.beginEditBlock();
    QTextBlock block = startBlock;
    while (block.isValid() && block.blockNumber() <= endBlock.blockNumber()) {
        QString lineText = block.text();
        QTextCursor lineCursor(block);
        lineCursor.movePosition(QTextCursor::StartOfBlock);

        if (m_useTabs) {
            if (lineText.startsWith('\t')) {
                lineCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
                lineCursor.removeSelectedText();
            }
        } else {
            int spaces = 0;
            while (spaces < lineText.length() && lineText[spaces] == ' ') {
                ++spaces;
            }
            int removeCount = qMin(spaces, m_tabWidth);
            if (removeCount > 0) {
                lineCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, removeCount);
                lineCursor.removeSelectedText();
            }
        }
        block = block.next();
    }
    cursor.endEditBlock();

    QTextCursor newCursor = textCursor();
    newCursor.setPosition(startBlock.position());
    newCursor.setPosition(endBlock.position() + endBlock.length() - 1, QTextCursor::KeepAnchor);
    setTextCursor(newCursor);
}

// ========== EditorSidebar 实现 ==========
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