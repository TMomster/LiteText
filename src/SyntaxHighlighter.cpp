#include "SyntaxHighlighter.h"
#include <QTextDocument>
#include <QPair>
#include <algorithm>

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent), m_currentLanguage("txt"), m_currentColorScheme(0)
{
    setupFormats();
    setupPlainRules();
}

SyntaxHighlighter::~SyntaxHighlighter() {}

void SyntaxHighlighter::setColorScheme(int scheme)
{
    if (m_currentColorScheme != scheme) {
        m_currentColorScheme = scheme;
        setupFormats();
        // 重新加载当前语言规则，使所有规则中的格式副本使用新颜色
        setLanguage(m_currentLanguage);  // 内部会调用 rehighlight()
    }
}

QStringList SyntaxHighlighter::getCurrentKeywords() const
{
    return m_currentKeywords;
}

void SyntaxHighlighter::setupFormats()
{
    if (m_currentColorScheme == 0) {
        m_keywordFormat.setForeground(QColor(0x56, 0x9C, 0xD6));
        m_keywordFormat.setFontWeight(QFont::Bold);
        m_typeFormat.setForeground(QColor(0x4E, 0xC9, 0xB0));
        m_constantFormat.setForeground(QColor(0xC5, 0x86, 0xC0));
        m_functionFormat.setForeground(QColor(0xDC, 0xDC, 0xAA));
        m_methodFormat.setForeground(QColor(0xDC, 0xDC, 0xAA));
        m_stringFormat.setForeground(QColor(0xCE, 0x91, 0x78));
        m_charFormat.setForeground(QColor(0xCE, 0x91, 0x78));
        m_numberFormat.setForeground(QColor(0xB5, 0xCE, 0xA8));
        m_commentFormat.setForeground(QColor(0x6A, 0x99, 0x55));
        m_preprocessorFormat.setForeground(QColor(0xC5, 0x86, 0xC0));
        m_operatorFormat.setForeground(QColor(0xD4, 0xD4, 0xD4));
        m_htmlTagFormat.setForeground(QColor(0x56, 0x9C, 0xD6));
        m_htmlAttributeFormat.setForeground(QColor(0x9C, 0xDC, 0xFE));
        m_cssSelectorFormat.setForeground(QColor(0xD7, 0xBA, 0x7D));
        m_cssPropertyFormat.setForeground(QColor(0x9C, 0xDC, 0xFE));
        m_cssValueFormat.setForeground(QColor(0xB5, 0xCE, 0xA8));
        m_variableFormat.setForeground(QColor(0x9C, 0xDC, 0xFE));
        m_classNameFormat.setForeground(QColor(0x4E, 0xC9, 0xB0));
        m_macroFormat.setForeground(QColor(0xC5, 0x86, 0xC0));
    }
    else if (m_currentColorScheme == 1) {
        m_keywordFormat.setForeground(QColor(0xFF, 0x7B, 0x72));
        m_keywordFormat.setFontWeight(QFont::Bold);
        m_typeFormat.setForeground(QColor(0x79, 0xC0, 0xFF));
        m_constantFormat.setForeground(QColor(0x79, 0xC0, 0xFF));
        m_functionFormat.setForeground(QColor(0xD2, 0xA8, 0xFF));
        m_methodFormat.setForeground(QColor(0xD2, 0xA8, 0xFF));
        m_stringFormat.setForeground(QColor(0xA5, 0xD6, 0xFF));
        m_charFormat.setForeground(QColor(0xA5, 0xD6, 0xFF));
        m_numberFormat.setForeground(QColor(0x79, 0xC0, 0xFF));
        m_commentFormat.setForeground(QColor(0x8B, 0x94, 0x9E));
        m_preprocessorFormat.setForeground(QColor(0x79, 0xC0, 0xFF));
        m_operatorFormat.setForeground(QColor(0xD4, 0xD4, 0xD4));
        m_htmlTagFormat.setForeground(QColor(0xFF, 0x7B, 0x72));
        m_htmlAttributeFormat.setForeground(QColor(0x79, 0xC0, 0xFF));
        m_cssSelectorFormat.setForeground(QColor(0xD2, 0xA8, 0xFF));
        m_cssPropertyFormat.setForeground(QColor(0x79, 0xC0, 0xFF));
        m_cssValueFormat.setForeground(QColor(0x79, 0xC0, 0xFF));
        m_variableFormat.setForeground(QColor(0x9C, 0xDC, 0xFE));
        m_classNameFormat.setForeground(QColor(0x79, 0xC0, 0xFF));
        m_macroFormat.setForeground(QColor(0x79, 0xC0, 0xFF));
    }
    else {
        m_keywordFormat.setForeground(QColor(0x33, 0x99, 0xFF));
        m_keywordFormat.setFontWeight(QFont::Bold);
        m_typeFormat.setForeground(QColor(0xFF, 0xB4, 0x3C));
        m_constantFormat.setForeground(QColor(0xFF, 0x64, 0xC8));
        m_functionFormat.setForeground(QColor(0x64, 0xFF, 0x64));
        m_methodFormat.setForeground(QColor(0x64, 0xFF, 0x64));
        m_stringFormat.setForeground(QColor(0xFF, 0xFF, 0x96));
        m_charFormat.setForeground(QColor(0xFF, 0xFF, 0x96));
        m_numberFormat.setForeground(QColor(0xFF, 0x64, 0xC8));
        m_commentFormat.setForeground(QColor(0x80, 0x80, 0xA0));
        m_preprocessorFormat.setForeground(QColor(0x64, 0xFF, 0xFF));
        m_operatorFormat.setForeground(QColor(0xF8, 0xF8, 0xF2));
        m_htmlTagFormat.setForeground(QColor(0xFF, 0x80, 0x80));
        m_htmlAttributeFormat.setForeground(QColor(0x80, 0xFF, 0x80));
        m_cssSelectorFormat.setForeground(QColor(0xFF, 0xB6, 0xC1));
        m_cssPropertyFormat.setForeground(QColor(0x80, 0xE0, 0xFF));
        m_cssValueFormat.setForeground(QColor(0xFF, 0x64, 0xC8));
        m_variableFormat.setForeground(QColor(0x80, 0xE0, 0xFF));
        m_classNameFormat.setForeground(QColor(0xFF, 0xD7, 0x00));
        m_macroFormat.setForeground(QColor(0x64, 0xFF, 0xFF));
    }
    m_multiLineCommentFormat = m_commentFormat;
    m_tripleQuoteFormat = m_stringFormat;
}

