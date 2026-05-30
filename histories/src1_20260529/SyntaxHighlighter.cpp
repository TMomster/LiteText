#include "SyntaxHighlighter.h"
#include <QTextDocument>

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
        rehighlight();
    }
}

void SyntaxHighlighter::setupFormats()
{
    // 根据配色方案选择不同的颜色值
    if (m_currentColorScheme == 0) {
        // ========== 方案0：标准 (VS Code Dark+) ==========
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
        // ========== 方案1：Web (GitHub Dark) ==========
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
        // ========== 方案2：鲜艳 ==========
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
    // 数字
    rule.pattern = QRegularExpression("\\b(0x[0-9A-Fa-f]+|\\d+(\\.\\d+)?)\\b");
    rule.format = m_numberFormat;
    m_rules.append(rule);

    // 运算符
    rule.pattern = QRegularExpression("[\\+\\-\\*/%=&|<>!~^?:]+");
    rule.format = m_operatorFormat;
    m_rules.append(rule);
}

void SyntaxHighlighter::addStringRules()
{
    HighlightingRule rule;
    // 双引号字符串
    rule.pattern = QRegularExpression("\"(\\\\.|[^\"\\\\])*\"");
    rule.format = m_stringFormat;
    m_rules.append(rule);

    // 单引号字符
    rule.pattern = QRegularExpression("'(\\\\.|[^'\\\\])*'");
    rule.format = m_charFormat;
    m_rules.append(rule);
}

// ========== C/C++ ==========
void SyntaxHighlighter::setupCppRules()
{
    clearRules();
    // 1. 通用规则（数字、运算符）
    applyCommonRules();

    // 2. 预处理指令
    HighlightingRule rule;
    rule.pattern = QRegularExpression("^\\s*#\\s*[a-zA-Z_][a-zA-Z0-9_]*");
    rule.format = m_preprocessorFormat;
    m_rules.append(rule);

    // -------------------- 标识符规则（从通用到具体）--------------------
    // 变量/普通标识符（小写或下划线开头）
    rule.pattern = QRegularExpression("\\b[a-z_][a-zA-Z0-9_]*\\b");
    rule.format = m_variableFormat;
    m_rules.append(rule);

    // 类名（大写开头）
    rule.pattern = QRegularExpression("\\b[A-Z][a-zA-Z0-9_]*\\b");
    rule.format = m_classNameFormat;
    m_rules.append(rule);

    // 函数调用/定义（后跟 '('）
    rule.pattern = QRegularExpression("\\b([a-zA-Z_][a-zA-Z0-9_]*)(?=\\s*\\()");
    rule.format = m_functionFormat;
    m_rules.append(rule);

    // 宏常量（全大写）
    rule.pattern = QRegularExpression("\\b[A-Z_][A-Z0-9_]*\\b");
    rule.format = m_macroFormat;
    m_rules.append(rule);

    // 基本数据类型关键字
    QStringList primitiveTypes = {
        "int", "char", "short", "long", "float", "double", "void", "bool",
        "wchar_t", "signed", "unsigned"
    };
    rule.pattern = QRegularExpression("\\b(" + primitiveTypes.join("|") + ")\\b");
    rule.format = m_typeFormat;
    m_rules.append(rule);

    // C++ 特有类型
    QStringList cppTypes = {
        "string", "vector", "map", "set", "pair", "unique_ptr", "shared_ptr", "weak_ptr",
        "size_t", "ptrdiff_t", "nullptr_t", "initializer_list"
    };
    rule.pattern = QRegularExpression("\\b(" + cppTypes.join("|") + ")\\b");
    rule.format = m_typeFormat;
    m_rules.append(rule);

    // 控制流和其他关键字
    QStringList keywords = {
        "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor",
        "break", "case", "catch", "class", "compl", "const", "constexpr", "const_cast",
        "continue", "decltype", "default", "delete", "do", "dynamic_cast", "else",
        "enum", "explicit", "export", "extern", "false", "for", "friend", "goto",
        "if", "inline", "mutable", "namespace", "new", "noexcept", "not", "not_eq",
        "nullptr", "operator", "or", "or_eq", "private", "protected", "public",
        "register", "reinterpret_cast", "return", "sizeof", "static", "static_assert",
        "static_cast", "struct", "switch", "template", "this", "thread_local", "throw",
        "true", "try", "typedef", "typeid", "typename", "union", "using", "virtual",
        "volatile", "while", "xor", "xor_eq"
    };
    rule.pattern = QRegularExpression("\\b(" + keywords.join("|") + ")\\b");
    rule.format = m_keywordFormat;
    m_rules.append(rule);

    // 修饰符
    QStringList modifiers = { "public", "private", "protected", "static", "const",
                              "virtual", "override", "final", "explicit", "mutable" };
    rule.pattern = QRegularExpression("\\b(" + modifiers.join("|") + ")\\b");
    rule.format = m_keywordFormat;
    m_rules.append(rule);

    // 字符串和字符（优先级最高，覆盖内部标识符）
    addStringRules();

    // 单行注释（最后，覆盖同一行内已被格式化的区域）
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = m_commentFormat;
    m_rules.append(rule);

    m_commentStartExpression = QRegularExpression("/\\*");
    m_commentEndExpression = QRegularExpression("\\*/");
}

// ========== Java ==========
void SyntaxHighlighter::setupJavaRules()
{
    clearRules();
    applyCommonRules();

    HighlightingRule rule;

    // 标识符规则
    rule.pattern = QRegularExpression("\\b[a-z_][a-zA-Z0-9_]*\\b");
    rule.format = m_variableFormat;
    m_rules.append(rule);

    rule.pattern = QRegularExpression("\\b[A-Z][a-zA-Z0-9_]*\\b");
    rule.format = m_classNameFormat;
    m_rules.append(rule);

    rule.pattern = QRegularExpression("\\b([a-zA-Z_][a-zA-Z0-9_]*)(?=\\s*\\()");
    rule.format = m_functionFormat;
    m_rules.append(rule);

    // 关键字
    QStringList keywords = {
        "abstract", "assert", "boolean", "break", "byte", "case", "catch", "char",
        "class", "const", "continue", "default", "do", "double", "else", "enum",
        "extends", "final", "finally", "float", "for", "goto", "if", "implements",
        "import", "instanceof", "int", "interface", "long", "native", "new",
        "package", "private", "protected", "public", "return", "short", "static",
        "strictfp", "super", "switch", "synchronized", "this", "throw", "throws",
        "transient", "try", "void", "volatile", "while"
    };
    rule.pattern = QRegularExpression("\\b(" + keywords.join("|") + ")\\b");
    rule.format = m_keywordFormat;
    m_rules.append(rule);

    // 类型
    QStringList types = { "int", "float", "double", "char", "void", "boolean",
                          "byte", "short", "long", "String", "Object", "Class",
                          "Integer", "Boolean" };
    rule.pattern = QRegularExpression("\\b(" + types.join("|") + ")\\b");
    rule.format = m_typeFormat;
    m_rules.append(rule);

    // 字符串和字符
    addStringRules();

    // 单行注释
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = m_commentFormat;
    m_rules.append(rule);

    m_commentStartExpression = QRegularExpression("/\\*");
    m_commentEndExpression = QRegularExpression("\\*/");
}

// ========== Python ==========
void SyntaxHighlighter::setupPythonRules()
{
    clearRules();
    applyCommonRules();

    HighlightingRule rule;

    // 装饰器
    rule.pattern = QRegularExpression("@[a-zA-Z_][a-zA-Z0-9_]*");
    rule.format = m_preprocessorFormat;
    m_rules.append(rule);

    // 标识符
    rule.pattern = QRegularExpression("\\b[a-z_][a-zA-Z0-9_]*\\b");
    rule.format = m_variableFormat;
    m_rules.append(rule);

    rule.pattern = QRegularExpression("\\b[A-Z][a-zA-Z0-9_]*\\b");
    rule.format = m_classNameFormat;
    m_rules.append(rule);

    rule.pattern = QRegularExpression("\\b([a-zA-Z_][a-zA-Z0-9_]*)(?=\\s*\\()");
    rule.format = m_functionFormat;
    m_rules.append(rule);

    // 内建函数/类型
    QStringList builtins = {
        "print", "len", "str", "int", "float", "list", "dict", "tuple", "set",
        "range", "enumerate", "zip", "map", "filter", "sorted", "open", "type",
        "isinstance", "issubclass", "super", "property", "staticmethod", "classmethod",
        "__init__", "__str__", "__repr__"
    };
    rule.pattern = QRegularExpression("\\b(" + builtins.join("|") + ")\\b");
    rule.format = m_typeFormat;
    m_rules.append(rule);

    // 关键字
    QStringList keywords = {
        "False", "None", "True", "and", "as", "assert", "async", "await", "break",
        "class", "continue", "def", "del", "elif", "else", "except", "finally",
        "for", "from", "global", "if", "import", "in", "is", "lambda", "nonlocal",
        "not", "or", "pass", "raise", "return", "try", "while", "with", "yield"
    };
    rule.pattern = QRegularExpression("\\b(" + keywords.join("|") + ")\\b");
    rule.format = m_keywordFormat;
    m_rules.append(rule);

    // 字符串（单行双引号/单引号）
    addStringRules();

    // 单行注释
    rule.pattern = QRegularExpression("#[^\n]*");
    rule.format = m_commentFormat;
    m_rules.append(rule);

    // 三引号字符串的多行匹配规则
    m_tripleQuoteStartExpression = QRegularExpression("'''|\"\"\"");
    m_tripleQuoteEndExpression = QRegularExpression("'''|\"\"\"");
}

// ========== JavaScript ==========
void SyntaxHighlighter::setupJavaScriptRules()
{
    clearRules();
    applyCommonRules();

    HighlightingRule rule;

    // 标识符
    rule.pattern = QRegularExpression("\\b[a-z_][a-zA-Z0-9_]*\\b");
    rule.format = m_variableFormat;
    m_rules.append(rule);

    rule.pattern = QRegularExpression("\\b([a-zA-Z_][a-zA-Z0-9_]*)(?=\\s*\\()");
    rule.format = m_functionFormat;
    m_rules.append(rule);

    // 关键字
    QStringList keywords = {
        "abstract", "arguments", "await", "boolean", "break", "byte", "case",
        "catch", "char", "class", "const", "continue", "debugger", "default",
        "delete", "do", "double", "else", "enum", "eval", "export", "extends",
        "false", "final", "finally", "float", "for", "function", "goto", "if",
        "implements", "import", "in", "instanceof", "int", "interface", "let",
        "long", "native", "new", "null", "package", "private", "protected",
        "public", "return", "short", "static", "super", "switch", "synchronized",
        "this", "throw", "throws", "transient", "true", "try", "typeof", "var",
        "void", "volatile", "while", "with", "yield"
    };
    rule.pattern = QRegularExpression("\\b(" + keywords.join("|") + ")\\b");
    rule.format = m_keywordFormat;
    m_rules.append(rule);

    // 字符串
    addStringRules();

    // 单行注释
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = m_commentFormat;
    m_rules.append(rule);

    m_commentStartExpression = QRegularExpression("/\\*");
    m_commentEndExpression = QRegularExpression("\\*/");
}

// ========== HTML (修复括号颜色) ==========
void SyntaxHighlighter::setupHtmlRules()
{
    clearRules();
    applyCommonRules();
    addStringRules();

    HighlightingRule rule;

    // 匹配整个标签（包括尖括号），自闭合标签也支持
    rule.pattern = QRegularExpression("<[/]?[a-zA-Z_][a-zA-Z0-9_]*[^>]*>");
    rule.format = m_htmlTagFormat;
    m_rules.append(rule);

    // 属性名（等号前的一部分）
    rule.pattern = QRegularExpression("\\b[a-zA-Z_][a-zA-Z0-9_-]*\\s*=");
    rule.format = m_htmlAttributeFormat;
    m_rules.append(rule);

    // HTML 注释
    rule.pattern = QRegularExpression("<!--[^-]*(-[^-]+)*-->");
    rule.format = m_commentFormat;
    m_rules.append(rule);
}

// ========== XML (直接复用 HTML 规则) ==========
void SyntaxHighlighter::setupXmlRules()
{
    setupHtmlRules();  // XML 语法与 HTML 类似，直接复用
}

// ========== CSS ==========
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

    // 注释（最后）
    m_commentStartExpression = QRegularExpression("/\\*");
    m_commentEndExpression = QRegularExpression("\\*/");
}

