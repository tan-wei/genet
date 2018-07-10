use attr::Attr;
use ptr::Ptr;
use slice::Slice;
use std::slice;
use token::Token;

#[repr(C)]
pub struct Layer {
    class: Ptr<LayerClass>,
    abi_unsafe_data: LayerData,
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
            abi_unsafe_data: LayerData {
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

    pub fn headers(&self) -> &[Ptr<Attr>] {
        self.class.headers()
    }

    pub fn attrs(&self) -> &[Ptr<Attr>] {
        self.class.attrs(self)
    }

    pub fn attr(&self, id: Token) -> Option<&Attr> {
        let id = self
            .class
            .aliases()
            .find(|alias| alias.id == id)
            .map(|alias| alias.target)
            .unwrap_or(id);
        self.class
            .headers()
            .iter()
            .find(|attr| attr.id() == id)
            .map(|attr| attr.as_ref())
            .or_else(|| {
                self.attrs()
                    .iter()
                    .find(|attr| attr.id() == id)
                    .map(|attr| attr.as_ref())
            })
    }

    pub fn add_attr(&mut self, attr: Attr) {
        let func = self.class.add_attr;
        (func)(self, Ptr::new(attr));
    }

    pub fn payloads(&self) -> &[Payload] {
        self.class.payloads(self)
    }

    pub fn add_payload<T: Into<Token>, U: Into<Token>>(&mut self, data: Slice, id: T, typ: U) {
        let func = self.class.add_payload;
        (func)(
            self,
            data.as_ptr(),
            data.len() as u64,
            id.into(),
            typ.into(),
        );
    }
}

#[repr(C)]
pub struct Payload {
    data: *const u8,
    len: u64,
    id: Token,
    typ: Token,
}

impl Payload {
    pub fn id(&self) -> Token {
        self.id
    }

    pub fn typ(&self) -> Token {
        self.typ
    }

    pub fn data(&self) -> Slice {
        unsafe { Slice::from_raw_parts(self.data, self.len as usize) }
    }
}

pub struct LayerBuilder {
    id: Token,
    aliases: Vec<Alias>,
    headers: Vec<Ptr<Attr>>,
}

impl LayerBuilder {
    pub fn new<T: Into<Token>>(id: T) -> LayerBuilder {
        Self {
            id: id.into(),
            aliases: Vec::new(),
            headers: Vec::new(),
        }
    }

    pub fn alias<T: Into<Token>, U: Into<Token>>(mut self, id: T, target: U) -> LayerBuilder {
        self.aliases.push(Alias {
            id: id.into(),
            target: target.into(),
        });
        self
    }

    pub fn header(mut self, attr: Attr) -> LayerBuilder {
        self.headers.push(Ptr::new(attr));
        self
    }

    pub fn build(self) -> Ptr<LayerClass> {
        Ptr::new(LayerClass {
            rev: Revision::AddPayload,
            abi_unsafe_data: Ptr::from_box(Box::new(LayerClassData {
                id: self.id,
                aliases: self.aliases,
                headers: self.headers,
            })),
            id: abi_id,
            data: abi_data,
            attrs_len: abi_attrs_len,
            attrs_data: abi_attrs_data,
            aliases_len: abi_aliases_len,
            aliases_data: abi_aliases_data,
            headers_len: abi_headers_len,
            headers_data: abi_headers_data,
            add_attr: abi_add_attr,
            payloads_len: abi_payloads_len,
            payloads_data: abi_payloads_data,
            add_payload: abi_add_payload,
        })
    }
}

#[repr(u64)]
#[derive(PartialEq, PartialOrd)]
#[allow(dead_code)]
enum Revision {
    AddPayload = 9,
}

#[repr(C)]
struct Alias {
    id: Token,
    target: Token,
}

struct LayerClassData {
    id: Token,
    aliases: Vec<Alias>,
    headers: Vec<Ptr<Attr>>,
}

#[repr(C)]
pub struct LayerClass {
    rev: Revision,
    abi_unsafe_data: Ptr<LayerClassData>,
    id: extern "C" fn(*const LayerClass) -> Token,
    aliases_len: extern "C" fn(*const LayerClass) -> u64,
    aliases_data: extern "C" fn(*const LayerClass) -> *const Alias,
    headers_len: extern "C" fn(*const LayerClass) -> u64,
    headers_data: extern "C" fn(*const LayerClass) -> *const Ptr<Attr>,
    data: extern "C" fn(*const Layer, *mut u64) -> *const u8,
    attrs_len: extern "C" fn(*const Layer) -> u64,
    attrs_data: extern "C" fn(*const Layer) -> *const Ptr<Attr>,
    add_attr: extern "C" fn(*mut Layer, Ptr<Attr>),
    payloads_len: extern "C" fn(*const Layer) -> u64,
    payloads_data: extern "C" fn(*const Layer) -> *const Payload,
    add_payload: extern "C" fn(*mut Layer, *const u8, u64, Token, Token),
}

impl LayerClass {
    fn id(&self) -> Token {
        (self.id)(self)
    }

    fn aliases(&self) -> impl Iterator<Item = &Alias> {
        let data = (self.aliases_data)(self);
        let len = (self.aliases_len)(self) as usize;
        let iter = unsafe { slice::from_raw_parts(data, len).iter() };
        iter.map(|v| &*v)
    }

    fn headers(&self) -> &[Ptr<Attr>] {
        let data = (self.headers_data)(self);
        let len = (self.headers_len)(self) as usize;
        unsafe { slice::from_raw_parts(data, len) }
    }

