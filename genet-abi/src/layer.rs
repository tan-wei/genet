use attr::{Attr, AttrClass, AttrContext, FixedAttrField, SizedAttrField};
use fixed::{Fixed, MutFixed};
use metadata::Metadata;
use slice::ByteSlice;
use std::{
    fmt,
    marker::PhantomData,
    ops::{Deref, DerefMut, Range},
    slice,
};
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
    pub fn attr(&self, id: Token) -> Option<Attr> {
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

/// A mutable proxy for a layer object.
#[repr(C)]
pub struct Parent<'a> {
    layer: *mut Layer,
    add_child: extern "C" fn(*mut Parent, *mut Layer),
    children_len: extern "C" fn(*const Parent) -> u64,
    children_data: extern "C" fn(*const Parent) -> *const *mut Layer,
    phantom: PhantomData<&'a ()>,
    children: Vec<*mut Layer>,
}

impl<'a> Parent<'a> {
    pub fn from_mut_ref(layer: &'a mut Layer) -> Parent {
        Parent {
            layer,
            add_child: abi_add_child,
            children_len: abi_children_len,
            children_data: abi_children_data,
            phantom: PhantomData,
            children: Vec::new(),
        }
    }

    /// Returns the ID of self.
    pub fn id(&self) -> Token {
        self.deref().id()
    }

    /// Returns the type of self.
    pub fn data(&self) -> ByteSlice {
        self.deref().data()
    }

    /// Returns the slice of headers.
    pub fn header(&self) -> &Fixed<AttrClass> {
        self.deref().header()
    }

    /// Returns the slice of attributes.
    pub fn attrs(&self) -> impl Iterator<Item = Attr> {
        self.deref().attrs()
    }

    /// Find the attribute in the Layer.
    pub fn attr<T: Into<Token>>(&self, id: T) -> Option<Attr> {
        self.deref().attr(id)
    }

    /// Adds an attribute to the Layer.
    pub fn add_attr<C: Into<Fixed<AttrClass>>>(&mut self, attr: C, range: Range<usize>) {
        self.deref_mut().add_attr(attr, range);
    }

    /// Returns the slice of payloads.
    pub fn payloads(&self) -> impl Iterator<Item = &Payload> {
        self.deref().payloads()
    }

    /// Adds a payload to the Layer.
    pub fn add_payload(&mut self, payload: Payload) {
        self.deref_mut().add_payload(payload);
    }

    pub fn add_child<T: Into<MutFixed<Layer>>>(&mut self, layer: T) {
        (self.add_child)(self, layer.into().as_mut_ptr());
    }

    pub fn children(&self) -> &[*mut Layer] {
        let data = (self.children_data)(self);
        let len = (self.children_len)(self) as usize;
        unsafe { slice::from_raw_parts(data, len) }
    }
}

impl<'a> Deref for Parent<'a> {
    type Target = Layer;

    fn deref(&self) -> &Layer {
        unsafe { &*self.layer }
    }
}

impl<'a> DerefMut for Parent<'a> {
    fn deref_mut(&mut self) -> &mut Layer {
        unsafe { &mut *self.layer }
    }
}

extern "C" fn abi_add_child(layer: *mut Parent, child: *mut Layer) {
    unsafe { (*layer).children.push(child) }
}

extern "C" fn abi_children_len(layer: *const Parent) -> u64 {
    unsafe { (*layer).children.len() as u64 }
}

extern "C" fn abi_children_data(layer: *const Parent) -> *const *mut Layer {
    unsafe { (*layer).children.as_ptr() }
}

#[repr(C)]
struct BoundAttr {
    attr: Fixed<AttrClass>,
    range: Range<usize>,
}

/// A layer object.
#[repr(C)]
pub struct Layer {
    class: Fixed<LayerClass>,
    data: ByteSlice,
    attrs: Vec<BoundAttr>,
    payloads: Vec<Payload>,
}

