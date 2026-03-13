/*
 * XENLY - Xenly Virtual Machine (XVM)
 * Compiler: Recursive-descent parser
 */
package compiler

import (
	"fmt"
	"strings"
)

// Parser builds an AST from a token stream.
type Parser struct {
	tokens  []Token
	pos     int
	errors  []string
}

// NewParser creates a Parser from a slice of tokens.
func NewParser(tokens []Token) *Parser {
	return &Parser{tokens: tokens}
}

func (p *Parser) peek() Token {
	if p.pos >= len(p.tokens) {
		return Token{Type: TOK_EOF}
	}
	return p.tokens[p.pos]
}

func (p *Parser) peekAt(offset int) Token {
	idx := p.pos + offset
	if idx >= len(p.tokens) {
		return Token{Type: TOK_EOF}
	}
	return p.tokens[idx]
}

func (p *Parser) advance() Token {
	tok := p.peek()
	if tok.Type != TOK_EOF {
		p.pos++
	}
	return tok
}

func (p *Parser) check(t TokenType) bool { return p.peek().Type == t }

func (p *Parser) match(types ...TokenType) bool {
	for _, t := range types {
		if p.check(t) {
			p.advance()
			return true
		}
	}
	return false
}

func (p *Parser) expect(t TokenType) (Token, error) {
	if p.check(t) {
		return p.advance(), nil
	}
	tok := p.peek()
	return tok, fmt.Errorf("line %d: expected %v, got %q", tok.Line, t, tok.Lexeme)
}

func (p *Parser) errorf(format string, args ...interface{}) {
	tok := p.peek()
	msg := fmt.Sprintf("line %d: "+format, append([]interface{}{tok.Line}, args...)...)
	p.errors = append(p.errors, msg)
}

func (p *Parser) synchronize() {
	for !p.check(TOK_EOF) {
		switch p.peek().Type {
		case TOK_SEMICOLON:
			p.advance()
			return
		case TOK_FN, TOK_VAR, TOK_LET, TOK_CONST, TOK_CLASS,
			TOK_RETURN, TOK_IF, TOK_WHILE, TOK_FOR:
			return
		}
		p.advance()
	}
}

// ─── Top-level ────────────────────────────────────────────────────────────────

// Parse parses the entire program and returns the root AST node.
func (p *Parser) Parse() (*ASTNode, []string) {
	root := NewNode(NodeProgram, 1)
	for !p.check(TOK_EOF) {
		stmt := p.parseStatement()
		if stmt != nil {
			root.AddChild(stmt)
		}
	}
	return root, p.errors
}

// ─── Statements ───────────────────────────────────────────────────────────────

func (p *Parser) parseStatement() *ASTNode {
	tok := p.peek()

	// Skip stray semicolons
	for p.check(TOK_SEMICOLON) {
		p.advance()
	}
	if p.check(TOK_EOF) {
		return nil
	}

	switch tok.Type {
	case TOK_VAR, TOK_LET:
		return p.parseVarDecl()
	case TOK_CONST:
		return p.parseConstDecl()
	case TOK_FN:
		return p.parseFnDecl(false)
	case TOK_ASYNC:
		p.advance()
		return p.parseFnDecl(true)
	case TOK_CLASS:
		return p.parseClassDecl()
	case TOK_ENUM:
		return p.parseEnumDecl()
	case TOK_RETURN:
		return p.parseReturn()
	case TOK_IF:
		return p.parseIf()
	case TOK_WHILE:
		return p.parseWhile()
	case TOK_DO:
		return p.parseDoWhile()
	case TOK_FOR:
		return p.parseFor()
	case TOK_SWITCH:
		return p.parseSwitch()
	case TOK_BREAK:
		p.advance()
		p.match(TOK_SEMICOLON)
		return NewNode(NodeBreak, tok.Line)
	case TOK_CONTINUE:
		p.advance()
		p.match(TOK_SEMICOLON)
		return NewNode(NodeContinue, tok.Line)
	case TOK_IMPORT:
		return p.parseImport()
	case TOK_EXPORT:
		return p.parseExport()
	case TOK_NAMESPACE:
		return p.parseNamespace()
	case TOK_TYPE:
		return p.parseTypeAlias()
	case TOK_LBRACE:
		return p.parseBlock()
	case TOK_PRINT:
		return p.parsePrint()
	case TOK_ASSERT:
		return p.parseAssert()
	case TOK_REQUIRES:
		return p.parseRequires()
	case TOK_ENSURES:
		return p.parseEnsures()
	case TOK_INVARIANT:
		return p.parseInvariantStmt()
	case TOK_SPAWN:
		p.advance()
		n := NewNode(NodeSpawn, tok.Line)
		n.AddChild(p.parseExpression())
		p.match(TOK_SEMICOLON)
		return n
	}

	// Expression statement
	expr := p.parseExpression()
	if expr == nil {
		p.errorf("unexpected token %q", p.peek().Lexeme)
		p.advance()
		return nil
	}
	p.match(TOK_SEMICOLON)
	n := NewNode(NodeExprStmt, tok.Line)
	n.AddChild(expr)
	return n
}

func (p *Parser) parseBlock() *ASTNode {
	tok, err := p.expect(TOK_LBRACE)
	if err != nil {
		p.errors = append(p.errors, err.Error())
		return nil
	}
	n := NewNode(NodeBlock, tok.Line)
	for !p.check(TOK_RBRACE) && !p.check(TOK_EOF) {
		stmt := p.parseStatement()
		if stmt != nil {
			n.AddChild(stmt)
		}
	}
	if _, err := p.expect(TOK_RBRACE); err != nil {
		p.errors = append(p.errors, err.Error())
	}
	return n
}

