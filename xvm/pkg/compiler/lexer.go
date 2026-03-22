/*
 * XENLY VIRTUAL MACHINE (XVM) - high-performance of the Virtual Machine
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Go programming language.
 *
 * It is available for the Linux, macOS, and Windows operating systems.
 *
 */
/*
 * XENLY - Xenly Virtual Machine (XVM)
 * Compiler: Lexer
 */
package compiler

import (
	"fmt"
	"strconv"
	"strings"
	"unicode"
	"unicode/utf8"
)

// Lexer tokenizes Xenly source code.
type Lexer struct {
	src    string
	pos    int
	line   int
	col    int
	tokens []Token
	errors []string
}

// NewLexer creates a new Lexer for the given source.
func NewLexer(src string) *Lexer {
	return &Lexer{src: src, line: 1, col: 1}
}

// Tokenize lexes the entire source and returns the token stream.
func (l *Lexer) Tokenize() ([]Token, []string) {
	for {
		tok := l.nextToken()
		l.tokens = append(l.tokens, tok)
		if tok.Type == TOK_EOF {
			break
		}
	}
	return l.tokens, l.errors
}

func (l *Lexer) peek() rune {
	if l.pos >= len(l.src) {
		return 0
	}
	r, _ := utf8.DecodeRuneInString(l.src[l.pos:])
	return r
}

func (l *Lexer) peekAt(offset int) rune {
	pos := l.pos + offset
	if pos >= len(l.src) {
		return 0
	}
	r, _ := utf8.DecodeRuneInString(l.src[pos:])
	return r
}

func (l *Lexer) advance() rune {
	r, sz := utf8.DecodeRuneInString(l.src[l.pos:])
	l.pos += sz
	if r == '\n' {
		l.line++
		l.col = 1
	} else {
		l.col++
	}
	return r
}

func (l *Lexer) match(ch rune) bool {
	if l.peek() == ch {
		l.advance()
		return true
	}
	return false
}

func (l *Lexer) skipWhitespaceAndComments() {
	for l.pos < len(l.src) {
		ch := l.peek()
		if ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' {
			l.advance()
		} else if ch == '/' && l.peekAt(1) == '/' {
			// single-line comment
			for l.pos < len(l.src) && l.peek() != '\n' {
				l.advance()
			}
		} else if ch == '/' && l.peekAt(1) == '*' {
			// multi-line comment
			l.advance()
			l.advance()
			for l.pos < len(l.src) {
				if l.peek() == '*' && l.peekAt(1) == '/' {
					l.advance()
					l.advance()
					break
				}
				l.advance()
			}
		} else if ch == '#' {
			// hash comment (shell-style, used in some .xe files)
			for l.pos < len(l.src) && l.peek() != '\n' {
				l.advance()
			}
		} else {
			break
		}
	}
}

func (l *Lexer) makeToken(t TokenType, lexeme string) Token {
	return Token{Type: t, Lexeme: lexeme, Line: l.line, Column: l.col}
}

