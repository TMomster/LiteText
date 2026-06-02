#include "MainWindow.h"
#include "Editor.h"
#include "SyntaxHighlighter.h"
#include "FindReplaceDialog.h"
#include "SettingsDialog.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QPlainTextEdit>
#include <QTextStream>
#include <QFile>
#include <QCloseEvent>
#include <QIODevice>
#include <QFileInfo>
#include <QWheelEvent>
#include <QFont>
#include <QSettings>
#include <QStringDecoder>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QProcess>
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QShortcut>
#include <QInputDialog>
#include <QGuiApplication>
#include <QStyleHints>

const QString MainWindow::COPYRIGHT_TEXT = "Copyright (C) 2026 Momster";
const QString MainWindow::VERSION_STRING = "1.0.0";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_isUntitled(true)
    , m_currentEncoding("UTF-8")
    , m_currentTheme(0)
    , m_isFollowingSystem(false)
{
    ensureConfigDir();

    setWindowTitle("LiteText - [新文件]");
    resize(1000, 700);

    m_editor = new Editor(this);
    setCentralWidget(m_editor);

    m_highlighter = new SyntaxHighlighter(m_editor->document());
    m_findDialog = new FindReplaceDialog(m_editor, this);

    connect(m_editor->document(), &QTextDocument::modificationChanged, this, &MainWindow::onEditorModificationChanged);
    connect(m_editor, &Editor::cursorPositionChanged, this, &MainWindow::updateCursorPosition);
    connect(m_editor, &Editor::statusMessage, this, &MainWindow::onEditorStatusMessage);
    connect(m_editor, &Editor::tabWidthChanged, this, &MainWindow::onTabWidthChanged);
    connect(m_editor->document(), &QTextDocument::contentsChanged, this, &MainWindow::updateStatusBar);
    m_editor->installEventFilter(this);

    // Ctrl+H 快速跳转
    QShortcut *gotoShortcut = new QShortcut(QKeySequence("Ctrl+H"), this);
    connect(gotoShortcut, &QShortcut::activated, this, &MainWindow::showGotoDialog);

    createMenuBar();
    createStatusBar();

    reloadSettings();
    updateStatusBar();
    statusBar()->showMessage("就绪", 2000);

    // 监听系统主题变化
    connect(qGuiApp->styleHints(), &QStyleHints::colorSchemeChanged, this, &MainWindow::onSystemThemeChanged);
}

MainWindow::~MainWindow() {}

QString MainWindow::configFilePath() const
{
    QString configDir = QDir::homePath() + "/MomsterTech/LiteText";
    return configDir + "/config.ini";
}

void MainWindow::ensureConfigDir()
{
    QString configDir = QDir::homePath() + "/MomsterTech/LiteText";
    QDir dir;
    if (!dir.exists(configDir))
        dir.mkpath(configDir);
}

void MainWindow::reloadSettings()
{
    QSettings settings(configFilePath(), QSettings::IniFormat);
    QString fontFamily = settings.value("editor/fontFamily", "Consolas").toString();
    int fontSize = settings.value("editor/fontSize", 10).toInt();
    int colorScheme = settings.value("editor/colorScheme", 0).toInt();
    int tabWidth = settings.value("editor/tabWidth", 4).toInt();
    bool useTabs = settings.value("editor/useTabs", false).toBool();
    bool wordWrap = settings.value("editor/wordWrap", true).toBool();
    bool autoCompletion = settings.value("editor/autoCompletion", true).toBool();
    int acceptKey = settings.value("editor/completionAcceptKey", Qt::Key_Tab).toInt();
    QString defaultEnc = settings.value("editor/defaultEncoding", "UTF-8").toString();
    m_largeNumberFormat = settings.value("editor/largeNumberFormat", 2).toInt();
    m_currentTheme = settings.value("editor/theme", 0).toInt();

    m_editor->setTabWidth(tabWidth);
    m_editor->setUseTabs(useTabs);
    m_editor->setWordWrapEnabled(wordWrap);
    m_editor->setAutoCompletionEnabled(autoCompletion);
    m_editor->setCompletionAcceptKey(acceptKey);

    QFont font(fontFamily, fontSize);
    font.setFixedPitch(true);
    m_editor->setFont(font);
    m_editor->setZoomBaseFontSize(fontSize);

    // 应用配色方案（语法高亮）
    m_highlighter->setColorScheme(colorScheme);
    m_editor->document()->markContentsDirty(0, m_editor->document()->characterCount());
    m_editor->viewport()->update();

    // 应用编辑器主题
    applyTheme();

    if (m_isUntitled) {
        m_currentEncoding = defaultEnc;
    }
    updateStatusBar();

    onLanguageChanged(m_highlighter->currentLanguage());
}

