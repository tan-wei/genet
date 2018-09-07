use attr::Attr;
use fixed::Fixed;
use slice::ByteSlice;
use std::{fmt, slice};
use token::Token;

/// A layer stack object.
pub struct LayerStack<'a> {
    buffer: &'a [*const Layer],
}

impl<'a> LayerStack<'a> {
    pub(crate) unsafe fn new(ptr: *const *const Layer, len: usize) -> LayerStack<'a> {
        Self {
            buffer: slice::from_raw_parts(ptr, len),
        }
    }

    /// Returns the top of the LayerStack.
    pub fn top(&self) -> Option<&Layer> {
        self.layers().last()
    }

    /// Returns the bottom of the LayerStack.
    pub fn bottom(&self) -> Option<&Layer> {
        self.layers().next()
    }

    /// Find the attribute in the LayerStack.
    pub fn attr(&self, id: Token) -> Option<&Attr> {
        for layer in self.layers().rev() {
            if let Some(attr) = layer.attr(id) {
                return Some(attr);
            }
        }
        None
    }

    /// Find the layer in the LayerStack.
    pub fn layer(&self, id: Token) -> Option<&Layer> {
        self.layers().find(|layer| layer.id() == id)
    }

    fn layers(&self) -> impl DoubleEndedIterator<Item = &'a Layer> {
        self.buffer.iter().map(|layer| unsafe { &**layer })
    }
}

/// A layer object.
#[repr(C)]
pub struct Layer {
    class: Fixed<LayerClass>,
    abi_unsafe_data: LayerData,
}

unsafe impl Send for Layer {}

struct LayerData {
    data: ByteSlice,
    attrs: Vec<Fixed<Attr>>,
    payloads: Vec<Payload>,
}

impl Layer {
    /// Creates a new Layer.
    pub fn new<C: Into<Fixed<LayerClass>>, B: Into<ByteSlice>>(class: C, data: B) -> Layer {
        Layer {
            class: class.into(),
            abi_unsafe_data: LayerData {
                data: data.into(),
                attrs: Vec::new(),
                payloads: Vec::new(),
            },
        }
    }

    /// Returns the ID of self.
    pub fn id(&self) -> Token {
        self.class.id()
    }

    /// Returns the type of self.
    pub fn data(&self) -> ByteSlice {
        self.class.data(self)
    }

    /// Returns the slice of headers.
    pub fn headers(&self) -> &[Fixed<Attr>] {
        self.class.headers()
    }

    /// Returns the slice of attributes.
    pub fn attrs(&self) -> &[Fixed<Attr>] {
        self.class.attrs(self)
    }

    /// Find the attribute in the Layer.
    pub fn attr<T: Into<Token>>(&self, id: T) -> Option<&Attr> {
        let id = id.into();
        let id = self
            .class
            .aliases()
            .find(|alias| alias.id == id)
            .map(|alias| alias.target)
            .unwrap_or(id);
        self.attrs()
            .iter()
            .chain(self.class.headers().iter())
            .find(|attr| attr.id() == id)
            .map(|attr| attr.as_ref())
    }

    /// Adds an attribute to the Layer.
    pub fn add_attr<T: Into<Fixed<Attr>>>(&mut self, attr: T) {
        let func = self.class.add_attr;
        (func)(self, attr.into());
    }

    /// Returns the slice of payloads.
    pub fn payloads(&self) -> &[Payload] {
        self.class.payloads(self)
    }

    /// Adds a payload to the Layer.
    pub fn add_payload(&mut self, payload: Payload) {
        let func = self.class.add_payload;
        (func)(self, payload);
    }
}

impl fmt::Debug for Layer {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Layer {:?}", self.id())
    }
}

/// A payload object.
#[repr(C)]
pub struct Payload {
    data: *const u8,
    len: u64,
    id: Token,
    typ: Token,
}

