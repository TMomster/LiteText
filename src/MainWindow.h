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
    void onEditorModificationChanged(bool modified);
    void showFindReplaceDialog();
    void showSettingsDialog();
    void showPrivacyPolicy();
    void reloadSettings();
    void setFileEncoding(const QString &encoding);
    void updateCursorPosition();
    void onEditorStatusMessage(const QString &msg);
    void changeColorScheme(int scheme);
    void onTabWidthChanged(int newWidth);   // 响应 Tab 宽度变化

private:
    void createMenuBar();
    void createStatusBar();
    bool maybeSave();
    bool saveFileToPath(const QString &path);
    void setCurrentFile(const QString &filePath);
    QString strippedName(const QString &fullFileName);
    void updateHighlighterForFile(const QString &filePath);
    void loadFileWithEncoding(const QString &filePath, const QString &encoding);
    bool saveFileWithEncoding(const QString &path, const QString &encoding);
    void updateStatusBar();
    void restartApplication(const QString &fileToOpen);
    QString configFilePath() const;
    void ensureConfigDir();

    Editor *m_editor;
    SyntaxHighlighter *m_highlighter;
    FindReplaceDialog *m_findDialog;
    QString m_currentFilePath;
    QString m_currentEncoding;
    bool m_isUntitled;
    
    static const QString COPYRIGHT_TEXT;
    static const QString VERSION_STRING;
};

#endif // MAINWINDOW_H