func (p *Parser) parseVarDecl() *ASTNode {
	tok := p.advance() // var or let
	n := NewNode(NodeVarDecl, tok.Line)
	nameTok, err := p.expect(TOK_IDENT)
	if err != nil {
		p.errors = append(p.errors, err.Error())
		return nil
	}
	n.StrVal = nameTok.Lexeme
	// optional type annotation
	if p.match(TOK_COLON) {
		n.TypeAnnotation = p.parseTypeName()
	}
	if p.match(TOK_ASSIGN) {
		n.AddChild(p.parseExpression())
	}
	p.match(TOK_SEMICOLON)
	return n
}

func (p *Parser) parseConstDecl() *ASTNode {
	tok := p.advance() // const
	n := NewNode(NodeConstDecl, tok.Line)
	nameTok, err := p.expect(TOK_IDENT)
	if err != nil {
		p.errors = append(p.errors, err.Error())
		return nil
	}
	n.StrVal = nameTok.Lexeme
	if p.match(TOK_COLON) {
		n.TypeAnnotation = p.parseTypeName()
	}
	if _, err := p.expect(TOK_ASSIGN); err != nil {
		p.errors = append(p.errors, err.Error())
	}
	n.AddChild(p.parseExpression())
	p.match(TOK_SEMICOLON)
	return n
}

func (p *Parser) parseFnDecl(isAsync bool) *ASTNode {
	tok, err := p.expect(TOK_FN)
	if err != nil {
		p.errors = append(p.errors, err.Error())
		return nil
	}
	n := NewNode(NodeFnDecl, tok.Line)
	n.IsAsync = isAsync

	// Optional name
	if p.check(TOK_IDENT) {
		n.StrVal = p.advance().Lexeme
	}

	// Skip generic type params <T, U>
	if p.check(TOK_LT) {
		depth := 0
		for !p.check(TOK_EOF) {
			if p.check(TOK_LT) {
				depth++
			} else if p.check(TOK_GT) {
				depth--
				if depth == 0 {
					p.advance()
					break
				}
			}
			p.advance()
		}
	}

	n.Params = p.parseFnParams()

	// Return type annotation
	if p.match(TOK_ARROW) || p.match(TOK_COLON) {
		n.ReturnType = p.parseTypeName()
	}

	// Contract clauses (requires/ensures)
	for p.check(TOK_REQUIRES) || p.check(TOK_ENSURES) {
		p.advance()
		p.expect(TOK_LPAREN)
		p.parseExpression() // skip for now
		p.expect(TOK_RPAREN)
	}

	body := p.parseBlock()
	n.AddChild(body)
	return n
}

func (p *Parser) parseFnParams() []*FnParam {
	if _, err := p.expect(TOK_LPAREN); err != nil {
		p.errors = append(p.errors, err.Error())
		return nil
	}
	var params []*FnParam
	for !p.check(TOK_RPAREN) && !p.check(TOK_EOF) {
		param := &FnParam{}
		isRest := p.match(TOK_ELLIPSIS)
		param.IsRest = isRest

		if p.check(TOK_IDENT) {
			param.Name = p.advance().Lexeme
		} else {
			break
		}
		if p.match(TOK_QUESTION) {
			param.IsOptional = true
		}
		if p.match(TOK_COLON) {
			param.TypeAnnot = p.parseTypeName()
		}
		if p.match(TOK_ASSIGN) {
			param.DefaultValue = p.parseExpression()
			param.IsOptional = true
		}
		params = append(params, param)
		if !p.match(TOK_COMMA) {
			break
		}
	}
	p.expect(TOK_RPAREN)
	return params
}

func (p *Parser) parseClassDecl() *ASTNode {
	tok := p.advance() // class
	n := NewNode(NodeClassDecl, tok.Line)
	nameTok, err := p.expect(TOK_IDENT)
	if err != nil {
		p.errors = append(p.errors, err.Error())
		return nil
	}
	n.StrVal = nameTok.Lexeme

	// Skip generic params
	if p.check(TOK_LT) {
		depth := 0
		for !p.check(TOK_EOF) {
			if p.check(TOK_LT) {
				depth++
			} else if p.check(TOK_GT) {
				depth--
				if depth == 0 {
					p.advance()
					break
				}
			}
			p.advance()
		}
	}

	if p.match(TOK_EXTENDS) {
		parentName := p.advance().Lexeme
		child := IdentNode(parentName, p.peek().Line)
		n.AddChild(child)
	} else {
		n.AddChild(nil) // no parent
	}

	// Class body
	if _, err := p.expect(TOK_LBRACE); err != nil {
		p.errors = append(p.errors, err.Error())
		return nil
	}
	for !p.check(TOK_RBRACE) && !p.check(TOK_EOF) {
		// Skip semicolons and access modifiers
		for p.check(TOK_SEMICOLON) {
			p.advance()
		}
		if p.check(TOK_RBRACE) {
			break
		}
		// Static / access modifier keywords (skip)
		for p.check(TOK_IDENT) && isAccessModifier(p.peek().Lexeme) {
			p.advance()
		}

		isAsync := false
		if p.check(TOK_ASYNC) {
			p.advance()
			isAsync = true
		}

		var method *ASTNode
		if p.check(TOK_FN) {
			method = p.parseFnDecl(isAsync)
		} else if p.check(TOK_IDENT) {
			// Method without fn keyword  e.g. init(...) { }
			method = p.parseMethodShorthand(isAsync)
		} else if p.check(TOK_VAR) || p.check(TOK_LET) {
			method = p.parseVarDecl()
		} else if p.check(TOK_CONST) {
			method = p.parseConstDecl()
		} else if p.check(TOK_INVARIANT) {
			method = p.parseInvariantStmt()
		} else {
			p.advance() // skip unknown
			continue
		}
		if method != nil {
			n.AddChild(method)
		}
	}
	p.expect(TOK_RBRACE)
	return n
}

