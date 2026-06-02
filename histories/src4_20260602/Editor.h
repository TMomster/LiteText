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

// 主题颜色结构体
struct EditorThemeColors
{
    QColor base;        // 背景色
    QColor text;        // 文字色
    QColor highlight;   // 选中背景
    QColor highlightedText; // 选中文字
    QColor lineHighlight;   // 当前行高亮
    QColor sidebarBg;       // 行号区域背景
    QColor sidebarFg;       // 行号文字
};

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

    // 主题设置
    void setThemeColors(const EditorThemeColors &colors);
    void updateSidebarGeometry(); // 需要公开让MainWindow调用更新侧边栏

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

    // 智慧联想增强
    QHash<QString, int> m_identifierFrequency;
    QHash<int, QSet<QString>> m_blockWords;
    QStringList m_currentCandidates;
    int m_currentCandidateIndex;
    bool m_needsFullCollect;

    // 主题颜色相关
    QColor m_currentLineHighlightColor;   // 当前行高亮颜色
    EditorThemeColors m_themeColors;      // 当前主题颜色

    void updateIdentifierFrequency(const QString &word);
    void rebuildCandidates(const QString &prefix);
    void selectNextCandidate();
    void selectPrevCandidate();
    void updateSuggestionFromCurrentCandidate();
    void updateIdentifiersForBlock(const QTextBlock &block, bool removeOld = true);
    void clearBlockWordsForBlock(int blockNumber);
    void collectIdentifiersFromDocument();

    // 当前行高亮
    void updateCurrentLineHighlight();

    QString longestCommonPrefix(const QStringList &strs);
    void buildSuggestion();
    void clearSuggestion();
    void acceptSuggestion();

    void updateTabStopWidth();
    void copyLine();
    void cutLine();
    void insertLineBelowKeepCursor();
    void duplicateLine();
    void insertIndentedNewLine();

    void toggleCommentSelection();
    void indentSelection();
    void unindentSelection();
};

class EditorSidebar : public QWidget
{
public:
    EditorSidebar(Editor *editor);
    QSize sizeHint() const override;
    void setColors(const QColor &bg, const QColor &fg); // 设置行号区域颜色

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Editor *m_editor;
    QColor m_bgColor;
    QColor m_fgColor;
};

#endif // EDITOR_H