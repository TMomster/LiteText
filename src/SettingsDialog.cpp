#include "SettingsDialog.h"
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFontDatabase>
#include <QSettings>
#include <QDir>
#include <QGroupBox>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("设置");
    setMinimumSize(480, 500);

    QVBoxLayout *layout = new QVBoxLayout(this);

    // 字体
    QGroupBox *fontGroup = new QGroupBox("字体");
    QHBoxLayout *fontLayout = new QHBoxLayout(fontGroup);
    fontLayout->addWidget(new QLabel("字体:"));
    m_fontCombo = new QComboBox;
    m_fontCombo->addItems(QFontDatabase().families());
    fontLayout->addWidget(m_fontCombo);
    fontLayout->addWidget(new QLabel("字号:"));
    m_fontSizeSpin = new QSpinBox;
    m_fontSizeSpin->setRange(6, 72);
    fontLayout->addWidget(m_fontSizeSpin);
    layout->addWidget(fontGroup);

    // 自动换行
    QGroupBox *wrapGroup = new QGroupBox("自动换行");
    QHBoxLayout *wrapLayout = new QHBoxLayout(wrapGroup);
    m_wordWrapCheck = new QCheckBox("启用自动换行 (仅影响显示，不改变行号)");
    wrapLayout->addWidget(m_wordWrapCheck);
    layout->addWidget(wrapGroup);

    // Tab键行为
    QGroupBox *tabGroup = new QGroupBox("Tab键行为");
    QHBoxLayout *tabLayout = new QHBoxLayout(tabGroup);
    tabLayout->addWidget(new QLabel("Tab 空格数:"));
    m_tabWidthSpin = new QSpinBox;
    m_tabWidthSpin->setRange(1, 8);
    tabLayout->addWidget(m_tabWidthSpin);
    tabLayout->addStretch();
    m_useTabsCheck = new QCheckBox("插入制表符 (否则插入空格)");
    tabLayout->addWidget(m_useTabsCheck);
    layout->addWidget(tabGroup);

    // 配色方案
    QGroupBox *schemeGroup = new QGroupBox("配色方案");
    QHBoxLayout *schemeLayout = new QHBoxLayout(schemeGroup);
    schemeLayout->addWidget(new QLabel("配色:"));
    m_colorSchemeCombo = new QComboBox;
    m_colorSchemeCombo->addItems({"标准", "Web", "鲜艳"});
    schemeLayout->addWidget(m_colorSchemeCombo);
    layout->addWidget(schemeGroup);

    // 智慧联想
    QGroupBox *completionGroup = new QGroupBox("智慧联想");
    QVBoxLayout *completionLayout = new QVBoxLayout(completionGroup);
    m_autoCompletionCheck = new QCheckBox("启用智慧联想");
    completionLayout->addWidget(m_autoCompletionCheck);
    QHBoxLayout *keyLayout = new QHBoxLayout;
    keyLayout->addWidget(new QLabel("采纳建议的快捷键:"));
    m_acceptKeyCombo = new QComboBox;
    m_acceptKeyCombo->addItem("Tab", Qt::Key_Tab);
    m_acceptKeyCombo->addItem("Enter", Qt::Key_Enter);
    m_acceptKeyCombo->addItem("Return", Qt::Key_Return);
    m_acceptKeyCombo->addItem("Space", Qt::Key_Space);
    keyLayout->addWidget(m_acceptKeyCombo);
    completionLayout->addLayout(keyLayout);
    layout->addWidget(completionGroup);

    // 文件编码
    QGroupBox *encodingGroup = new QGroupBox("文件编码");
    QHBoxLayout *encodingLayout = new QHBoxLayout(encodingGroup);
    encodingLayout->addWidget(new QLabel("默认编码:"));
    m_encodingCombo = new QComboBox;
    m_encodingCombo->addItems({"UTF-8", "UTF-8-BOM", "GBK", "ANSI"});
    encodingLayout->addWidget(m_encodingCombo);
    layout->addWidget(encodingGroup);

    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_okButton = new QPushButton("确定");
    m_cancelButton = new QPushButton("取消");
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    layout->addLayout(buttonLayout);

    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    loadSettings();
}

SettingsDialog::~SettingsDialog() {}

QString SettingsDialog::configFilePath() const
{
    return QDir::homePath() + "/MomsterTech/LiteText/config.ini";
}

void SettingsDialog::loadSettings()
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
    QString encoding = settings.value("editor/defaultEncoding", "UTF-8").toString();

    int idx = m_fontCombo->findText(fontFamily);
    if (idx >= 0) m_fontCombo->setCurrentIndex(idx);
    m_fontSizeSpin->setValue(fontSize);
    m_colorSchemeCombo->setCurrentIndex(colorScheme);
    m_tabWidthSpin->setValue(tabWidth);
    m_useTabsCheck->setChecked(useTabs);
    m_wordWrapCheck->setChecked(wordWrap);
    m_autoCompletionCheck->setChecked(autoCompletion);
    int keyIdx = m_acceptKeyCombo->findData(acceptKey);
    if (keyIdx >= 0) m_acceptKeyCombo->setCurrentIndex(keyIdx);
    int encIdx = m_encodingCombo->findText(encoding);
    if (encIdx >= 0) m_encodingCombo->setCurrentIndex(encIdx);
}

void SettingsDialog::saveSettings()
{
    QSettings settings(configFilePath(), QSettings::IniFormat);
    settings.setValue("editor/fontFamily", m_fontCombo->currentText());
    settings.setValue("editor/fontSize", m_fontSizeSpin->value());
    settings.setValue("editor/colorScheme", m_colorSchemeCombo->currentIndex());
    settings.setValue("editor/tabWidth", m_tabWidthSpin->value());
    settings.setValue("editor/useTabs", m_useTabsCheck->isChecked());
    settings.setValue("editor/wordWrap", m_wordWrapCheck->isChecked());
    settings.setValue("editor/autoCompletion", m_autoCompletionCheck->isChecked());
    settings.setValue("editor/completionAcceptKey", m_acceptKeyCombo->currentData().toInt());
    settings.setValue("editor/defaultEncoding", m_encodingCombo->currentText());
}

int SettingsDialog::getTabWidth() const { return m_tabWidthSpin->value(); }
bool SettingsDialog::getUseTabs() const { return m_useTabsCheck->isChecked(); }
QFont SettingsDialog::getSelectedFont() const { return QFont(m_fontCombo->currentText(), m_fontSizeSpin->value()); }
int SettingsDialog::getFontSize() const { return m_fontSizeSpin->value(); }
bool SettingsDialog::isWordWrapEnabled() const { return m_wordWrapCheck->isChecked(); }
bool SettingsDialog::isAutoCompletionEnabled() const { return m_autoCompletionCheck->isChecked(); }
int SettingsDialog::getCompletionAcceptKey() const { return m_acceptKeyCombo->currentData().toInt(); }
int SettingsDialog::getColorScheme() const { return m_colorSchemeCombo->currentIndex(); }
QString SettingsDialog::getDefaultEncoding() const { return m_encodingCombo->currentText(); }

void SettingsDialog::accept()
{
    saveSettings();
    QDialog::accept();
}