func isAccessModifier(s string) bool {
	return s == "public" || s == "private" || s == "protected" || s == "static" || s == "abstract" || s == "override"
}

func (p *Parser) parseMethodShorthand(isAsync bool) *ASTNode {
	tok := p.peek()
	name := p.advance().Lexeme
	if !p.check(TOK_LPAREN) {
		// field declaration with default value
		n := NewNode(NodeVarDecl, tok.Line)
		n.StrVal = name
		if p.match(TOK_ASSIGN) {
			n.AddChild(p.parseExpression())
		}
		p.match(TOK_SEMICOLON)
		return n
	}
	n := NewNode(NodeFnDecl, tok.Line)
	n.StrVal = name
	n.IsAsync = isAsync
	n.Params = p.parseFnParams()
	if p.match(TOK_ARROW) || p.match(TOK_COLON) {
		n.ReturnType = p.parseTypeName()
	}
	body := p.parseBlock()
	n.AddChild(body)
	return n
}

func (p *Parser) parseEnumDecl() *ASTNode {
	tok := p.advance() // enum
	n := NewNode(NodeEnumDecl, tok.Line)
	nameTok, err := p.expect(TOK_IDENT)
	if err != nil {
		p.errors = append(p.errors, err.Error())
		return nil
	}
	n.StrVal = nameTok.Lexeme
	p.expect(TOK_LBRACE)
	for !p.check(TOK_RBRACE) && !p.check(TOK_EOF) {
		// Allow comma, pipe, or semicolon as variant separators
		if p.check(TOK_SEMICOLON) || p.check(TOK_COMMA) || p.check(TOK_PIPE) {
			p.advance()
			continue
		}
		variantTok, _ := p.expect(TOK_IDENT)
		variant := NewNode(NodeEnumVariant, variantTok.Line)
		variant.StrVal = variantTok.Lexeme
		if p.match(TOK_LPAREN) {
			for !p.check(TOK_RPAREN) && !p.check(TOK_EOF) {
				field := p.parseExpression()
				variant.AddChild(field)
				if !p.match(TOK_COMMA) {
					break
				}
			}
			p.expect(TOK_RPAREN)
		}
		n.AddChild(variant)
		// Allow comma or pipe as trailing separator
		if !p.match(TOK_COMMA) {
			p.match(TOK_PIPE)
		}
	}
	p.expect(TOK_RBRACE)
	return n
}

func (p *Parser) parseReturn() *ASTNode {
	tok := p.advance() // return
	n := NewNode(NodeReturn, tok.Line)
	if !p.check(TOK_SEMICOLON) && !p.check(TOK_RBRACE) && !p.check(TOK_EOF) {
		n.AddChild(p.parseExpression())
	}
	p.match(TOK_SEMICOLON)
	return n
}

func (p *Parser) parseIf() *ASTNode {
	tok := p.advance() // if
	n := NewNode(NodeIf, tok.Line)
	p.expect(TOK_LPAREN)
	n.AddChild(p.parseExpression())
	p.expect(TOK_RPAREN)
	n.AddChild(p.parseStatement())
	if p.match(TOK_ELSE) {
		n.AddChild(p.parseStatement())
	}
	return n
}

func (p *Parser) parseWhile() *ASTNode {
	tok := p.advance() // while
	n := NewNode(NodeWhile, tok.Line)
	p.expect(TOK_LPAREN)
	n.AddChild(p.parseExpression())
	p.expect(TOK_RPAREN)
	n.AddChild(p.parseStatement())
	return n
}

func (p *Parser) parseDoWhile() *ASTNode {
	tok := p.advance() // do
	n := NewNode(NodeDoWhile, tok.Line)
	n.AddChild(p.parseStatement())
	p.expect(TOK_WHILE)
	p.expect(TOK_LPAREN)
	n.AddChild(p.parseExpression())
	p.expect(TOK_RPAREN)
	p.match(TOK_SEMICOLON)
	return n
}

func (p *Parser) parseFor() *ASTNode {
	tok := p.advance() // for
	p.expect(TOK_LPAREN)

	// Determine if this is for-in
	// for (var/let x in expr) or for (let x in expr)
	savedPos := p.pos
	isForIn := false
	{
		if p.check(TOK_VAR) || p.check(TOK_LET) || p.check(TOK_CONST) {
			p.advance()
			if p.check(TOK_IDENT) {
				p.advance()
				if p.check(TOK_IN) {
					isForIn = true
				}
			}
		} else if p.check(TOK_IDENT) {
			p.advance()
			if p.check(TOK_IN) {
				isForIn = true
			}
		}
		p.pos = savedPos
	}

	if isForIn {
		n := NewNode(NodeForIn, tok.Line)
		if p.check(TOK_VAR) || p.check(TOK_LET) || p.check(TOK_CONST) {
			p.advance()
		}
		varTok, _ := p.expect(TOK_IDENT)
		n.StrVal = varTok.Lexeme
		p.expect(TOK_IN)
		n.AddChild(p.parseExpression())
		p.expect(TOK_RPAREN)
		n.AddChild(p.parseStatement())
		return n
	}

	// Traditional for (init; cond; update)
	n := NewNode(NodeFor, tok.Line)
	// init
	if !p.check(TOK_SEMICOLON) {
		if p.check(TOK_VAR) || p.check(TOK_LET) || p.check(TOK_CONST) {
			n.AddChild(p.parseVarDecl())
		} else {
			n.AddChild(p.parseExpression())
			p.match(TOK_SEMICOLON)
		}
	} else {
		n.AddChild(nil)
		p.advance()
	}
	// cond
	if !p.check(TOK_SEMICOLON) {
		n.AddChild(p.parseExpression())
	} else {
		n.AddChild(nil)
	}
	p.match(TOK_SEMICOLON)
	// update
	if !p.check(TOK_RPAREN) {
		n.AddChild(p.parseExpression())
	} else {
		n.AddChild(nil)
	}
	p.expect(TOK_RPAREN)
	n.AddChild(p.parseStatement())
	return n
}

