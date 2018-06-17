use attr::Attr;
use ptr::Ptr;
use slice::Slice;
use std::slice;
use token::Token;

#[repr(C)]
pub struct Layer {
    class: Ptr<LayerClass>,
    ffi_unsafe_data: LayerData,
}

unsafe impl Send for Layer {}

struct LayerData {
    data: Slice,
    attrs: Vec<Ptr<Attr>>,
    payloads: Vec<Payload>,
}

impl Layer {
    pub fn new(class: &Ptr<LayerClass>, data: Slice) -> Layer {
        Layer {
            class: class.clone(),
            ffi_unsafe_data: LayerData {
                data,
                attrs: Vec::new(),
                payloads: Vec::new(),
            },
        }
    }

    pub fn id(&self) -> Token {
        self.class.id()
    }

    pub fn data(&self) -> Slice {
        self.class.data(self)
    }

    pub fn attrs(&self) -> impl Iterator<Item = &Attr> {
        self.class.attrs(self)
    }

    pub fn add_attr(&mut self, attr: Attr) {
        let func = self.class.add_attr;
        (func)(self, Ptr::new(attr));
    }

    pub fn payloads(&self) -> impl Iterator<Item = &Payload> {
        self.class.payloads(self)
    }

    pub fn add_payload(&mut self, data: Slice, typ: Token) {
        let func = self.class.add_payload;
        (func)(self, data.as_ptr(), data.len() as u64, typ);
    }
}

#[repr(C)]
pub struct Payload {
    data: *const u8,
    len: u64,
    typ: Token,
}

impl Payload {
    pub fn typ(&self) -> Token {
        self.typ
    }

    pub fn data(&self) -> Slice {
        unsafe { slice::from_raw_parts(self.data, self.len as usize) }
    }
}

#[repr(u64)]
#[derive(PartialEq, PartialOrd)]
#[allow(dead_code)]
enum Revision {
    AddPayload = 6,
}

struct LayerClassData {
    id: Token,
}

#[repr(C)]
pub struct LayerClass {
    rev: Revision,
    ffi_unsafe_data: Ptr<LayerClassData>,
    id: extern "C" fn(class: *const LayerClass) -> Token,
    data: extern "C" fn(*const Layer, *mut u64) -> *const u8,
    attrs_len: extern "C" fn(*const Layer) -> u64,
    attrs_data: extern "C" fn(*const Layer) -> *const Ptr<Attr>,
    add_attr: extern "C" fn(*mut Layer, Ptr<Attr>),
    payloads_len: extern "C" fn(*const Layer) -> u64,
    payloads_data: extern "C" fn(*const Layer) -> *const Payload,
    add_payload: extern "C" fn(*mut Layer, *const u8, u64, Token),
}

impl LayerClass {
    pub fn new(id: Token) -> Ptr<LayerClass> {
        Ptr::new(LayerClass {
            rev: Revision::AddPayload,
            ffi_unsafe_data: Ptr::from_box(Box::new(LayerClassData { id })),
            id: ffi_id,
            data: ffi_data,
            attrs_len: ffi_attrs_len,
            attrs_data: ffi_attrs_data,
            add_attr: ffi_add_attr,
            payloads_len: ffi_payloads_len,
            payloads_data: ffi_payloads_data,
            add_payload: ffi_add_payload,
        })
    }

    fn id(&self) -> Token {
        (self.id)(self)
    }

    fn data(&self, layer: &Layer) -> Slice {
        let mut len = 0;
        let data = (self.data)(layer, &mut len);
        unsafe { slice::from_raw_parts(data, len as usize) }
    }

    fn attrs(&self, layer: &Layer) -> impl Iterator<Item = &Attr> {
        let data = (self.attrs_data)(layer);
        let len = (self.attrs_len)(layer) as usize;
        let iter = unsafe { slice::from_raw_parts(data, len).iter() };
        iter.map(|v| &**v)
    }