unsafe impl Send for Layer {}

impl Layer {
    /// Creates a new Layer.
    pub fn new<C: Into<Fixed<LayerClass>>, B: Into<ByteSlice>>(class: C, data: B) -> Layer {
        Layer {
            class: class.into(),
            data: data.into(),
            attrs: Vec::new(),
            payloads: Vec::new(),
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
    pub fn header(&self) -> &Fixed<AttrClass> {
        self.class.header()
    }

    /// Returns the slice of attributes.
    pub fn attrs(&self) -> impl Iterator<Item = Attr> {
        self.class.attrs(self)
    }

    /// Find the attribute in the Layer.
    pub fn attr<T: Into<Token>>(&self, id: T) -> Option<Attr> {
        let id = id.into();
        let id = self
            .class
            .aliases()
            .find(|alias| alias.id == id)
            .map(|alias| alias.target)
            .unwrap_or(id);

        AttrClass::expand(self.class.header(), &self.data, None)
            .into_iter()
            .chain(
                self.attrs
                    .iter()
                    .map(|c| {
                        AttrClass::expand(&c.attr, &self.data(), Some(c.range.clone())).into_iter()
                    })
                    .flatten(),
            )
            .find(|attr| attr.id() == id)
    }

    /// Adds an attribute to the Layer.
    pub fn add_attr<C: Into<Fixed<AttrClass>>>(&mut self, attr: C, range: Range<usize>) {
        let func = self.class.add_attr;
        (func)(
            self,
            BoundAttr {
                attr: attr.into(),
                range: (range.start * 8)..(range.end * 8),
            },
        );
    }

    pub fn add_attr_offset<T: SizedAttrField + AsRef<Fixed<AttrClass>>>(
        &mut self,
        attr: &T,
        offset: usize,
    ) {
        let func = self.class.add_attr;
        let bit_offset = offset * 8;
        let attr = Attr::builder(attr.as_ref().clone())
            .range(bit_offset..(bit_offset + attr.bit_size()))
            .build();
        (func)(self, attr.into());
    }

    pub fn add_attr_range<T: FixedAttrField + AsRef<Fixed<AttrClass>>>(&mut self, attr: &T) {
        let func = self.class.add_attr;
        let bit_offset = attr.bit_offset();
        let attr = Attr::builder(attr.as_ref().clone())
            .range(bit_offset..(bit_offset + attr.bit_size()))
            .build();
        (func)(self, attr.into());
    }

    /// Returns the slice of payloads.
    pub fn payloads(&self) -> impl Iterator<Item = &Payload> {
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

impl Into<MutFixed<Layer>> for Layer {
    fn into(self) -> MutFixed<Layer> {
        MutFixed::new(self)
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
    header: Fixed<AttrClass>,
    aliases: Vec<Alias>,
    meta: Metadata,
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

    /// Sets a name of LayerClass.
    pub fn name(mut self, name: &'static str) -> LayerClassBuilder {
        self.meta.set_name(name);
        self
    }

    /// Sets a description of LayerClass.
    pub fn description(mut self, desc: &'static str) -> LayerClassBuilder {
        self.meta.set_description(desc);
        self
    }

    /// Builds a new LayerClass.
    pub fn build(self) -> LayerClass {
        LayerClass {
            get_id: abi_id,
            data: abi_data,
            attrs_len: abi_attrs_len,
            attrs_data: abi_attrs_data,
            aliases_len: abi_aliases_len,
            aliases_data: abi_aliases_data,
            add_attr: abi_add_attr,
            payloads_len: abi_payloads_len,
            payloads_data: abi_payloads_data,
            add_payload: abi_add_payload,
            meta: self.meta,
            aliases: self.aliases,
            header: self.header,
        }
    }
}

#[repr(C)]
struct Alias {
    id: Token,
    target: Token,
}

pub struct LayerBox<A: SizedAttrField> {
    fields: A,
}

impl<A: SizedAttrField> LayerBox<A> {
    pub fn new<T: Into<Token>>(id: T, attr: A) -> Self {
        let id: Token = id.into();
        let mut attr = attr;
        let res = attr.init(&AttrContext {
            path: id.to_string(),
            ..AttrContext::default()
        });
        println!("{:#?}", res);
        Self { fields: attr }
    }

    pub fn fields(&self) -> &A {
        &self.fields
    }
}

/// A layer class object.
#[repr(C)]
pub struct LayerClass {
    get_id: extern "C" fn(*const LayerClass) -> Token,
    aliases_len: extern "C" fn(*const LayerClass) -> u64,
    aliases_data: extern "C" fn(*const LayerClass) -> *const Alias,
    data: extern "C" fn(*const Layer, *mut u64) -> *const u8,
    attrs_len: extern "C" fn(*const Layer) -> u64,
    attrs_data: extern "C" fn(*const Layer) -> *const BoundAttr,
    add_attr: extern "C" fn(*mut Layer, BoundAttr),
    payloads_len: extern "C" fn(*const Layer) -> u64,
    payloads_data: extern "C" fn(*const Layer) -> *const Payload,
    add_payload: extern "C" fn(*mut Layer, Payload),
    header: Fixed<AttrClass>,
    meta: Metadata,
    aliases: Vec<Alias>,
}

impl LayerClass {
    /// Creates a new builder object for LayerClass.
    pub fn builder<H: Into<Fixed<AttrClass>>>(header: H) -> LayerClassBuilder {
        LayerClassBuilder {
            meta: Metadata::new(),
            aliases: Vec::new(),
            header: header.into(),
        }
    }

    pub fn new<T: Into<Token>, A: SizedAttrField>(id: T, attr: A) -> LayerClass {
        let id = id.into();
        let mut attr = attr;
        let tree = attr.init(&AttrContext {
            path: id.to_string(),
            ..AttrContext::default()
        });
        let mut meta = Metadata::new();
        meta.set_name(tree.class.name());
        meta.set_name(tree.class.description());
        LayerClass {
            get_id: abi_id,
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
            id: id,
            meta,
            aliases: tree
                .aliases
                .into_iter()
                .map(|(id, target)| Alias {
                    id: id.into(),
                    target: target.into(),
                })
                .collect(),
            headers: tree.attrs.into_iter().map(Fixed::new).collect(),
        }
    }

    fn id(&self) -> Token {
        (self.get_id)(self)
    }

    fn aliases(&self) -> impl Iterator<Item = &Alias> {
        let data = (self.aliases_data)(self);
        let len = (self.aliases_len)(self) as usize;
        let iter = unsafe { slice::from_raw_parts(data, len).iter() };
        iter.map(|v| &*v)
    }

    fn header(&self) -> &Fixed<AttrClass> {
        &self.header
    }

    fn data(&self, layer: &Layer) -> ByteSlice {
        let mut len = 0;
        let data = (self.data)(layer, &mut len);
        unsafe { ByteSlice::from_raw_parts(data, len as usize) }
    }

    fn attrs(&self, layer: &Layer) -> impl Iterator<Item = Attr> {
        let data = (self.attrs_data)(layer);
        let len = (self.attrs_len)(layer) as usize;
        let attrs = unsafe { slice::from_raw_parts(data, len) }
            .iter()
            .map(|c| AttrClass::expand(&c.attr, &layer.data(), Some(c.range.clone())).into_iter())
            .flatten()
            .collect::<Vec<_>>();
        attrs.into_iter()
    }

    fn payloads(&self, layer: &Layer) -> impl Iterator<Item = &Payload> {
        let data = (self.payloads_data)(layer);
        let len = (self.payloads_len)(layer) as usize;
        unsafe { slice::from_raw_parts(data, len) }.iter()
    }
}

impl Into<Fixed<LayerClass>> for &'static LayerClass {
    fn into(self) -> Fixed<LayerClass> {
        Fixed::from_static(self)
    }
}

extern "C" fn abi_id(class: *const LayerClass) -> Token {
    unsafe { (*class).header.id() }
}

extern "C" fn abi_aliases_len(class: *const LayerClass) -> u64 {
    unsafe { (*class).aliases.len() as u64 }
}

extern "C" fn abi_aliases_data(class: *const LayerClass) -> *const Alias {
    unsafe { (*class).aliases.as_ptr() }
}

extern "C" fn abi_data(layer: *const Layer, len: *mut u64) -> *const u8 {
    unsafe {
        let data = &(*layer).data;
        *len = data.len() as u64;
        data.as_ptr()
    }
}

extern "C" fn abi_attrs_len(layer: *const Layer) -> u64 {
    unsafe { (*layer).attrs.len() as u64 }
}

extern "C" fn abi_attrs_data(layer: *const Layer) -> *const BoundAttr {
    unsafe { (*layer).attrs.as_ptr() }
}

extern "C" fn abi_add_attr(layer: *mut Layer, attr: BoundAttr) {
    let attrs = unsafe { &mut (*layer).attrs };
    attrs.push(attr);
}

extern "C" fn abi_payloads_len(layer: *const Layer) -> u64 {
    unsafe { (*layer).payloads.len() as u64 }
}

extern "C" fn abi_payloads_data(layer: *const Layer) -> *const Payload {
    unsafe { (*layer).payloads.as_ptr() }
}

extern "C" fn abi_add_payload(layer: *mut Layer, payload: Payload) {
    let payloads = unsafe { &mut (*layer).payloads };
    payloads.push(payload);
}

#[cfg(test)]
mod tests {
    use attr::AttrClass;
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
        let attr = Fixed::new(AttrClass::builder(id).build());
        let class = Fixed::new(LayerClass::builder(attr).build());
        let layer = Layer::new(class, ByteSlice::new());
        assert_eq!(layer.id(), id);
    }

    #[test]
    fn data() {
        let data = b"hello";
        let attr = Fixed::new(AttrClass::builder(Token::null()).build());
        let class = Fixed::new(LayerClass::builder(attr).build());
        let layer = Layer::new(class, ByteSlice::from(&data[..]));
        assert_eq!(layer.data(), ByteSlice::from(&data[..]));
    }

    #[test]
    fn payloads() {
        let attr = Fixed::new(AttrClass::builder(Token::null()).build());
        let class = Fixed::new(LayerClass::builder(attr).build());
        let mut layer = Layer::new(class, ByteSlice::new());
        assert!(layer.payloads().next().is_none());

        let count = 100;
        let data = b"hello";

        for i in 0..count {
            layer.add_payload(Payload::new(ByteSlice::from(&data[..]), Token::from(i)));
        }

        let mut iter = layer.payloads();
        for i in 0..count {
            let payload = iter.next().unwrap();
            assert_eq!(payload.data(), ByteSlice::from(&data[..]));
            assert_eq!(payload.id(), Token::from(i));
        }
        assert!(iter.next().is_none());
    }

    #[test]
    fn attrs() {
        let attr = Fixed::new(AttrClass::builder(Token::null()).build());
        let class = Fixed::new(LayerClass::builder(attr).build());
        let mut layer = Layer::new(class, ByteSlice::new());
        assert!(layer.attrs().next().is_none());

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
            layer.add_attr(class.clone(), 0..i);
        }
        let mut iter = layer.attrs();
        for i in 0..count {
            let attr = iter.next().unwrap();
            assert_eq!(attr.id(), Token::from("nil"));
            assert_eq!(attr.typ(), Token::from("@nil"));
            assert_eq!(attr.range(), 0..i);
        }
        assert!(iter.next().is_none());
    }
}