void MainWindow::applyTheme()
{
    int effectiveTheme = m_currentTheme;
    if (m_currentTheme == 3) { // 跟随系统
        m_isFollowingSystem = true;
        Qt::ColorScheme scheme = qGuiApp->styleHints()->colorScheme();
        effectiveTheme = (scheme == Qt::ColorScheme::Dark) ? 0 : 1;
    } else {
        m_isFollowingSystem = false;
        effectiveTheme = m_currentTheme;
    }
    EditorThemeColors colors = getThemeColorsForIndex(effectiveTheme);
    m_editor->setThemeColors(colors);
}

EditorThemeColors MainWindow::getThemeColorsForIndex(int index) const
{
    EditorThemeColors colors;
    switch (index) {
    case 0: // 深色
        colors.base = QColor(30, 30, 30);
        colors.text = QColor(220, 220, 220);
        colors.highlight = QColor(75, 110, 175);
        colors.highlightedText = Qt::white;
        colors.lineHighlight = QColor(45, 45, 60);
        colors.sidebarBg = QColor(40, 40, 40);
        colors.sidebarFg = Qt::white;
        break;
    case 1: // 浅色
        colors.base = QColor(250, 250, 250);
        colors.text = QColor(0, 0, 0);
        colors.highlight = QColor(200, 200, 255);
        colors.highlightedText = Qt::black;
        colors.lineHighlight = QColor(230, 230, 240);
        colors.sidebarBg = QColor(240, 240, 240);
        colors.sidebarFg = Qt::black;
        break;
    case 2: // 海洋
        colors.base = QColor(20, 40, 50);
        colors.text = QColor(210, 230, 240);
        colors.highlight = QColor(60, 140, 180);
        colors.highlightedText = Qt::white;
        colors.lineHighlight = QColor(40, 60, 70);
        colors.sidebarBg = QColor(35, 50, 60);
        colors.sidebarFg = QColor(200, 220, 230);
        break;
    default:
        // 默认深色
        colors.base = QColor(30, 30, 30);
        colors.text = QColor(220, 220, 220);
        colors.highlight = QColor(75, 110, 175);
        colors.highlightedText = Qt::white;
        colors.lineHighlight = QColor(45, 45, 60);
        colors.sidebarBg = QColor(40, 40, 40);
        colors.sidebarFg = Qt::white;
        break;
    }
    return colors;
}

void MainWindow::onSystemThemeChanged()
{
    if (m_currentTheme == 3) {
        applyTheme();
    }
}

