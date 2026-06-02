#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

class Editor;
class SyntaxHighlighter;
class FindReplaceDialog;
class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void openFileFromPath(const QString &filePath);

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void about();
    void showPrivacyPolicy();
    void onEditorModificationChanged(bool modified);
    void showFindReplaceDialog();
    void showSettingsDialog();
    void reloadSettings();                     // 完整加载（启动时）
    void updateCursorPosition();
    void onEditorStatusMessage(const QString &msg);
    void onTabWidthChanged(int newWidth);
    void onLanguageChanged(const QString &suffix);
    void showGotoDialog();

private:
    void createMenuBar();
    void createStatusBar();
    bool maybeSave();
    void setCurrentFile(const QString &filePath);
    QString strippedName(const QString &fullFileName);
    void updateHighlighterForFile(const QString &filePath);
    void loadFileWithEncoding(const QString &filePath, const QString &encoding);
    bool saveFileWithEncoding(const QString &path, const QString &encoding);
    void updateStatusBar();
    void restartApplication(const QString &fileToOpen);
    QString configFilePath() const;
    void ensureConfigDir();
    void applyDefaultEncoding();
    void reloadSettingsExceptColorScheme();    // 加载除配色方案外的设置（设置对话框确定后）
    QString formatLargeNumber(int value, int format) const;

    Editor *m_editor;
    SyntaxHighlighter *m_highlighter;
    FindReplaceDialog *m_findDialog;
    QString m_currentFilePath;
    QString m_currentEncoding;
    int m_largeNumberFormat; // 0:精确,1:千位(k),2:万位(w)
    bool m_isUntitled;

    static const QString COPYRIGHT_TEXT;
    static const QString VERSION_STRING;
};

#endif // MAINWINDOW_H