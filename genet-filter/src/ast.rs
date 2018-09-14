use genet_abi::token::Token;
use ops::*;
use variant::Variant;

pub struct Context {}

#[derive(Clone, Debug)]
pub enum Expression {
    Literal(Variant),
    Id(String, Vec<String>),
    Token(Token),
    Multiplication(Box<Expression>, Box<Expression>),
    Division(Box<Expression>, Box<Expression>),
    Reminder(Box<Expression>, Box<Expression>),
    Addition(Box<Expression>, Box<Expression>),
    Subtraction(Box<Expression>, Box<Expression>),
    LeftShift(Box<Expression>, Box<Expression>),
    RightShift(Box<Expression>, Box<Expression>),
    UnsignedRightShift(Box<Expression>, Box<Expression>),
    CmpEq(Box<Expression>, Box<Expression>),
    CmpNotEq(Box<Expression>, Box<Expression>),
    CmpLt(Box<Expression>, Box<Expression>),
    CmpGt(Box<Expression>, Box<Expression>),
    CmpLte(Box<Expression>, Box<Expression>),
    CmpGte(Box<Expression>, Box<Expression>),
    BitwiseAnd(Box<Expression>, Box<Expression>),
    BitwiseOr(Box<Expression>, Box<Expression>),
    BitwiseXor(Box<Expression>, Box<Expression>),
    LogicalAnd(Box<Expression>, Box<Expression>),
    LogicalOr(Box<Expression>, Box<Expression>),
    LogicalNegation(Box<Expression>),
    BitwiseNot(Box<Expression>),
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

    fn constant(self) -> Result<Variant, Expression> {
        Ok(match self {
            Expression::Literal(v) => v.clone(),
            Expression::CmpEq(l, r) => Variant::Bool(l.constant()?.op_eq(&r.constant()?)),
            Expression::CmpNotEq(l, r) => Variant::Bool(!l.constant()?.op_eq(&r.constant()?)),
            Expression::CmpLt(l, r) => Variant::Bool(variant_lt(&l.constant()?, &r.constant()?)),
            Expression::CmpGt(l, r) => Variant::Bool(variant_gt(&l.constant()?, &r.constant()?)),
            Expression::CmpLte(l, r) => Variant::Bool(variant_lte(&l.constant()?, &r.constant()?)),
            Expression::CmpGte(l, r) => Variant::Bool(variant_gte(&l.constant()?, &r.constant()?)),
            Expression::LogicalAnd(l, r) => {
                Variant::Bool(l.constant()?.is_truthy() && r.constant()?.is_truthy())
            }
            Expression::LogicalOr(l, r) => {
                Variant::Bool(l.constant()?.is_truthy() || r.constant()?.is_truthy())
            }
            Expression::LogicalNegation(v) => Variant::Bool(!v.constant()?.is_truthy()),
            Expression::UnaryPlus(v) => v.constant()?.op_unary_plus(),
            Expression::UnaryNegation(v) => v.constant()?.op_unary_negation(),
            _ => return Err(self),
        })
    }

    pub fn fold(self) -> Expression {
        match self.constant() {
            Ok(v) => Expression::Literal(v),
            Err(e) => e,
        }
    }
}