void SyntaxHighlighter::clearRules()
{
    m_rules.clear();
}

void SyntaxHighlighter::applyCommonRules()
{
    HighlightingRule rule;
    rule.pattern = QRegularExpression("\\b(0x[0-9A-Fa-f]+|\\d+(\\.\\d+)?)\\b");
    rule.format = m_numberFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("[\\+\\-\\*/%=&|<>!~^?:]+");
    rule.format = m_operatorFormat;
    m_rules.append(rule);
}

void SyntaxHighlighter::addStringRules()
{
    HighlightingRule rule;
    rule.pattern = QRegularExpression("\"(\\\\.|[^\"\\\\])*\"");
    rule.format = m_stringFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("'(\\\\.|[^'\\\\])*'");
    rule.format = m_charFormat;
    m_rules.append(rule);
}

void SyntaxHighlighter::setupCppRules()
{
    clearRules();
    applyCommonRules();
    HighlightingRule rule;
    rule.pattern = QRegularExpression("^\\s*#\\s*[a-zA-Z_][a-zA-Z0-9_]*");
    rule.format = m_preprocessorFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("\\b[a-z_][a-zA-Z0-9_]*\\b");
    rule.format = m_variableFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("\\b[A-Z][a-zA-Z0-9_]*\\b");
    rule.format = m_classNameFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("\\b([a-zA-Z_][a-zA-Z0-9_]*)(?=\\s*\\()");
    rule.format = m_functionFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("\\b[A-Z_][A-Z0-9_]*\\b");
    rule.format = m_macroFormat;
    m_rules.append(rule);
    QStringList primitiveTypes = {"int","char","short","long","float","double","void","bool","wchar_t","signed","unsigned"};
    rule.pattern = QRegularExpression("\\b(" + primitiveTypes.join("|") + ")\\b");
    rule.format = m_typeFormat;
    m_rules.append(rule);
    QStringList cppTypes = {"string","vector","map","set","pair","unique_ptr","shared_ptr","weak_ptr","size_t","ptrdiff_t","nullptr_t","initializer_list"};
    rule.pattern = QRegularExpression("\\b(" + cppTypes.join("|") + ")\\b");
    rule.format = m_typeFormat;
    m_rules.append(rule);
    QStringList keywords = {"alignas","alignof","and","and_eq","asm","auto","bitand","bitor","break","case","catch","class","compl","const","constexpr","const_cast","continue","decltype","default","delete","do","dynamic_cast","else","enum","explicit","export","extern","false","for","friend","goto","if","inline","mutable","namespace","new","noexcept","not","not_eq","nullptr","operator","or","or_eq","private","protected","public","register","reinterpret_cast","return","sizeof","static","static_assert","static_cast","struct","switch","template","this","thread_local","throw","true","try","typedef","typeid","typename","union","using","virtual","volatile","while","xor","xor_eq"};
    rule.pattern = QRegularExpression("\\b(" + keywords.join("|") + ")\\b");
    rule.format = m_keywordFormat;
    m_rules.append(rule);
    QStringList modifiers = {"public","private","protected","static","const","virtual","override","final","explicit","mutable"};
    rule.pattern = QRegularExpression("\\b(" + modifiers.join("|") + ")\\b");
    rule.format = m_keywordFormat;
    m_rules.append(rule);
    addStringRules();
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = m_commentFormat;
    m_rules.append(rule);
    m_commentStartExpression = QRegularExpression("/\\*");
    m_commentEndExpression = QRegularExpression("\\*/");

    m_currentKeywords = primitiveTypes + cppTypes + keywords + modifiers;
    m_currentKeywords << "QString" << "QObject" << "QWidget" << "QMainWindow" << "QAction" << "QMenu" << "QMenuBar" << "QStatusBar";
}

