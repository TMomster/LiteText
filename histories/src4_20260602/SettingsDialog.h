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
    bool isWordWrapEnabled() const;
    bool isAutoCompletionEnabled() const;
    int getCompletionAcceptKey() const;
    int getColorScheme() const;
    int getLargeNumberFormat() const;  // 0:精确,1:千位(k),2:万位(w)
    QString getDefaultEncoding() const;

private slots:
    void accept() override;

private:
    QComboBox *m_fontCombo;
    QSpinBox *m_fontSizeSpin;
    QComboBox *m_colorSchemeCombo;
    QSpinBox *m_tabWidthSpin;
    QCheckBox *m_useTabsCheck;
    QCheckBox *m_wordWrapCheck;          // 自动换行
    QCheckBox *m_autoCompletionCheck;
    QComboBox *m_acceptKeyCombo;
    QComboBox *m_encodingCombo;          // 文件编码
    QComboBox *m_numberFormatCombo;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;

    void loadSettings();
    void saveSettings();
    QString configFilePath() const;
};

#endif // SETTINGSDIALOG_H