#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "Editor.h"

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
    void reloadSettings();
    void updateCursorPosition();
    void onEditorStatusMessage(const QString &msg);
    void onTabWidthChanged(int newWidth);
    void onLanguageChanged(const QString &suffix);
    void showGotoDialog();
    void onSystemThemeChanged();

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
    void applyTheme();
    EditorThemeColors getThemeColorsForIndex(int themeIndex) const;
    QString formatLargeNumber(int value, int format) const;

    Editor *m_editor;
    SyntaxHighlighter *m_highlighter;
    FindReplaceDialog *m_findDialog;
    QString m_currentFilePath;
    QString m_currentEncoding;
    int m_largeNumberFormat;
    bool m_isUntitled;
    int m_currentTheme;
    bool m_isFollowingSystem;

    static const QString COPYRIGHT_TEXT;
    static const QString VERSION_STRING;
};

#endif // MAINWINDOW_H