void SyntaxHighlighter::setupJavaRules()
{
    clearRules();
    applyCommonRules();
    HighlightingRule rule;
    rule.pattern = QRegularExpression("\\b[a-z_][a-zA-Z0-9_]*\\b");
    rule.format = m_variableFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("\\b[A-Z][a-zA-Z0-9_]*\\b");
    rule.format = m_classNameFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("\\b([a-zA-Z_][a-zA-Z0-9_]*)(?=\\s*\\()");
    rule.format = m_functionFormat;
    m_rules.append(rule);
    QStringList keywords = {"abstract","assert","boolean","break","byte","case","catch","char","class","const","continue","default","do","double","else","enum","extends","final","finally","float","for","goto","if","implements","import","instanceof","int","interface","long","native","new","package","private","protected","public","return","short","static","strictfp","super","switch","synchronized","this","throw","throws","transient","try","void","volatile","while"};
    rule.pattern = QRegularExpression("\\b(" + keywords.join("|") + ")\\b");
    rule.format = m_keywordFormat;
    m_rules.append(rule);
    QStringList types = {"int","float","double","char","void","boolean","byte","short","long","String","Object","Class","Integer","Boolean"};
    rule.pattern = QRegularExpression("\\b(" + types.join("|") + ")\\b");
    rule.format = m_typeFormat;
    m_rules.append(rule);
    addStringRules();
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = m_commentFormat;
    m_rules.append(rule);
    m_commentStartExpression = QRegularExpression("/\\*");
    m_commentEndExpression = QRegularExpression("\\*/");
    m_currentKeywords = keywords + types;
}