    fn payloads(&self, layer: &Layer) -> impl Iterator<Item = &Payload> {
        let data = (self.payloads_data)(layer);
        let len = (self.payloads_len)(layer) as usize;
        let iter = unsafe { slice::from_raw_parts(data, len).iter() };
        iter.map(|v| &*v)
    }
}

extern "C" fn ffi_id(class: *const LayerClass) -> Token {
    unsafe { (*class).ffi_unsafe_data.id }
}

extern "C" fn ffi_data(layer: *const Layer, len: *mut u64) -> *const u8 {
    unsafe {
        let data = (*layer).ffi_unsafe_data.data;
        *len = data.len() as u64;
        data.as_ptr()
    }
}

extern "C" fn ffi_attrs_len(layer: *const Layer) -> u64 {
    unsafe { (*layer).ffi_unsafe_data.attrs.len() as u64 }
}

extern "C" fn ffi_attrs_data(layer: *const Layer) -> *const Ptr<Attr> {
    unsafe { (*layer).ffi_unsafe_data.attrs.as_ptr() }
}

extern "C" fn ffi_add_attr(layer: *mut Layer, attr: Ptr<Attr>) {
    let attrs = unsafe { &mut (*layer).ffi_unsafe_data.attrs };
    attrs.push(attr);
}

extern "C" fn ffi_payloads_len(layer: *const Layer) -> u64 {
    unsafe { (*layer).ffi_unsafe_data.payloads.len() as u64 }
}

extern "C" fn ffi_payloads_data(layer: *const Layer) -> *const Payload {
    unsafe { (*layer).ffi_unsafe_data.payloads.as_ptr() }
}

extern "C" fn ffi_add_payload(layer: *mut Layer, data: *const u8, len: u64, typ: Token) {
    let payloads = unsafe { &mut (*layer).ffi_unsafe_data.payloads };
    payloads.push(Payload { data, len, typ });
}

#[cfg(test)]
mod tests {
    use attr::{Attr, AttrClass};
    use decoder::Decoder;
    use env;
    use layer::*;
    use layer::{Layer, LayerClass};
    use slice::Slice;
    use std::io::Result;
    use variant::Variant;

    #[test]
    fn id() {
        let id = 123;
        let class = LayerClass::new(id);
        let layer = Layer::new(&class, &[]);
        assert_eq!(layer.id(), id);
    }

    #[test]
    fn data() {
        let data = b"hello";
        let class = LayerClass::new(0);
        let layer = Layer::new(&class, &data[..]);
        assert_eq!(layer.data(), &data[..]);
    }

    #[test]
    fn payloads() {
        let class = LayerClass::new(0);
        let mut layer = Layer::new(&class, &[]);
        assert!(layer.payloads().next().is_none());

        let count = 100;
        let data = b"hello";

        for i in 0..count {
            layer.add_payload(data, i);
        }

        let mut iter = layer.payloads();
        for i in 0..count {
            let payload = iter.next().unwrap();
            assert_eq!(payload.data(), data);
            assert_eq!(payload.typ(), i);
        }
        assert!(iter.next().is_none());
    }

    #[test]
    fn attrs() {
        let class = LayerClass::new(0);
        let mut layer = Layer::new(&class, &[]);
        assert!(layer.attrs().next().is_none());

        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn decode(&self, _: &Slice) -> Result<Variant> {
                Ok(Variant::Nil)
            }
        }
        let class = AttrClass::new(env::token("nil"), env::token("@nil"), TestDecoder {});

        let count = 100;
        for i in 0..count {
            let attr = Attr::new(&class, 0..i);
            layer.add_attr(attr);
        }
        let mut iter = layer.attrs();
        for i in 0..count {
            let attr = iter.next().unwrap();
            assert_eq!(attr.id(), env::token("nil"));
            assert_eq!(attr.typ(), env::token("@nil"));
            assert_eq!(attr.range(), 0..i);
        }
        assert!(iter.next().is_none());
    }
}