func (p *Parser) parseSwitch() *ASTNode {
	tok := p.advance() // switch
	n := NewNode(NodeSwitch, tok.Line)
	p.expect(TOK_LPAREN)
	n.AddChild(p.parseExpression())
	p.expect(TOK_RPAREN)
	p.expect(TOK_LBRACE)
	for !p.check(TOK_RBRACE) && !p.check(TOK_EOF) {
		if p.check(TOK_CASE) {
			caseTok := p.advance()
			caseNode := NewNode(NodeIf, caseTok.Line) // reuse NodeIf as case arm
			caseNode.StrVal = "case"
			caseNode.AddChild(p.parseExpression())
			p.match(TOK_COLON)
			body := NewNode(NodeBlock, caseTok.Line)
			for !p.check(TOK_CASE) && !p.check(TOK_DEFAULT) && !p.check(TOK_RBRACE) && !p.check(TOK_EOF) {
				s := p.parseStatement()
				if s != nil {
					body.AddChild(s)
				}
			}
			caseNode.AddChild(body)
			n.AddChild(caseNode)
		} else if p.check(TOK_DEFAULT) {
			p.advance()
			p.match(TOK_COLON)
			body := NewNode(NodeBlock, tok.Line)
			body.StrVal = "default"
			for !p.check(TOK_CASE) && !p.check(TOK_RBRACE) && !p.check(TOK_EOF) {
				s := p.parseStatement()
				if s != nil {
					body.AddChild(s)
				}
			}
			n.AddChild(body)
		} else {
			p.advance()
		}
	}
	p.expect(TOK_RBRACE)
	return n
}

func (p *Parser) parsePrint() *ASTNode {
	tok := p.advance() // print
	n := NewNode(NodePrint, tok.Line)
	if p.match(TOK_LPAREN) {
		for !p.check(TOK_RPAREN) && !p.check(TOK_EOF) {
			n.AddChild(p.parseExpression())
			if !p.match(TOK_COMMA) {
				break
			}
		}
		p.expect(TOK_RPAREN)
	}
	p.match(TOK_SEMICOLON)
	return n
}

func (p *Parser) parseAssert() *ASTNode {
	tok := p.advance() // assert
	n := NewNode(NodeAssert, tok.Line)
	p.expect(TOK_LPAREN)
	n.AddChild(p.parseExpression())
	if p.match(TOK_COMMA) {
		n.AddChild(p.parseExpression())
	}
	p.expect(TOK_RPAREN)
	p.match(TOK_SEMICOLON)
	return n
}

func (p *Parser) parseRequires() *ASTNode {
	tok := p.advance()
	n := NewNode(NodeRequires, tok.Line)
	p.expect(TOK_LPAREN)
	n.AddChild(p.parseExpression())
	p.expect(TOK_RPAREN)
	p.match(TOK_SEMICOLON)
	return n
}

func (p *Parser) parseEnsures() *ASTNode {
	tok := p.advance()
	n := NewNode(NodeEnsures, tok.Line)
	p.expect(TOK_LPAREN)
	n.AddChild(p.parseExpression())
	p.expect(TOK_RPAREN)
	p.match(TOK_SEMICOLON)
	return n
}

func (p *Parser) parseInvariantStmt() *ASTNode {
	tok := p.advance()
	n := NewNode(NodeInvariant, tok.Line)
	p.expect(TOK_LPAREN)
	n.AddChild(p.parseExpression())
	p.expect(TOK_RPAREN)
	p.match(TOK_SEMICOLON)
	return n
}

func (p *Parser) parseImport() *ASTNode {
	tok := p.advance() // import
	n := NewNode(NodeImport, tok.Line)

	// from "module" import name1, name2
	if p.check(TOK_FROM) {
		p.advance()
		pathTok, _ := p.expect(TOK_STRING)
		n.StrVal = pathTok.Lexeme
		p.expect(TOK_IMPORT)
		for p.check(TOK_IDENT) || p.check(TOK_STAR) {
			if p.check(TOK_STAR) {
				p.advance()
				if p.match(TOK_AS) {
					aliasTok, _ := p.expect(TOK_IDENT)
					child := IdentNode(aliasTok.Lexeme, aliasTok.Line)
					child.StrVal = "*"
					n.AddChild(child)
				}
			} else {
				nameTok := p.advance()
				child := IdentNode(nameTok.Lexeme, nameTok.Line)
				if p.match(TOK_AS) {
					aliasTok, _ := p.expect(TOK_IDENT)
					child.TypeAnnotation = aliasTok.Lexeme
				}
				n.AddChild(child)
			}
			if !p.match(TOK_COMMA) {
				break
			}
		}
	} else {
		// import "module" [as alias]
		pathTok, _ := p.expect(TOK_STRING)
		n.StrVal = pathTok.Lexeme
		if p.match(TOK_AS) {
			aliasTok, _ := p.expect(TOK_IDENT)
			n.TypeAnnotation = aliasTok.Lexeme
		}
	}
	p.match(TOK_SEMICOLON)
	return n
}

func (p *Parser) parseExport() *ASTNode {
	tok := p.advance() // export
	n := NewNode(NodeExport, tok.Line)
	inner := p.parseStatement()
	if inner != nil {
		inner.IsExport = true
		n.AddChild(inner)
	}
	return n
}

