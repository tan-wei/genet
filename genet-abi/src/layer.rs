use attr::Attr;
use fixed::{Fixed, MutFixed};
use slice::ByteSlice;
use std::{
    fmt,
    marker::PhantomData,
    mem,
    ops::{Deref, DerefMut},
    ptr, slice,
    sync::atomic::{AtomicPtr, Ordering},
};
use token::Token;

/// A mutable proxy for a layer object.
#[repr(C)]
pub struct Parent<'a> {
    layer: &'a mut Layer,
    attrs: Vec<Fixed<Attr>>,
    payloads: Vec<Payload>,
    children: Vec<MutFixed<Layer>>,
    phantom: PhantomData<&'a ()>,
}

impl<'a> Parent<'a> {
    pub fn from_ref(layer: &'a mut Layer) -> Parent {
        Parent {
            layer,
            attrs: Vec::new(),
            payloads: Vec::new(),
            children: Vec::new(),
            phantom: PhantomData,
        }
    }

    pub fn apply(mut self) {
        let next = Box::into_raw(Box::new(LayerData {
            next: AtomicPtr::default(),
            attrs: self.attrs.as_ptr(),
            attrs_len: self.attrs.len() as u16,
            payloads: self.payloads.as_ptr(),
            payloads_len: self.payloads.len() as u16,
            children: self.children.as_ptr(),
            children_len: self.children.len() as u16,
        }));
        let mut data = &mut self.deref_mut().root as *mut LayerData;
        while !data.is_null() {
            data =
                unsafe { &(*data).next }.compare_and_swap(ptr::null_mut(), next, Ordering::Relaxed);
        }
        mem::forget(self.attrs);
        mem::forget(self.payloads);
        mem::forget(self.children);
    }

    /// Returns the ID of self.
    pub fn id(&self) -> Token {
        self.deref().id()
    }

    /// Returns the type of self.
    pub fn data(&self) -> ByteSlice {
        self.deref().data()
    }

    /// Returns an iterator of attributes.
    pub fn attrs(&self) -> impl Iterator<Item = &Fixed<Attr>> {
        self.deref().attrs()
    }

    /// Find the attribute in the Layer.
    pub fn attr<T: Into<Token>>(&self, id: T) -> Option<&Attr> {
        self.deref().attr(id)
    }

    /// Adds an attribute to the Layer.
    pub fn add_attr<T: Into<Fixed<Attr>>>(&mut self, attr: T) {
        self.attrs.push(attr.into());
    }

    /// Returns an iterator of payloads.
    pub fn payloads(&self) -> impl Iterator<Item = &Payload> {
        self.deref().payloads()
    }

    /// Adds a payload to the Layer.
    pub fn add_payload(&mut self, payload: Payload) {
        self.payloads.push(payload);
    }

    pub fn add_child<T: Into<MutFixed<Layer>>>(&mut self, layer: T) {
        let mut layer = layer.into();
        layer.parent = self.layer;
        self.children.push(layer);
    }

    pub fn children(&self) -> &[MutFixed<Layer>] {
        &self.children
    }
}

impl<'a> Deref for Parent<'a> {
    type Target = Layer;

    fn deref(&self) -> &Layer {
        self.layer
    }
}

impl<'a> DerefMut for Parent<'a> {
    fn deref_mut(&mut self) -> &mut Layer {
        self.layer
    }
}

#[repr(C)]
struct LayerData {
    next: AtomicPtr<LayerData>,
    attrs: *const Fixed<Attr>,
    payloads: *const Payload,
    children: *const MutFixed<Layer>,
    attrs_len: u16,
    payloads_len: u16,
    children_len: u16,
}

impl LayerData {
    fn attrs(&self) -> &[Fixed<Attr>] {
        if self.attrs_len == 0 {
            &[]
        } else {
            unsafe { slice::from_raw_parts(self.attrs, self.attrs_len as usize) }
        }
    }

    fn payloads(&self) -> &[Payload] {
        if self.payloads_len == 0 {
            &[]
        } else {
            unsafe { slice::from_raw_parts(self.payloads, self.payloads_len as usize) }
        }
    }

    fn children(&self) -> &[MutFixed<Layer>] {
        if self.children_len == 0 {
            &[]
        } else {
            unsafe { slice::from_raw_parts(self.children, self.children_len as usize) }
        }
    }

    fn revisions<'a>(&self) -> LayerDataIter<'a> {
        LayerDataIter {
            data: self,
            back: None,
        }
    }
}

struct LayerDataIter<'a> {
    data: *const LayerData,
    back: Option<Vec<&'a LayerData>>,
}

impl<'a> Iterator for LayerDataIter<'a> {
    type Item = &'a LayerData;

    fn next(&mut self) -> Option<Self::Item> {
        if self.data.is_null() {
            None
        } else {
            let data = unsafe { &*self.data };
            self.data = data.next.load(Ordering::Relaxed);
            Some(data)
        }
    }
}

