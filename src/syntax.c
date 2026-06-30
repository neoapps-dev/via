#include <string.h>
#include <ctype.h>
#include "syntax.h"
#include "theme.h"
static const char *c_keywords[] = {
    "auto", "break", "case", "const", "continue", "default", "do",
    "else", "enum", "extern", "for", "goto", "if", "inline",
    "register", "return", "signed", "sizeof", "static", "struct",
    "switch", "typedef", "union", "unsigned", "volatile", "while",
    NULL
};

static const char *c_types[] = {
    "bool", "char", "double", "float", "int", "long", "short",
    "size_t", "ssize_t", "void",
    "int8_t", "int16_t", "int32_t", "int64_t",
    "uint8_t", "uint16_t", "uint32_t", "uint64_t",
    "FILE", "NULL", "off_t", "time_t", "pid_t",
    NULL
};

static const char *cpp_keywords[] = {
    "auto", "break", "case", "catch", "class", "const", "constexpr",
    "continue", "decltype", "default", "delete", "do", "else",
    "enum", "explicit", "export", "extern", "for", "friend", "goto",
    "if", "inline", "mutable", "namespace", "new", "noexcept",
    "operator", "override", "private", "protected", "public",
    "register", "return", "signed", "sizeof", "static", "struct",
    "switch", "template", "this", "throw", "try", "typedef",
    "typeid", "typename", "union", "unsigned", "using", "virtual",
    "volatile", "while",
    NULL
};

static const char *cpp_types[] = {
    "bool", "char", "double", "float", "int", "long", "short",
    "size_t", "ssize_t", "void", "wchar_t",
    "int8_t", "int16_t", "int32_t", "int64_t",
    "uint8_t", "uint16_t", "uint32_t", "uint64_t",
    "string", "vector", "map", "set", "pair", "shared_ptr",
    "unique_ptr", "weak_ptr", "function", "initializer_list",
    "FILE", "NULL", "nullptr", "true", "false",
    NULL
};

static const char *python_keywords[] = {
    "and", "as", "assert", "async", "await", "break", "class",
    "continue", "def", "del", "elif", "else", "except",
    "finally", "for", "from", "global", "if", "import", "in",
    "is", "lambda", "nonlocal", "not", "or", "pass",
    "raise", "return", "try", "while", "with", "yield",
    NULL
};

static const char *python_types[] = {
    "True", "False", "None", "self",
    "int", "float", "bool", "str", "bytes", "list", "dict",
    "tuple", "set", "frozenset", "type", "object",
    NULL
};

static const char *rust_keywords[] = {
    "as", "async", "await", "break", "const", "continue", "crate",
    "dyn", "else", "enum", "extern", "false", "fn", "for",
    "if", "impl", "in", "let", "loop", "match", "mod",
    "move", "mut", "pub", "ref", "return", "self", "Self",
    "static", "struct", "super", "trait", "true", "type",
    "unsafe", "use", "where", "while", "yield",
    NULL
};

static const char *rust_types[] = {
    "bool", "char", "f32", "f64", "i8", "i16", "i32", "i64",
    "isize", "str", "String", "u8", "u16", "u32", "u64",
    "usize", "Vec", "Option", "Result", "Box", "HashMap",
    "None", "Some", "Ok", "Err",
    NULL
};

static const char *js_keywords[] = {
    "async", "await", "break", "case", "catch", "class", "const",
    "continue", "debugger", "default", "delete", "do", "else",
    "export", "extends", "finally", "for", "function", "if",
    "import", "in", "instanceof", "let", "new", "of",
    "return", "static", "super", "switch", "this", "throw",
    "try", "typeof", "var", "void", "while", "with", "yield",
    NULL
};

static const char *js_types[] = {
    "true", "false", "null", "undefined",
    "Number", "String", "Boolean", "Object", "Array",
    "Function", "RegExp", "Date", "Promise", "Error",
    "Map", "Set", "WeakMap", "WeakSet", "Symbol",
    "Infinity", "NaN", "BigInt",
    NULL
};

static const char *go_keywords[] = {
    "break", "case", "chan", "const", "continue", "default", "defer",
    "else", "fallthrough", "for", "func", "go", "goto", "if",
    "import", "interface", "map", "package", "range", "return",
    "select", "struct", "switch", "type", "var",
    NULL
};

static const char *go_types[] = {
    "bool", "byte", "complex64", "complex128",
    "error", "float32", "float64",
    "int", "int8", "int16", "int32", "int64",
    "rune", "string",
    "uint", "uint8", "uint16", "uint32", "uint64",
    "uintptr", "nil", "true", "false",
    NULL
};

