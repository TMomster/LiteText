#include "Editor.h"
#include <QKeyEvent>
#include <QFont>
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QTextOption>

Editor::Editor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_zoomBaseFontSize(10)
    , m_tabWidth(4)
    , m_useTabs(false)
{
    setLineWrapMode(QPlainTextEdit::NoWrap);
    m_sidebar = new EditorSidebar(this);
    connect(this, &Editor::blockCountChanged, this, &Editor::updateSidebarGeometry);
    connect(this, &Editor::updateRequest, this, &Editor::updateSidebar);
    connect(this, &Editor::cursorPositionChanged, this, &Editor::updateSidebar);
    updateSidebarGeometry();
    updateTabStopWidth();
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

void Editor::keyPressEvent(QKeyEvent *event)
{
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
        return;
    }
    QPlainTextEdit::keyPressEvent(event);
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

// ---------- EditorSidebar ----------
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
