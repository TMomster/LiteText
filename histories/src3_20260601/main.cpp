#include <QApplication>
#include <QIcon>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFileInfo>
#include <QTranslator>
#include <QDir>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTranslator translator;
    if (translator.load("qt_zh_CN.qm")) {
        app.installTranslator(&translator);
    }
    app.setApplicationName("LiteText");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Momster");

    app.setWindowIcon(QIcon(":/icon.ico"));

    // 确保配置目录存在（MainWindow 内部也会做，但先做一次可避免后续问题）
    QString configDir = QDir::homePath() + "/MomsterTech/LiteText";
    QDir dir;
    if (!dir.exists(configDir))
        dir.mkpath(configDir);

    QCommandLineParser parser;
    parser.setApplicationDescription("轻量级代码编辑器");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "要打开的文件路径", "[file]");
    parser.process(app);

    QStringList files = parser.positionalArguments();

    MainWindow window;
    if (!files.isEmpty()) {
        QString filePath = files.first();
        QFileInfo fileInfo(filePath);
        if (fileInfo.exists())
            window.openFileFromPath(fileInfo.absoluteFilePath());
        else
            window.openFileFromPath(filePath);
    }
    window.show();

    return app.exec();
}