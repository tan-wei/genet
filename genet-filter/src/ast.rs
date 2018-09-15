use genet_abi::token::Token;
use ops::*;
use variant::Variant;

pub struct Context {}

#[derive(Clone, Debug)]
pub enum Expression {
    Literal(Variant),
    Token(Token),
    CmpEq(Box<Expression>, Box<Expression>),
    CmpNotEq(Box<Expression>, Box<Expression>),
    CmpLt(Box<Expression>, Box<Expression>),
    CmpGt(Box<Expression>, Box<Expression>),
    CmpLte(Box<Expression>, Box<Expression>),
    CmpGte(Box<Expression>, Box<Expression>),
    LogicalAnd(Box<Expression>, Box<Expression>),
    LogicalOr(Box<Expression>, Box<Expression>),
    LogicalNegation(Box<Expression>),
    UnaryPlus(Box<Expression>),
    UnaryNegation(Box<Expression>),
}

impl Expression {
    pub fn eval(&self, ctx: &Context) -> Variant {
        match self {
            Expression::Literal(v) => v.clone(),
            Expression::CmpEq(l, r) => Variant::Bool(l.eval(ctx).op_eq(&r.eval(ctx))),
            Expression::CmpNotEq(l, r) => Variant::Bool(!l.eval(ctx).op_eq(&r.eval(ctx))),
            Expression::CmpLt(l, r) => Variant::Bool(variant_lt(&l.eval(ctx), &r.eval(ctx))),
            Expression::CmpGt(l, r) => Variant::Bool(variant_gt(&l.eval(ctx), &r.eval(ctx))),
            Expression::CmpLte(l, r) => Variant::Bool(variant_lte(&l.eval(ctx), &r.eval(ctx))),
            Expression::CmpGte(l, r) => Variant::Bool(variant_gte(&l.eval(ctx), &r.eval(ctx))),
            Expression::LogicalAnd(l, r) => {
                Variant::Bool(l.eval(ctx).is_truthy() && r.eval(ctx).is_truthy())
            }
            Expression::LogicalOr(l, r) => {
                Variant::Bool(l.eval(ctx).is_truthy() || r.eval(ctx).is_truthy())
            }
            Expression::LogicalNegation(v) => Variant::Bool(!v.eval(ctx).is_truthy()),
            Expression::UnaryPlus(v) => v.eval(ctx).op_unary_plus(),
            Expression::UnaryNegation(v) => v.eval(ctx).op_unary_negation(),
            _ => Variant::Nil,
        }
    }
}
