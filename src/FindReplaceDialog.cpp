#include "FindReplaceDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QMessageBox>
#include <QTextCursor>
#include <QTextDocument>

FindReplaceDialog::FindReplaceDialog(QPlainTextEdit *editor, QWidget *parent)
    : QDialog(parent), m_editor(editor)
{
    setWindowTitle("查找替换");
    setModal(false);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 查找行
    QHBoxLayout *findLayout = new QHBoxLayout();
    findLayout->addWidget(new QLabel("查找："));
    m_findEdit = new QLineEdit();
    findLayout->addWidget(m_findEdit);
    mainLayout->addLayout(findLayout);

    // 替换行
    QHBoxLayout *replaceLayout = new QHBoxLayout();
    replaceLayout->addWidget(new QLabel("替换："));
    m_replaceEdit = new QLineEdit();
    replaceLayout->addWidget(m_replaceEdit);
    mainLayout->addLayout(replaceLayout);

    // 大小写敏感
    m_caseSensitive = new QCheckBox("大小写敏感");
    mainLayout->addWidget(m_caseSensitive);

    // 按钮行
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_nextButton = new QPushButton("下一个");
    m_prevButton = new QPushButton("上一个");
    m_replaceButton = new QPushButton("替换");
    m_replaceAllButton = new QPushButton("替换全部");
    buttonLayout->addWidget(m_nextButton);
    buttonLayout->addWidget(m_prevButton);
    buttonLayout->addWidget(m_replaceButton);
    buttonLayout->addWidget(m_replaceAllButton);
    mainLayout->addLayout(buttonLayout);

    connect(m_nextButton, &QPushButton::clicked, this, &FindReplaceDialog::findNext);
    connect(m_prevButton, &QPushButton::clicked, this, &FindReplaceDialog::findPrevious);
    connect(m_replaceButton, &QPushButton::clicked, this, &FindReplaceDialog::replace);
    connect(m_replaceAllButton, &QPushButton::clicked, this, &FindReplaceDialog::replaceAll);
}

bool FindReplaceDialog::findText(bool forward)
{
    QString searchText = m_findEdit->text();
    if (searchText.isEmpty())
        return false;

    QTextDocument::FindFlags flags;
    if (m_caseSensitive->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (!forward)
        flags |= QTextDocument::FindBackward;

    QTextCursor cursor = m_editor->textCursor();
    QTextCursor found = m_editor->document()->find(searchText, cursor, flags);

    // 如果没找到，尝试从头（或从尾）循环查找
    if (found.isNull()) {
        QTextCursor startCursor(m_editor->document());
        if (forward) {
            startCursor.movePosition(QTextCursor::Start);
        } else {
            startCursor.movePosition(QTextCursor::End);
        }
        found = m_editor->document()->find(searchText, startCursor, flags);
        if (!found.isNull()) {
            // 可选：状态栏提示已绕回
            // 这里不做确认对话框，直接跳转
        }
    }

    if (!found.isNull()) {
        m_editor->setTextCursor(found);
        return true;
    }
    return false;
}

void FindReplaceDialog::findNext()
{
    if (!findText(true)) {
        QMessageBox::information(this, "查找", "未找到文本。");
    }
}

void FindReplaceDialog::findPrevious()
{
    if (!findText(false)) {
        QMessageBox::information(this, "查找", "未找到文本。");
    }
}

void FindReplaceDialog::replace()
{
    QString searchText = m_findEdit->text();
    if (searchText.isEmpty()) return;

    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection() && cursor.selectedText() == searchText) {
        cursor.insertText(m_replaceEdit->text());
        // 替换后光标移动到替换文本之后，再查找下一个（向前）
        m_editor->setTextCursor(cursor);
    }
    findNext();  // 自动定位到下一个匹配项
}

void FindReplaceDialog::replaceAll()
{
    QString searchText = m_findEdit->text();
    if (searchText.isEmpty()) return;

    QTextDocument *doc = m_editor->document();
    QTextCursor cursor(doc);
    cursor.beginEditBlock();

    QTextDocument::FindFlags flags;
    if (m_caseSensitive->isChecked())
        flags |= QTextDocument::FindCaseSensitively;

    int count = 0;
    cursor.movePosition(QTextCursor::Start);
    while (!cursor.isNull() && !cursor.atEnd()) {
        cursor = doc->find(searchText, cursor, flags);
        if (!cursor.isNull()) {
            cursor.insertText(m_replaceEdit->text());
            count++;
        }
    }
    cursor.endEditBlock();

    QMessageBox::information(this, "替换全部", QString("已替换 %1 处。").arg(count));
}