use binding::{attr::AttrWrapper, JsClass};
use genet_abi::{layer::Layer, token::Token};
use genet_napi::napi::{
    CallbackInfo, Env, PropertyAttributes, PropertyDescriptor, Result, Status, TypedArrayType,
    Value, ValueRef, ValueType,
};
use std::rc::Rc;

pub fn wrapper(env: &Env) -> Rc<ValueRef> {
    fn ctor<'env>(env: &'env Env, _info: &CallbackInfo) -> Result<&'env Value> {
        env.get_null()
    }

    fn layer_id<'env>(env: &'env Env, info: &CallbackInfo) -> Result<&'env Value> {
        let layer = env.unwrap::<Layer>(info.this())?;
        env.create_string(&layer.id().to_string())
    }

    fn layer_attr<'env>(env: &'env Env, info: &CallbackInfo) -> Result<&'env Value> {
        let layer = env.unwrap::<Layer>(info.this())?;
        if let Some(id) = info.argv().get(0) {
            let id = match env.type_of(id)? {
                ValueType::Number => Token::from(env.get_value_uint32(id)?),
                _ => Token::from(env.get_value_string(env.coerce_to_string(id)?)?.as_str()),
            };
            if let Some(attr) = layer.attr(id) {
                let attr_class = env.get_constructor(JsClass::Attr as usize).unwrap();
                let instance = env.new_instance(&attr_class, &[])?;
                env.wrap(instance, AttrWrapper::new(attr, layer))?;
                Ok(instance)
            } else {
                env.get_null()
            }
        } else {
            Err(Status::InvalidArg)
        }
    }

    fn layer_attrs<'env>(env: &'env Env, info: &CallbackInfo) -> Result<&'env Value> {
        let layer = env.unwrap::<Layer>(info.this())?;
        let attrs = layer.attrs().collect::<Vec<_>>();
        let attr_class = env.get_constructor(JsClass::Attr as usize).unwrap();
        let array = env.create_array(attrs.len())?;
        for i in 0..attrs.len() {
            let instance = env.new_instance(&attr_class, &[])?;
            env.wrap(instance, AttrWrapper::new(&attrs[i], layer))?;
            env.set_element(array, i as u32, instance)?;
        }
        Ok(array)
    }

    fn layer_payloads<'env>(env: &'env Env, info: &CallbackInfo) -> Result<&'env Value> {
        let layer = env.unwrap::<Layer>(info.this())?;
        let payloads = layer.payloads().collect::<Vec<_>>();
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
                env.create_typedarray(
                    TypedArrayType::Uint8Array,
                    paylaod.data().len(),
                    env.create_arraybuffer_from_slice(&paylaod.data())?,
                    0,
                )?,
            )?;
            env.set_element(array, i as u32, object)?;
        }
        Ok(array)
    }

    fn layer_children<'env>(env: &'env Env, info: &CallbackInfo) -> Result<&'env Value> {
        let layer = env.unwrap::<Layer>(info.this())?;
        let children = layer.children().collect::<Vec<_>>();
        let layer_class = env.get_constructor(JsClass::Layer as usize).unwrap();
        let array = env.create_array(children.len())?;
        for i in 0..children.len() {
            let instance = env.new_instance(&layer_class, &[])?;
            env.wrap_mut_fixed(instance, &children[i])?;
            env.set_element(array, i as u32, instance)?;
        }
        Ok(array)
    }

    fn layer_parent<'env>(env: &'env Env, info: &CallbackInfo) -> Result<&'env Value> {
        let layer = env.unwrap::<Layer>(info.this())?;
        let layer_class = env.get_constructor(JsClass::Layer as usize).unwrap();
        if let Some(parent) = layer.parent() {
            let instance = env.new_instance(&layer_class, &[])?;
            env.wrap_ptr(instance, parent)?;
            Ok(instance)
        } else {
            env.get_null()
        }
    }

    fn layer_data<'env>(env: &'env Env, info: &CallbackInfo) -> Result<&'env Value> {
        let layer = env.unwrap::<Layer>(info.this())?;
        env.create_typedarray(
            TypedArrayType::Uint8Array,
            layer.data().len(),
            env.create_arraybuffer_from_slice(&layer.data())?,
            0,
        )
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
                    "attr",
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
                    "children",
                    PropertyAttributes::Default,
                    layer_children,
                    false,
                ),
                PropertyDescriptor::new_property(
                    env,
                    "parent",
                    PropertyAttributes::Default,
                    layer_parent,
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
        )
        .unwrap();
    env.create_ref(class)
}