    fn data(&self, layer: &Layer) -> Slice {
        let mut len = 0;
        let data = (self.data)(layer, &mut len);
        unsafe { Slice::from_raw_parts(data, len as usize) }
    }

    fn attrs(&self, layer: &Layer) -> &[Ptr<Attr>] {
        let data = (self.attrs_data)(layer);
        let len = (self.attrs_len)(layer) as usize;
        unsafe { slice::from_raw_parts(data, len) }
    }

    fn payloads(&self, layer: &Layer) -> &[Payload] {
        let data = (self.payloads_data)(layer);
        let len = (self.payloads_len)(layer) as usize;
        unsafe { slice::from_raw_parts(data, len) }
    }
}

extern "C" fn abi_id(class: *const LayerClass) -> Token {
    unsafe { (*class).abi_unsafe_data.id }
}

extern "C" fn abi_aliases_len(class: *const LayerClass) -> u64 {
    unsafe { (*class).abi_unsafe_data.aliases.len() as u64 }
}

extern "C" fn abi_aliases_data(class: *const LayerClass) -> *const Alias {
    unsafe { (*class).abi_unsafe_data.aliases.as_ptr() }
}

extern "C" fn abi_headers_len(class: *const LayerClass) -> u64 {
    unsafe { (*class).abi_unsafe_data.headers.len() as u64 }
}

extern "C" fn abi_headers_data(class: *const LayerClass) -> *const Ptr<Attr> {
    unsafe { (*class).abi_unsafe_data.headers.as_ptr() }
}

extern "C" fn abi_data(layer: *const Layer, len: *mut u64) -> *const u8 {
    unsafe {
        let data = &(*layer).abi_unsafe_data.data;
        *len = data.len() as u64;
        data.as_ptr()
    }
}

extern "C" fn abi_attrs_len(layer: *const Layer) -> u64 {
    unsafe { (*layer).abi_unsafe_data.attrs.len() as u64 }
}

extern "C" fn abi_attrs_data(layer: *const Layer) -> *const Ptr<Attr> {
    unsafe { (*layer).abi_unsafe_data.attrs.as_ptr() }
}

extern "C" fn abi_add_attr(layer: *mut Layer, attr: Ptr<Attr>) {
    let attrs = unsafe { &mut (*layer).abi_unsafe_data.attrs };
    attrs.push(attr);
}

extern "C" fn abi_payloads_len(layer: *const Layer) -> u64 {
    unsafe { (*layer).abi_unsafe_data.payloads.len() as u64 }
}

extern "C" fn abi_payloads_data(layer: *const Layer) -> *const Payload {
    unsafe { (*layer).abi_unsafe_data.payloads.as_ptr() }
}

extern "C" fn abi_add_payload(layer: *mut Layer, data: *const u8, len: u64, id: Token, typ: Token) {
    let payloads = unsafe { &mut (*layer).abi_unsafe_data.payloads };
    payloads.push(Payload { data, len, id, typ });
}

#[cfg(test)]
mod tests {
    use attr::{Attr, AttrBuilder};
    use decoder::Decoder;
    use layer::{Layer, LayerBuilder};
    use slice::Slice;
    use std::io::Result;
    use token::Token;
    use variant::Variant;

    #[test]
    fn id() {
        let id = Token::from(123);
        let class = LayerBuilder::new(id).build();
        let layer = Layer::new(&class, Slice::new());
        assert_eq!(layer.id(), id);
    }

    #[test]
    fn data() {
        let data = b"hello";
        let class = LayerBuilder::new(Token::null()).build();
        let layer = Layer::new(&class, Slice::from(&data[..]));
        assert_eq!(layer.data(), Slice::from(&data[..]));
    }

    #[test]
    fn payloads() {
        let class = LayerBuilder::new(Token::null()).build();
        let mut layer = Layer::new(&class, Slice::new());
        assert!(layer.payloads().iter().next().is_none());

        let count = 100;
        let data = b"hello";

        for i in 0..count {
            layer.add_payload(Slice::from(&data[..]), Token::from(i));
        }

        let mut iter = layer.payloads().iter();
        for i in 0..count {
            let payload = iter.next().unwrap();
            assert_eq!(payload.data(), Slice::from(&data[..]));
            assert_eq!(payload.id(), Token::from(i));
        }
        assert!(iter.next().is_none());
    }

    #[test]
    fn attrs() {
        let class = LayerBuilder::new(Token::null()).build();
        let mut layer = Layer::new(&class, Slice::new());
        assert!(layer.attrs().is_empty());

        #[derive(Clone)]
        struct TestDecoder {}

        impl Decoder for TestDecoder {
            fn decode(&self, _: &Slice) -> Result<Variant> {
                Ok(Variant::Nil)
            }
        }
        let class = AttrBuilder::new("nil")
            .typ("@nil")
            .decoder(TestDecoder {})
            .build();

        let count = 100;
        for i in 0..count {
            let attr = Attr::new(&class, 0..i);
            layer.add_attr(attr);
        }
        let mut iter = layer.attrs().iter();
        for i in 0..count {
            let attr = iter.next().unwrap();
            assert_eq!(attr.id(), Token::from("nil"));
            assert_eq!(attr.typ(), Token::from("@nil"));
            assert_eq!(attr.range(), 0..i);
        }
        assert!(iter.next().is_none());
    }
}
