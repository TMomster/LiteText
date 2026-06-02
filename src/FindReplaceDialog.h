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

    bool findText(bool forward);  // 鏍稿績鏌ユ壘鍑芥暟锛宖orward=true涓哄悜涓嬶紝false涓哄悜涓婏紝杩斿洖鏄惁鎵惧埌
};

#endif // FINDREPLACEDIALOG_H