#include "attribute.hpp"

namespace plugkit {

class Attribute::Private {
public:
  Private(miniid id, const Variant &value);
  miniid id;
  Variant value;
};

Attribute::Private::Private(miniid id, const Variant &value)
    : id(id), value(value) {}

Attribute::Attribute(miniid id, const Variant &value)
    : d(new Private(id, value)) {}

Attribute::~Attribute() {}

miniid Attribute::id() const { return d->id; }

void Attribute::setId(miniid id) { d->id = id; }

Variant Attribute::value() const { return d->value; }

void Attribute::setValue(const Variant &value) { d->value = value; }
}