static const char *java_keywords[] = {
    "abstract", "assert", "break", "case", "catch", "class",
    "const", "continue", "default", "do", "else", "enum",
    "extends", "final", "finally", "for", "goto", "if",
    "implements", "import", "instanceof", "interface",
    "native", "new", "package", "private", "protected",
    "public", "return", "static", "strictfp", "super",
    "switch", "synchronized", "this", "throw", "throws",
    "transient", "try", "void", "volatile", "while",
    NULL
};

static const char *java_types[] = {
    "boolean", "byte", "char", "double", "float",
    "int", "long", "short",
    "String", "Object", "Class", "System",
    "null", "true", "false",
    NULL
};

typedef struct {
    const char *name;
    const char **keywords;
    const char **types;
    const char *line_comment;
    const char *block_start;
    const char *block_end;
    bool has_preproc;
} Lang;

static const Lang langs[] = {
    {"c",        c_keywords, c_types,     "//", "/*", "*/", true},
    {"cpp",      cpp_keywords, cpp_types, "//", "/*", "*/", true},
    {"python",   python_keywords, python_types, "#",  NULL, NULL, false},
    {"rust",     rust_keywords, rust_types, "//", "/*", "*/", false},
    {"javascript", js_keywords, js_types, "//", "/*", "*/", false},
    {"go",       go_keywords, go_types,   "//", "/*", "*/", false},
    {"java",     java_keywords, java_types, "//", "/*", "*/", false},
    {"sh",       NULL, NULL, "#",  NULL, NULL, false},
    {"ruby",     NULL, NULL, "#",  NULL, NULL, false},
    {"lua",      NULL, NULL, "--", NULL, NULL, false},
    {"sql",      NULL, NULL, "--", "/*", "*/", false},
    {"yaml",     NULL, NULL, "#",  NULL, NULL, false},
    {"toml",     NULL, NULL, "#",  NULL, NULL, false},
    {"make",     NULL, NULL, "#",  NULL, NULL, false},
    {"markdown", NULL, NULL, NULL, NULL, NULL, false},
};

static const Lang *find_lang(const char *ft)
{
    if (!ft || !ft[0]) return NULL;
    int n = sizeof(langs) / sizeof(langs[0]);
    for (int i = 0; i < n; i++) {
        if (strcmp(ft, langs[i].name) == 0) return &langs[i];
    }
    return NULL;
}

const char *syn_detect(const char *filename)
{
    if (!filename) return "";
    if (filename == "Makefile") return "make";
    const char *ext = strrchr(filename, '.');
    if (!ext) return "";
    ext++;
    if (strcmp(ext, "c") == 0 || strcmp(ext, "h") == 0) return "c";
    if (strcmp(ext, "cpp") == 0 || strcmp(ext, "hpp") == 0 ||
        strcmp(ext, "cc") == 0 || strcmp(ext, "hh") == 0 ||
        strcmp(ext, "cxx") == 0 || strcmp(ext, "hxx") == 0) return "cpp";
    if (strcmp(ext, "py") == 0) return "python";
    if (strcmp(ext, "rs") == 0) return "rust";
    if (strcmp(ext, "js") == 0 || strcmp(ext, "jsx") == 0 ||
        strcmp(ext, "ts") == 0 || strcmp(ext, "tsx") == 0 ||
        strcmp(ext, "mjs") == 0 || strcmp(ext, "cjs") == 0) return "javascript";
    if (strcmp(ext, "go") == 0) return "go";
    if (strcmp(ext, "java") == 0) return "java";
    if (strcmp(ext, "sh") == 0 || strcmp(ext, "bash") == 0) return "sh";
    if (strcmp(ext, "rb") == 0) return "ruby";
    if (strcmp(ext, "lua") == 0) return "lua";
    if (strcmp(ext, "sql") == 0) return "sql";
    if (strcmp(ext, "yaml") == 0 || strcmp(ext, "yml") == 0) return "yaml";
    if (strcmp(ext, "toml") == 0) return "toml";
    if (strcmp(ext, "mk") == 0) return "make";
    if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0) return "html";
    if (strcmp(ext, "css") == 0) return "css";
    if (strcmp(ext, "json") == 0) return "javascript";
    if (strcmp(ext, "xml") == 0 || strcmp(ext, "svg") == 0) return "xml";
    if (strcmp(ext, "php") == 0) return "php";
    if (strcmp(ext, "md") == 0 || strcmp(ext, "markdown") == 0) return "markdown";
    return "";
}

#define ADD_SPAN(o, l, e) do { \
    if (count < max) { spans[count].offset = (o); spans[count].length = (l); spans[count].elem = (e); count++; } \
} while(0)