void SyntaxHighlighter::setupPythonRules()
{
    clearRules();
    applyCommonRules();
    HighlightingRule rule;
    rule.pattern = QRegularExpression("@[a-zA-Z_][a-zA-Z0-9_]*");
    rule.format = m_preprocessorFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("\\b[a-z_][a-zA-Z0-9_]*\\b");
    rule.format = m_variableFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("\\b[A-Z][a-zA-Z0-9_]*\\b");
    rule.format = m_classNameFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("\\b([a-zA-Z_][a-zA-Z0-9_]*)(?=\\s*\\()");
    rule.format = m_functionFormat;
    m_rules.append(rule);
    QStringList builtins = {"print","len","str","int","float","list","dict","tuple","set","range","enumerate","zip","map","filter","sorted","open","type","isinstance","issubclass","super","property","staticmethod","classmethod","__init__","__str__","__repr__"};
    rule.pattern = QRegularExpression("\\b(" + builtins.join("|") + ")\\b");
    rule.format = m_typeFormat;
    m_rules.append(rule);
    QStringList keywords = {"False","None","True","and","as","assert","async","await","break","class","continue","def","del","elif","else","except","finally","for","from","global","if","import","in","is","lambda","nonlocal","not","or","pass","raise","return","try","while","with","yield"};
    rule.pattern = QRegularExpression("\\b(" + keywords.join("|") + ")\\b");
    rule.format = m_keywordFormat;
    m_rules.append(rule);
    addStringRules();
    rule.pattern = QRegularExpression("#[^\n]*");
    rule.format = m_commentFormat;
    m_rules.append(rule);
    m_tripleQuoteStartExpression = QRegularExpression("'''|\"\"\"");
    m_tripleQuoteEndExpression = QRegularExpression("'''|\"\"\"");
    m_currentKeywords = builtins + keywords;
}

void SyntaxHighlighter::setupJavaScriptRules()
{
    clearRules();
    applyCommonRules();
    HighlightingRule rule;
    rule.pattern = QRegularExpression("\\b[a-z_][a-zA-Z0-9_]*\\b");
    rule.format = m_variableFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("\\b([a-zA-Z_][a-zA-Z0-9_]*)(?=\\s*\\()");
    rule.format = m_functionFormat;
    m_rules.append(rule);
    QStringList keywords = {"abstract","arguments","await","boolean","break","byte","case","catch","char","class","const","continue","debugger","default","delete","do","double","else","enum","eval","export","extends","false","final","finally","float","for","function","goto","if","implements","import","in","instanceof","int","interface","let","long","native","new","null","package","private","protected","public","return","short","static","super","switch","synchronized","this","throw","throws","transient","true","try","typeof","var","void","volatile","while","with","yield"};
    rule.pattern = QRegularExpression("\\b(" + keywords.join("|") + ")\\b");
    rule.format = m_keywordFormat;
    m_rules.append(rule);
    addStringRules();
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = m_commentFormat;
    m_rules.append(rule);
    m_commentStartExpression = QRegularExpression("/\\*");
    m_commentEndExpression = QRegularExpression("\\*/");
    m_currentKeywords = keywords;
}

void SyntaxHighlighter::setupHtmlRules()
{
    clearRules();
    applyCommonRules();
    addStringRules();
    HighlightingRule rule;
    rule.pattern = QRegularExpression("<[/]?[a-zA-Z_][a-zA-Z0-9_]*[^>]*>");
    rule.format = m_htmlTagFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("\\b[a-zA-Z_][a-zA-Z0-9_-]*\\s*=");
    rule.format = m_htmlAttributeFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("<!--[^-]*(-[^-]+)*-->");
    rule.format = m_commentFormat;
    m_rules.append(rule);
    m_currentKeywords = QStringList() << "html" << "head" << "body" << "div" << "span" << "p" << "a" << "img" << "script" << "style" << "table" << "tr" << "td" << "th";
}

