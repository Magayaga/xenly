/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 */
extern crate nom;

use nom::{
    branch::alt,
    bytes::complete::{tag, take_while1},
    character::complete::{char, digit1, multispace0, multispace1},
    combinator::{map, recognize},
    sequence::{delimited, preceded, terminated, tuple},
    IResult,
};

#[derive(Debug)]
enum Expression {
    Literal(Literal),
    Identifier(String),
}

#[derive(Debug)]
enum Literal {
    Integer(i64),
    String(String),
}

#[derive(Debug)]
enum Statement {
    Print(Expression),
    Declaration(String, Expression),
}

fn parse_integer(input: &str) -> IResult<&str, i64> {
    map(recognize(digit1), |s: &str| s.parse::<i64>().unwrap())(input)
}

fn parse_string(input: &str) -> IResult<&str, String> {
    map(
        delimited(char('"'), take_while1(|c| c != '"'), char('"')),
        |s: &str| s.to_string(),
    )(input)
}

fn parse_identifier(input: &str) -> IResult<&str, String> {
    map(recognize(preceded(
        take_while1(|c: char| c.is_alphabetic()),
        take_while1(|c: char| c.is_alphanumeric() || c == '_'),
    )), |s: &str| s.to_string())(input)
}

fn parse_expression(input: &str) -> IResult<&str, Expression> {
    alt((
        map(parse_integer, |n| Expression::Literal(Literal::Integer(n))),
        map(parse_string, |s| Expression::Literal(Literal::String(s))),
        map(parse_identifier, |id| Expression::Identifier(id)),
    ))(input)
}

fn parse_print_statement(input: &str) -> IResult<&str, Statement> {
    let (input, _) = tag("print")(input)?;
    let (input, _) = delimited(char('('), multispace0, char(')'))(input)?;
    let (input, exp) = parse_expression(input)?;
    let (input, _) = tuple((multispace0, char(';')))(input)?;
    Ok((input, Statement::Print(exp)))
}

fn parse_declaration(input: &str) -> IResult<&str, Statement> {
    let (input, _) = tag("var")(input)?;
    let (input, _) = multispace1(input)?;
    let (input, id) = parse_identifier(input)?;
    let (input, _) = multispace0(input)?;
    let (input, _) = char('=')(input)?;
    let (input, _) = multispace0(input)?;
    let (input, exp) = parse_expression(input)?;
    let (input, _) = tuple((multispace0, char(';')))(input)?;
    Ok((input, Statement::Declaration(id, exp)))
}

fn parse_statement(input: &str) -> IResult<&str, Statement> {
    alt((parse_print_statement, parse_declaration))(input)
}

fn parse_program(input: &str) -> IResult<&str, Vec<Statement>> {
    let (input, _) = multispace0(input)?;
    let (input, statements) = nom::multi::many0(parse_statement)(input)?;
    Ok((input, statements))
}

fn main() {
    let program = r#"
        var x = 42;
        print(x);
        print("Hello, World!");
    "#;

    match parse_program(program) {
        Ok((_, statements)) => {
            println!("Parsed statements: {:#?}", statements);
        }
        Err(e) => {
            println!("Error: {:?}", e);
        }
    }
}