func (p *Parser) parseNamespace() *ASTNode {
	tok := p.advance() // namespace
	n := NewNode(NodeNamespace, tok.Line)
	nameTok, _ := p.expect(TOK_IDENT)
	n.StrVal = nameTok.Lexeme
	body := p.parseBlock()
	n.AddChild(body)
	return n
}

func (p *Parser) parseTypeAlias() *ASTNode {
	tok := p.advance() // type
	n := NewNode(NodeTypeAlias, tok.Line)
	nameTok, _ := p.expect(TOK_IDENT)
	n.StrVal = nameTok.Lexeme
	p.expect(TOK_ASSIGN)
	n.TypeAnnotation = p.parseTypeName()
	p.match(TOK_SEMICOLON)
	return n
}

// ─── Expressions ──────────────────────────────────────────────────────────────

// parseExpression handles assignment.
func (p *Parser) parseExpression() *ASTNode {
	return p.parseAssignment()
}

func (p *Parser) parseAssignment() *ASTNode {
	expr := p.parseTernary()
	if expr == nil {
		return nil
	}

	switch p.peek().Type {
	case TOK_ASSIGN, TOK_PLUS_EQ, TOK_MINUS_EQ, TOK_STAR_EQ,
		TOK_SLASH_EQ, TOK_PERCENT_EQ, TOK_STARSTAR_EQ:
		opTok := p.advance()
		right := p.parseAssignment()
		if opTok.Type == TOK_ASSIGN {
			n := NewNode(NodeAssign, opTok.Line)
			n.AddChild(expr)
			n.AddChild(right)
			return n
		}
		n := NewNode(NodeCompoundAssign, opTok.Line)
		n.StrVal = opTok.Lexeme
		n.AddChild(expr)
		n.AddChild(right)
		return n
	}
	return expr
}

func (p *Parser) parseTernary() *ASTNode {
	cond := p.parseNullish()
	if p.check(TOK_QUESTION) {
		tok := p.advance()
		then := p.parseExpression()
		p.expect(TOK_COLON)
		els := p.parseExpression()
		n := NewNode(NodeTernary, tok.Line)
		n.AddChild(cond)
		n.AddChild(then)
		n.AddChild(els)
		return n
	}
	return cond
}

func (p *Parser) parseNullish() *ASTNode {
	left := p.parseOr()
	for p.check(TOK_NULLISH) {
		tok := p.advance()
		right := p.parseOr()
		n := NewNode(NodeNullish, tok.Line)
		n.AddChild(left)
		n.AddChild(right)
		left = n
	}
	return left
}

func (p *Parser) parseOr() *ASTNode {
	left := p.parseAnd()
	for p.check(TOK_OR) {
		tok := p.advance()
		right := p.parseAnd()
		n := NewNode(NodeBinary, tok.Line)
		n.StrVal = "||"
		n.AddChild(left)
		n.AddChild(right)
		left = n
	}
	return left
}

func (p *Parser) parseAnd() *ASTNode {
	left := p.parseBitOr()
	for p.check(TOK_AND) {
		tok := p.advance()
		right := p.parseBitOr()
		n := NewNode(NodeBinary, tok.Line)
		n.StrVal = "&&"
		n.AddChild(left)
		n.AddChild(right)
		left = n
	}
	return left
}

func (p *Parser) parseBitOr() *ASTNode {
	left := p.parseBitXor()
	for p.check(TOK_PIPE) {
		tok := p.advance()
		right := p.parseBitXor()
		n := NewNode(NodeBinary, tok.Line)
		n.StrVal = "|"
		n.AddChild(left)
		n.AddChild(right)
		left = n
	}
	return left
}

func (p *Parser) parseBitXor() *ASTNode {
	left := p.parseBitAnd()
	for p.check(TOK_CARET) {
		tok := p.advance()
		right := p.parseBitAnd()
		n := NewNode(NodeBinary, tok.Line)
		n.StrVal = "^"
		n.AddChild(left)
		n.AddChild(right)
		left = n
	}
	return left
}

func (p *Parser) parseBitAnd() *ASTNode {
	left := p.parseEquality()
	for p.check(TOK_AMPERSAND) {
		tok := p.advance()
		right := p.parseEquality()
		n := NewNode(NodeBinary, tok.Line)
		n.StrVal = "&"
		n.AddChild(left)
		n.AddChild(right)
		left = n
	}
	return left
}

func (p *Parser) parseEquality() *ASTNode {
	left := p.parseComparison()
	for p.check(TOK_EQ) || p.check(TOK_NEQ) {
		tok := p.advance()
		right := p.parseComparison()
		n := NewNode(NodeBinary, tok.Line)
		n.StrVal = tok.Lexeme
		n.AddChild(left)
		n.AddChild(right)
		left = n
	}
	return left
}

func (p *Parser) parseComparison() *ASTNode {
	left := p.parseShift()
	for p.check(TOK_LT) || p.check(TOK_LE) || p.check(TOK_GT) || p.check(TOK_GE) ||
		p.check(TOK_INSTANCEOF) {
		tok := p.advance()
		right := p.parseShift()
		if tok.Type == TOK_INSTANCEOF {
			n := NewNode(NodeInstanceof, tok.Line)
			n.AddChild(left)
			n.AddChild(right)
			left = n
		} else {
			n := NewNode(NodeBinary, tok.Line)
			n.StrVal = tok.Lexeme
			n.AddChild(left)
			n.AddChild(right)
			left = n
		}
	}
	return left
}

func (p *Parser) parseShift() *ASTNode {
	left := p.parseAdditive()
	for p.check(TOK_SHL) || p.check(TOK_SHR) {
		tok := p.advance()
		right := p.parseAdditive()
		n := NewNode(NodeBinary, tok.Line)
		n.StrVal = tok.Lexeme
		n.AddChild(left)
		n.AddChild(right)
		left = n
	}
	return left
}

