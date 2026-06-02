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
    void findPrevious();
    void replace();
    void replaceAll();

private:
    QPlainTextEdit *m_editor;
    QLineEdit *m_findEdit;
    QLineEdit *m_replaceEdit;
    QCheckBox *m_caseSensitive;
    QPushButton *m_nextButton;
    QPushButton *m_prevButton;
    QPushButton *m_replaceButton;
    QPushButton *m_replaceAllButton;

    bool findText(bool forward);  // 核心查找函数，forward=true为向下，false为向上，返回是否找到
};

#endif // FINDREPLACEDIALOG_H