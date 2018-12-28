//! Cast objects for the usual data structures.

use crate::{
    attr::{Attr, MetadataOption, NodeBuilder, SizedField},
    context::Context,
    slice,
};
use byteorder::{BigEndian, LittleEndian, ReadBytesExt};
pub use genet_abi::cast::{Cast, Map, Nil, Typed};
use genet_abi::slice::TryGet;

use std::{
    io::{Cursor, Error, ErrorKind, Result},
    mem::size_of,
};

/// Cast for 8bit unsigned integer.
#[derive(Clone, Default)]
pub struct UInt8();

impl Typed for UInt8 {
    type Output = u8;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<u8> {
        Cursor::new(data.try_get(attr.range())?).read_u8()
    }
}

impl SizedField for UInt8 {
    fn bit_size(&self) -> usize {
        size_of::<u8>() * 8
    }
}

pub struct UInt8Builder {}

impl MetadataOption for UInt8Builder {}

impl Into<UInt8> for UInt8Builder {
    fn into(self) -> UInt8 {
        UInt8()
    }
}

impl NodeBuilder<Self> for UInt8 {
    type Builder = UInt8Builder;

    fn build(_ctx: &Context) -> Self::Builder {
        UInt8Builder {}
    }
}

/// Cast for 8bit signed integer.
#[derive(Clone, Default)]
pub struct Int8();

impl Typed for Int8 {
    type Output = i8;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<i8> {
        Cursor::new(data.try_get(attr.range())?).read_i8()
    }
}

impl SizedField for Int8 {
    fn bit_size(&self) -> usize {
        size_of::<i8>() * 8
    }
}

pub struct Int8Builder {}

impl MetadataOption for Int8Builder {}

impl Into<Int8> for Int8Builder {
    fn into(self) -> Int8 {
        Int8()
    }
}

impl NodeBuilder<Self> for Int8 {
    type Builder = Int8Builder;

    fn build(_ctx: &Context) -> Self::Builder {
        Int8Builder {}
    }
}

/// Cast for big-endian 16bit unsigned integer.
#[derive(Clone, Default)]
pub struct UInt16BE();

impl Typed for UInt16BE {
    type Output = u16;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<u16> {
        Cursor::new(data.try_get(attr.range())?).read_u16::<BigEndian>()
    }
}

impl SizedField for UInt16BE {
    fn bit_size(&self) -> usize {
        size_of::<u16>() * 8
    }
}

pub struct UInt16BEBuilder {}

impl MetadataOption for UInt16BEBuilder {}

impl Into<UInt16BE> for UInt16BEBuilder {
    fn into(self) -> UInt16BE {
        UInt16BE()
    }
}

impl NodeBuilder<Self> for UInt16BE {
    type Builder = UInt16BEBuilder;

    fn build(_ctx: &Context) -> Self::Builder {
        UInt16BEBuilder {}
    }
}

/// Cast for big-endian 32bit unsigned integer.
#[derive(Clone, Default)]
pub struct UInt32BE();

impl Typed for UInt32BE {
    type Output = u32;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<u32> {
        Cursor::new(data.try_get(attr.range())?).read_u32::<BigEndian>()
    }
}

impl SizedField for UInt32BE {
    fn bit_size(&self) -> usize {
        size_of::<u32>() * 8
    }
}

pub struct UInt32BEBuilder {}

impl MetadataOption for UInt32BEBuilder {}

impl Into<UInt32BE> for UInt32BEBuilder {
    fn into(self) -> UInt32BE {
        UInt32BE()
    }
}

impl NodeBuilder<Self> for UInt32BE {
    type Builder = UInt32BEBuilder;

    fn build(_ctx: &Context) -> Self::Builder {
        UInt32BEBuilder {}
    }
}

/// Cast for big-endian 64bit unsigned integer.
#[derive(Clone, Default)]
pub struct UInt64BE();

impl Typed for UInt64BE {
    type Output = u64;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<u64> {
        Cursor::new(data.try_get(attr.range())?).read_u64::<BigEndian>()
    }
}

impl SizedField for UInt64BE {
    fn bit_size(&self) -> usize {
        size_of::<u64>() * 8
    }
}

pub struct UInt64BEBuilder {}

impl MetadataOption for UInt64BEBuilder {}

impl Into<UInt64BE> for UInt64BEBuilder {
    fn into(self) -> UInt64BE {
        UInt64BE()
    }
}

impl NodeBuilder<Self> for UInt64BE {
    type Builder = UInt64BEBuilder;

