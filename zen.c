// Important C data structures
// 1. Strechy buffer
// 2. Pointer/uintptr hash table
// 3. String intern

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#define _CRT_SECURE_NO_WARNINGS
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// error
static char err_buf[1024*1024];
// fatal error
void fatal(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("FATAL: ");
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
    exit(1);
}
void syntax_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("SYNTAX ERROR: ");
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

//strechy buffer
typedef struct bufHdr {
    size_t len;
    size_t cap;
    char buf[0];
} bufHdr; 

#define buf__hdr(b) ((bufHdr *)((char *)(b) - offsetof(bufHdr, buf))) // return buffer header
#define buf__fits(b, n) (buf_len(b) + (n) <= buf_cap(b)) // check if buffer fits
#define buf__fit(b, n) (buf__fits(b, n) ? 0 : ((b) = buf__grow((b),buf_len(b)+n,sizeof((*b))))) // grow buffer if needed

#define buf_len(b) ((b)? buf__hdr(b)->len:0) // return buffer length 
#define buf_cap(b) ((b)? buf__hdr(b)->cap:0) // return buffer capacity
// #define buf_push(b, x) (buf__fit((b), 1), (b)[buf__hdr(b)->len++] = (x)) // push element to buffer
// idk how it is fixed now, thanks to chatgpt
#define buf_push(b, ...) (buf__fit((b), 1), (b)[buf__hdr(b)->len++] = (__VA_ARGS__))
#define buf_free(b) ((b) ? free(buf__hdr(b)), (b) = NULL : 0) // free buffer

void *buf__grow(const void *buf, size_t new_len, size_t elem_size) {
    size_t new_cap = MAX(1 + 2 * buf_cap(buf), new_len);
    size_t new_size = offsetof(bufHdr, buf) + new_cap * elem_size;
    bufHdr *new_hdr;
    if (buf) {
        new_hdr = realloc(buf__hdr(buf), new_size);
    } else {
        new_hdr = malloc(new_size);
        new_hdr->len = 0;
    }
    new_hdr->cap = new_cap;
    return new_hdr->buf;
}
void buf_test() {
    int *buf = NULL;
    assert(buf_len(buf) == 0);
    enum { N = 1024 };
    for (int i = 0; i < N; i++) {
        buf_push(buf, i);
    }
    assert(buf_len(buf) == N);
    for (int i = 0; i < buf_len(buf); i++) {
        assert(buf[i] == i);
    }
    buf_free(buf);
    assert(buf == NULL);
    assert(buf_len(buf) == 0);
}

// hash table
typedef struct internStr{
    size_t len;
    const char *str;
} internStr;

static internStr *interns;

const char *str_intern_r(const char *start,const char *end){
    size_t len=end-start;
    for(size_t i=0;i<buf_len(interns);i++){
        if(interns[i].len==len && strncmp(interns[i].str, start, len)==0){
            return interns[i].str;
        }
    }
    char *str=malloc(len+1);
    memcpy(str,start,len);
    str[len]=0;
    buf_push(interns, (internStr){len, str});
    const char *ret=str;
    return ret;
}
const char *str_intern(const char *str){
    return str_intern_r(str, str+strlen(str));
}

void str_intern_test(){
    char x[]="hello";
    char y[]="hello";
    assert(x!=y);
    const char *s1=str_intern(x);
    const char *s2=str_intern(y);

    assert(s1==s2);
    const char *s3=str_intern("hello");
    assert(s3==s1);
}
// lexing
typedef enum TokenKind {
    // Reserve first 128 values for one-char tokens
    TOKEN_LAST_CHAR = 127,
    TOKEN_INT,
    TOKEN_NAME,
} TokenKind;



// Warning: this returns a pointer to a static buffer, so it will be overwritten by the next call to this function.
const char *token_kind_name(TokenKind kind){
    static char buf[256];
    switch (kind){
        case TOKEN_INT:
            sprintf(buf,"TOKEN_INT");
            break;
        case TOKEN_NAME:
            sprintf(buf,"TOKEN_NAME");
            break;
        default:
            if (kind<128 && isprint(kind)){
                sprintf(buf,"'%c'",kind);
            }
            else{
                sprintf(buf,"<ASCII %d>",kind);
            }
            break;
    }
    return buf;
}

typedef struct Token{
    TokenKind kind; 
    const char *start;
    const char *end;
    union{
        uint64_t int_val;
        const char *name;
    };
} Token;

Token token;
const char *stream;


const char *keyword_if;
const char *keyword_for;
const char *keyword_while;

void init_keywords(){
    keyword_if=str_intern("if");
    keyword_for=str_intern("for");
    keyword_while=str_intern("while");
}
void parse_statement(){
    if(token.kind==TOKEN_NAME){
        if(token.name==keyword_if){
            //...
        }else if(token.name==keyword_for){
            //...
        }else if(token.name==keyword_while){
            //...
        }else{
            //...
        }
    }
}

// designated initializers
uint8_t char_to_digit[256] = {
    ['0'] = 0,
    ['1'] = 1,
    ['2'] = 2,
    ['3'] = 3,
    ['4'] = 4,
    ['5'] = 5,
    ['6'] = 6,
    ['7'] = 7,
    ['8'] = 8,
    ['9'] = 9,
    ['a'] = 10, ['A'] = 10,
    ['b'] = 11, ['B'] = 11,
    ['c'] = 12, ['C'] = 12,
    ['d'] = 13, ['D'] = 13,
    ['e'] = 14, ['E'] = 14,
    ['f'] = 15, ['F'] = 15,
};