impl Payload {
    /// Creates a new payload.
    pub fn new<B: Into<ByteSlice>, T: Into<Token>>(data: B, id: T) -> Payload {
        Self::with_typ(data, id, "")
    }

    /// Creates a new payload with the given type.
    pub fn with_typ<B: Into<ByteSlice>, T: Into<Token>, U: Into<Token>>(
        data: B,
        id: T,
        typ: U,
    ) -> Payload {
        let data: ByteSlice = data.into();
        Self {
            data: data.as_ptr(),
            len: data.len() as u64,
            id: id.into(),
            typ: typ.into(),
        }
    }

    /// Returns the ID of self.
    pub fn id(&self) -> Token {
        self.id
    }

    /// Returns the type of self.
    pub fn typ(&self) -> Token {
        self.typ
    }

    /// Returns the data of self.
    pub fn data(&self) -> ByteSlice {
        unsafe { ByteSlice::from_raw_parts(self.data, self.len as usize) }
    }
}

/// A builder object for LayerClass.
pub struct LayerClassBuilder {
    id: Token,
    aliases: Vec<Alias>,
    headers: Vec<Fixed<Attr>>,
}

impl LayerClassBuilder {
    /// Adds an attribute alias for LayerClass.
    pub fn alias<T: Into<Token>, U: Into<Token>>(mut self, id: T, target: U) -> LayerClassBuilder {
        self.aliases.push(Alias {
            id: id.into(),
            target: target.into(),
        });
        self
    }

    /// Adds a header attribute for LayerClass.
    pub fn header<T: Into<Fixed<Attr>>>(mut self, attr: T) -> LayerClassBuilder {
        self.headers.push(attr.into());
        self
    }

    /// Builds a new LayerClass.
    pub fn build(self) -> LayerClass {
        LayerClass {
            abi_unsafe_data: Fixed::new(LayerClassData {
                id: self.id,
                aliases: self.aliases,
                headers: self.headers,
            }),
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
        }
    }
}

#[repr(C)]
struct Alias {
    id: Token,
    target: Token,
}

struct LayerClassData {
    id: Token,
    aliases: Vec<Alias>,
    headers: Vec<Fixed<Attr>>,
}

/// A layer class object.
#[repr(C)]
pub struct LayerClass {
    abi_unsafe_data: Fixed<LayerClassData>,
    id: extern "C" fn(*const LayerClass) -> Token,
    aliases_len: extern "C" fn(*const LayerClass) -> u64,
    aliases_data: extern "C" fn(*const LayerClass) -> *const Alias,
    headers_len: extern "C" fn(*const LayerClass) -> u64,
    headers_data: extern "C" fn(*const LayerClass) -> *const Fixed<Attr>,
    data: extern "C" fn(*const Layer, *mut u64) -> *const u8,
    attrs_len: extern "C" fn(*const Layer) -> u64,
    attrs_data: extern "C" fn(*const Layer) -> *const Fixed<Attr>,
    add_attr: extern "C" fn(*mut Layer, Fixed<Attr>),
    payloads_len: extern "C" fn(*const Layer) -> u64,
    payloads_data: extern "C" fn(*const Layer) -> *const Payload,
    add_payload: extern "C" fn(*mut Layer, Payload),
}

impl LayerClass {
    /// Creates a new builder object for LayerClass.
    pub fn builder<T: Into<Token>>(id: T) -> LayerClassBuilder {
        LayerClassBuilder {
            id: id.into(),
            aliases: Vec::new(),
            headers: Vec::new(),
        }
    }

    fn id(&self) -> Token {
        (self.id)(self)
    }

    fn aliases(&self) -> impl Iterator<Item = &Alias> {
        let data = (self.aliases_data)(self);
        let len = (self.aliases_len)(self) as usize;
        let iter = unsafe { slice::from_raw_parts(data, len).iter() };
        iter.map(|v| &*v)
    }