void SyntaxHighlighter::setupXmlRules()
{
    setupHtmlRules();
    m_currentKeywords = QStringList() << "xml" << "version" << "encoding" << "standalone";
}

void SyntaxHighlighter::setupCssRules()
{
    clearRules();
    applyCommonRules();
    addStringRules();
    HighlightingRule rule;
    rule.pattern = QRegularExpression("([.#]?[a-zA-Z_][a-zA-Z0-9_-]*|\\*)(?=\\s*[{,])");
    rule.format = m_cssSelectorFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("@[a-zA-Z-]+\\s+[a-zA-Z_][a-zA-Z0-9_-]*");
    rule.format = m_cssSelectorFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("[a-zA-Z-]+(?=\\s*:)");
    rule.format = m_cssPropertyFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("\\b(\\d+(\\.\\d+)?(px|em|rem|%|vw|vh|deg|rad|s|ms)?|#[0-9a-fA-F]{3,6}|[a-z]+)\\b");
    rule.format = m_cssValueFormat;
    m_rules.append(rule);
    m_commentStartExpression = QRegularExpression("/\\*");
    m_commentEndExpression = QRegularExpression("\\*/");
    m_currentKeywords = QStringList() << "color" << "background" << "margin" << "padding" << "border" << "font" << "display" << "position" << "top" << "left" << "width" << "height";
}

void SyntaxHighlighter::setupGitignoreRules()
{
    clearRules();
    HighlightingRule rule;
    rule.pattern = QRegularExpression("^\\s*#.*");
    rule.format = m_commentFormat;
    m_rules.append(rule);
    m_currentKeywords.clear();
}

void SyntaxHighlighter::setupPropertiesRules()
{
    clearRules();
    HighlightingRule rule;
    rule.pattern = QRegularExpression("^\\s*[#!].*");
    rule.format = m_commentFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("^\\s*([^#!\\s=:][^=:]*?)\\s*[=:]");
    rule.format = m_keywordFormat;
    m_rules.append(rule);
    m_currentKeywords.clear();
}

void SyntaxHighlighter::setupIniRules()
{
    clearRules();
    HighlightingRule rule;
    rule.pattern = QRegularExpression("^\\s*\\[[^\\]]+\\]");
    rule.format = m_classNameFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("^\\s*[;#].*");
    rule.format = m_commentFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("^\\s*([^;#=]+)\\s*=");
    rule.format = m_keywordFormat;
    m_rules.append(rule);
    m_currentKeywords.clear();
}

void SyntaxHighlighter::setupYamlRules()
{
    clearRules();
    HighlightingRule rule;
    rule.pattern = QRegularExpression("#.*");
    rule.format = m_commentFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("^\\s*[a-zA-Z_][a-zA-Z0-9_/-]*\\s*(?=:)");
    rule.format = m_keywordFormat;
    m_rules.append(rule);
    QStringList constants = {"true","false","yes","no","on","off","null","Null","NULL","~"};
    rule.pattern = QRegularExpression("\\b(" + constants.join("|") + ")\\b");
    rule.format = m_constantFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("\\b[-+]?\\d+\\.?\\d*(?:[eE][-+]?\\d+)?\\b");
    rule.format = m_numberFormat;
    m_rules.append(rule);
    addStringRules();
    rule.pattern = QRegularExpression("^\\s*-\\s+");
    rule.format = m_operatorFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("&[a-zA-Z_][a-zA-Z0-9_]*|\\*[a-zA-Z_][a-zA-Z0-9_]*");
    rule.format = m_preprocessorFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("[|>][-+]?");
    rule.format = m_operatorFormat;
    m_rules.append(rule);
    m_currentKeywords = constants;
}

