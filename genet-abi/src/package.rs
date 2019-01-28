pub trait IntoBuilder<T> {
    fn into_builder(self) -> T;
}
