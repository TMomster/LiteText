#ifndef EDITOR_H
#define EDITOR_H

#include <QPlainTextEdit>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QTextCharFormat>
#include <QSet>
#include <QRegularExpression>
#include <QTextCursor>
#include <QHash>

class QWidget;
class EditorSidebar;

class Editor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit Editor(QWidget *parent = nullptr);
    ~Editor();

    void zoomIn();
    void zoomOut();
    void setZoomBaseFontSize(int size);
    void setTabWidth(int spaces);
    int tabWidth() const;
    void setUseTabs(bool use);
    bool useTabs() const { return m_useTabs; }
    void setFont(const QFont &font);
    void setAutoCompletionEnabled(bool enabled);
    bool isAutoCompletionEnabled() const { return m_autoCompletionEnabled; }
    void setCompletionAcceptKey(int key);
    int completionAcceptKey() const { return m_completionAcceptKey; }
    void setKeywordList(const QStringList &keywords);

    // 自动换行
    void setWordWrapEnabled(bool enabled);
    bool wordWrapEnabled() const { return m_wordWrapEnabled; }

    // 公共包装器（行号侧边栏需要）
    QTextBlock firstVisibleBlockPublic() const;
    QRectF blockBoundingGeometryPublic(const QTextBlock &block) const;
    QPointF contentOffsetPublic() const;
    QRectF blockBoundingRectPublic(const QTextBlock &block) const;

    // 由 MainWindow 调用，用于根据语法高亮语言设置注释前缀
    void setCurrentLanguage(const QString &lang);

signals:
    void statusMessage(const QString &msg);
    void tabWidthChanged(int newWidth);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateSidebarGeometry();
    void updateSidebar();
    void onDocumentContentsChanged();
    void onCursorPositionChanged();

private:
    EditorSidebar *m_sidebar;
    int m_zoomBaseFontSize;
    int m_tabWidth;
    bool m_useTabs;
    bool m_autoCompletionEnabled;
    int m_completionAcceptKey;
    QString m_pendingCompletion;
    QTextCharFormat m_suggestionFormat;
    QSet<QString> m_identifierSet;
    QStringList m_keywordList;

    bool m_wordWrapEnabled;   // 自动换行开关

    // 注释功能
    QString m_currentLanguage;
    QString m_commentPrefix;

    QString longestCommonPrefix(const QStringList &strs);
    void buildSuggestion();
    void collectIdentifiersFromDocument();
    void clearSuggestion();
    void acceptSuggestion();

    void updateTabStopWidth();
    void copyLine();
    void cutLine();
    void insertLineBelowKeepCursor();
    void duplicateLine();
    void insertIndentedNewLine();

    void toggleCommentSelection();  // Ctrl+/ 切换注释
    void indentSelection();     // 选中行增加缩进
    void unindentSelection();   // 选中行减少缩进
};

class EditorSidebar : public QWidget
{
public:
    EditorSidebar(Editor *editor);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Editor *m_editor;
};

#endif // EDITOR_H