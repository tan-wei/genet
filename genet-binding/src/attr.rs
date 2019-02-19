use genet_abi::{self, attr::Attr, variant::Variant};
use genet_filter::{
    ast::Expr,
    unparser::{unparse, unparse_attr},
};
use genet_napi::napi::{
    CallbackInfo, Env, PropertyAttributes, PropertyDescriptor, Result, TypedArrayType, Value,
    ValueRef,
};
use std::rc::Rc;

fn variant_to_js<'env>(
    env: &'env Env,
    value: &genet_abi::result::Result<Variant>,
) -> Result<&'env Value> {
    match value {
        Err(err) => env.create_error(
            env.create_string("")?,
            env.create_string(&format!("{}", err))?,
        ),
        Ok(Variant::Bool(b)) => env.get_boolean(*b),
        Ok(Variant::Int64(v)) => env.create_bigint_from_i64(*v),
        Ok(Variant::UInt64(v)) => env.create_bigint_from_u64(*v),
        Ok(Variant::Float64(v)) => env.create_double(*v),
        Ok(Variant::BigInt(v)) => env.create_bigint(&v),
        Ok(Variant::Buffer(v)) => env.create_typedarray(
            TypedArrayType::Uint8Array,
            v.len(),
            env.create_arraybuffer_copy(&v)?,
            0,
        ),
        Ok(Variant::Bytes(v)) => env.create_typedarray(
            TypedArrayType::Uint8Array,
            v.len(),
            env.create_arraybuffer_from_slice(&v)?,
            0,
        ),
        _ => env.get_null(),
    }
}

pub fn wrapper(env: &Env) -> Rc<ValueRef> {
    fn ctor<'env>(env: &'env Env, _info: &CallbackInfo) -> Result<&'env Value> {
        env.get_null()
    }

    fn attr_id<'env>(env: &'env Env, info: &CallbackInfo) -> Result<&'env Value> {
        let attr = env.unwrap::<Attr>(info.this())?;
        env.create_string(&attr.id().as_str())
    }

    fn attr_type<'env>(env: &'env Env, info: &CallbackInfo) -> Result<&'env Value> {
        let attr = env.unwrap::<Attr>(info.this())?;
        env.create_string(attr.typ().as_str())
    }

    fn attr_bit_range<'env>(env: &'env Env, info: &CallbackInfo) -> Result<&'env Value> {
        let attr = env.unwrap::<Attr>(info.this())?;
        let range = attr.bit_range();
        let array = env.create_array(2)?;
        env.set_element(array, 0, env.create_uint32(range.start as u32)?)?;
        env.set_element(array, 1, env.create_uint32(range.end as u32)?)?;
        Ok(array)
    }

    fn attr_range<'env>(env: &'env Env, info: &CallbackInfo) -> Result<&'env Value> {
        let attr = env.unwrap::<Attr>(info.this())?;
        let range = attr.byte_range();
        let array = env.create_array(2)?;
        env.set_element(array, 0, env.create_uint32(range.start as u32)?)?;
        env.set_element(array, 1, env.create_uint32(range.end as u32)?)?;
        Ok(array)
    }

    fn attr_value<'env>(env: &'env Env, info: &CallbackInfo) -> Result<&'env Value> {
        let attr = env.unwrap::<Attr>(info.this())?;
        variant_to_js(env, &attr.get())
    }

    fn attr_filter_expression<'env>(env: &'env Env, info: &CallbackInfo) -> Result<&'env Value> {
        let attr = env.unwrap::<Attr>(info.this())?;
        match attr.get() {
            Ok(val) => env.create_string(&unparse(&Expr::CmpEq(
                Box::new(Expr::Token(attr.id())),
                Box::new(unparse_attr(attr.typ(), &val)),
            ))),
            Err(_) => env.get_null(),
        }
    }

    let class = env
        .define_class(
            "Attr",
            ctor,
            &[
                PropertyDescriptor::new_property(
                    env,
                    "id",
                    PropertyAttributes::DEFAULT,
                    attr_id,
                    false,
                ),
                PropertyDescriptor::new_property(
                    env,
                    "type",
                    PropertyAttributes::DEFAULT,
                    attr_type,
                    false,
                ),
                PropertyDescriptor::new_property(
                    env,
                    "bitRange",
                    PropertyAttributes::DEFAULT,
                    attr_bit_range,
                    false,
                ),
                PropertyDescriptor::new_property(
                    env,
                    "range",
                    PropertyAttributes::DEFAULT,
                    attr_range,
                    false,
                ),
                PropertyDescriptor::new_property(
                    env,
                    "value",
                    PropertyAttributes::DEFAULT,
                    attr_value,
                    false,
                ),
                PropertyDescriptor::new_property(
                    env,
                    "filterExpression",
                    PropertyAttributes::DEFAULT,
                    attr_filter_expression,
                    false,
                ),
            ],
        )
        .unwrap();

    env.create_ref(class)
}