    fn build(_ctx: &Context) -> Self::Builder {
        UInt64BEBuilder {}
    }
}

/// Cast for big-endian 16bit signed integer.
#[derive(Clone, Default)]
pub struct Int16BE();

impl Typed for Int16BE {
    type Output = i16;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<i16> {
        Cursor::new(data.try_get(attr.range())?).read_i16::<BigEndian>()
    }
}

pub struct Int16BEBuilder {}

impl MetadataOption for Int16BEBuilder {}

impl Into<Int16BE> for Int16BEBuilder {
    fn into(self) -> Int16BE {
        Int16BE()
    }
}

impl NodeBuilder<Self> for Int16BE {
    type Builder = Int16BEBuilder;

    fn build(_ctx: &Context) -> Self::Builder {
        Int16BEBuilder {}
    }
}

impl SizedField for Int16BE {
    fn bit_size(&self) -> usize {
        size_of::<i16>() * 8
    }
}

/// Cast for big-endian 32bit signed integer.
#[derive(Clone, Default)]
pub struct Int32BE();

impl Typed for Int32BE {
    type Output = i32;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<i32> {
        Cursor::new(data.try_get(attr.range())?).read_i32::<BigEndian>()
    }
}

impl SizedField for Int32BE {
    fn bit_size(&self) -> usize {
        size_of::<i32>() * 8
    }
}

pub struct Int32BEBuilder {}

impl MetadataOption for Int32BEBuilder {}

impl Into<Int32BE> for Int32BEBuilder {
    fn into(self) -> Int32BE {
        Int32BE()
    }
}

impl NodeBuilder<Self> for Int32BE {
    type Builder = Int32BEBuilder;

    fn build(_ctx: &Context) -> Self::Builder {
        Int32BEBuilder {}
    }
}

/// Cast for big-endian 64bit signed integer.
#[derive(Clone, Default)]
pub struct Int64BE();

impl Typed for Int64BE {
    type Output = i64;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<i64> {
        Cursor::new(data.try_get(attr.range())?).read_i64::<BigEndian>()
    }
}

impl SizedField for Int64BE {
    fn bit_size(&self) -> usize {
        size_of::<i64>() * 8
    }
}

pub struct Int64BEBuilder {}

impl MetadataOption for Int64BEBuilder {}

impl Into<Int64BE> for Int64BEBuilder {
    fn into(self) -> Int64BE {
        Int64BE()
    }
}

impl NodeBuilder<Self> for Int64BE {
    type Builder = Int64BEBuilder;

    fn build(_ctx: &Context) -> Self::Builder {
        Int64BEBuilder {}
    }
}

/// Cast for big-endian 32bit floating point number.
#[derive(Clone, Default)]
pub struct Float32BE();

impl Typed for Float32BE {
    type Output = f32;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<f32> {
        Cursor::new(data.try_get(attr.range())?).read_f32::<BigEndian>()
    }
}

impl SizedField for Float32BE {
    fn bit_size(&self) -> usize {
        size_of::<f32>() * 8
    }
}

pub struct Float32BEBuilder {}

impl MetadataOption for Float32BEBuilder {}

impl Into<Float32BE> for Float32BEBuilder {
    fn into(self) -> Float32BE {
        Float32BE()
    }
}

impl NodeBuilder<Self> for Float32BE {
    type Builder = Float32BEBuilder;

    fn build(_ctx: &Context) -> Self::Builder {
        Float32BEBuilder {}
    }
}

/// Cast for big-endian 64bit floating point number.
#[derive(Clone, Default)]
pub struct Float64BE();

impl Typed for Float64BE {
    type Output = f64;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<f64> {
        Cursor::new(data.try_get(attr.range())?).read_f64::<BigEndian>()
    }
}

impl SizedField for Float64BE {
    fn bit_size(&self) -> usize {
        size_of::<f64>() * 8
    }
}

pub struct Float64BEBuilder {}

impl MetadataOption for Float64BEBuilder {}

impl Into<Float64BE> for Float64BEBuilder {
    fn into(self) -> Float64BE {
        Float64BE()
    }
}

impl NodeBuilder<Self> for Float64BE {
    type Builder = Float64BEBuilder;

    fn build(_ctx: &Context) -> Self::Builder {
        Float64BEBuilder {}
    }
}

/// Cast for little-endian 16bit unsigned integer.
#[derive(Clone, Default)]
pub struct UInt16LE();

impl Typed for UInt16LE {
    type Output = u16;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<u16> {
        Cursor::new(data.try_get(attr.range())?).read_u16::<LittleEndian>()
    }
}

