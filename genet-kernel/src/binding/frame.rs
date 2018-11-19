use binding::JsClass;
use frame::Frame;
use genet_abi::token::Token;
use genet_napi::napi::{
    CallbackInfo, Env, PropertyAttributes, PropertyDescriptor, Result, Status, Value, ValueRef,
    ValueType,
};
use std::rc::Rc;

pub fn wrapper(env: &Env) -> Rc<ValueRef> {
    fn ctor<'env>(env: &'env Env, _info: &CallbackInfo) -> Result<&'env Value> {
        env.get_null()
    }

    fn frame_index<'env>(env: &'env Env, info: &CallbackInfo) -> Result<&'env Value> {
        let frame = env.unwrap::<Frame>(info.this())?;
        env.create_uint32(frame.index())
    }

    fn frame_tree_indices<'env>(env: &'env Env, info: &CallbackInfo) -> Result<&'env Value> {
        let frame = env.unwrap::<Frame>(info.this())?;
        let indices = frame.tree_indices();
        let array = env.create_array(indices.len())?;
        for (i, item) in indices.iter().enumerate() {
            env.set_element(array, i as u32, env.create_uint32(u32::from(*item))?)?;
        }
        Ok(array)
    }

    fn frame_query<'env>(env: &'env Env, info: &CallbackInfo) -> Result<&'env Value> {
        let frame = env.unwrap::<Frame>(info.this())?;
        if let Some(id) = info.argv().get(0) {
            let id = match env.type_of(id)? {
                ValueType::Number => Token::from(env.get_value_uint32(id)?),
                _ => Token::from(env.get_value_string(env.coerce_to_string(id)?)?.as_str()),
            };

            for layer in frame.layers().iter().rev() {
                if layer.id() == id {
                    let layer_class = env.get_constructor(JsClass::Layer as usize).unwrap();
                    let instance = env.new_instance(&layer_class, &[])?;
                    env.wrap_mut_fixed(instance, layer)?;
                    return Ok(instance);
                }
                if let Some(attr) = layer.attr(id) {
                    let attr_class = env.get_constructor(JsClass::Attr as usize).unwrap();
                    let instance = env.new_instance(&attr_class, &[])?;
                    env.wrap(instance, attr)?;
                    return Ok(instance);
                }
            }
            env.get_null()
        } else {
            Err(Status::InvalidArg)
        }
    }

    fn frame_layers<'env>(env: &'env Env, info: &CallbackInfo) -> Result<&'env Value> {
        let frame = env.unwrap::<Frame>(info.this())?;
        let layers = frame.layers();
        let layer_class = env.get_constructor(JsClass::Layer as usize).unwrap();
        let array = env.create_array(layers.len())?;
        for (i, item) in layers.iter().enumerate() {
            let instance = env.new_instance(&layer_class, &[])?;
            env.wrap_mut_fixed(instance, item)?;
            env.set_element(array, i as u32, instance)?;
        }
        Ok(array)
    }

    let class = env
        .define_class(
            "Frame",
            ctor,
            &[
                PropertyDescriptor::new_property(
                    env,
                    "index",
                    PropertyAttributes::DEFAULT,
                    frame_index,
                    false,
                ),
                PropertyDescriptor::new_property(
                    env,
                    "layers",
                    PropertyAttributes::DEFAULT,
                    frame_layers,
                    false,
                ),
                PropertyDescriptor::new_property(
                    env,
                    "treeIndices",
                    PropertyAttributes::DEFAULT,
                    frame_tree_indices,
                    false,
                ),
                PropertyDescriptor::new_method(
                    env,
                    "query",
                    PropertyAttributes::DEFAULT,
                    frame_query,
                ),
            ],
        )
        .unwrap();
    env.create_ref(class)
}
