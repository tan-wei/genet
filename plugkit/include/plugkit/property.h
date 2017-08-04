#ifndef PLUGKIT_PROPERTY_H
#define PLUGKIT_PROPERTY_H

#include "token.h"
#include "range.h"

extern "C" {

namespace plugkit {

class Variant;
class Property;

/// Gets id
Token Property_id(const Property *prop);

/// Allocates a new Property and adds it as a child property.
Property *Property_addProperty(Property *prop, Token id);

/// Finds the first child property with the given id and returns it.
///
/// If no property is found, returns nullptr.
const Property *Property_propertyFromId(const Property *prop, Token id);

Range Property_range(const Property *prop);
void Property_setRange(Property *prop, Range range);

const Variant *Property_value(const Property *prop);
Variant *Property_valueRef(Property *prop);
}
}

#endif