impl SizedField for UInt16LE {
    fn bit_size(&self) -> usize {
        size_of::<u16>() * 8
    }
}

pub struct UInt16LEBuilder {}

impl MetadataOption for UInt16LEBuilder {}

impl Into<UInt16LE> for UInt16LEBuilder {
    fn into(self) -> UInt16LE {
        UInt16LE()
    }
}

impl NodeBuilder<Self> for UInt16LE {
    type Builder = UInt16LEBuilder;

    fn build(_ctx: &Context) -> Self::Builder {
        UInt16LEBuilder {}
    }
}

/// Cast for little-endian 32bit unsigned integer.
#[derive(Clone, Default)]
pub struct UInt32LE();

impl Typed for UInt32LE {
    type Output = u32;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<u32> {
        Cursor::new(data.try_get(attr.range())?).read_u32::<LittleEndian>()
    }
}

impl SizedField for UInt32LE {
    fn bit_size(&self) -> usize {
        size_of::<u32>() * 8
    }
}

pub struct UInt32LEBuilder {}

impl MetadataOption for UInt32LEBuilder {}

impl Into<UInt32LE> for UInt32LEBuilder {
    fn into(self) -> UInt32LE {
        UInt32LE()
    }
}

impl NodeBuilder<Self> for UInt32LE {
    type Builder = UInt32LEBuilder;

    fn build(_ctx: &Context) -> Self::Builder {
        UInt32LEBuilder {}
    }
}

/// Cast for little-endian 64bit unsigned integer.
#[derive(Clone, Default)]
pub struct UInt64LE();

impl Typed for UInt64LE {
    type Output = u64;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<u64> {
        Cursor::new(data.try_get(attr.range())?).read_u64::<LittleEndian>()
    }
}

impl SizedField for UInt64LE {
    fn bit_size(&self) -> usize {
        size_of::<u64>() * 8
    }
}

pub struct UInt64LEBuilder {}

impl MetadataOption for UInt64LEBuilder {}

impl Into<UInt64LE> for UInt64LEBuilder {
    fn into(self) -> UInt64LE {
        UInt64LE()
    }
}

impl NodeBuilder<Self> for UInt64LE {
    type Builder = UInt64LEBuilder;

    fn build(_ctx: &Context) -> Self::Builder {
        UInt64LEBuilder {}
    }
}

/// Cast for little-endian 16bit signed integer.
#[derive(Clone, Default)]
pub struct Int16LE();

impl Typed for Int16LE {
    type Output = i16;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<i16> {
        Cursor::new(data.try_get(attr.range())?).read_i16::<LittleEndian>()
    }
}

impl SizedField for Int16LE {
    fn bit_size(&self) -> usize {
        size_of::<i16>() * 8
    }
}

pub struct Int16LEBuilder {}

impl MetadataOption for Int16LEBuilder {}

impl Into<Int16LE> for Int16LEBuilder {
    fn into(self) -> Int16LE {
        Int16LE()
    }
}

impl NodeBuilder<Self> for Int16LE {
    type Builder = Int16LEBuilder;

    fn build(_ctx: &Context) -> Self::Builder {
        Int16LEBuilder {}
    }
}

/// Cast for little-endian 32bit signed integer.
#[derive(Clone, Default)]
pub struct Int32LE();

impl Typed for Int32LE {
    type Output = i32;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<i32> {
        Cursor::new(data.try_get(attr.range())?).read_i32::<LittleEndian>()
    }
}

impl SizedField for Int32LE {
    fn bit_size(&self) -> usize {
        size_of::<i32>() * 8
    }
}

pub struct Int32LEBuilder {}

impl MetadataOption for Int32LEBuilder {}

impl Into<Int32LE> for Int32LEBuilder {
    fn into(self) -> Int32LE {
        Int32LE()
    }
}

impl NodeBuilder<Self> for Int32LE {
    type Builder = Int32LEBuilder;

    fn build(_ctx: &Context) -> Self::Builder {
        Int32LEBuilder {}
    }
}

/// Cast for little-endian 64bit signed integer.
#[derive(Clone, Default)]
pub struct Int64LE();

impl Typed for Int64LE {
    type Output = i64;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<i64> {
        Cursor::new(data.try_get(attr.range())?).read_i64::<LittleEndian>()
    }
}

impl SizedField for Int64LE {
    fn bit_size(&self) -> usize {
        size_of::<i64>() * 8
    }
}

pub struct Int64LEBuilder {}

impl MetadataOption for Int64LEBuilder {}