    fn headers(&self) -> &[Fixed<Attr>] {
        let data = (self.headers_data)(self);
        let len = (self.headers_len)(self) as usize;
        unsafe { slice::from_raw_parts(data, len) }
    }

    fn data(&self, layer: &Layer) -> ByteSlice {
        let mut len = 0;
        let data = (self.data)(layer, &mut len);
        unsafe { ByteSlice::from_raw_parts(data, len as usize) }
    }

    fn attrs(&self, layer: &Layer) -> &[Fixed<Attr>] {
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

impl Into<Fixed<LayerClass>> for &'static LayerClass {
    fn into(self) -> Fixed<LayerClass> {
        Fixed::from_static(self)
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

extern "C" fn abi_headers_data(class: *const LayerClass) -> *const Fixed<Attr> {
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

extern "C" fn abi_attrs_data(layer: *const Layer) -> *const Fixed<Attr> {
    unsafe { (*layer).abi_unsafe_data.attrs.as_ptr() }
}

extern "C" fn abi_add_attr(layer: *mut Layer, attr: Fixed<Attr>) {
    let attrs = unsafe { &mut (*layer).abi_unsafe_data.attrs };
    attrs.push(attr);
}

extern "C" fn abi_payloads_len(layer: *const Layer) -> u64 {
    unsafe { (*layer).abi_unsafe_data.payloads.len() as u64 }
}

extern "C" fn abi_payloads_data(layer: *const Layer) -> *const Payload {
    unsafe { (*layer).abi_unsafe_data.payloads.as_ptr() }
}

extern "C" fn abi_add_payload(layer: *mut Layer, payload: Payload) {
    let payloads = unsafe { &mut (*layer).abi_unsafe_data.payloads };
    payloads.push(payload);
}

#[cfg(test)]
mod tests {
    use attr::{Attr, AttrClass};
    use cast::Cast;
    use fixed::Fixed;
    use layer::{Layer, LayerClass, Payload};
    use slice::ByteSlice;
    use std::io::Result;
    use token::Token;
    use variant::Variant;

    #[test]
    fn id() {
        let id = Token::from(123);
        let class = Fixed::new(LayerClass::builder(id).build());
        let layer = Layer::new(class, ByteSlice::new());
        assert_eq!(layer.id(), id);
    }

    #[test]
    fn data() {
        let data = b"hello";
        let class = Fixed::new(LayerClass::builder(Token::null()).build());
        let layer = Layer::new(class, ByteSlice::from(&data[..]));
        assert_eq!(layer.data(), ByteSlice::from(&data[..]));
    }

    #[test]
    fn payloads() {
        let class = Fixed::new(LayerClass::builder(Token::null()).build());
        let mut layer = Layer::new(class, ByteSlice::new());
        assert!(layer.payloads().iter().next().is_none());

        let count = 100;
        let data = b"hello";

        for i in 0..count {
            layer.add_payload(Payload::new(ByteSlice::from(&data[..]), Token::from(i)));
        }

        let mut iter = layer.payloads().iter();
        for i in 0..count {
            let payload = iter.next().unwrap();
            assert_eq!(payload.data(), ByteSlice::from(&data[..]));
            assert_eq!(payload.id(), Token::from(i));
        }
        assert!(iter.next().is_none());
    }

    #[test]
    fn attrs() {
        let class = Fixed::new(LayerClass::builder(Token::null()).build());
        let mut layer = Layer::new(class, ByteSlice::new());
        assert!(layer.attrs().is_empty());

        #[derive(Clone)]
        struct TestCast {}

        impl Cast for TestCast {
            fn cast(&self, _: &ByteSlice) -> Result<Variant> {
                Ok(Variant::Nil)
            }
        }
        let class = Fixed::new(
            AttrClass::builder("nil")
                .typ("@nil")
                .cast(TestCast {})
                .build(),
        );

        let count = 100;
        for i in 0..count {
            let attr = Attr::builder(class.clone()).range(0..i).build();
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
