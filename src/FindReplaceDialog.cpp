#include "FindReplaceDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QMessageBox>
#include <QTextCursor>

FindReplaceDialog::FindReplaceDialog(QPlainTextEdit *editor, QWidget *parent)
    : QDialog(parent), m_editor(editor)
{
    setWindowTitle("Find/Replace");
    setModal(false);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *findLayout = new QHBoxLayout();
    findLayout->addWidget(new QLabel("Find:"));
    m_findEdit = new QLineEdit();
    findLayout->addWidget(m_findEdit);
    mainLayout->addLayout(findLayout);

    QHBoxLayout *replaceLayout = new QHBoxLayout();
    replaceLayout->addWidget(new QLabel("Replace:"));
    m_replaceEdit = new QLineEdit();
    replaceLayout->addWidget(m_replaceEdit);
    mainLayout->addLayout(replaceLayout);

    m_caseSensitive = new QCheckBox("Case sensitive");
    mainLayout->addWidget(m_caseSensitive);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_findButton = new QPushButton("Find Next");
    m_replaceButton = new QPushButton("Replace");
    m_replaceAllButton = new QPushButton("Replace All");
    buttonLayout->addWidget(m_findButton);
    buttonLayout->addWidget(m_replaceButton);
    buttonLayout->addWidget(m_replaceAllButton);
    mainLayout->addLayout(buttonLayout);

    connect(m_findButton, &QPushButton::clicked, this, &FindReplaceDialog::findNext);
    connect(m_replaceButton, &QPushButton::clicked, this, &FindReplaceDialog::replace);
    connect(m_replaceAllButton, &QPushButton::clicked, this, &FindReplaceDialog::replaceAll);
}

void FindReplaceDialog::findNext()
{
    QString searchText = m_findEdit->text();
    if (searchText.isEmpty()) return;

    QTextDocument::FindFlags flags;
    if (m_caseSensitive->isChecked())
        flags |= QTextDocument::FindCaseSensitively;

    if (!m_editor->find(searchText, flags)) {
        QMessageBox::information(this, "Find", "Text not found.");
    }
}

void FindReplaceDialog::replace()
{
    QString searchText = m_findEdit->text();
    if (searchText.isEmpty()) return;

    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection() && cursor.selectedText() == searchText) {
        cursor.insertText(m_replaceEdit->text());
        m_editor->setTextCursor(cursor);
    }
    findNext();
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
    while (!cursor.isNull() && !cursor.atEnd()) {
        cursor = doc->find(searchText, cursor, flags);
        if (!cursor.isNull()) {
            cursor.insertText(m_replaceEdit->text());
            count++;
        }
    }
    cursor.endEditBlock();
    QMessageBox::information(this, "Replace All", QString("Replaced %1 occurrence(s).").arg(count));
}