void SyntaxHighlighter::setupJsonRules()
{
    clearRules();
    applyCommonRules();
    addStringRules();

    HighlightingRule rule;
    // 键名（带引号的字符串后跟冒号）
    rule.pattern = QRegularExpression("\"(\\\\.|[^\"\\\\])*\"(?=\\s*:)");
    rule.format = m_keywordFormat;  // 键名使用关键字颜色
    m_rules.append(rule);

    // 布尔值
    rule.pattern = QRegularExpression("\\b(true|false)\\b");
    rule.format = m_constantFormat;
    m_rules.append(rule);

    // null
    rule.pattern = QRegularExpression("\\bnull\\b");
    rule.format = m_constantFormat;
    m_rules.append(rule);

    // 数字（整数、浮点、科学计数）
    rule.pattern = QRegularExpression("\\b[-+]?\\d+\\.?\\d*(?:[eE][-+]?\\d+)?\\b");
    rule.format = m_numberFormat;
    m_rules.append(rule);

    // 注释（JSON 标准不支持注释，但很多编辑器支持 // 和 /* */，可选）
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = m_commentFormat;
    m_rules.append(rule);
    m_commentStartExpression = QRegularExpression("/\\*");
    m_commentEndExpression = QRegularExpression("\\*/");

    // 补全关键字：JSON 中常用的词汇（可扩展）
    m_currentKeywords = QStringList() << "true" << "false" << "null";
}

void SyntaxHighlighter::setupPlainRules()
{
    clearRules();
    m_currentKeywords.clear();
}

