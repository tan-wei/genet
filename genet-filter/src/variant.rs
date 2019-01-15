use genet_abi::variant::Variant;
use num_bigint::{BigInt, Sign};
use num_traits::ToPrimitive;
use serde_json;
use std::cmp::Ordering;

pub trait VariantExt {
    fn shrink(self) -> Variant;
    fn is_truthy(&self) -> bool;
    fn op_unary_plus(&self) -> Variant;
    fn op_unary_negation(&self) -> Variant;
    fn ord(&self, other: &Variant) -> Option<Ordering>;
    fn op_lt(&self, other: &Variant) -> bool;
    fn op_gt(&self, other: &Variant) -> bool;
    fn op_lte(&self, other: &Variant) -> bool;
    fn op_gte(&self, other: &Variant) -> bool;
    fn op_eq(&self, other: &Variant) -> bool;
    fn to_string(&self) -> String;
}

impl VariantExt for Variant {
    fn shrink(self) -> Variant {
        if let Variant::BigInt(v) = &self {
            if let Some(i) = v.to_u64() {
                return Variant::UInt64(i);
            } else if let Some(i) = v.to_i64() {
                return Variant::Int64(i);
            }
        }
        self
    }

    fn is_truthy(&self) -> bool {
        match self {
            Variant::Nil | Variant::Bool(false) => false,
            _ => true,
        }
    }

    fn op_unary_plus(&self) -> Variant {
        match self {
            Variant::Int64(_) | Variant::UInt64(_) | Variant::Float64(_) | Variant::BigInt(_) => {
                self.clone()
            }
            _ => Variant::Int64(0),
        }
    }

    fn op_unary_negation(&self) -> Variant {
        match self {
            Variant::Int64(v) => Variant::Int64(-v),
            Variant::UInt64(v) => Variant::Int64(-(*v as i64)),
            Variant::Float64(v) => Variant::Float64(-v),
            Variant::BigInt(v) => Variant::BigInt(-v),
            _ => Variant::Int64(0),
        }
    }

    fn ord(&self, other: &Variant) -> Option<Ordering> {
        let lhs = match self {
            Variant::Buffer(v) => Variant::BigInt(BigInt::from_bytes_be(Sign::Plus, &v)),
            Variant::Slice(v) => Variant::BigInt(BigInt::from_bytes_be(Sign::Plus, &v)),
            _ => self.clone(),
        };
        let rhs = match other {
            Variant::Buffer(v) => Variant::BigInt(BigInt::from_bytes_be(Sign::Plus, &v)),
            Variant::Slice(v) => Variant::BigInt(BigInt::from_bytes_be(Sign::Plus, &v)),
            _ => other.clone(),
        };

        match (&lhs, &rhs) {
            (Variant::Int64(a), Variant::Int64(b)) => a.partial_cmp(b),
            (Variant::UInt64(a), Variant::UInt64(b)) => a.partial_cmp(b),
            (Variant::Float64(a), Variant::Float64(b)) => a.partial_cmp(b),
            (Variant::BigInt(a), Variant::BigInt(b)) => a.partial_cmp(b),

            (Variant::Int64(a), Variant::UInt64(b)) => a.partial_cmp(&(*b as i64)),
            (Variant::Int64(a), Variant::Float64(b)) => a.partial_cmp(&(*b as i64)),
            (Variant::UInt64(a), Variant::Int64(b)) => a.partial_cmp(&(*b as u64)),
            (Variant::UInt64(a), Variant::Float64(b)) => a.partial_cmp(&(*b as u64)),
            (Variant::Float64(a), Variant::Int64(b)) => a.partial_cmp(&(*b as f64)),
            (Variant::Float64(a), Variant::UInt64(b)) => a.partial_cmp(&(*b as f64)),

            (Variant::Int64(a), Variant::BigInt(b)) => BigInt::from(*a).partial_cmp(&b),
            (Variant::UInt64(a), Variant::BigInt(b)) => BigInt::from(*a).partial_cmp(&b),
            (Variant::Float64(a), Variant::BigInt(b)) => BigInt::from(*a as i64).partial_cmp(&b),

            (Variant::BigInt(a), Variant::Int64(b)) => a.partial_cmp(&BigInt::from(*b)),
            (Variant::BigInt(a), Variant::UInt64(b)) => a.partial_cmp(&BigInt::from(*b)),
            (Variant::BigInt(a), Variant::Float64(b)) => a.partial_cmp(&BigInt::from(*b as i64)),
            _ => None,
        }
    }

    fn op_lt(&self, other: &Variant) -> bool {
        match self.ord(other) {
            Some(Ordering::Less) => true,
            _ => false,
        }
    }

    fn op_gt(&self, other: &Variant) -> bool {
        match self.ord(other) {
            Some(Ordering::Greater) => true,
            _ => false,
        }
    }

    fn op_lte(&self, other: &Variant) -> bool {
        match self.ord(other) {
            Some(Ordering::Less) | Some(Ordering::Equal) => true,
            _ => false,
        }
    }

    fn op_gte(&self, other: &Variant) -> bool {
        match self.ord(other) {
            Some(Ordering::Greater) | Some(Ordering::Equal) => true,
            _ => false,
        }
    }

    fn op_eq(&self, other: &Variant) -> bool {
        if self == other {
            return true;
        }
        match self.ord(other) {
            Some(Ordering::Equal) => true,
            _ => false,
        }
    }

    fn to_string(&self) -> String {
        match self {
            Variant::Nil => "nil".to_string(),
            Variant::Bool(v) => if *v { "true" } else { "false" }.to_string(),
            Variant::Int64(v) => format!("{}", v),
            Variant::UInt64(v) => format!("{}", v),
            Variant::Float64(v) => format!("{}", v),
            Variant::BigInt(b) => "0x".to_string() + &b.to_str_radix(16),
            Variant::Buffer(b) => {
                "0x".to_string() + &BigInt::from_bytes_be(Sign::Plus, &b).to_str_radix(16)
            }
            Variant::Slice(b) => {
                "0x".to_string() + &BigInt::from_bytes_be(Sign::Plus, &b).to_str_radix(16)
            }
        }
    }
}
