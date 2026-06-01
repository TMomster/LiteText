#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

#include <QDialog>
#include <QPlainTextEdit>

class QLineEdit;
class QCheckBox;
class QPushButton;

class FindReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindReplaceDialog(QPlainTextEdit *editor, QWidget *parent = nullptr);

private slots:
    void findNext();
    void replace();
    void replaceAll();

private:
    QPlainTextEdit *m_editor;
    QLineEdit *m_findEdit;
    QLineEdit *m_replaceEdit;
    QCheckBox *m_caseSensitive;
    QPushButton *m_findButton;
    QPushButton *m_replaceButton;
    QPushButton *m_replaceAllButton;
};

#endif // FINDREPLACEDIALOG_H