func (l *Lexer) nextToken() Token {
	l.skipWhitespaceAndComments()
	if l.pos >= len(l.src) {
		return l.makeToken(TOK_EOF, "")
	}

	startLine := l.line
	startCol := l.col
	ch := l.advance()

	mkTok := func(t TokenType, lex string) Token {
		return Token{Type: t, Lexeme: lex, Line: startLine, Column: startCol}
	}

	switch ch {
	case '(':
		return mkTok(TOK_LPAREN, "(")
	case ')':
		return mkTok(TOK_RPAREN, ")")
	case '[':
		return mkTok(TOK_LBRACKET, "[")
	case ']':
		return mkTok(TOK_RBRACKET, "]")
	case '{':
		return mkTok(TOK_LBRACE, "{")
	case '}':
		return mkTok(TOK_RBRACE, "}")
	case ',':
		return mkTok(TOK_COMMA, ",")
	case ';':
		return mkTok(TOK_SEMICOLON, ";")
	case ':':
		return mkTok(TOK_COLON, ":")
	case '~':
		return mkTok(TOK_TILDE, "~")
	case '^':
		return mkTok(TOK_CARET, "^")
	case '.':
		if l.peek() == '.' && l.peekAt(1) == '.' {
			l.advance()
			l.advance()
			return mkTok(TOK_ELLIPSIS, "...")
		}
		return mkTok(TOK_DOT, ".")
	case '+':
		if l.match('+') {
			return mkTok(TOK_INC, "++")
		}
		if l.match('=') {
			return mkTok(TOK_PLUS_EQ, "+=")
		}
		return mkTok(TOK_PLUS, "+")
	case '-':
		if l.match('-') {
			return mkTok(TOK_DEC, "--")
		}
		if l.match('=') {
			return mkTok(TOK_MINUS_EQ, "-=")
		}
		if l.match('>') {
			return mkTok(TOK_ARROW, "->")
		}
		return mkTok(TOK_MINUS, "-")
	case '*':
		if l.match('*') {
			if l.match('=') {
				return mkTok(TOK_STARSTAR_EQ, "**=")
			}
			return mkTok(TOK_STARSTAR, "**")
		}
		if l.match('=') {
			return mkTok(TOK_STAR_EQ, "*=")
		}
		return mkTok(TOK_STAR, "*")
	case '/':
		if l.match('=') {
			return mkTok(TOK_SLASH_EQ, "/=")
		}
		return mkTok(TOK_SLASH, "/")
	case '%':
		if l.match('=') {
			return mkTok(TOK_PERCENT_EQ, "%=")
		}
		return mkTok(TOK_PERCENT, "%")
	case '=':
		if l.match('>') {
			return mkTok(TOK_FAT_ARROW, "=>")
		}
		if l.match('=') {
			return mkTok(TOK_EQ, "==")
		}
		return mkTok(TOK_ASSIGN, "=")
	case '!':
		if l.match('=') {
			return mkTok(TOK_NEQ, "!=")
		}
		return mkTok(TOK_NOT, "!")
	case '<':
		if l.match('<') {
			return mkTok(TOK_SHL, "<<")
		}
		if l.match('=') {
			return mkTok(TOK_LE, "<=")
		}
		return mkTok(TOK_LT, "<")
	case '>':
		if l.match('>') {
			return mkTok(TOK_SHR, ">>")
		}
		if l.match('=') {
			return mkTok(TOK_GE, ">=")
		}
		return mkTok(TOK_GT, ">")
	case '&':
		if l.match('&') {
			return mkTok(TOK_AND, "&&")
		}
		return mkTok(TOK_AMPERSAND, "&")
	case '|':
		if l.match('>') {
			return mkTok(TOK_PIPE_FWD, "|>")
		}
		if l.match('|') {
			return mkTok(TOK_OR, "||")
		}
		return mkTok(TOK_PIPE, "|")
	case '?':
		if l.match('?') {
			return mkTok(TOK_NULLISH, "??")
		}
		return mkTok(TOK_QUESTION, "?")
	case '"', '\'', '`':
		// String literal
		return l.lexString(ch, startLine, startCol)
	}

	// Raw string  r"..."  or  r'...'
	if ch == 'r' && (l.peek() == '"' || l.peek() == '\'' || l.peek() == '`') {
		q := l.advance()
		return l.lexRawString(q, startLine, startCol)
	}

	// Number
	if unicode.IsDigit(ch) || (ch == '0' && (l.peek() == 'x' || l.peek() == 'X')) {
		return l.lexNumber(ch, startLine, startCol)
	}

	// Identifier / keyword
	if ch == '_' || unicode.IsLetter(ch) {
		return l.lexIdent(ch, startLine, startCol)
	}

	l.errors = append(l.errors, fmt.Sprintf("line %d: unexpected character %q", startLine, ch))
	return mkTok(TOK_EOF, "")
}