void MainWindow::createMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
    QAction *newAction = new QAction("新建(&N)", this);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAction);

    QAction *openAction = new QAction("打开(&O)...", this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(openAction);

    QAction *saveAction = new QAction("保存(&S)", this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    fileMenu->addAction(saveAction);

    QAction *saveAsAction = new QAction("另存为(&A)...", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);
    fileMenu->addAction(saveAsAction);

    fileMenu->addSeparator();
    QAction *exitAction = new QAction("退出(&X)", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);

    QMenu *editMenu = menuBar()->addMenu("编辑(&E)");
    QAction *findAction = new QAction("查找/替换(&F)", this);
    findAction->setShortcut(QKeySequence::Find);
    connect(findAction, &QAction::triggered, this, &MainWindow::showFindReplaceDialog);
    editMenu->addAction(findAction);

    QAction *settingsAction = new QAction("设置(&S)", this);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettingsDialog);
    menuBar()->addAction(settingsAction);

    QMenu *helpMenu = menuBar()->addMenu("帮助(&H)");
    QAction *aboutAction = new QAction("关于(&A)", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);
    helpMenu->addAction(aboutAction);

    QAction *privacyAction = new QAction("隐私政策(&P)", this);
    connect(privacyAction, &QAction::triggered, this, &MainWindow::showPrivacyPolicy);
    helpMenu->addAction(privacyAction);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage("就绪");
}

void MainWindow::newFile()
{
    if (maybeSave()) {
        m_editor->clear();
        m_currentFilePath.clear();
        m_isUntitled = true;
        applyDefaultEncoding();
        setCurrentFile("");
        m_editor->document()->setModified(false);
        updateHighlighterForFile("");
        updateStatusBar();
    }
}

void MainWindow::openFile()
{
    if (!maybeSave()) return;

    QString filePath = QFileDialog::getOpenFileName(this,
        "打开文件", QString(),
        "代码文件 (*.cpp *.c *.h *.hpp *.java *.py *.js *.html *.htm *.css *.txt *.json);;所有文件 (*.*)");

    if (filePath.isEmpty()) return;
    openFileFromPath(filePath);
}

void MainWindow::openFileFromPath(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "错误", QString("无法打开文件: %1").arg(file.errorString()));
        return;
    }
    QByteArray data = file.readAll();
    file.close();

    QString encoding = "UTF-8";
    QString text;

    if (data.size() >= 3 && (uchar(data[0]) == 0xEF && uchar(data[1]) == 0xBB && uchar(data[2]) == 0xBF)) {
        encoding = "UTF-8-BOM";
        QByteArray withoutBom = data.mid(3);
        text = QString::fromUtf8(withoutBom);
    } else {
        QString utf8Text = QString::fromUtf8(data);
        bool hasReplacement = utf8Text.contains(QChar::ReplacementCharacter);
        if (!hasReplacement) {
            text = utf8Text;
            encoding = "UTF-8";
        } else {
            text = QString::fromLocal8Bit(data);
            encoding = "ANSI";
        }
    }

    m_editor->setPlainText(text);
    setCurrentFile(filePath);
    m_currentEncoding = encoding;
    m_editor->document()->setModified(false);
    updateHighlighterForFile(filePath);
    updateStatusBar();
    statusBar()->showMessage("已打开: " + filePath + " [" + encoding + "]", 3000);
}

void MainWindow::loadFileWithEncoding(const QString &filePath, const QString &encoding)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", QString("无法打开文件: %1").arg(file.errorString()));
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QString text;
    if (encoding == "UTF-8") {
        text = QString::fromUtf8(data);
    } else if (encoding == "UTF-8-BOM") {
        if (data.size() >= 3 && (uchar(data[0]) == 0xEF && uchar(data[1]) == 0xBB && uchar(data[2]) == 0xBF))
            data = data.mid(3);
        text = QString::fromUtf8(data);
    } else if (encoding == "GBK") {
        text = QString::fromLocal8Bit(data);
    } else if (encoding == "ANSI") {
        text = QString::fromLocal8Bit(data);
    } else {
        text = QString::fromUtf8(data);
    }

    m_editor->setPlainText(text);
    setCurrentFile(filePath);
    m_currentEncoding = encoding;
    m_editor->document()->setModified(false);
    updateHighlighterForFile(filePath);
    updateStatusBar();
    statusBar()->showMessage("已打开: " + filePath + " [" + encoding + "]", 3000);
}

