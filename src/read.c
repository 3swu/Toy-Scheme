//
// Created by wulei on 19-3-12.
//
// read & parse

#include "header/read.h"
#include "header/error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

extern int isatty(int fd);

#ifdef HAVE_READLINE
#include <readline/history.h>
#include <readline/readline.h>
#endif

#define MAXSIZE 10240

static object* parse_vector_elements(token_list* list, size_t* length);
static object* parse_character(const char* token_value);
static bool is_str_character(const char* str);

#ifdef HAVE_READLINE
static bool repl_keymap_initialized = false;

static bool cursor_in_string(void) {
    bool in_string = false;
    bool escape_in_string = false;

    for(int i = 0; i < rl_point; i++) {
        char ch = rl_line_buffer[i];

        if(in_string) {
            if(escape_in_string) {
                escape_in_string = false;
            }
            else if(ch == '\\') {
                escape_in_string = true;
            }
            else if(ch == '"') {
                in_string = false;
            }
        }
        else if(ch == '"') {
            in_string = true;
        }
    }

    return in_string;
}

static int insert_pair_text(const char* pair_text) {
    if(rl_insert_text(pair_text) != 0)
        return 0;
    if(rl_point > 0)
        rl_point--;
    rl_redisplay();
    return 0;
}

static int insert_paren_pair(int count, int key) {
    if(cursor_in_string())
        return rl_insert(count, key);

    return insert_pair_text("()");
}

static int smart_close_paren(int count, int key) {
    if(!cursor_in_string() && rl_line_buffer[rl_point] == ')') {
        rl_point++;
        rl_redisplay();
        return 0;
    }
    return rl_insert(count, key);
}

static void ensure_repl_keymap_initialized(void) {
    if(repl_keymap_initialized)
        return;

    rl_bind_key('(', insert_paren_pair);
    rl_bind_key(')', smart_close_paren);
    repl_keymap_initialized = true;
}

static void append_char_with_resize(char** buf, size_t* capacity, int* index, char ch) {
    if((size_t)(*index) >= *capacity - 1) {
        *capacity *= 2;
        *buf = (char*) realloc(*buf, *capacity * sizeof(char));
        if(*buf == NULL)
            error_handle(stderr, "out of memory", EXIT_FAILURE);
    }
    (*buf)[(*index)++] = ch;
}

static char* read_with_readline(void) {
    char* buf = (char*) malloc(MAXSIZE * sizeof(char));
    size_t capacity = MAXSIZE;
    int i = 0;
    int paren_depth = 0;
    bool in_string = false;
    bool escape_in_string = false;
    bool seen_non_whitespace = false;

    if(buf == NULL)
        error_handle(stderr, "out of memory", EXIT_FAILURE);

    ensure_repl_keymap_initialized();

    while(true) {
        const char* prompt = (i == 0) ? "> " : "... ";
        char* line = readline(prompt);

        if(line == NULL) {
            if(!seen_non_whitespace) {
                free(buf);
                return NULL;
            }
            break;
        }

        if(line[0] != '\0')
            add_history(line);

        for(size_t j = 0; line[j] != '\0'; j++) {
            char ch = line[j];
            append_char_with_resize(&buf, &capacity, &i, ch);

            if(!isspace((unsigned char)ch))
                seen_non_whitespace = true;

            if(in_string) {
                if(escape_in_string) {
                    escape_in_string = false;
                }
                else if(ch == '\\') {
                    escape_in_string = true;
                }
                else if(ch == '"') {
                    in_string = false;
                }
            }
            else {
                if(ch == '"')
                    in_string = true;
                else if(ch == '(')
                    paren_depth++;
                else if(ch == ')' && paren_depth > 0)
                    paren_depth--;
            }
        }

        append_char_with_resize(&buf, &capacity, &i, '\n');
        free(line);

        if(!seen_non_whitespace)
            continue;
        if(!in_string && paren_depth == 0)
            break;
    }

    append_char_with_resize(&buf, &capacity, &i, '\0');
    return buf;
}
#endif

