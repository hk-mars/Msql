

#ifndef __CHAR_H__
#define __CHAR_H__
 
#define SPACE ' '
#define DOUBLE_QUOTE '"'
#define PERCENT '%'
#define AMPERSAND '&'
#define QUOTE '\''
#define LEFT_PAREN '('
#define RIGHT_PAREN ')'
#define ASTERISK '*'
#define PLUSIGN '+'
#define COMMA ','
#define MINUSIGN '-'
#define PERIOD '.'
#define SOLIDUS '/'
#define COLON ':'
#define SEMICOLON ';'
#define LS_OP '<'
#define EQ_OP '='
#define GT_OP '>'
#define QUESTION_MARK '?'
#define UNDERSCORE '_'
#define VERTICAL_BAR '|'
#define LEFT_BRACKET '['
#define RIGHT_BRACKET ']'

#define is_latin_ch(c) \
(((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))

#define is_digit_ch(c) ((c) >= '0' && (c) <= '9')

#define is_special_ch(c) \
( \
((c) == DOUBLE_QUOTE) || \
((c) == PERCENT) || \
((c) == AMPERSAND) || \
((c) == QUOTE) || \
((c) == LEFT_PAREN) || \
((c) == RIGHT_PAREN) || \
((c) == ASTERISK) || \
((c) == PLUSIGN) || \
((c) == COMMA) || \
((c) == MINUSIGN) || \
((c) == PERIOD) || \
((c) == SOLIDUS) || \
((c) == COLON) || \
((c) == SEMICOLON) || \
((c) == LS_OP) || \
((c) == EQ_OP) || \
((c) == GT_OP) || \
((c) == QUESTION_MARK) || \
((c) == UNDERSCORE) || \
((c) == VERTICAL_BAR) || \
((c) == LEFT_BRACKET) || \
((c) == RIGHT_BRACKET) \
)

#endif /* __CHAR_H__ */