func (p *Parser) parseAdditive() *ASTNode {
	left := p.parseMultiplicative()
	for p.check(TOK_PLUS) || p.check(TOK_MINUS) {
		tok := p.advance()
		right := p.parseMultiplicative()
		n := NewNode(NodeBinary, tok.Line)
		n.StrVal = tok.Lexeme
		n.AddChild(left)
		n.AddChild(right)
		left = n
	}
	return left
}

func (p *Parser) parseMultiplicative() *ASTNode {
	left := p.parseExponent()
	for p.check(TOK_STAR) || p.check(TOK_SLASH) || p.check(TOK_PERCENT) {
		tok := p.advance()
		right := p.parseExponent()
		n := NewNode(NodeBinary, tok.Line)
		n.StrVal = tok.Lexeme
		n.AddChild(left)
		n.AddChild(right)
		left = n
	}
	return left
}

func (p *Parser) parseExponent() *ASTNode {
	base := p.parseUnary()
	if p.check(TOK_STARSTAR) {
		tok := p.advance()
		exp := p.parseExponent() // right-associative
		n := NewNode(NodeBinary, tok.Line)
		n.StrVal = "**"
		n.AddChild(base)
		n.AddChild(exp)
		return n
	}
	return base
}

func (p *Parser) parseUnary() *ASTNode {
	switch p.peek().Type {
	case TOK_MINUS:
		tok := p.advance()
		n := NewNode(NodeUnary, tok.Line)
		n.StrVal = "-"
		n.AddChild(p.parseUnary())
		return n
	case TOK_NOT:
		tok := p.advance()
		n := NewNode(NodeUnary, tok.Line)
		n.StrVal = "!"
		n.AddChild(p.parseUnary())
		return n
	case TOK_TILDE:
		tok := p.advance()
		n := NewNode(NodeUnary, tok.Line)
		n.StrVal = "~"
		n.AddChild(p.parseUnary())
		return n
	case TOK_TYPEOF:
		tok := p.advance()
		n := NewNode(NodeTypeof, tok.Line)
		if p.match(TOK_LPAREN) {
			n.AddChild(p.parseExpression())
			p.expect(TOK_RPAREN)
		} else {
			n.AddChild(p.parseUnary())
		}
		return n
	case TOK_SPAWN:
		tok := p.advance()
		n := NewNode(NodeSpawn, tok.Line)
		n.AddChild(p.parseUnary())
		return n
	case TOK_AWAIT:
		tok := p.advance()
		n := NewNode(NodeAwait, tok.Line)
		n.AddChild(p.parseUnary())
		return n
	case TOK_INC, TOK_DEC:
		tok := p.advance()
		inner := p.parsePostfix()
		n := NewNode(NodeUnary, tok.Line)
		n.StrVal = tok.Lexeme + "pre"
		n.AddChild(inner)
		return n
	}
	return p.parsePostfix()
}

func (p *Parser) parsePostfix() *ASTNode {
	expr := p.parsePrimary()
	if expr == nil {
		return nil
	}

	for {
		switch p.peek().Type {
		case TOK_DOT:
			tok := p.advance()
			propTok, _ := p.expect(TOK_IDENT)
			// Method call or property access
			if p.check(TOK_LPAREN) {
				n := NewNode(NodeMethodCall, tok.Line)
				n.StrVal = propTok.Lexeme
				n.AddChild(expr)
				p.advance() // (
				for !p.check(TOK_RPAREN) && !p.check(TOK_EOF) {
					n.AddChild(p.parseArg())
					if !p.match(TOK_COMMA) {
						break
					}
				}
				p.expect(TOK_RPAREN)
				expr = n
			} else {
				n := NewNode(NodePropertyGet, tok.Line)
				n.StrVal = propTok.Lexeme
				n.AddChild(expr)
				expr = n
			}
		case TOK_LBRACKET:
			tok := p.advance()
			idx := p.parseExpression()
			p.expect(TOK_RBRACKET)
			n := NewNode(NodeIndex, tok.Line)
			n.AddChild(expr)
			n.AddChild(idx)
			expr = n
		case TOK_LPAREN:
			tok := p.peek()
			p.advance()
			n := NewNode(NodeCallExpr, tok.Line)
			n.AddChild(expr)
			for !p.check(TOK_RPAREN) && !p.check(TOK_EOF) {
				n.AddChild(p.parseArg())
				if !p.match(TOK_COMMA) {
					break
				}
			}
			p.expect(TOK_RPAREN)
			expr = n
		case TOK_INC:
			tok := p.advance()
			n := NewNode(NodeIncrement, tok.Line)
			n.AddChild(expr)
			expr = n
		case TOK_DEC:
			tok := p.advance()
			n := NewNode(NodeDecrement, tok.Line)
			n.AddChild(expr)
			expr = n
		default:
			return expr
		}
	}
}

// parseArg parses a function argument (possibly named or spread).
func (p *Parser) parseArg() *ASTNode {
	// Spread argument
	if p.check(TOK_ELLIPSIS) {
		tok := p.advance()
		n := NewNode(NodeSpread, tok.Line)
		n.AddChild(p.parseExpression())
		return n
	}
	// Named argument: name: value
	if p.check(TOK_IDENT) && p.peekAt(1).Type == TOK_COLON {
		nameTok := p.advance()
		p.advance() // :
		n := NewNode(NodeNamedArg, nameTok.Line)
		n.StrVal = nameTok.Lexeme
		n.AddChild(p.parseExpression())
		return n
	}
	return p.parseExpression()
}