// ========== .gitignore ==========
void SyntaxHighlighter::setupGitignoreRules()
{
    clearRules();
    HighlightingRule rule;
    // 注释行
    rule.pattern = QRegularExpression("^\\s*#.*");
    rule.format = m_commentFormat;
    m_rules.append(rule);
    // 其他行保持默认（无额外高亮）
}

// ========== .properties ==========
void SyntaxHighlighter::setupPropertiesRules()
{
    clearRules();
    HighlightingRule rule;

    // 注释： # 或 !
    rule.pattern = QRegularExpression("^\\s*[#!].*");
    rule.format = m_commentFormat;
    m_rules.append(rule);

    // 键值对：键部分（等号或冒号前的非空白）
    rule.pattern = QRegularExpression("^\\s*([^#!\\s=:][^=:]*?)\\s*[=:]");
    rule.format = m_keywordFormat;   // 将键名高亮为关键字色
    m_rules.append(rule);
}

// ========== .ini ==========
void SyntaxHighlighter::setupIniRules()
{
    clearRules();
    HighlightingRule rule;

    // 节标题：[section]
    rule.pattern = QRegularExpression("^\\s*\\[[^\\]]+\\]");
    rule.format = m_classNameFormat;   // 使用类名颜色
    m_rules.append(rule);

    // 注释： ; 或 #
    rule.pattern = QRegularExpression("^\\s*[;#].*");
    rule.format = m_commentFormat;
    m_rules.append(rule);

    // 键值对：键部分
    rule.pattern = QRegularExpression("^\\s*([^;#=]+)\\s*=");
    rule.format = m_keywordFormat;
    m_rules.append(rule);
}

