use genet_abi::variant::Variant;
use ops::*;

pub struct Context {}

#[derive(Clone, Debug)]
pub enum Expression {
    Literal(Variant),
    Multiplication(Box<Expression>, Box<Expression>),
    Division(Box<Expression>, Box<Expression>),
    Reminder(Box<Expression>, Box<Expression>),
    Addition(Box<Expression>, Box<Expression>),
    Subtraction(Box<Expression>, Box<Expression>),
    LeftShift(Box<Expression>, Box<Expression>),
    RightShit(Box<Expression>, Box<Expression>),
    UnsignedRightShit(Box<Expression>, Box<Expression>),
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
            Expression::CmpEq(l, r) => Variant::Bool(variant_eq(&l.eval(ctx), &r.eval(ctx))),
            Expression::CmpLt(l, r) => Variant::Bool(variant_lt(&l.eval(ctx), &r.eval(ctx))),
            Expression::CmpGt(l, r) => Variant::Bool(variant_gt(&l.eval(ctx), &r.eval(ctx))),
            Expression::CmpLte(l, r) => Variant::Bool(variant_lte(&l.eval(ctx), &r.eval(ctx))),
            Expression::CmpGte(l, r) => Variant::Bool(variant_gte(&l.eval(ctx), &r.eval(ctx))),
            Expression::LogicalAnd(l, r) => {
                Variant::Bool(is_truthy(&l.eval(ctx)) && is_truthy(&r.eval(ctx)))
            }
            Expression::LogicalOr(l, r) => {
                Variant::Bool(is_truthy(&l.eval(ctx)) || is_truthy(&r.eval(ctx)))
            }
            Expression::LogicalNegation(v) => Variant::Bool(!is_truthy(&v.eval(ctx))),
            _ => Variant::Nil,
        }
    }

    fn constant(self) -> Result<Variant, Expression> {
        match self {
            Expression::Literal(v) => Ok(v.clone()),
            Expression::CmpEq(l, r) => {
                Ok(Variant::Bool(variant_eq(&l.constant()?, &r.constant()?)))
            }
            Expression::LogicalAnd(l, r) => Ok(Variant::Bool(
                is_truthy(&l.constant()?) && is_truthy(&r.constant()?),
            )),
            Expression::LogicalNegation(v) => Ok(Variant::Bool(!is_truthy(&v.constant()?))),
            _ => Err(self),
        }
    }

    pub fn fold(self) -> Expression {
        match self.constant() {
            Ok(v) => Expression::Literal(v),
            Err(e) => e,
        }
    }
}
