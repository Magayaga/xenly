/*
 * XENLY - Xenly Virtual Machine (XVM)
 * Compiler: Token definitions
 */
package compiler

// TokenType identifies the kind of lexical token.
type TokenType int

const (
	// ── Literals ──────────────────────────────────────────────────────────
	TOK_EOF    TokenType = iota
	TOK_NUMBER           // 42, 3.14, 0xFF
	TOK_STRING           // "hello"
	TOK_IDENT            // myVar

	// ── Keywords ──────────────────────────────────────────────────────────
	TOK_VAR       // var
	TOK_LET       // let
	TOK_CONST     // const
	TOK_FN        // fn
	TOK_RETURN    // return
	TOK_IF        // if
	TOK_ELSE      // else
	TOK_WHILE     // while
	TOK_FOR       // for
	TOK_DO        // do
	TOK_BREAK     // break
	TOK_CONTINUE  // continue
	TOK_IN        // in
	TOK_SWITCH    // switch
	TOK_CASE      // case
	TOK_DEFAULT   // default
	TOK_IMPORT    // import
	TOK_FROM      // from
	TOK_AS        // as
	TOK_EXPORT    // export
	TOK_CLASS     // class
	TOK_EXTENDS   // extends
	TOK_NEW       // new
	TOK_THIS      // this
	TOK_SUPER     // super
	TOK_ASYNC     // async
	TOK_SPAWN     // spawn
	TOK_AWAIT     // await
	TOK_SLEEP     // sleep
	TOK_TYPEOF    // typeof
	TOK_INSTANCEOF // instanceof
	TOK_ENUM      // enum
	TOK_MATCH     // match
	TOK_NAMESPACE // namespace
	TOK_TYPE      // type
	TOK_REQUIRES  // requires
	TOK_ENSURES   // ensures
	TOK_INVARIANT // invariant
	TOK_ASSERT    // assert
	TOK_PRINT     // print
	TOK_INPUT     // input
	TOK_TRUE      // true
	TOK_FALSE     // false
	TOK_NULL      // null

	// ── Operators ─────────────────────────────────────────────────────────
	TOK_PLUS        // +
	TOK_MINUS       // -
	TOK_STAR        // *
	TOK_SLASH       // /
	TOK_PERCENT     // %
	TOK_STARSTAR    // **
	TOK_ASSIGN      // =
	TOK_PLUS_EQ     // +=
	TOK_MINUS_EQ    // -=
	TOK_STAR_EQ     // *=
	TOK_SLASH_EQ    // /=
	TOK_PERCENT_EQ  // %=
	TOK_STARSTAR_EQ // **=
	TOK_EQ          // ==
	TOK_NEQ         // !=
	TOK_LT          // <
	TOK_LE          // <=
	TOK_GT          // >
	TOK_GE          // >=
	TOK_AND         // &&
	TOK_OR          // ||
	TOK_NOT         // !
	TOK_NULLISH     // ??
	TOK_INC         // ++
	TOK_DEC         // --
	TOK_ARROW       // ->
	TOK_FAT_ARROW   // =>
	TOK_DOT         // .
	TOK_COMMA       // ,
	TOK_SEMICOLON   // ;
	TOK_COLON       // :
	TOK_QUESTION    // ?
	TOK_PIPE        // |
	TOK_AMPERSAND   // &
	TOK_CARET       // ^
	TOK_TILDE       // ~
	TOK_SHL         // <<
	TOK_SHR         // >>
	TOK_ELLIPSIS    // ...

	// ── Delimiters ────────────────────────────────────────────────────────
	TOK_LPAREN   // (
	TOK_RPAREN   // )
	TOK_LBRACKET // [
	TOK_RBRACKET // ]
	TOK_LBRACE   // {
	TOK_RBRACE   // }
)

// Token is a single lexical token.
type Token struct {
	Type    TokenType
	Lexeme  string
	NumVal  float64
	Line    int
	Column  int
}

// keywords maps keyword strings to their token types.
var keywords = map[string]TokenType{
	"var":        TOK_VAR,
	"let":        TOK_LET,
	"const":      TOK_CONST,
	"fn":         TOK_FN,
	"return":     TOK_RETURN,
	"if":         TOK_IF,
	"else":       TOK_ELSE,
	"while":      TOK_WHILE,
	"for":        TOK_FOR,
	"do":         TOK_DO,
	"break":      TOK_BREAK,
	"continue":   TOK_CONTINUE,
	"in":         TOK_IN,
	"switch":     TOK_SWITCH,
	"case":       TOK_CASE,
	"default":    TOK_DEFAULT,
	"import":     TOK_IMPORT,
	"from":       TOK_FROM,
	"as":         TOK_AS,
	"export":     TOK_EXPORT,
	"class":      TOK_CLASS,
	"extends":    TOK_EXTENDS,
	"new":        TOK_NEW,
	"this":       TOK_THIS,
	"super":      TOK_SUPER,
	"async":      TOK_ASYNC,
	"spawn":      TOK_SPAWN,
	"await":      TOK_AWAIT,
	"sleep":      TOK_SLEEP,
	"typeof":     TOK_TYPEOF,
	"instanceof": TOK_INSTANCEOF,
	"enum":       TOK_ENUM,
	"match":      TOK_MATCH,
	"namespace":  TOK_NAMESPACE,
	"type":       TOK_TYPE,
	"requires":   TOK_REQUIRES,
	"ensures":    TOK_ENSURES,
	"invariant":  TOK_INVARIANT,
	"assert":     TOK_ASSERT,
	"print":      TOK_PRINT,
	"input":      TOK_INPUT,
	"true":       TOK_TRUE,
	"false":      TOK_FALSE,
	"null":       TOK_NULL,
}