char* read_source(FILE* in_stream) { /* from in_stream to buffer */
    char *buf;
    int ch;
    int i = 0;
    size_t capacity = MAXSIZE;

    buf = (char*) malloc(capacity * sizeof(char));
    if(buf == NULL)
        error_handle(stderr, "out of memory", EXIT_FAILURE);

    if(in_stream == stdin) {
#ifdef HAVE_READLINE
        if(isatty(fileno(in_stream)))
            return read_with_readline();
#endif

        int paren_depth = 0;
        bool in_string = false;
        bool seen_non_whitespace = false;

        while((ch = getc(in_stream)) != EOF) {
            if(ch == '\n' && !in_string && paren_depth == 0) {
                if(seen_non_whitespace)
                    break;
                else
                    continue;
            }

            buf[i++] = (char)ch;
            if(!isspace((unsigned char)ch))
                seen_non_whitespace = true;

            if(ch == '"' && (i < 2 || buf[i - 2] != '\\'))
                in_string = !in_string;
            else if(!in_string) {
                if(ch == '(')
                    paren_depth++;
                else if(ch == ')' && paren_depth > 0)
                    paren_depth--;
            }

            if(ch == '\n' && !in_string && paren_depth > 0) {
                printf("... ");
                fflush(stdout);
            }

            if((size_t)i >= capacity - 1) {
                capacity *= 2;
                buf = (char*) realloc(buf, capacity * sizeof(char));
                if(buf == NULL)
                    error_handle(stderr, "out of memory", EXIT_FAILURE);
            }
        }

        if(ch == EOF && !seen_non_whitespace) {
            free(buf);
            return NULL;
        }
    }
    else {
        while((ch = getc(in_stream)) != EOF) {
            buf[i++] = (char)ch;

            if((size_t)i >= capacity - 1) {
                capacity *= 2;
                buf = (char*) realloc(buf, capacity * sizeof(char));
                if(buf == NULL)
                    error_handle(stderr, "out of memory", EXIT_FAILURE);
            }
        }
    }
    buf[i] = '\0';

    return buf;
}

char* buf_pre_handle(char* pre_buf) { /* add space and remove comments */
    char* buf;
    bool in_string = false;
    size_t capacity = MAXSIZE;
    buf = (char*) malloc(capacity * sizeof(char));
    if(buf == NULL)
        error_handle(stderr, "out of memory", EXIT_FAILURE);

    int buf_i = 0, pbuf_i = 0;
    while(pre_buf[pbuf_i] != '\0') {
        if(in_string) {
            buf[buf_i++] = pre_buf[pbuf_i];
            if(pre_buf[pbuf_i] == '"')
                in_string = false;
            pbuf_i++;
            continue;
        }

        switch(pre_buf[pbuf_i]) {
            case '"':
                in_string = true;
                buf[buf_i++] = pre_buf[pbuf_i++];
                break;
            case '(':
                buf[buf_i++] = ' ';
                buf[buf_i++] = '(';
                buf[buf_i++] = ' ';
                pbuf_i++;
                break;
            case ')':
                buf[buf_i++] = ' ';
                buf[buf_i++] = ')';
                buf[buf_i++] = ' ';
                pbuf_i++;
                break;
            case '\'':
                buf[buf_i++] = ' ';
                buf[buf_i++] = '\'';
                buf[buf_i++] = ' ';
                pbuf_i++;
                break;
            case '`':
                buf[buf_i++] = ' ';
                buf[buf_i++] = '`';
                buf[buf_i++] = ' ';
                pbuf_i++;
                break;
            case ',':
                buf[buf_i++] = ' ';
                buf[buf_i++] = ',';
                if(pre_buf[pbuf_i + 1] == '@') {
                    buf[buf_i++] = '@';
                    pbuf_i++;
                }
                buf[buf_i++] = ' ';
                pbuf_i++;
                break;
            case '#':
                if(pre_buf[pbuf_i + 1] == '(') {
                    buf[buf_i++] = ' ';
                    buf[buf_i++] = '#';
                    buf[buf_i++] = '(';
                    buf[buf_i++] = ' ';
                    pbuf_i += 2;
                    break;
                }
                buf[buf_i++] = pre_buf[pbuf_i++];
                break;
            case ';':
                while(pre_buf[pbuf_i] != '\0' && pre_buf[pbuf_i] != '\n')
                    pbuf_i++;
                break;
            case '\n':
                buf[buf_i++] = ' ';
                buf[buf_i++] = '\n';
                pbuf_i++;
                break;
            default:
                buf[buf_i++] = pre_buf[pbuf_i++];
        }

        if((size_t)buf_i >= capacity - 3) {
            capacity *= 2;
            buf = (char*) realloc(buf, capacity * sizeof(char));
            if(buf == NULL)
                error_handle(stderr, "out of memory", EXIT_FAILURE);
        }
    }
    buf[buf_i] = '\0';
    return buf;
}

