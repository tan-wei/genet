use num_bigint::{BigInt, Sign};
use std::{io, result::Result};
use variant::Variant;

pub fn variant_lt(lhs: &Variant, rhs: &Variant) -> bool {
    match (lhs, rhs) {
        (Variant::Int64(a), Variant::Int64(b)) => a < b,
        (Variant::UInt64(a), Variant::UInt64(b)) => a < b,
        (Variant::Float64(a), Variant::Float64(b)) => a < b,

        (Variant::Int64(a), Variant::UInt64(b)) => *a < *b as i64,
        (Variant::Int64(a), Variant::Float64(b)) => *a < *b as i64,
        (Variant::UInt64(a), Variant::Int64(b)) => *a < *b as u64,
        (Variant::UInt64(a), Variant::Float64(b)) => *a < *b as u64,
        (Variant::Float64(a), Variant::Int64(b)) => *a < *b as f64,
        (Variant::Float64(a), Variant::UInt64(b)) => *a < *b as f64,

        (Variant::Buffer(a), Variant::Buffer(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) < BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::Slice(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) < BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Buffer(a), Variant::Slice(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) < BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::Buffer(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) < BigInt::from_bytes_be(Sign::Plus, &b)
        }

        (Variant::Int64(a), Variant::Buffer(b)) => {
            BigInt::from(*a) < BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Buffer(a), Variant::Int64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) < BigInt::from(*b)
        }
        (Variant::UInt64(a), Variant::Buffer(b)) => {
            BigInt::from(*a) < BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Buffer(a), Variant::UInt64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) < BigInt::from(*b)
        }
        (Variant::Float64(a), Variant::Buffer(b)) => {
            BigInt::from(*a as u64) < BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Buffer(a), Variant::Float64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) < BigInt::from(*b as u64)
        }

        (Variant::Int64(a), Variant::Slice(b)) => {
            BigInt::from(*a) < BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::Int64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) < BigInt::from(*b)
        }
        (Variant::UInt64(a), Variant::Slice(b)) => {
            BigInt::from(*a) < BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::UInt64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) < BigInt::from(*b)
        }
        (Variant::Float64(a), Variant::Slice(b)) => {
            BigInt::from(*a as u64) < BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::Float64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) < BigInt::from(*b as u64)
        }

        _ => false,
    }
}

pub fn variant_gt(lhs: &Variant, rhs: &Variant) -> bool {
    match (lhs, rhs) {
        (Variant::Int64(a), Variant::Int64(b)) => a > b,
        (Variant::UInt64(a), Variant::UInt64(b)) => a > b,
        (Variant::Float64(a), Variant::Float64(b)) => a > b,

        (Variant::Int64(a), Variant::UInt64(b)) => *a > *b as i64,
        (Variant::Int64(a), Variant::Float64(b)) => *a > *b as i64,
        (Variant::UInt64(a), Variant::Int64(b)) => *a > *b as u64,
        (Variant::UInt64(a), Variant::Float64(b)) => *a > *b as u64,
        (Variant::Float64(a), Variant::Int64(b)) => *a > *b as f64,
        (Variant::Float64(a), Variant::UInt64(b)) => *a > *b as f64,

        (Variant::Buffer(a), Variant::Buffer(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) > BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::Slice(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) > BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Buffer(a), Variant::Slice(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) > BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::Buffer(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) > BigInt::from_bytes_be(Sign::Plus, &b)
        }

        (Variant::Int64(a), Variant::Buffer(b)) => {
            BigInt::from(*a) > BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Buffer(a), Variant::Int64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) > BigInt::from(*b)
        }
        (Variant::UInt64(a), Variant::Buffer(b)) => {
            BigInt::from(*a) > BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Buffer(a), Variant::UInt64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) > BigInt::from(*b)
        }
        (Variant::Float64(a), Variant::Buffer(b)) => {
            BigInt::from(*a as u64) > BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Buffer(a), Variant::Float64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) > BigInt::from(*b as u64)
        }

        (Variant::Int64(a), Variant::Slice(b)) => {
            BigInt::from(*a) > BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::Int64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) > BigInt::from(*b)
        }
        (Variant::UInt64(a), Variant::Slice(b)) => {
            BigInt::from(*a) > BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::UInt64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) > BigInt::from(*b)
        }
        (Variant::Float64(a), Variant::Slice(b)) => {
            BigInt::from(*a as u64) > BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::Float64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) > BigInt::from(*b as u64)
        }

        _ => false,
    }
}