impl<'a> DoubleEndedIterator for LayerDataIter<'a> {
    fn next_back(&mut self) -> Option<Self::Item> {
        if self.back.is_none() {
            let mut layers = Vec::new();
            while !self.data.is_null() {
                let data = unsafe { &*self.data };
                layers.push(data);
                self.data = data.next.load(Ordering::Relaxed);
            }
            self.back = Some(layers);
        }
        self.back.as_mut().unwrap().pop()
    }
}

pub struct LayerBuilder {
    class: Fixed<LayerClass>,
    data: ByteSlice,
    attrs: Vec<Fixed<Attr>>,
    payloads: Vec<Payload>,
}

impl Into<Layer> for LayerBuilder {
    fn into(self) -> Layer {
        let attrs = self.attrs.as_ptr();
        let attrs_len = self.attrs.len() as u16;
        let payloads = self.payloads.as_ptr();
        let payloads_len = self.payloads.len() as u16;
        mem::forget(self.attrs);
        mem::forget(self.payloads);
        let layer = Layer {
            class: self.class,
            data: self.data,
            root: LayerData {
                next: AtomicPtr::default(),
                attrs,
                attrs_len,
                payloads,
                payloads_len,
                children: ptr::null(),
                children_len: 0,
            },
            parent: ptr::null(),
        };
        layer
    }
}

impl LayerBuilder {
    /// Returns the type of self.
    pub fn data(&self) -> ByteSlice {
        self.data
    }

    /// Adds an attribute to the Layer.
    pub fn add_attr<T: Into<Fixed<Attr>>>(&mut self, attr: T) {
        self.attrs.push(attr.into());
    }

    /// Adds a payload to the Layer.
    pub fn add_payload(&mut self, payload: Payload) {
        self.payloads.push(payload);
    }
}

impl Into<MutFixed<Layer>> for LayerBuilder {
    fn into(self) -> MutFixed<Layer> {
        MutFixed::new(self.into())
    }
}

/// A layer object.
#[repr(C)]
pub struct Layer {
    class: Fixed<LayerClass>,
    data: ByteSlice,
    root: LayerData,
    parent: *const Layer,
}

unsafe impl Send for Layer {}

impl Layer {
    /// Creates a new Layer.
    pub fn new<C: Into<Fixed<LayerClass>>, B: Into<ByteSlice>>(class: C, data: B) -> LayerBuilder {
        LayerBuilder {
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
        self.data
    }

    /// Returns an iterator of attributes.
    pub fn attrs(&self) -> impl Iterator<Item = &Fixed<Attr>> {
        self.class
            .headers()
            .chain(self.root.revisions().map(|r| r.attrs().iter()).flatten())
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
            .find(|attr| attr.id() == id)
            .map(|attr| attr.as_ref())
    }

    /// Returns an iterator of payloads.
    pub fn payloads(&self) -> impl Iterator<Item = &Payload> {
        self.root.revisions().map(|r| r.payloads().iter()).flatten()
    }

    /// Returns an iterator of children.
    pub fn children(&self) -> impl DoubleEndedIterator<Item = &MutFixed<Layer>> {
        self.root.revisions().map(|r| r.children().iter()).flatten()
    }

    /// Returns the parent layer.
    pub fn parent(&self) -> Option<&Layer> {
        if self.parent.is_null() {
            None
        } else {
            Some(unsafe { &*self.parent })
        }
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
        let aliases_len = self.aliases.len() as u16;
        let aliases = self.aliases.as_ptr();
        let headers_len = self.headers.len() as u16;
        let headers = self.headers.as_ptr();
        mem::forget(self.aliases);
        mem::forget(self.headers);
        let class = LayerClass {
            id: self.id,
            aliases,
            headers,
            aliases_len,
            headers_len,
        };
        class
    }
}

#[repr(C)]
struct Alias {
    id: Token,
    target: Token,
}

/// A layer class object.
#[repr(C)]
pub struct LayerClass {
    id: Token,
    aliases: *const Alias,
    headers: *const Fixed<Attr>,
    aliases_len: u16,
    headers_len: u16,
}

unsafe impl Send for LayerClass {}
unsafe impl Sync for LayerClass {}

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
        self.id
    }

    fn aliases(&self) -> impl Iterator<Item = &Alias> {
        let iter = unsafe { slice::from_raw_parts(self.aliases, self.aliases_len as usize).iter() };
        iter.map(|v| &*v)
    }

    fn headers(&self) -> impl Iterator<Item = &Fixed<Attr>> {
        if self.headers_len == 0 {
            [].iter()
        } else {
            unsafe { slice::from_raw_parts(self.headers, self.headers_len as usize) }.iter()
        }
    }
}

impl Into<Fixed<LayerClass>> for &'static LayerClass {
    fn into(self) -> Fixed<LayerClass> {
        Fixed::from_static(self)
    }
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