token* gen_token(char* buf) {
    token* token_list, * token_p; /*init token list with head token*/
    size_t i = 0;
    size_t j = 0;
    size_t len = strlen(buf);

    token_list = (token*) malloc(sizeof(token));
    if(token_list == NULL)
        error_handle(stderr, "out of memory while parse token", EXIT_FAILURE);
    token_list->value = NULL;
    token_list->next = NULL;
    token_p = token_list;

    while(i < len) {
        size_t token_len;
        token* t = (token*) malloc(sizeof(token));
        if(t == NULL)
            error_handle(stderr, "out of memory while parse token", EXIT_FAILURE);

        while(i < len && (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\t' || buf[i] == '\r'))
            i++;
        if(i >= len) {
            free(t);
            break;
        }

        if(buf[i] == '"') {
            j = i + 1;
            while(j < len && buf[j] != '"')
                j++;
            if(j >= len)
                error_handle(stderr, "unterminated string literal\n", EXIT_FAILURE);
            j++;
        }
        else {
            j = i;
            while(j < len &&
                  buf[j] != ' ' &&
                  buf[j] != '\n' &&
                  buf[j] != '\t' &&
                  buf[j] != '\r')
                j++;
        }

        token_len = j - i;
        t->value = (char*) malloc((token_len + 1) * sizeof(char));
        if(t->value == NULL)
            error_handle(stderr, "out of memory while parse token", EXIT_FAILURE);
        t->next = NULL;

        memcpy(t->value, buf + i, token_len);
        t->value[token_len] = '\0';
        i = j;

        token_p->next = t;
        token_p = t;
    }
    return token_list;
}

token_list* gen_token_list(token* token_l) {
    token_list* list = (token_list*) malloc(sizeof(token_list));
    if(list == NULL)
        error_handle(stderr, "out of memory while generate token list", EXIT_FAILURE);

    list->haed_token = token_l;
    list->token_pointer = token_l->next;

    return list;
}

void destroy_token_list(token_list* list) {
    token* current;
    token* next;

    if(list == NULL)
        return;

    current = list->haed_token;
    while(current != NULL) {
        next = current->next;
        if(current->value != NULL)
            free(current->value);
        free(current);
        current = next;
    }
    free(list);
}

object* parse(token_list* list) {
    if(list == NULL || list->token_pointer == NULL)
        error_handle(stderr, "unexpected EOF while reading", EXIT_FAILURE);

    char* token_value = list->token_pointer->value;

    if(strcmp(token_value, "(") == 0) {
        list_iter(list);
        return parse_pair(list);
    }

    if(strcmp(token_value, "#(") == 0) {
        size_t length = 0;
        list_iter(list);
        return make_vector(parse_vector_elements(list, &length), length);
    }

    if(strcmp(token_value, "'") == 0) {
        object* quoted_exp;
        list_iter(list);
        if(list->token_pointer == NULL)
            error_handle(stderr, "quote missing expression\n", EXIT_FAILURE);

        quoted_exp = parse(list);
        return cons(quote_symbol, cons(quoted_exp, the_empty_list));
    }

    if(strcmp(token_value, "`") == 0) {
        object* quoted_exp;
        list_iter(list);
        if(list->token_pointer == NULL)
            error_handle(stderr, "quasiquote missing expression\n", EXIT_FAILURE);

        quoted_exp = parse(list);
        return cons(quasiquote_symbol, cons(quoted_exp, the_empty_list));
    }

    if(strcmp(token_value, ",") == 0) {
        object* quoted_exp;
        list_iter(list);
        if(list->token_pointer == NULL)
            error_handle(stderr, "unquote missing expression\n", EXIT_FAILURE);

        quoted_exp = parse(list);
        return cons(unquote_symbol, cons(quoted_exp, the_empty_list));
    }

    if(strcmp(token_value, ",@") == 0) {
        object* quoted_exp;
        list_iter(list);
        if(list->token_pointer == NULL)
            error_handle(stderr, "unquote-splicing missing expression\n", EXIT_FAILURE);

        quoted_exp = parse(list);
        return cons(unquote_splicing_symbol, cons(quoted_exp, the_empty_list));
    }

    if(strcmp(token_value, "...") == 0) {
        list_iter(list);
        return ellipsis_symbol;
    }

    if(is_str_digit(token_value)) {
        list_iter(list);
        return make_fixnum(atol(token_value));
    }

    if(is_str_character(token_value)) {
        list_iter(list);
        return parse_character(token_value);
    }

    if(is_str_symbol(token_value)) {
        list_iter(list);
        return make_symbol(token_value);
    }

    if(is_str_string(token_value)) {
        list_iter(list);
        return parse_string(token_value);
    }

    if(strcmp(token_value, "#t") == 0) {
        list_iter(list);
        return make_boolean(true);
    }

    if(strcmp(token_value, "#f") == 0) {
        list_iter(list);
        return make_boolean(false);
    }

    char error_msg[TOKEN_MAX + 50];
    sprintf(error_msg, "unexcepted symbol : %s", token_value);
    error_handle(stderr, error_msg, EXIT_FAILURE);
    return NULL;
}

object* parse_pair(token_list* list) {
    if(list->token_pointer == NULL)
        error_handle(stderr, "unexpected EOF while reading list", EXIT_FAILURE);

    if(strcmp(list->token_pointer->value, ")") == 0) {
        list_iter(list);
        return the_empty_list;
    }

    if(strcmp(list->token_pointer->value, ".") == 0) {
        object* tail;
        list_iter(list);
        if(list->token_pointer == NULL)
            error_handle(stderr, "dotted pair missing cdr", EXIT_FAILURE);
        tail = parse(list);
        if(list->token_pointer == NULL || strcmp(list->token_pointer->value, ")") != 0)
            error_handle(stderr, "dotted pair missing closing ')'", EXIT_FAILURE);
        list_iter(list);
        return tail;
    }

    object * car, * cdr;
    car = parse(list);
    cdr = parse_pair(list);
    return cons(car, cdr);
}

bool is_str_digit(char* str) {
    int i = 0;

    if(str == NULL || str[0] == '\0')
        return false;

    if(str[0] == '+' || str[0] == '-') {
        if(str[1] == '\0')
            return false;
        i = 1;
    }

    for(; str[i] != '\0'; i++)
        if(!isdigit((unsigned char)str[i]))
            return false;

    return true;
}

bool is_str_string(char* str) {
    return str[0] == '"' && str[strlen(str) - 1] == '"' ? true : false;
}

static bool is_str_character(const char* str) {
    return str != NULL &&
           str[0] == '#' &&
           str[1] == '\\' &&
           str[2] != '\0';
}

bool is_str_symbol(char* str) {
    char c = str[0];
    if(isalpha(c) ||
       c == '*' ||
       c == '/' ||
       c == '+' ||
       c == '-' ||
       c == '>' ||
       c == '<' ||
       c == '=' ||
       c == '?' ||
       c == '!')
        return true;
    return false;
}

void list_iter(token_list* list) {
    list->token_pointer = list->token_pointer->next;
}

object* parse_string(char* str) {
    size_t str_len = strlen(str);
    char* s = (char*) malloc((str_len - 1) * sizeof(char));
    object* string_obj;
    if(s == NULL)
        error_handle(stderr, "out of memory", EXIT_FAILURE);

    for(size_t i = 0; i < str_len - 2; i++)
        s[i] = str[i + 1];
    s[str_len - 2] = '\0';

    string_obj = make_string(s);
    free(s);
    return string_obj;
}

static object* parse_character(const char* token_value) {
    const char* name = token_value + 2;
    if(strcmp(name, "space") == 0)
        return make_character(' ');
    if(strcmp(name, "newline") == 0)
        return make_character('\n');
    if(name[0] != '\0' && name[1] == '\0')
        return make_character(name[0]);
    error_handle(stderr, "invalid character literal", EXIT_FAILURE);
    return NULL;
}

static object* parse_vector_elements(token_list* list, size_t* length) {
    if(list->token_pointer == NULL)
        error_handle(stderr, "unexpected EOF while reading vector", EXIT_FAILURE);
    if(strcmp(list->token_pointer->value, ")") == 0) {
        list_iter(list);
        return the_empty_list;
    }

    *length += 1;
    return cons(parse(list), parse_vector_elements(list, length));
}

object* reader(FILE* in) {
    while(true) {
        char* pre_buf = read_source(in);
        char* buf;
        token* t;
        token_list* list;
        object* obj;

        if(pre_buf == NULL)
            return NULL;

        buf = buf_pre_handle(pre_buf);
        free(pre_buf);

        t = gen_token(buf);
        free(buf);

//        for(token* p = t; p != NULL; p = p->next)
//            printf("%s\n", p->value);

        list = gen_token_list(t);
        if(list->token_pointer == NULL) {
            destroy_token_list(list);
            if(in == stdin)
                continue;
            return NULL;
        }
        obj = parse(list);
        destroy_token_list(list);
        return obj;
    }
}