func (p *Parser) parsePrimary() *ASTNode {
	tok := p.peek()

	switch tok.Type {
	case TOK_NUMBER:
		p.advance()
		return NumberNode(tok.NumVal, tok.Line)
	case TOK_STRING:
		p.advance()
		return StringNode(tok.Lexeme, tok.Line)
	case TOK_TRUE:
		p.advance()
		return BoolNode(true, tok.Line)
	case TOK_FALSE:
		p.advance()
		return BoolNode(false, tok.Line)
	case TOK_NULL:
		p.advance()
		return NewNode(NodeNull, tok.Line)
	case TOK_THIS:
		p.advance()
		return NewNode(NodeThis, tok.Line)
	case TOK_SUPER:
		p.advance()
		n := NewNode(NodeSuper, tok.Line)
		if p.match(TOK_LPAREN) {
			for !p.check(TOK_RPAREN) && !p.check(TOK_EOF) {
				n.AddChild(p.parseArg())
				if !p.match(TOK_COMMA) {
					break
				}
			}
			p.expect(TOK_RPAREN)
		}
		return n
	case TOK_NEW:
		return p.parseNew()
	case TOK_FN:
		return p.parseFnDecl(false)
	case TOK_ASYNC:
		p.advance()
		return p.parseFnDecl(true)
	case TOK_MATCH:
		return p.parseMatch()
	case TOK_INPUT:
		return p.parseInput()
	case TOK_SLEEP:
		return p.parseSleep()
	case TOK_LBRACKET:
		return p.parseArrayLiteral()
	case TOK_LBRACE:
		return p.parseObjectLiteral()
	case TOK_LPAREN:
		return p.parseParenOrTupleOrArrow()
	case TOK_IDENT:
		// Could be a plain call, variant construction, or arrow function
		return p.parseIdentOrCall()
	case TOK_ELLIPSIS:
		tok2 := p.advance()
		n := NewNode(NodeSpread, tok2.Line)
		n.AddChild(p.parsePrimary())
		return n
	}

	return nil
}

func (p *Parser) parseNew() *ASTNode {
	tok := p.advance() // new
	n := NewNode(NodeNew, tok.Line)
	nameTok, _ := p.expect(TOK_IDENT)
	n.StrVal = nameTok.Lexeme
	// Skip generic args
	if p.check(TOK_LT) {
		depth := 0
		for !p.check(TOK_EOF) {
			if p.check(TOK_LT) {
				depth++
			} else if p.check(TOK_GT) {
				depth--
				if depth == 0 {
					p.advance()
					break
				}
			}
			p.advance()
		}
	}
	if p.match(TOK_LPAREN) {
		for !p.check(TOK_RPAREN) && !p.check(TOK_EOF) {
			n.AddChild(p.parseArg())
			if !p.match(TOK_COMMA) {
				break
			}
		}
		p.expect(TOK_RPAREN)
	}
	return n
}

func (p *Parser) parseMatch() *ASTNode {
	tok := p.advance() // match
	n := NewNode(NodeMatch, tok.Line)
	n.AddChild(p.parseExpression())
	p.expect(TOK_LBRACE)
	for !p.check(TOK_RBRACE) && !p.check(TOK_EOF) {
		arm := p.parseMatchArm()
		if arm != nil {
			n.AddChild(arm)
		}
		p.match(TOK_COMMA)
	}
	p.expect(TOK_RBRACE)
	return n
}

func (p *Parser) parseMatchArm() *ASTNode {
	tok := p.peek()
	pat := p.parsePattern()
	n := NewNode(NodeMatchArm, tok.Line)
	n.AddChild(pat)
	p.expect(TOK_FAT_ARROW)
	n.AddChild(p.parseExpression())
	return n
}

func (p *Parser) parsePattern() *ASTNode {
	tok := p.peek()
	// Wildcard _
	if p.check(TOK_IDENT) && tok.Lexeme == "_" {
		p.advance()
		n := NewNode(NodePattern, tok.Line)
		n.StrVal = "_"
		return n
	}
	// Variant  Name(bindings)  or  Name
	if p.check(TOK_IDENT) {
		nameTok := p.advance()
		n := NewNode(NodePattern, nameTok.Line)
		n.StrVal = nameTok.Lexeme
		if p.match(TOK_LPAREN) {
			for !p.check(TOK_RPAREN) && !p.check(TOK_EOF) {
				bindTok, _ := p.expect(TOK_IDENT)
				bind := IdentNode(bindTok.Lexeme, bindTok.Line)
				n.AddChild(bind)
				if !p.match(TOK_COMMA) {
					break
				}
			}
			p.expect(TOK_RPAREN)
		}
		return n
	}
	// Literal pattern
	expr := p.parseExpression()
	n := NewNode(NodePattern, tok.Line)
	n.AddChild(expr)
	return n
}

func (p *Parser) parseInput() *ASTNode {
	tok := p.advance()
	n := NewNode(NodeInput, tok.Line)
	if p.match(TOK_LPAREN) {
		if !p.check(TOK_RPAREN) {
			n.AddChild(p.parseExpression())
		}
		p.expect(TOK_RPAREN)
	}
	return n
}

func (p *Parser) parseSleep() *ASTNode {
	tok := p.advance()
	n := NewNode(NodeSleep, tok.Line)
	if p.match(TOK_LPAREN) {
		n.AddChild(p.parseExpression())
		p.expect(TOK_RPAREN)
	}
	return n
}

func (p *Parser) parseArrayLiteral() *ASTNode {
	tok := p.advance() // [
	n := NewNode(NodeArrayLiteral, tok.Line)
	for !p.check(TOK_RBRACKET) && !p.check(TOK_EOF) {
		n.AddChild(p.parseArg())
		if !p.match(TOK_COMMA) {
			break
		}
	}
	p.expect(TOK_RBRACKET)
	return n
}

