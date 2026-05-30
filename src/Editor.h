#ifndef EDITOR_H
#define EDITOR_H

#include <QPlainTextEdit>
#include <QPaintEvent>
#include <QResizeEvent>

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
    void setTabWidth(int spaces);   // 设置 Tab 展开的空格数（插入空格时使用）
    int tabWidth() const;

    // Tab 行为：true=插入制表符，false=插入空格
    void setUseTabs(bool use);
    bool useTabs() const { return m_useTabs; }

    // 设置字体并更新制表符显示宽度（公共方法）
    void setFont(const QFont &font);

    // 公共包装器
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

private slots:
    void updateSidebarGeometry();
    void updateSidebar();

private:
    EditorSidebar *m_sidebar;
    int m_zoomBaseFontSize;
    int m_tabWidth;          // Tab 键插入的空格数（仅在 m_useTabs==false 时生效）
    bool m_useTabs;          // true=插入\t，false=插入空格

    void updateTabStopWidth();   // 更新制表符显示宽度（固定为四个空格）
    void copyLine();
    void cutLine();
    void insertLineBelowKeepCursor();
    void duplicateLine();
    void insertIndentedNewLine();   // 换行并保持缩进
    void deleteIndentation();       // 删除一个缩进级别（如果光标前是空格）
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