uint64_t scan_int(){
    uint64_t base=10;
    if(*stream==0){
        stream++;
        if(tolower(*stream)=='x'){
            stream++;
            base=16;
        }   
        else{
            syntax_error("invalid integer literal",*stream);
            stream++;       
        }
    }
    uint64_t val=0;
    for(;;){
        int digit=*stream++ -'0';
        if(digit==0 && *stream!='0'){
            break;
        }
        if(digit>=base){
            syntax_error("Digit '%c' out of range for base %lu",*stream,base);
            stream;
        }
        if(val>=(INT64_MAX-digit)/base){
            syntax_error("intefer literal overflow");
            while(isdigit(*stream)) stream++;
            val=0;
        }
        assert(val<= (INT64_MAX-digit)/10);
        val=val*base+digit;
    }
    return val;
}

void next_token(){
    token.start=stream;
    switch (*stream)
    {
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        token.kind=TOKEN_INT;
        token.int_val=scan_int();
        break;

    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z': case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z': case '_':
        const char *start=stream++;
        while(isalnum(*stream) || *stream=='_'){
            *stream++;
        }
        token.kind=TOKEN_NAME;
        token.name=str_intern_r(start,stream);
        break;
    default:
        token.kind=*stream++;
        break;
    }
    token.end=stream;
}

char output[1024*1024];
void init_stream(const char *str) {
    stream = str;
    next_token();
}

void print_token(Token token){
    switch (token.kind)
    {
    case TOKEN_INT:
        sprintf(output,"TOKEN_INT:%lu\n",token.int_val);
        break;
    case TOKEN_NAME:
        int length=(int)(token.end-token.start);
        sprintf(output,"TOKEN_NAME:%.*s\n",length,token.start);
        break;
    default:
        sprintf(output,"TOKEN:%c\n",token.kind);
        break;
    }
}

static inline bool is_token(TokenKind kind){
    return token.kind==kind;
}

static inline bool is_token_name(const char *name){
    return token.kind==TOKEN_NAME && token.name==name;
}

static inline bool match_token(TokenKind kind){
    if(is_token(kind)){
        next_token();
        return true;
    }
    return false;
}

static inline bool expect_token(TokenKind kind){
    if(is_token(kind)){
        next_token();
        return true;
    }
    else{
        fatal("expected token %c, got %c\n",kind,token.kind);
        return false;
    }
}

#define assert_token(x) assert(match_token(x))
#define assert_token_name(x) assert(token.name == str_intern(x) && match_token(TOKEN_NAME))
#define assert_token_int(x) assert(token.int_val == (x) && match_token(TOKEN_INT))
#define assert_token_eof() assert(is_token(0))

void lex_test(){
    const char *str = "XY+(XY)_HELLO1,234+994";
    init_stream(str);
    assert_token_name("XY");
    assert_token('+');
    assert_token('(');
    assert_token_name("XY");
    assert_token(')');
    assert_token_name("_HELLO1");
    assert_token(',');
    assert_token_int(234);
    assert_token('+');
    assert_token_int(994);
    assert_token_eof();
    // make sure Uint64_max doesn't trigger overflow
    init_stream("18446744073709551615");
    assert_token_int(18446744073709551615ull);


}

#undef assert_token_eof
#undef assert_token_int
#undef assert_token_name
#undef assert_token


// The functions for the parser in particular have been picked up from github 

#if 0
    expr3 = INT | '(' expr ')' 
    expr2 = '-' expr2 | expr3
    expr1 = expr2 ([*/] expr2)*
    expr0 = expr1 ([+-] expr1)*
    expr = expr0
#endif

int parse_expr();

int parse_expr3() {
    if (is_token(TOKEN_INT)) {
        int val = token.int_val;
        next_token();
        return val;
    } else if (match_token('(')) {
        int val = parse_expr();
        expect_token(')');
        return val;
    } else {
        fatal("expected integer or (, got %s", token_kind_name(token.kind));
        return 0;
    }
}

int parse_expr2() {
    if (match_token('-')) {
        return -parse_expr2();
    } else if (match_token('+')) {
        return parse_expr2();
    } else {
        return parse_expr3();
    }
}

int parse_expr1() {
    int val = parse_expr2();
    while (is_token('*') || is_token('/')) {
        char op = token.kind;
        next_token();
        int rval = parse_expr2();
        if (op == '*') {
            val *= rval;
        } else {
            assert(op == '/');
            assert(rval != 0);
            val /= rval;
        }
    }
    return val;
}

int parse_expr0() {
    int val = parse_expr1();
    while (is_token('+') || is_token('-')) {
        char op = token.kind;
        next_token();
        int rval = parse_expr1();
        if (op == '+') {
            val += rval;
        } else {
            assert(op == '-');
            val -= rval;
        }
    }
    return val;
}

int parse_expr() {
    return parse_expr0();
}

int parse_expr_str(const char *str) {
    init_stream(str);
    return parse_expr();
}

#define TEST_EXPR(x) assert(parse_expr_str(#x) == (x))

void parse_test() {
    TEST_EXPR(1);
    TEST_EXPR((1));
    TEST_EXPR(-+1);
    TEST_EXPR(1-2-3);
    TEST_EXPR(2*3+4*5);
    TEST_EXPR(2*(3+4)*5);
    TEST_EXPR(2+-3);
}

#undef TEST_EXPR

int main(int argc,char **argv) {
    buf_test();
    lex_test();
    str_intern_test();
    parse_test();
    return 0;
}