func (l *Lexer) lexString(quote rune, line, col int) Token {
	var sb strings.Builder
	for l.pos < len(l.src) {
		ch := l.peek()
		if rune(ch) == quote {
			l.advance()
			break
		}
		if ch == '\\' {
			l.advance()
			esc := l.advance()
			switch esc {
			case 'n':
				sb.WriteByte('\n')
			case 't':
				sb.WriteByte('\t')
			case 'r':
				sb.WriteByte('\r')
			case '\\':
				sb.WriteByte('\\')
			case '"':
				sb.WriteByte('"')
			case '\'':
				sb.WriteByte('\'')
			case '`':
				sb.WriteByte('`')
			case '0':
				sb.WriteByte(0)
			default:
				sb.WriteRune('\\')
				sb.WriteRune(esc)
			}
		} else {
			sb.WriteRune(l.advance())
		}
	}
	return Token{Type: TOK_STRING, Lexeme: sb.String(), Line: line, Column: col}
}

func (l *Lexer) lexRawString(quote rune, line, col int) Token {
	var sb strings.Builder
	for l.pos < len(l.src) {
		ch := l.peek()
		if rune(ch) == quote {
			l.advance()
			break
		}
		sb.WriteRune(l.advance())
	}
	return Token{Type: TOK_STRING, Lexeme: sb.String(), Line: line, Column: col}
}

func (l *Lexer) lexNumber(first rune, line, col int) Token {
	var sb strings.Builder
	sb.WriteRune(first)

	// Hex literal
	if first == '0' && (l.peek() == 'x' || l.peek() == 'X') {
		sb.WriteRune(l.advance()) // x
		for isHexDigit(l.peek()) || l.peek() == '_' {
			if l.peek() != '_' {
				sb.WriteRune(l.advance())
			} else {
				l.advance()
			}
		}
		s := sb.String()
		n, err := strconv.ParseInt(s[2:], 16, 64)
		if err != nil {
			n = 0
		}
		return Token{Type: TOK_NUMBER, Lexeme: s, NumVal: float64(n), Line: line, Column: col}
	}

	// Decimal integer or float
	for unicode.IsDigit(l.peek()) || l.peek() == '_' {
		if l.peek() != '_' {
			sb.WriteRune(l.advance())
		} else {
			l.advance()
		}
	}
	if l.peek() == '.' && unicode.IsDigit(l.peekAt(1)) {
		sb.WriteRune(l.advance()) // .
		for unicode.IsDigit(l.peek()) || l.peek() == '_' {
			if l.peek() != '_' {
				sb.WriteRune(l.advance())
			} else {
				l.advance()
			}
		}
	}
	// Exponent
	if l.peek() == 'e' || l.peek() == 'E' {
		sb.WriteRune(l.advance())
		if l.peek() == '+' || l.peek() == '-' {
			sb.WriteRune(l.advance())
		}
		for unicode.IsDigit(l.peek()) {
			sb.WriteRune(l.advance())
		}
	}
	s := sb.String()
	n, _ := strconv.ParseFloat(s, 64)
	return Token{Type: TOK_NUMBER, Lexeme: s, NumVal: n, Line: line, Column: col}
}

func (l *Lexer) lexIdent(first rune, line, col int) Token {
	var sb strings.Builder
	sb.WriteRune(first)
	for {
		ch := l.peek()
		if ch == '_' || unicode.IsLetter(ch) || unicode.IsDigit(ch) {
			sb.WriteRune(l.advance())
		} else {
			break
		}
	}
	s := sb.String()
	if tt, ok := keywords[s]; ok {
		return Token{Type: tt, Lexeme: s, Line: line, Column: col}
	}
	return Token{Type: TOK_IDENT, Lexeme: s, Line: line, Column: col}
}

func isHexDigit(r rune) bool {
	return (r >= '0' && r <= '9') || (r >= 'a' && r <= 'f') || (r >= 'A' && r <= 'F')
}
