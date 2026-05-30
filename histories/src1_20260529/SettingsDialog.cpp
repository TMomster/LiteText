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

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("设置");
    setFixedSize(400, 280);

    QVBoxLayout *layout = new QVBoxLayout(this);

    // 字体行
    QHBoxLayout *fontLayout = new QHBoxLayout;
    fontLayout->addWidget(new QLabel("字体:"));
    m_fontCombo = new QComboBox;
    m_fontCombo->addItems(QFontDatabase().families());
    fontLayout->addWidget(m_fontCombo);
    layout->addLayout(fontLayout);

    // 字号行
    QHBoxLayout *sizeLayout = new QHBoxLayout;
    sizeLayout->addWidget(new QLabel("字号:"));
    m_fontSizeSpin = new QSpinBox;
    m_fontSizeSpin->setRange(6, 72);
    sizeLayout->addWidget(m_fontSizeSpin);
    layout->addLayout(sizeLayout);

    // Tab 宽度行（空格数量）
    QHBoxLayout *tabLayout = new QHBoxLayout;
    tabLayout->addWidget(new QLabel("Tab 空格数:"));
    m_tabWidthSpin = new QSpinBox;
    m_tabWidthSpin->setRange(1, 8);
    m_tabWidthSpin->setToolTip("按 Tab 键时插入的空格数量（仅在下方选择“插入空格”时生效）");
    tabLayout->addWidget(m_tabWidthSpin);
    layout->addLayout(tabLayout);

    // Tab 行为行
    QHBoxLayout *behaviorLayout = new QHBoxLayout;
    behaviorLayout->addWidget(new QLabel("Tab 行为:"));
    m_useTabsCheck = new QCheckBox("插入制表符（否则插入空格）");
    behaviorLayout->addWidget(m_useTabsCheck);
    layout->addLayout(behaviorLayout);

    // 配色方案行
    QHBoxLayout *schemeLayout = new QHBoxLayout;
    schemeLayout->addWidget(new QLabel("配色方案:"));
    m_colorSchemeCombo = new QComboBox;
    m_colorSchemeCombo->addItems({"标准 (VS Code)", "Web (GitHub)", "鲜艳 (高对比)"});
    schemeLayout->addWidget(m_colorSchemeCombo);
    layout->addLayout(schemeLayout);

    // 按钮行
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
}

void SettingsDialog::saveSettings()
{
    QSettings settings(configFilePath(), QSettings::IniFormat);
    settings.setValue("editor/fontFamily", m_fontCombo->currentText());
    settings.setValue("editor/fontSize", m_fontSizeSpin->value());
    settings.setValue("editor/colorScheme", m_colorSchemeCombo->currentIndex());
    settings.setValue("editor/tabWidth", m_tabWidthSpin->value());
    settings.setValue("editor/useTabs", m_useTabsCheck->isChecked());
}

int SettingsDialog::getTabWidth() const
{
    return m_tabWidthSpin->value();
}

bool SettingsDialog::getUseTabs() const
{
    return m_useTabsCheck->isChecked();
}

QFont SettingsDialog::getSelectedFont() const
{
    return QFont(m_fontCombo->currentText(), m_fontSizeSpin->value());
}

int SettingsDialog::getFontSize() const
{
    return m_fontSizeSpin->value();
}

void SettingsDialog::accept()
{
    saveSettings();
    QDialog::accept();
}