impl Into<Int64LE> for Int64LEBuilder {
    fn into(self) -> Int64LE {
        Int64LE()
    }
}

impl NodeBuilder<Self> for Int64LE {
    type Builder = Int64LEBuilder;

    fn build(_ctx: &Context) -> Self::Builder {
        Int64LEBuilder {}
    }
}

/// Cast for little-endian 32bit floating point number.
#[derive(Clone, Default)]
pub struct Float32LE();

impl Typed for Float32LE {
    type Output = f32;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<f32> {
        Cursor::new(data.try_get(attr.range())?).read_f32::<LittleEndian>()
    }
}

impl SizedField for Float32LE {
    fn bit_size(&self) -> usize {
        size_of::<f32>() * 8
    }
}

pub struct Float32LEBuilder {}

impl MetadataOption for Float32LEBuilder {}

impl Into<Float32LE> for Float32LEBuilder {
    fn into(self) -> Float32LE {
        Float32LE()
    }
}

impl NodeBuilder<Self> for Float32LE {
    type Builder = Float32LEBuilder;

    fn build(_ctx: &Context) -> Self::Builder {
        Float32LEBuilder {}
    }
}

/// Cast for little-endian 64bit floating point number.
#[derive(Clone, Default)]
pub struct Float64LE();

impl Typed for Float64LE {
    type Output = f64;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<f64> {
        Cursor::new(data.try_get(attr.range())?).read_f64::<LittleEndian>()
    }
}

impl SizedField for Float64LE {
    fn bit_size(&self) -> usize {
        size_of::<f64>() * 8
    }
}

pub struct Float64LEBuilder {}

impl MetadataOption for Float64LEBuilder {}

impl Into<Float64LE> for Float64LEBuilder {
    fn into(self) -> Float64LE {
        Float64LE()
    }
}

impl NodeBuilder<Self> for Float64LE {
    type Builder = Float64LEBuilder;

    fn build(_ctx: &Context) -> Self::Builder {
        Float64LEBuilder {}
    }
}

/// Cast for UTF-8 string.
#[derive(Clone, Default)]
pub struct Utf8();

impl Typed for Utf8 {
    type Output = Box<str>;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<Box<str>> {
        if let Ok(s) = String::from_utf8(data.try_get(attr.range())?.to_vec()) {
            Ok(s.into_boxed_str())
        } else {
            Err(Error::new(ErrorKind::InvalidData, "Invalid UTF-8"))
        }
    }
}

pub struct Utf8Builder {}

impl MetadataOption for Utf8Builder {}

impl Into<Utf8> for Utf8Builder {
    fn into(self) -> Utf8 {
        Utf8()
    }
}

impl NodeBuilder<Self> for Utf8 {
    type Builder = Utf8Builder;

    fn build(_ctx: &Context) -> Self::Builder {
        Utf8Builder {}
    }
}

/// Cast for ByteSlice.
#[derive(Clone, Default)]
pub struct ByteSlice();

impl Typed for ByteSlice {
    type Output = slice::ByteSlice;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<slice::ByteSlice> {
        Ok(data.try_get(attr.range())?)
    }
}

pub struct ByteSliceBuilder {}

impl MetadataOption for ByteSliceBuilder {}

impl Into<ByteSlice> for ByteSliceBuilder {
    fn into(self) -> ByteSlice {
        ByteSlice()
    }
}

impl NodeBuilder<Self> for ByteSlice {
    type Builder = ByteSliceBuilder;

    fn build(_ctx: &Context) -> Self::Builder {
        ByteSliceBuilder {}
    }
}

#[derive(Clone, Default)]
pub struct BitFlag();

impl Typed for BitFlag {
    type Output = bool;

    fn cast(&self, attr: &Attr, data: &slice::ByteSlice) -> Result<bool> {
        let byte = Cursor::new(data.try_get(attr.range())?).read_u8()?;
        Ok((byte & (0b1000_0000 >> (attr.bit_range().start % 8))) != 0)
    }
}

impl SizedField for BitFlag {
    fn bit_size(&self) -> usize {
        1
    }
}

pub struct BitFlagBuilder {}

impl MetadataOption for BitFlagBuilder {}

impl Into<BitFlag> for BitFlagBuilder {
    fn into(self) -> BitFlag {
        BitFlag()
    }
}

impl NodeBuilder<Self> for BitFlag {
    type Builder = BitFlagBuilder;

    fn build(_ctx: &Context) -> Self::Builder {
        BitFlagBuilder {}
    }
}
