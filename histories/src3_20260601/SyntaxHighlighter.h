#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>
#include <QStringList>

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit SyntaxHighlighter(QTextDocument *parent = nullptr);
    ~SyntaxHighlighter();

    void setLanguage(const QString &suffix);
    QString currentLanguage() const { return m_currentLanguage; }
    void setColorScheme(int scheme);
    QStringList getCurrentKeywords() const;

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> m_rules;

    QRegularExpression m_commentStartExpression;
    QRegularExpression m_commentEndExpression;
    QTextCharFormat m_multiLineCommentFormat;

    QRegularExpression m_tripleQuoteStartExpression;
    QRegularExpression m_tripleQuoteEndExpression;
    QTextCharFormat m_tripleQuoteFormat;

    QTextCharFormat m_keywordFormat;
    QTextCharFormat m_typeFormat;
    QTextCharFormat m_constantFormat;
    QTextCharFormat m_functionFormat;
    QTextCharFormat m_methodFormat;
    QTextCharFormat m_stringFormat;
    QTextCharFormat m_charFormat;
    QTextCharFormat m_numberFormat;
    QTextCharFormat m_commentFormat;
    QTextCharFormat m_preprocessorFormat;
    QTextCharFormat m_operatorFormat;
    QTextCharFormat m_htmlTagFormat;
    QTextCharFormat m_htmlAttributeFormat;
    QTextCharFormat m_cssSelectorFormat;
    QTextCharFormat m_cssPropertyFormat;
    QTextCharFormat m_cssValueFormat;
    QTextCharFormat m_variableFormat;
    QTextCharFormat m_classNameFormat;
    QTextCharFormat m_macroFormat;

    QString m_currentLanguage;
    int m_currentColorScheme;
    QStringList m_currentKeywords;

    void setupFormats();
    void clearRules();
    void applyCommonRules();
    void addStringRules();
    void setupCppRules();
    void setupJavaRules();
    void setupPythonRules();
    void setupJavaScriptRules();
    void setupHtmlRules();
    void setupCssRules();
    void setupPlainRules();
    void setupXmlRules();
    void setupGitignoreRules();
    void setupPropertiesRules();
    void setupIniRules();
    void setupYamlRules();
    void setupJsonRules();          // 新增 JSON 高亮
};

#endif // SYNTAXHIGHLIGHTER_H