bool MainWindow::saveFileWithEncoding(const QString &path, const QString &encoding)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, "错误", QString("无法保存文件: %1").arg(file.errorString()));
        return false;
    }

    QString text = m_editor->toPlainText();
    QByteArray data;
    if (encoding == "UTF-8") {
        data = text.toUtf8();
    } else if (encoding == "UTF-8-BOM") {
        data = text.toUtf8();
        data.prepend("\xEF\xBB\xBF");
    } else if (encoding == "GBK") {
        data = text.toLocal8Bit();
    } else if (encoding == "ANSI") {
        data = text.toLocal8Bit();
    } else {
        data = text.toUtf8();
    }

    if (file.write(data) != data.size()) {
        QMessageBox::warning(this, "错误", "写入文件失败");
        return false;
    }

    file.close();
    setCurrentFile(path);
    m_currentEncoding = encoding;
    m_editor->document()->setModified(false);
    updateStatusBar();
    statusBar()->showMessage("已保存: " + path + " [" + encoding + "]", 3000);
    return true;
}

bool MainWindow::saveFile()
{
    if (m_isUntitled || m_currentFilePath.isEmpty())
        return saveFileAs();
    return saveFileWithEncoding(m_currentFilePath, m_currentEncoding);
}

bool MainWindow::saveFileAs()
{
    QString filePath = QFileDialog::getSaveFileName(this,
        "保存文件", QString(),
        "代码文件 (*.cpp *.c *.h *.hpp *.java *.py *.js *.html *.htm *.css *.txt *.json);;所有文件 (*.*)");

    if (filePath.isEmpty()) return false;
    return saveFileWithEncoding(filePath, m_currentEncoding);
}

