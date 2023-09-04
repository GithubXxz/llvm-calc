#include "Parser.h"

AST *Parser::parse() {
  AST *Res = parseCalc();
  expect(Token::eoi);
  return Res;
}

AST *Parser::parseCalc() {
  Expr *E;
  llvm::SmallVector<llvm::StringRef, 8> Vars;
  if (Tok.is(Token::KW_with)) {
    advance();
    if (expect(Token::ident))
      goto _error;
    Vars.push_back(Tok.getText());
    advance();
    while (Tok.is(Token::comma)) {
      advance();
      if (expect(Token::ident))
        goto _error;
      Vars.push_back(Tok.getText());
      advance();
    }
    if (consume(Token::colon))
      goto _error;
  }
  E = parseExpr();
  if (expect(Token::eoi))
    goto _error;
  if (Vars.empty())
    return E;
  return new WithDecl(Vars, E);
_error:
  while (Tok.getKind() != Token::eoi)
    advance();
  return nullptr;
}

Expr *Parser::parseExpr() {
  Expr *Left = parseFactor();
  while (Tok.isOneOf(Token::plus, Token::minus)) {
    BinaryOp::Operator Op =
        Tok.is(Token::plus) ? BinaryOp::Plus : BinaryOp::Minus;
    advance();
    Expr *Right = parseFactor();
    Left = new BinaryOp(Op, Left, Right);
  }
  return Left;
}

Expr *Parser::parseFactor() {
  Expr *Res = nullptr;
  switch (Tok.getKind()) {
  case Token::number:
    Res = new Factor(Factor::Number, Tok.getText());
    advance();
    break;
  case Token::ident:
    Res = new Factor(Factor::Ident, Tok.getText());
    advance();
    break;
  default:
    if (!Res)
      error();
    while (!Tok.isOneOf(Token::plus, Token::minus, Token::eoi))
      advance();
  }
  return Res;
}