pub fn variant_lte(lhs: &Variant, rhs: &Variant) -> bool {
    match (lhs, rhs) {
        (Variant::Int64(a), Variant::Int64(b)) => a <= b,
        (Variant::UInt64(a), Variant::UInt64(b)) => a <= b,
        (Variant::Float64(a), Variant::Float64(b)) => a <= b,

        (Variant::Int64(a), Variant::UInt64(b)) => *a <= *b as i64,
        (Variant::Int64(a), Variant::Float64(b)) => *a <= *b as i64,
        (Variant::UInt64(a), Variant::Int64(b)) => *a <= *b as u64,
        (Variant::UInt64(a), Variant::Float64(b)) => *a <= *b as u64,
        (Variant::Float64(a), Variant::Int64(b)) => *a <= *b as f64,
        (Variant::Float64(a), Variant::UInt64(b)) => *a <= *b as f64,

        (Variant::Buffer(a), Variant::Buffer(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) <= BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::Slice(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) <= BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Buffer(a), Variant::Slice(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) <= BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::Buffer(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) <= BigInt::from_bytes_be(Sign::Plus, &b)
        }

        (Variant::Int64(a), Variant::Buffer(b)) => {
            BigInt::from(*a) <= BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Buffer(a), Variant::Int64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) <= BigInt::from(*b)
        }
        (Variant::UInt64(a), Variant::Buffer(b)) => {
            BigInt::from(*a) <= BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Buffer(a), Variant::UInt64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) <= BigInt::from(*b)
        }
        (Variant::Float64(a), Variant::Buffer(b)) => {
            BigInt::from(*a as u64) <= BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Buffer(a), Variant::Float64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) <= BigInt::from(*b as u64)
        }

        (Variant::Int64(a), Variant::Slice(b)) => {
            BigInt::from(*a) <= BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::Int64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) <= BigInt::from(*b)
        }
        (Variant::UInt64(a), Variant::Slice(b)) => {
            BigInt::from(*a) <= BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::UInt64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) <= BigInt::from(*b)
        }
        (Variant::Float64(a), Variant::Slice(b)) => {
            BigInt::from(*a as u64) <= BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::Float64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) <= BigInt::from(*b as u64)
        }

        _ => lhs.op_eq(rhs) || variant_lt(lhs, rhs),
    }
}

pub fn variant_gte(lhs: &Variant, rhs: &Variant) -> bool {
    match (lhs, rhs) {
        (Variant::Int64(a), Variant::Int64(b)) => a >= b,
        (Variant::UInt64(a), Variant::UInt64(b)) => a >= b,
        (Variant::Float64(a), Variant::Float64(b)) => a >= b,

        (Variant::Int64(a), Variant::UInt64(b)) => *a >= *b as i64,
        (Variant::Int64(a), Variant::Float64(b)) => *a >= *b as i64,
        (Variant::UInt64(a), Variant::Int64(b)) => *a >= *b as u64,
        (Variant::UInt64(a), Variant::Float64(b)) => *a >= *b as u64,
        (Variant::Float64(a), Variant::Int64(b)) => *a >= *b as f64,
        (Variant::Float64(a), Variant::UInt64(b)) => *a >= *b as f64,

        (Variant::Buffer(a), Variant::Buffer(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) >= BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::Slice(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) >= BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Buffer(a), Variant::Slice(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) >= BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::Buffer(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) >= BigInt::from_bytes_be(Sign::Plus, &b)
        }

        (Variant::Int64(a), Variant::Buffer(b)) => {
            BigInt::from(*a) >= BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Buffer(a), Variant::Int64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) >= BigInt::from(*b)
        }
        (Variant::UInt64(a), Variant::Buffer(b)) => {
            BigInt::from(*a) >= BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Buffer(a), Variant::UInt64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) >= BigInt::from(*b)
        }
        (Variant::Float64(a), Variant::Buffer(b)) => {
            BigInt::from(*a as u64) >= BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Buffer(a), Variant::Float64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) >= BigInt::from(*b as u64)
        }

        (Variant::Int64(a), Variant::Slice(b)) => {
            BigInt::from(*a) >= BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::Int64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) >= BigInt::from(*b)
        }
        (Variant::UInt64(a), Variant::Slice(b)) => {
            BigInt::from(*a) >= BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::UInt64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) >= BigInt::from(*b)
        }
        (Variant::Float64(a), Variant::Slice(b)) => {
            BigInt::from(*a as u64) >= BigInt::from_bytes_be(Sign::Plus, &b)
        }
        (Variant::Slice(a), Variant::Float64(b)) => {
            BigInt::from_bytes_be(Sign::Plus, &a) >= BigInt::from(*b as u64)
        }

        _ => lhs.op_eq(rhs) || variant_gt(lhs, rhs),
    }
}