func (p *Parser) parseObjectLiteral() *ASTNode {
	tok := p.advance() // {
	n := NewNode(NodeObjectLiteral, tok.Line)
	for !p.check(TOK_RBRACE) && !p.check(TOK_EOF) {
		var key string
		if p.check(TOK_IDENT) {
			key = p.advance().Lexeme
		} else if p.check(TOK_STRING) {
			key = p.advance().Lexeme
		} else {
			break
		}
		pair := NewNode(NodeNamedArg, tok.Line)
		pair.StrVal = key
		if p.match(TOK_COLON) {
			pair.AddChild(p.parseExpression())
		} else {
			// shorthand { x } == { x: x }
			pair.AddChild(IdentNode(key, tok.Line))
		}
		n.AddChild(pair)
		if !p.match(TOK_COMMA) {
			break
		}
	}
	p.expect(TOK_RBRACE)
	return n
}

// parseParenOrTupleOrArrow handles (...) groups, tuples, and arrow functions.
func (p *Parser) parseParenOrTupleOrArrow() *ASTNode {
	tok := p.advance() // (

	// Empty arrow:  () => ...
	if p.check(TOK_RPAREN) {
		p.advance()
		if p.match(TOK_FAT_ARROW) {
			return p.finishArrowFn(nil, tok.Line)
		}
		// empty tuple
		return NewNode(NodeTupleLiteral, tok.Line)
	}

	// Try to detect arrow:  (params) =>
	// We do a speculative parse of a comma-separated list of possible params.
	savedPos := p.pos
	isArrow := p.looksLikeArrowParams()
	p.pos = savedPos

	if isArrow {
		params := p.parseFnParams()
		p.match(TOK_ARROW)
		p.match(TOK_FAT_ARROW)
		return p.finishArrowFn(params, tok.Line)
	}

	// Grouped expression or tuple
	first := p.parseExpression()
	if p.check(TOK_RPAREN) {
		p.advance()
		return first
	}
	// Tuple
	n := NewNode(NodeTupleLiteral, tok.Line)
	n.AddChild(first)
	for p.match(TOK_COMMA) {
		n.AddChild(p.parseExpression())
	}
	p.expect(TOK_RPAREN)
	return n
}

func (p *Parser) looksLikeArrowParams() bool {
	// Look ahead for patterns like:
	//   ()  (a)  (a, b)  (a: T)  (a = expr)  ...a
	depth := 0
	i := p.pos - 1 // we already consumed (
	for i < len(p.tokens) {
		t := p.tokens[i]
		switch t.Type {
		case TOK_LPAREN:
			depth++
		case TOK_RPAREN:
			depth--
			if depth < 0 {
				// Check what follows the closing )
				if i+1 < len(p.tokens) {
					next := p.tokens[i+1].Type
					return next == TOK_FAT_ARROW || next == TOK_ARROW
				}
				return false
			}
		case TOK_EOF:
			return false
		}
		i++
	}
	return false
}

func (p *Parser) finishArrowFn(params []*FnParam, line int) *ASTNode {
	n := NewNode(NodeArrowFn, line)
	n.Params = params
	if p.check(TOK_LBRACE) {
		n.AddChild(p.parseBlock())
	} else {
		// Expression body
		body := NewNode(NodeReturn, line)
		body.AddChild(p.parseExpression())
		n.AddChild(body)
	}
	return n
}

func (p *Parser) parseIdentOrCall() *ASTNode {
	tok := p.advance()

	// Enum variant construction: Tag(fields)  — when followed by (
	// Plain function call:         fn(args)
	n := IdentNode(tok.Lexeme, tok.Line)

	// Detect arrow function  name =>
	if p.check(TOK_FAT_ARROW) {
		p.advance()
		param := &FnParam{Name: tok.Lexeme}
		arrowN := NewNode(NodeArrowFn, tok.Line)
		arrowN.Params = []*FnParam{param}
		if p.check(TOK_LBRACE) {
			arrowN.AddChild(p.parseBlock())
		} else {
			ret := NewNode(NodeReturn, tok.Line)
			ret.AddChild(p.parseExpression())
			arrowN.AddChild(ret)
		}
		return arrowN
	}

	if p.check(TOK_LPAREN) {
		p.advance()
		call := NewNode(NodeFnCall, tok.Line)
		call.StrVal = tok.Lexeme
		for !p.check(TOK_RPAREN) && !p.check(TOK_EOF) {
			call.AddChild(p.parseArg())
			if !p.match(TOK_COMMA) {
				break
			}
		}
		p.expect(TOK_RPAREN)
		return call
	}
	return n
}

// ─── Type name parser (simple) ────────────────────────────────────────────────

func (p *Parser) parseTypeName() string {
	var sb strings.Builder
	if p.check(TOK_IDENT) || isTypeKeyword(p.peek().Type) {
		sb.WriteString(p.advance().Lexeme)
	}
	// Generic params
	if p.check(TOK_LT) {
		depth := 0
		for !p.check(TOK_EOF) {
			t := p.advance()
			sb.WriteString(t.Lexeme)
			if t.Type == TOK_LT {
				depth++
			} else if t.Type == TOK_GT {
				depth--
				if depth == 0 {
					break
				}
			}
		}
	}
	// Array type
	for p.check(TOK_LBRACKET) {
		sb.WriteString(p.advance().Lexeme)
		if p.check(TOK_RBRACKET) {
			sb.WriteString(p.advance().Lexeme)
		}
	}
	// Union
	for p.check(TOK_PIPE) {
		sb.WriteString(p.advance().Lexeme)
		if p.check(TOK_IDENT) || isTypeKeyword(p.peek().Type) {
			sb.WriteString(p.advance().Lexeme)
		}
	}
	return sb.String()
}

func isTypeKeyword(t TokenType) bool {
	return t == TOK_NULL || t == TOK_TRUE || t == TOK_FALSE
}
