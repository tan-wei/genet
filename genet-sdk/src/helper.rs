#[macro_export]
macro_rules! layer_class {
    ($id:expr) => (::genet_sdk::layer::LayerClass::builder($id).build());
    ($id:expr, $($key:ident : $($arg:expr)*),*) => (::genet_sdk::layer::LayerClass::builder($id)
                $( . $key ( $($arg),* ) )*
                .build()
    );
}

#[macro_export]
macro_rules! layer_class_lazy {
    ($id:expr) => (
        {
            lazy_static! {
                static ref LAYER_CLASS : ::genet_sdk::layer::LayerClass = layer_class!($id);
            }
            &LAYER_CLASS
        }
    );
    ($id:expr, $($key:ident : $($arg:expr)*),*) => (
        {
            lazy_static! {
                static ref LAYER_CLASS : ::genet_sdk::layer::LayerClass = layer_class!($id,  $($key : $($arg)* ),* );
            }
            &LAYER_CLASS
        }
    );
}

#[macro_export]
macro_rules! def_layer_class {
    ($name:ident, $id:expr) => (
        lazy_static! {
            static ref $name : ::genet_sdk::layer::LayerClass = layer_class!($id);
        }
    );
    ($name:ident, $id:expr, $($key:ident : $($arg:expr)*),*) => (
        lazy_static! {
            static ref $name : ::genet_sdk::layer::LayerClass = layer_class!($id,  $($key : $($arg)* ),* );
        }
    );
}

#[macro_export]
macro_rules! attr_class {
    ($id:expr) => (::genet_sdk::attr::AttrClass::builder($id).build());
    ($id:expr, $($key:ident : $($arg:expr)*),*) => (::genet_sdk::attr::AttrClass::builder($id)
                $( . $key ( $($arg),* ) )*
                .build());
}

#[macro_export]
macro_rules! attr_class_lazy {
    ($id:expr) => (
        {
            lazy_static! {
                static ref ATTR_CLASS : ::genet_sdk::attr::AttrClass = attr_class!($id);
            }
            &ATTR_CLASS
        }
    );
    ($id:expr, $($key:ident : $($arg:expr)*),*) => (
        {
            lazy_static! {
                static ref ATTR_CLASS : ::genet_sdk::attr::AttrClass = attr_class!($id, $($key : $($arg)* ),* );
            }
            &ATTR_CLASS
        }
    );
}

#[macro_export]
macro_rules! def_attr_class {
    ($name:ident, $id:expr) => (
        lazy_static! {
            static ref $name : ::genet_sdk::attr::AttrClass = attr_class!($id);
        }
    );
    ($name:ident, $id:expr, $($key:ident : $($arg:expr)*),*) => (
        lazy_static! {
            static ref $name : ::genet_sdk::attr::AttrClass = attr_class!($id, $($key : $($arg)* ),* );
        }
    );
}

#[macro_export]
macro_rules! attr {
    ($class:expr) => (::genet_sdk::attr::Attr::builder($class).build());
    ($class:expr, $($key:ident : $($arg:expr)*),*) => (::genet_sdk::attr::Attr::builder($class)
        $( . $key ( $($arg),* ) )*
        .build());
}

#[macro_export]
macro_rules! attr_lazy {
    ($class:expr) => {
        {
            lazy_static! {
                static ref ATTR : ::genet_sdk::attr::Attr = attr!($class);
            }
            &ATTR
        }
    };
    ($class:expr, $($key:ident : $($arg:expr)*),*) => {
        {
            lazy_static! {
                static ref ATTR : ::genet_sdk::attr::Attr = attr!($class, $($key : $($arg)* ),* );
            }
            &ATTR
        }
    };
}

#[macro_export]
macro_rules! def_attr {
    ($name:ident, $class:expr) => {
        lazy_static! {
            static ref $name: ::genet_sdk::attr::Attr = attr!($class);
        }
    };
    ($name:ident, $class:expr, $($key:ident : $($arg:expr)*),*) => {
        lazy_static! {
            static ref $name: ::genet_sdk::attr::Attr = attr!($class, $($key : $($arg)* ),* );
        }
    };
}