// ========== YAML ==========
void SyntaxHighlighter::setupYamlRules()
{
    clearRules();
    HighlightingRule rule;

    // 注释
    rule.pattern = QRegularExpression("#.*");
    rule.format = m_commentFormat;
    m_rules.append(rule);

    // 键（后面跟着冒号和空格或换行）
    rule.pattern = QRegularExpression("^\\s*[a-zA-Z_][a-zA-Z0-9_/-]*\\s*(?=:)");
    rule.format = m_keywordFormat;
    m_rules.append(rule);

    // 布尔值、null 等常量
    QStringList constants = {"true", "false", "yes", "no", "on", "off", "null", "Null", "NULL", "~"};
    rule.pattern = QRegularExpression("\\b(" + constants.join("|") + ")\\b");
    rule.format = m_constantFormat;
    m_rules.append(rule);

    // 数字（整数、浮点、科学计数）
    rule.pattern = QRegularExpression("\\b[-+]?\\d+\\.?\\d*(?:[eE][-+]?\\d+)?\\b");
    rule.format = m_numberFormat;
    m_rules.append(rule);

    // 字符串（双引号、单引号）
    addStringRules();

    // 列表项（以 - 开头，后面跟空格）
    rule.pattern = QRegularExpression("^\\s*-\\s+");
    rule.format = m_operatorFormat;
    m_rules.append(rule);

    // 锚点和别名
    rule.pattern = QRegularExpression("&[a-zA-Z_][a-zA-Z0-9_]*|\\*[a-zA-Z_][a-zA-Z0-9_]*");
    rule.format = m_preprocessorFormat;
    m_rules.append(rule);

    // 块标量指示符（|, >）
    rule.pattern = QRegularExpression("[|>][-+]?");
    rule.format = m_operatorFormat;
    m_rules.append(rule);
}

