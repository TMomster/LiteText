#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QFont>

class QComboBox;
class QSpinBox;
class QPushButton;
class QCheckBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    QFont getSelectedFont() const;
    int getFontSize() const;
    int getTabWidth() const;
    bool getUseTabs() const;

private slots:
    void accept() override;

private:
    QComboBox *m_fontCombo;
    QSpinBox *m_fontSizeSpin;
    QComboBox *m_colorSchemeCombo;
    QSpinBox *m_tabWidthSpin;      // Tab 空格数（仅当不使用制表符时有效）
    QCheckBox *m_useTabsCheck;     // true=插入制表符，false=插入空格
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;

    void loadSettings();
    void saveSettings();
    QString configFilePath() const;
};

#endif // SETTINGSDIALOG_H
