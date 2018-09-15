use genet_abi::slice::ByteSlice;
use num_bigint::{BigInt, Sign};
use num_traits::ToPrimitive;
use std::cmp::Ordering;

#[derive(Debug, Clone, PartialEq)]
pub enum Variant {
    Nil,
    Bool(bool),
    Int64(i64),
    UInt64(u64),
    Float64(f64),
    BigInt(BigInt),
    String(Box<str>),
    Buffer(Box<[u8]>),
    Slice(ByteSlice),
}

impl Variant {
    pub fn shrink(self) -> Variant {
        match &self {
            Variant::BigInt(v) => {
                if let Some(i) = v.to_u64() {
                    return Variant::UInt64(i);
                } else if let Some(i) = v.to_i64() {
                    return Variant::Int64(i);
                }
            }
            _ => (),
        }
        self
    }

    pub fn is_truthy(&self) -> bool {
        match self {
            Variant::Nil | Variant::Bool(false) => false,
            _ => true,
        }
    }

    pub fn op_unary_plus(&self) -> Variant {
        match self {
            Variant::Int64(_) | Variant::UInt64(_) | Variant::Float64(_) | Variant::BigInt(_) => {
                self.clone()
            }
            _ => Variant::Int64(0),
        }
    }

    pub fn op_unary_negation(&self) -> Variant {
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
        let rhs = match self {
            Variant::Buffer(v) => Variant::BigInt(BigInt::from_bytes_be(Sign::Plus, &v)),
            Variant::Slice(v) => Variant::BigInt(BigInt::from_bytes_be(Sign::Plus, &v)),
            _ => self.clone(),
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

            (Variant::Int64(a), Variant::BigInt(b)) => BigInt::from(*a).partial_cmp(b),
            (Variant::UInt64(a), Variant::BigInt(b)) => BigInt::from(*a).partial_cmp(b),
            (Variant::Float64(a), Variant::BigInt(b)) => BigInt::from((*a as i64)).partial_cmp(b),

            (Variant::BigInt(a), Variant::Int64(b)) => a.partial_cmp(&BigInt::from(*b)),
            (Variant::BigInt(a), Variant::UInt64(b)) => a.partial_cmp(&BigInt::from(*b)),
            (Variant::BigInt(a), Variant::Float64(b)) => a.partial_cmp(&BigInt::from((*b as i64))),
            _ => return None,
        }
    }

    pub fn op_lt(&self, other: &Variant) -> bool {
        match self.ord(other) {
            Some(Ordering::Less) => true,
            _ => false,
        }
    }

    pub fn op_gt(&self, other: &Variant) -> bool {
        match self.ord(other) {
            Some(Ordering::Greater) => true,
            _ => false,
        }
    }

    pub fn op_lte(&self, other: &Variant) -> bool {
        match self.ord(other) {
            Some(Ordering::Less) | Some(Ordering::Equal) => true,
            _ => false,
        }
    }

    pub fn op_gte(&self, other: &Variant) -> bool {
        match self.ord(other) {
            Some(Ordering::Greater) | Some(Ordering::Equal) => true,
            _ => false,
        }
    }

    pub fn op_eq(&self, other: &Variant) -> bool {
        if self == other {
            return true;
        }
        match self.ord(other) {
            Some(Ordering::Equal) => true,
            _ => false,
        }
        /*
            (Variant::Slice(a), Variant::Buffer(b)) | (Variant::Buffer(b), Variant::Slice(a)) => {
                a.as_ref() == b.as_ref()
            }

            (Variant::Int64(a), Variant::UInt64(b)) | (Variant::UInt64(b), Variant::Int64(a)) => {
                *a == *b as i64
            }
            (Variant::Int64(a), Variant::Float64(b)) | (Variant::Float64(b), Variant::Int64(a)) => {
                *a == *b as i64
            }
            (Variant::UInt64(a), Variant::Float64(b))
            | (Variant::Float64(b), Variant::UInt64(a)) => *a == *b as u64,

            (Variant::Int64(a), Variant::Buffer(b)) | (Variant::Buffer(b), Variant::Int64(a)) => {
                BigInt::from(*a) == BigInt::from_bytes_be(Sign::Plus, &b)
            }
            (Variant::UInt64(a), Variant::Buffer(b)) | (Variant::Buffer(b), Variant::UInt64(a)) => {
                BigInt::from(*a) == BigInt::from_bytes_be(Sign::Plus, &b)
            }
            (Variant::Float64(a), Variant::Buffer(b))
            | (Variant::Buffer(b), Variant::Float64(a)) => {
                BigInt::from(*a as u64) == BigInt::from_bytes_be(Sign::Plus, &b)
            }

            (Variant::Int64(a), Variant::Slice(b)) | (Variant::Slice(b), Variant::Int64(a)) => {
                BigInt::from(*a) == BigInt::from_bytes_be(Sign::Plus, &b)
            }
            (Variant::UInt64(a), Variant::Slice(b)) | (Variant::Slice(b), Variant::UInt64(a)) => {
                BigInt::from(*a) == BigInt::from_bytes_be(Sign::Plus, &b)
            }
            (Variant::Float64(a), Variant::Slice(b)) | (Variant::Slice(b), Variant::Float64(a)) => {
                BigInt::from(*a as u64) == BigInt::from_bytes_be(Sign::Plus, &b)
            }

            _ => false,
        }
        */
    }
}
