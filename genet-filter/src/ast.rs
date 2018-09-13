use ops::*;
use genet_abi::variant::Variant;

pub struct Context {}

#[derive(Clone, Debug)]
pub enum Expression {
    Literal(Variant),
    CmpEq(Box<Expression>, Box<Expression>),
    CmpLt(Box<Expression>, Box<Expression>),
    CmpGt(Box<Expression>, Box<Expression>),
    CmpLte(Box<Expression>, Box<Expression>),
    CmpGte(Box<Expression>, Box<Expression>),
    LogicalAnd(Box<Expression>, Box<Expression>),
    LogicalOr(Box<Expression>, Box<Expression>),
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
