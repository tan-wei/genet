#[macro_export]
macro_rules! def_layer_class {
    ($name:ident, $id:expr) => (
        lazy_static! {
            static ref $name : ::genet_sdk::layer::LayerClass = ::genet_sdk::layer::LayerClass::builder($id)
                .build();
        }
    );
    ($name:ident, $id:expr, $($key:ident : $($arg:expr)*),*) => (
        lazy_static! {
            static ref $name : ::genet_sdk::layer::LayerClass = ::genet_sdk::layer::LayerClass::builder($id)
                $( . $key ( $($arg),* ) )*
                .build();
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
macro_rules! def_attr_class {
    ($name:ident, $id:expr) => (
        lazy_static! {
            static ref $name : ::genet_sdk::attr::AttrClass = ::genet_sdk::attr::AttrClass::builder($id)
                .build();
        }
    );
    ($name:ident, $id:expr, $($key:ident : $($arg:expr)*),*) => (
        lazy_static! {
            static ref $name : ::genet_sdk::attr::AttrClass = ::genet_sdk::attr::AttrClass::builder($id)
                $( . $key ( $($arg),* ) )*
                .build();
        }
    );
}

#[macro_export]
macro_rules! def_attr {
    ($name:ident, $class:expr, $range:expr) => {
        lazy_static! {
            static ref $name: ::genet_sdk::attr::Attr = attr!($class, $range);
        }
    };
}

#[macro_export]
macro_rules! attr {
    ($class:expr, $range:expr) => {
        ::genet_sdk::attr::Attr::new($class, $range)
    };
}
