#ifndef EDITOR_H
#define EDITOR_H

#include <QPlainTextEdit>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QTextCharFormat>
#include <QSet>
#include <QRegularExpression>
#include <QTextCursor>

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

    // 公共包装器（行号侧边栏需要）
    QTextBlock firstVisibleBlockPublic() const;
    QRectF blockBoundingGeometryPublic(const QTextBlock &block) const;
    QPointF contentOffsetPublic() const;
    QRectF blockBoundingRectPublic(const QTextBlock &block) const;

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

    // 新的辅助函数：最长公共前缀计算
    QString longestCommonPrefix(const QStringList &strs);
    // 新的核心函数：更聪明的上下文感知补全构建
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
