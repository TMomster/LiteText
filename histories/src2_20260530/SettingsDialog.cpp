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
    setFixedSize(450, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QHBoxLayout *fontLayout = new QHBoxLayout;
    fontLayout->addWidget(new QLabel("字体:"));
    m_fontCombo = new QComboBox;
    m_fontCombo->addItems(QFontDatabase().families());
    fontLayout->addWidget(m_fontCombo);
    layout->addLayout(fontLayout);

    QHBoxLayout *sizeLayout = new QHBoxLayout;
    sizeLayout->addWidget(new QLabel("字号:"));
    m_fontSizeSpin = new QSpinBox;
    m_fontSizeSpin->setRange(6, 72);
    sizeLayout->addWidget(m_fontSizeSpin);
    layout->addLayout(sizeLayout);

    QHBoxLayout *tabLayout = new QHBoxLayout;
    tabLayout->addWidget(new QLabel("Tab 空格数:"));
    m_tabWidthSpin = new QSpinBox;
    m_tabWidthSpin->setRange(1, 8);
    tabLayout->addWidget(m_tabWidthSpin);
    layout->addLayout(tabLayout);

    QHBoxLayout *behaviorLayout = new QHBoxLayout;
    behaviorLayout->addWidget(new QLabel("Tab 行为:"));
    m_useTabsCheck = new QCheckBox("插入制表符（否则插入空格）");
    behaviorLayout->addWidget(m_useTabsCheck);
    layout->addLayout(behaviorLayout);

    QHBoxLayout *schemeLayout = new QHBoxLayout;
    schemeLayout->addWidget(new QLabel("配色方案:"));
    m_colorSchemeCombo = new QComboBox;
    m_colorSchemeCombo->addItems({"标准 (VS Code)", "Web (GitHub)", "鲜艳 (高对比)"});
    schemeLayout->addWidget(m_colorSchemeCombo);
    layout->addLayout(schemeLayout);

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

    int index = m_fontCombo->findText(fontFamily);
    if (index >= 0) m_fontCombo->setCurrentIndex(index);
    m_fontSizeSpin->setValue(fontSize);
    m_colorSchemeCombo->setCurrentIndex(colorScheme);
    m_tabWidthSpin->setValue(tabWidth);
    m_useTabsCheck->setChecked(useTabs);

    bool autoCompletion = settings.value("editor/autoCompletion", true).toBool();
    int acceptKey = settings.value("editor/completionAcceptKey", Qt::Key_Tab).toInt();
    m_autoCompletionCheck->setChecked(autoCompletion);
    int keyIndex = m_acceptKeyCombo->findData(acceptKey);
    if (keyIndex >= 0) m_acceptKeyCombo->setCurrentIndex(keyIndex);
}

void SettingsDialog::saveSettings()
{
    QSettings settings(configFilePath(), QSettings::IniFormat);
    settings.setValue("editor/fontFamily", m_fontCombo->currentText());
    settings.setValue("editor/fontSize", m_fontSizeSpin->value());
    settings.setValue("editor/colorScheme", m_colorSchemeCombo->currentIndex());
    settings.setValue("editor/tabWidth", m_tabWidthSpin->value());
    settings.setValue("editor/useTabs", m_useTabsCheck->isChecked());
    settings.setValue("editor/autoCompletion", m_autoCompletionCheck->isChecked());
    settings.setValue("editor/completionAcceptKey", m_acceptKeyCombo->currentData().toInt());
}

int SettingsDialog::getTabWidth() const { return m_tabWidthSpin->value(); }
bool SettingsDialog::getUseTabs() const { return m_useTabsCheck->isChecked(); }
QFont SettingsDialog::getSelectedFont() const { return QFont(m_fontCombo->currentText(), m_fontSizeSpin->value()); }
int SettingsDialog::getFontSize() const { return m_fontSizeSpin->value(); }
bool SettingsDialog::isAutoCompletionEnabled() const { return m_autoCompletionCheck->isChecked(); }
int SettingsDialog::getCompletionAcceptKey() const { return m_acceptKeyCombo->currentData().toInt(); }

void SettingsDialog::accept()
{
    saveSettings();
    QDialog::accept();
}