int syn_highlight(const char *line, int len, const char *ft, SynSpan *spans, int max, SynState *state)
{
    if (!line || len <= 0 || !ft || !ft[0] || max <= 0) return 0;
    const Lang *lang = find_lang(ft);
    if (!lang) return 0;
    int count = 0;
    int i = 0;
    int lc_len = lang->line_comment ? (int)strlen(lang->line_comment) : 0;
    int bs_len = lang->block_start ? (int)strlen(lang->block_start) : 0;
    int be_len = lang->block_end ? (int)strlen(lang->block_end) : 0;
    if (state && state->in_block && be_len > 0) {
        int end = 0;
        bool closed = false;
        while (end < len) {
            if (strncmp(line + end, lang->block_end, (size_t)be_len) == 0) {
                end += be_len;
                closed = true;
                break;
            }
            end++;
        }
        if (closed) {
            ADD_SPAN(0, end, VIA_COMMENT);
            state->in_block = false;
            i = end;
        } else {
            ADD_SPAN(0, len, VIA_COMMENT);
            return count;
        }
    }

    if (i == 0 && lang->has_preproc) {
        int j = 0;
        while (j < len && (line[j] == ' ' || line[j] == '\t')) j++;
        if (j < len && line[j] == '#') {
            ADD_SPAN(0, len, VIA_PREPROC);
            return count;
        }
    }

    if (strcmp(ft, "markdown") == 0) {
        int j = 0;
        while (j < len && (line[j] == ' ' || line[j] == '\t')) j++;
        if (j < len && line[j] == '#') {
            ADD_SPAN(0, len, VIA_KEYWORD);
            return count;
        }
        if (j < len && line[j] == '>') {
            ADD_SPAN(0, len, VIA_COMMENT);
            return count;
        }
        {
            bool hr = true;
            int hc = 0;
            for (int k = 0; k < len; k++) {
                char c = line[k];
                if (c == ' ' || c == '\t') continue;
                if (c == '-' || c == '*' || c == '_') { hc++; continue; }
                hr = false;
                break;
            }
            if (hr && hc >= 3) {
                ADD_SPAN(0, len, VIA_COMMENT);
                return count;
            }
        }
    }

    while (i < len) {
        if (lc_len > 0 && i + lc_len <= len &&
            strncmp(line + i, lang->line_comment, (size_t)lc_len) == 0) {
            ADD_SPAN(i, len - i, VIA_COMMENT);
            break;
        }

        if (bs_len > 0 && be_len > 0 && i + bs_len <= len &&
            strncmp(line + i, lang->block_start, (size_t)bs_len) == 0) {
            int start = i;
            i += bs_len;
            int end = i;
            bool closed = false;
            while (end < len) {
                if (strncmp(line + end, lang->block_end, (size_t)be_len) == 0) {
                    end += be_len;
                    closed = true;
                    break;
                }
                end++;
            }
            if (closed) {
                ADD_SPAN(start, end - start, VIA_COMMENT);
                i = end;
            } else {
                ADD_SPAN(start, len - start, VIA_COMMENT);
                if (state) state->in_block = true;
                break;
            }
            continue;
        }

        if (line[i] == '"' || line[i] == '\'' || line[i] == '`') {
            int delim = (unsigned char)line[i];
            int start = i;
            i++;
            while (i < len) {
                if (line[i] == '\\' && i + 1 < len)
                    i += 2;
                else if (line[i] == delim) {
                    i++;
                    break;
                } else
                    i++;
            }
            if (i >= len) i = len;
            ADD_SPAN(start, i - start, VIA_STRING);
            continue;
        }

        if (isdigit((unsigned char)line[i])) {
            int start = i;
            i++;
            while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '.')) i++;
            ADD_SPAN(start, i - start, VIA_NUMBER);
            continue;
        }

        if (isalpha((unsigned char)line[i]) || line[i] == '_') {
            int start = i;
            i++;
            while (i < len && (isalnum((unsigned char)line[i]) || line[i] == '_')) i++;
            int word_len = i - start;
            int elem = VIA_NORMAL;
            if (lang->keywords) {
                for (int k = 0; lang->keywords[k]; k++) {
                    int kw_len = (int)strlen(lang->keywords[k]);
                    if (word_len == kw_len &&
                        strncmp(line + start, lang->keywords[k], (size_t)kw_len) == 0) {
                        elem = VIA_KEYWORD;
                        break;
                    }
                }
            }

            if (elem == VIA_NORMAL && lang->types) {
                for (int k = 0; lang->types[k]; k++) {
                    int t_len = (int)strlen(lang->types[k]);
                    if (word_len == t_len &&
                        strncmp(line + start, lang->types[k], (size_t)t_len) == 0) {
                        elem = VIA_TYPE;
                        break;
                    }
                }
            }

            if (elem != VIA_NORMAL) ADD_SPAN(start, word_len, elem);
            continue;
        }

        i++;
    }

    return count;
}
