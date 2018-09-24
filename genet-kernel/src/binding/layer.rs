use binding::JsClass;
use genet_abi::{
    attr::Attr,
    layer::{Layer, Payload},
    token::Token,
};
use genet_napi::napi::{
    CallbackInfo, Env, HandleScope, PropertyAttributes, PropertyDescriptor, Result, Status, Value,
    ValueRef, ValueType,
};
use std::{ptr, rc::Rc};

#[repr(C)]
pub struct Range {
    start: u32,
    end: u32,
}

#[no_mangle]
pub unsafe extern "C" fn genet_layer_id(layer: *const Layer) -> Token {
    (*layer).id()
}

#[no_mangle]
pub unsafe extern "C" fn genet_layer_attr(layer: *const Layer, id: Token) -> *const Attr {
    if let Some(attr) = (*layer).attr(id) {
        attr
    } else {
        ptr::null()
    }
}

#[no_mangle]
pub unsafe extern "C" fn genet_layer_data(layer: *const Layer, len: *mut u64) -> *const u8 {
    let data = (*layer).data();
    *len = data.len() as u64;
    data.as_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn genet_layer_attrs(
    layer: *const Layer,
    len: *mut u32,
) -> *const *const Attr {
    let attrs = (*layer).attrs();
    *len = attrs.len() as u32;
    attrs.as_ptr() as *const *const Attr
}

#[no_mangle]
pub unsafe extern "C" fn genet_layer_headers(
    layer: *const Layer,
    len: *mut u32,
) -> *const *const Attr {
    let headers = (*layer).headers();
    *len = headers.len() as u32;
    headers.as_ptr() as *const *const Attr
}

#[no_mangle]
pub unsafe extern "C" fn genet_layer_payloads(
    layer: *const Layer,
    len: *mut u32,
) -> *const Payload {
    let payloads = (*layer).payloads();
    *len = payloads.len() as u32;
    payloads.as_ptr()
}

pub fn wrapper(env: &Env) -> Rc<ValueRef> {
    fn ctor<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
        env.get_null()
    }

    fn layer_id<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
        let layer = env.unwrap::<Layer>(info.this())?;
        env.create_string(&layer.id().to_string())
    }

    fn layer_attr<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
        let layer = env.unwrap::<Layer>(info.this())?;
        if let Some(id) = info.argv().get(0) {
            let id = match env.type_of(id)? {
                ValueType::Number => Token::from(env.get_value_uint32(id)?),
                _ => Token::from(env.get_value_string(env.coerce_to_string(id)?)?.as_str()),
            };
            if let Some(attr) = layer.attr(id) {
                let attr_class = env.get_constructor(JsClass::Attr as usize).unwrap();
                let instance = env.new_instance(&attr_class, &[])?;
                env.wrap_ptr(instance, attr)?;
                Ok(instance)
            } else {
                env.get_null()
            }
        } else {
            Err(Status::InvalidArg)
        }
    }

    fn layer_attrs<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
        let layer = env.unwrap::<Layer>(info.this())?;
        let headers = layer.headers();
        let attrs = layer.attrs();
        let attr_class = env.get_constructor(JsClass::Attr as usize).unwrap();
        let array = env.create_array(headers.len() + attrs.len())?;
        for i in 0..headers.len() {
            let instance = env.new_instance(&attr_class, &[])?;
            env.wrap_fixed(instance, &headers[i])?;
            env.set_element(array, i as u32, instance)?;
        }
        for i in 0..attrs.len() {
            let instance = env.new_instance(&attr_class, &[])?;
            env.wrap_fixed(instance, &attrs[i])?;
            env.set_element(array, (headers.len() + i) as u32, instance)?;
        }
        Ok(array)
    }

    fn layer_payloads<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
        let layer = env.unwrap::<Layer>(info.this())?;
        let payloads = layer.payloads();
        let array = env.create_array(payloads.len())?;
        for i in 0..payloads.len() {
            let paylaod = &payloads[i];
            let object = env.create_object()?;
            env.set_named_property(object, "id", env.create_string(&paylaod.id().to_string())?)?;
            env.set_named_property(
                object,
                "type",
                env.create_string(&paylaod.typ().to_string())?,
            )?;
            env.set_named_property(
                object,
                "data",
                env.create_arraybuffer_from_slice(&paylaod.data())?,
            )?;
            env.set_element(array, i as u32, object)?;
        }
        Ok(array)
    }

    fn layer_data<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
        let layer = env.unwrap::<Layer>(info.this())?;
        env.create_arraybuffer_from_slice(&layer.data())
    }

    let class = env
        .define_class(
            "Layer",
            ctor,
            &[
                PropertyDescriptor::new_property(
                    env,
                    "id",
                    PropertyAttributes::Default,
                    layer_id,
                    false,
                ),
                PropertyDescriptor::new_method(
                    env,
                    "attrs",
                    PropertyAttributes::Default,
                    layer_attr,
                ),
                PropertyDescriptor::new_property(
                    env,
                    "attrs",
                    PropertyAttributes::Default,
                    layer_attrs,
                    false,
                ),
                PropertyDescriptor::new_property(
                    env,
                    "payloads",
                    PropertyAttributes::Default,
                    layer_payloads,
                    false,
                ),
                PropertyDescriptor::new_property(
                    env,
                    "data",
                    PropertyAttributes::Default,
                    layer_data,
                    false,
                ),
            ],
        ).unwrap();
    env.create_ref(class)
}