void SyntaxHighlighter::setLanguage(const QString &suffix)
{
    m_currentLanguage = suffix;
    if (suffix == "cpp" || suffix == "c" || suffix == "h" || suffix == "hpp" || suffix == "cc" || suffix == "cxx")
        setupCppRules();
    else if (suffix == "java")
        setupJavaRules();
    else if (suffix == "py")
        setupPythonRules();
    else if (suffix == "js")
        setupJavaScriptRules();
    else if (suffix == "html")
        setupHtmlRules();
    else if (suffix == "xml")
        setupXmlRules();
    else if (suffix == "css")
        setupCssRules();
    else if (suffix == "gitignore")
        setupGitignoreRules();
    else if (suffix == "properties")
        setupPropertiesRules();
    else if (suffix == "ini")
        setupIniRules();
    else if (suffix == "yaml" || suffix == "yml")
        setupYamlRules();
    else if (suffix == "json")
        setupJsonRules();
    else
        setupPlainRules();
    rehighlight();
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    // 第一步：扫描当前块，标记三类区域：字符串、单行注释、多行注释
    enum SegmentType { Normal, String, LineComment, MultiLineComment };
    QVector<QPair<int, SegmentType>> segments; // 每个元素是 (起始位置, 类型)

    int i = 0;
    int n = text.length();
    bool inMultiLine = (previousBlockState() == 1); // 上块未结束的多行注释

    if (inMultiLine) {
        segments.append(qMakePair(0, MultiLineComment));
    }

    while (i < n) {
        // 如果当前已经在多行注释中
        if (!segments.isEmpty() && segments.last().second == MultiLineComment) {
            // 查找结束标志 */
            int endPos = text.indexOf("*/", i);
            if (endPos == -1) {
                // 注释一直延续到块尾
                setCurrentBlockState(1);
                break;
            } else {
                // 注释在本块内结束
                i = endPos + 2;
                segments.pop_back(); // 结束多行注释
                inMultiLine = false;
                continue;
            }
        }

        // 普通状态（不是多行注释）
        QChar ch = text[i];
        if (ch == '"') {
            // 字符串开始：正确解析转义字符
            int start = i;
            i++; // 跳过开始引号
            while (i < n) {
                if (text[i] == '\\') {
                    // 反斜杠转义：跳过下一个字符（无论是什么）
                    i += 2;
                    continue;
                }
                if (text[i] == '"') {
                    // 找到未转义的结束引号
                    i++; // 包含结束引号
                    break;
                }
                i++;
            }
            segments.append(qMakePair(start, String));
        }
        else if (ch == '\'') {
            // 字符常量开始：同样处理转义
            int start = i;
            i++; // 跳过开始单引号
            while (i < n) {
                if (text[i] == '\\') {
                    i += 2;
                    continue;
                }
                if (text[i] == '\'') {
                    i++;
                    break;
                }
                i++;
            }
            segments.append(qMakePair(start, String));
        }
        else if (ch == '/' && i+1 < n && text[i+1] == '/') {
            // 单行注释：从当前到行尾
            segments.append(qMakePair(i, LineComment));
            break; // 后面全是注释，无需继续扫描
        }
        else if (ch == '/' && i+1 < n && text[i+1] == '*') {
            // 多行注释开始
            segments.append(qMakePair(i, MultiLineComment));
            inMultiLine = true;
            i += 2;
            // 继续循环，下次迭代会进入上面的多行注释处理分支
            continue;
        }
        else {
            i++;
        }
    }

    // 第二步：应用字符串和注释格式（覆盖所有规则）
    QTextCharFormat normalFormat;
    setFormat(0, n, normalFormat); // 清除所有格式

    for (const auto &seg : segments) {
        int start = seg.first;
        int length = 0;
        if (seg.second == LineComment) {
            length = n - start;
            setFormat(start, length, m_commentFormat);
        } else if (seg.second == MultiLineComment) {
            if (seg == segments.first() && previousBlockState() == 1) {
                // 可能跨块，从块头开始
                start = 0;
            }
            int endPos = text.indexOf("*/", start);
            if (endPos == -1)
                length = n - start;
            else
                length = endPos - start + 2;
            setFormat(start, length, m_multiLineCommentFormat);
        } else if (seg.second == String) {
            // 字符串 / 字符常量：计算实际长度
            int endPos = start;
            if (text[start] == '"') {
                // 找到匹配的结束引号（已包含在扫描中，这里简单计算）
                endPos = start;
                int j = start + 1;
                while (j < n) {
                    if (text[j] == '\\') j += 2;
                    else if (text[j] == '"') { endPos = j; break; }
                    else j++;
                }
                if (endPos == start) endPos = n - 1;
            } else if (text[start] == '\'') {
                endPos = start;
                int j = start + 1;
                while (j < n) {
                    if (text[j] == '\\') j += 2;
                    else if (text[j] == '\'') { endPos = j; break; }
                    else j++;
                }
                if (endPos == start) endPos = n - 1;
            }
            length = endPos - start + 1;
            setFormat(start, length, m_stringFormat);
        }
    }

    // 第三步：应用普通规则（关键字、数字、运算符等），但要避开字符串和注释区域
    auto isInSpecialRegion = [&](int pos) -> bool {
        for (const auto &seg : segments) {
            int start = seg.first;
            int end = start;
            if (seg.second == LineComment) {
                end = n;
            } else if (seg.second == MultiLineComment) {
                int endPos = text.indexOf("*/", start);
                if (endPos == -1) end = n;
                else end = endPos + 2;
            } else if (seg.second == String) {
                if (text[start] == '"') {
                    int j = start + 1;
                    while (j < n) {
                        if (text[j] == '\\') j += 2;
                        else if (text[j] == '"') { end = j + 1; break; }
                        else j++;
                    }
                    if (end == start) end = n;
                } else if (text[start] == '\'') {
                    int j = start + 1;
                    while (j < n) {
                        if (text[j] == '\\') j += 2;
                        else if (text[j] == '\'') { end = j + 1; break; }
                        else j++;
                    }
                    if (end == start) end = n;
                }
            }
            if (pos >= start && pos < end) return true;
        }
        return false;
    };

    for (const HighlightingRule &rule : m_rules) {
        // 跳过注释规则（已经单独处理）
        if (rule.pattern.pattern() == "//[^\n]*") continue;
        if (rule.pattern.pattern() == "/\\*" || rule.pattern.pattern() == "\\*/") continue;

        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            int start = match.capturedStart();
            int len = match.capturedLength();
            if (!isInSpecialRegion(start)) {
                setFormat(start, len, rule.format);
            }
        }
    }
}