void MainWindow::setCurrentFile(const QString &filePath)
{
    m_currentFilePath = filePath;
    m_isUntitled = filePath.isEmpty();

    QString windowTitle;
    if (m_isUntitled)
        windowTitle = "LiteText - [新文件]";
    else
        windowTitle = QString("LiteText - %1").arg(strippedName(filePath));
    setWindowTitle(windowTitle);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::onEditorModificationChanged(bool modified)
{
    QString title = windowTitle();
    if (modified && !title.startsWith('*'))
        setWindowTitle("*" + title);
    else if (!modified && title.startsWith('*'))
        setWindowTitle(title.mid(1));
}

bool MainWindow::maybeSave()
{
    if (!m_editor->document()->isModified()) return true;

    QMessageBox::StandardButton ret = QMessageBox::warning(this,
        "未保存更改",
        "文档已修改，是否保存？",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save)
        return saveFile();
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

void MainWindow::about()
{
    QString aboutText =
        QString("<h2>LiteText %1</h2>")
        .arg(VERSION_STRING) +
        QString("<p>%1</p>")
        .arg(COPYRIGHT_TEXT) +
        "<p>轻量级代码编辑器</p>"
        "<hr>"
        "<p><b>使用的 Qt 框架与 LGPLv3 合规声明</b><br>"
        "本软件使用 Qt 框架开发。Qt 框架由 The Qt Company Ltd. 及其贡献者版权所有，"
        "并根据 GNU 宽通用公共许可证 (LGPL) 版本 3 提供。<br>"
        "本软件是作为“使用该库的作品”与 Qt 库动态链接而创建的。因此，LGPL 版本 3 的条款适用于 Qt 库，"
        "但不强制要求公开本软件自身的源代码。然而，您拥有修改 Qt 库并重新链接本软件、对本软件进行反向工程以调试这些修改、"
        "以及将本软件与您修改的 Qt 库版本重新组合的权利。<br>"
        "完整的 GNU LGPL 版本 3 许可协议文本可在以下位置找到：<a href='https://www.gnu.org/licenses/lgpl-3.0.html'>https://www.gnu.org/licenses/lgpl-3.0.html</a><br>"
        "Qt 库的源代码可通过以下渠道获取：<a href='https://download.qt.io/'>官方下载页面</a> 或 "
        "<a href='https://code.qt.io/cgit/'>Qt 官方代码仓库</a>。<br>"
        "Qt 是 The Qt Company Ltd. 的注册商标。</p>"
        "<hr>"
        "<p>特性：</p>"
        "<ul>"
        "<li>深色/浅色/海洋/跟随系统 主题</li>"
        "<li>多语言语法高亮：C/C++, Java, Python, JavaScript, HTML, CSS, XML, JSON, .gitignore, .properties, .ini</li>"
        "<li>根据文件后缀自动识别高亮规则</li>"
        "<li>行号显示</li>"
        "<li>查找/替换（Ctrl+F）</li>"
        "<li>文件编码切换（UTF-8/GBK/ANSI等）</li>"
        "<li>自定义字体</li>"
        "<li>Ctrl+滚轮缩放</li>"
        "<li>Tab 行为配置（插入制表符或空格）</li>"
        "<li>自动换行（可开关）</li>"
        "<li>智慧联想：内联建议，自动补全关键字和文档内标识符，可自定义开关及采纳键</li>"
        "<li>快速跳转：Ctrl+H，输入“行:列”或仅行号</li>"
        "</ul>"
        "<p>使用 Qt 6.11.0 + C++17 构建</p>";

    QMessageBox::about(this, QString("关于 LiteText %1").arg(VERSION_STRING), aboutText);
}

void MainWindow::showPrivacyPolicy()
{
    QDialog dialog(this);
    dialog.setWindowTitle("隐私政策");
    dialog.setMinimumSize(550, 450);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);

    QLabel *titleLabel = new QLabel("<h2>LiteText 隐私政策</h2>");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    QLabel *versionLabel = new QLabel(QString("<p><b>版本:</b> %1</p>").arg(VERSION_STRING));
    versionLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(versionLabel);

    QLabel *contentLabel = new QLabel(
        "<p><b>1. 信息收集</b><br>"
        "LiteText 不会主动收集、存储或传输任何个人身份信息。本软件完全在本地运行，"
        "不与任何远程服务器通信，也不包含任何遥测、分析或崩溃报告功能。</p>"

        "<p><b>2. 文件处理</b><br>"
        "您使用 LiteText 打开和保存的文件完全存储在您本地的设备上。"
        "本软件不会以任何形式上传、分享或分析您的文件内容。</p>"

        "<p><b>3. 日志与错误报告</b><br>"
        "本软件不生成任何用户日志，也不自动收集任何错误报告。"
        "如果发生程序崩溃，所有相关的错误信息仅会本地存储于 Qt 的调试输出中，"
        "LiteText 不会自动上传这些信息。</p>"

        "<p><b>4. 第三方服务</b><br>"
        "LiteText 不使用任何网络服务或第三方 API，也不嵌入任何第三方跟踪工具或分析 SDK。</p>"

        "<p><b>5. 开源许可证与第三方组件</b><br>"
        "LiteText 是基于 Qt 6.11.0 框架使用 C++17 语言开发的。Qt 框架由 The Qt Company Ltd. 及其贡献者版权所有，"
        "根据 <a href='https://www.gnu.org/licenses/lgpl-3.0.html'>GNU 宽通用公共许可证 (LGPL) 版本 3</a> 提供。"
        "本软件在开发过程中动态链接到 Qt 库，因此 LGPL 版本 3 的条款适用于 Qt 库本身，"
        "但不强制要求公开本软件自身的源代码。关于本软件所包含的第三方组件的完整许可信息，"
        "请参阅本软件的“关于”对话框。</p>"

        "<p><b>6. 隐私政策的变更</b><br>"
        "由于 LiteText 不收集任何用户数据，因此该隐私政策的任何变更都不会对您的隐私造成实质性影响。"
        "如有重大变更（例如新增网络功能），会通过软件更新时的发布说明提前通知您。</p>"

        "<p><b>7. 联系我们</b><br>"
        "如果您对本隐私政策有任何疑问，请通过 LiteText 的开源代码仓库提交 Issue 或联系作者 Momster。</p>"

        "<p><i>最后更新: 2026年5月</i></p>"
    );
    contentLabel->setWordWrap(true);
    contentLabel->setOpenExternalLinks(true);
    mainLayout->addWidget(contentLabel);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *closeButton = new QPushButton("关闭");
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    dialog.exec();
}

void MainWindow::updateHighlighterForFile(const QString &filePath)
{
    QString suffix;
    QFileInfo info(filePath);
    QString fileName = info.fileName();

    if (fileName == ".gitignore") {
        suffix = "gitignore";
    } else {
        suffix = info.suffix().toLower();
    }

    if (suffix.isEmpty())
        suffix = "txt";

    if (suffix == "c" || suffix == "cc" || suffix == "cxx" || suffix == "cpp" || suffix == "h" || suffix == "hpp")
        suffix = "cpp";
    else if (suffix == "py")
        suffix = "py";
    else if (suffix == "js" || suffix == "mjs")
        suffix = "js";
    else if (suffix == "html" || suffix == "htm")
        suffix = "html";
    else if (suffix == "java")
        suffix = "java";
    else if (suffix == "css")
        suffix = "css";
    else if (suffix == "xml")
        suffix = "xml";
    else if (suffix == "properties")
        suffix = "properties";
    else if (suffix == "ini")
        suffix = "ini";
    else if (suffix == "yaml" || suffix == "yml")
        suffix = "yaml";
    else if (suffix == "json")
        suffix = "json";
    else if (suffix == "gitignore")
        suffix = "gitignore";
    else
        suffix = "txt";

    m_highlighter->setLanguage(suffix);
    onLanguageChanged(suffix);
}

void MainWindow::onLanguageChanged(const QString &suffix)
{
    Q_UNUSED(suffix);
    QStringList keywords = m_highlighter->getCurrentKeywords();
    m_editor->setKeywordList(keywords);
    m_editor->setCurrentLanguage(suffix);
}

void MainWindow::updateCursorPosition()
{
    updateStatusBar();
}

static QString formatFileSize(qint64 size)
{
    if (size < 1024)
        return QString::number(size) + " B";
    else if (size < 1024 * 1024)
        return QString::number(size / 1024.0, 'f', 2) + " KB";
    else
        return QString::number(size / (1024.0 * 1024.0), 'f', 2) + " MB";
}

static QString formatCharCount(int count)
{
    if (count < 10000) {
        return QString::number(count) + "字";
    }
    double w = count / 10000.0;
    if (qAbs(w - qRound(w)) < 0.05) {
        return QString::number(qRound(w)) + "w字";
    } else {
        return QString::number(w, 'f', 1) + "w字";
    }
}

void MainWindow::updateStatusBar()
{
    QTextDocument *doc = m_editor->document();
    if (!doc) return;

    int totalChars = doc->characterCount();
    int totalLines = doc->blockCount();

    QTextCursor cursor = m_editor->textCursor();
    int selectedChars = cursor.selectedText().length();

    QString totalFormatted = formatLargeNumber(totalChars, m_largeNumberFormat);
    QString selectedFormatted = formatLargeNumber(selectedChars, m_largeNumberFormat);

    QString wordDisplay;
    if (selectedChars > 0)
        wordDisplay = QString("%1 / %2 字").arg(selectedFormatted).arg(totalFormatted);
    else
        wordDisplay = QString("%1 字").arg(totalFormatted);

    qint64 fileSize = 0;
    if (!m_isUntitled && !m_currentFilePath.isEmpty()) {
        QFileInfo fi(m_currentFilePath);
        if (fi.exists())
            fileSize = fi.size();
    }
    QString sizeText;
    if (m_isUntitled)
        sizeText = "未保存";
    else
        sizeText = formatFileSize(fileSize);

    int tabWidth = m_editor->tabWidth();
    QString tabMode = m_editor->useTabs() ? "Tab:T" : QString("Tab:%1S").arg(tabWidth);

    int line = cursor.blockNumber() + 1;
    int col = cursor.columnNumber() + 1;

    QString statusText = QString("编码: %1 | 大小: %2 | %3 | %4 行 | %5 | %6:%7")
        .arg(m_currentEncoding)
        .arg(sizeText)
        .arg(wordDisplay)
        .arg(totalLines)
        .arg(tabMode)
        .arg(line)
        .arg(col);

    statusBar()->showMessage(statusText);
}

void MainWindow::showGotoDialog()
{
    bool ok;
    QString text = QInputDialog::getText(this, "跳转到位置",
                                         "输入行号 或 行:列 (例如 17 或 17:5):",
                                         QLineEdit::Normal, "", &ok);
    if (!ok || text.isEmpty())
        return;

    int line = -1, column = -1;
    if (text.contains(':')) {
        QStringList parts = text.split(':');
        if (parts.size() == 2) {
            line = parts[0].toInt(&ok);
            if (ok) column = parts[1].toInt(&ok);
        }
    } else {
        line = text.toInt(&ok);
        if (ok) column = 1;
    }

    if (!ok || line < 1) {
        statusBar()->showMessage("无效的行号或格式", 2000);
        return;
    }

    QTextDocument *doc = m_editor->document();
    int totalLines = doc->blockCount();
    if (line > totalLines) {
        statusBar()->showMessage(QString("行号超出范围 (最多 %1 行)").arg(totalLines), 2000);
        return;
    }

    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, line - 1);
    if (column > 1) {
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, column - 1);
    } else {
        cursor.movePosition(QTextCursor::StartOfLine);
    }

    m_editor->setTextCursor(cursor);
    m_editor->ensureCursorVisible();
    statusBar()->showMessage(QString("跳转到 %1:%2").arg(line).arg(column > 0 ? column : 1), 1500);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_editor && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        if (wheelEvent->modifiers() & Qt::ControlModifier) {
            int delta = wheelEvent->angleDelta().y();
            if (delta > 0) {
                m_editor->zoomIn();
            } else if (delta < 0) {
                m_editor->zoomOut();
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::showFindReplaceDialog()
{
    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
}

void MainWindow::showSettingsDialog()
{
    SettingsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        reloadSettings();
        if (m_isUntitled) {
            applyDefaultEncoding();
        }
        updateStatusBar();
    }
}

void MainWindow::onEditorStatusMessage(const QString &msg)
{
    statusBar()->showMessage(msg, 1500);
}

void MainWindow::onTabWidthChanged(int newWidth)
{
    Q_UNUSED(newWidth);
    updateStatusBar();
}

void MainWindow::restartApplication(const QString &fileToOpen)
{
    QString appPath = QCoreApplication::applicationFilePath();
    QStringList arguments;
    if (!fileToOpen.isEmpty()) {
        arguments << fileToOpen;
    }
    QProcess::startDetached(appPath, arguments);
    QCoreApplication::quit();
}

QString MainWindow::formatLargeNumber(int value, int format) const
{
    if (format == 0) return QString::number(value);
    if (format == 1) {
        double v = value / 1000.0;
        if (value < 1000) return QString::number(value);
        return QString::number(v, 'f', 1) + "k";
    }
    if (format == 2) {
        double v = value / 10000.0;
        if (value < 10000) return QString::number(value);
        return QString::number(v, 'f', 1) + "w";
    }
    return QString::number(value);
}

void MainWindow::applyDefaultEncoding()
{
    QSettings settings(configFilePath(), QSettings::IniFormat);
    QString defaultEnc = settings.value("editor/defaultEncoding", "UTF-8").toString();
    m_currentEncoding = defaultEnc;
    updateStatusBar();
}