void SyntaxHighlighter::setupPlainRules()
{
    clearRules();
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
    else
        setupPlainRules();
    rehighlight();
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    // 先应用所有单行规则（现在字符串规则优先级最高，因为后添加）
    for (const HighlightingRule &rule : m_rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // 处理多行注释（C/C++/Java/JS/CSS）
    if (m_currentLanguage == "cpp" || m_currentLanguage == "java" ||
        m_currentLanguage == "js" || m_currentLanguage == "css") {
        setCurrentBlockState(0);
        int startIndex = 0;
        if (previousBlockState() != 1)
            startIndex = text.indexOf(m_commentStartExpression);

        while (startIndex >= 0) {
            QRegularExpressionMatch endMatch = m_commentEndExpression.match(text, startIndex);
            int endIndex = endMatch.capturedStart();
            int commentLength;
            if (endIndex == -1) {
                setCurrentBlockState(1);
                commentLength = text.length() - startIndex;
            } else {
                commentLength = endIndex - startIndex + endMatch.capturedLength();
            }
            setFormat(startIndex, commentLength, m_multiLineCommentFormat);
            startIndex = text.indexOf(m_commentStartExpression, startIndex + commentLength);
        }
    }

    // 处理 Python 三引号字符串（跨行）
    if (m_currentLanguage == "py") {
        setCurrentBlockState(0);
        int startIndex = 0;
        if (previousBlockState() != 1)
            startIndex = text.indexOf(m_tripleQuoteStartExpression);

        while (startIndex >= 0) {
            QRegularExpressionMatch endMatch = m_tripleQuoteEndExpression.match(text, startIndex + 3);
            int endIndex = endMatch.capturedStart();
            int stringLength;
            if (endIndex == -1) {
                setCurrentBlockState(1);
                stringLength = text.length() - startIndex;
            } else {
                stringLength = endIndex - startIndex + endMatch.capturedLength();
            }
            setFormat(startIndex, stringLength, m_tripleQuoteFormat);
            startIndex = text.indexOf(m_tripleQuoteStartExpression, startIndex + stringLength);
        }
    }
}
