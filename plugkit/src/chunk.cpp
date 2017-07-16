#include "chunk.hpp"

namespace plugkit {

class Chunk::Private {
public:
  Private(const Layer *layer, Range range);

public:
  const Layer *layer;
  Range range;
};

Chunk::Private::Private(const Layer *layer, Range range)
    : layer(layer), range(range) {}

Chunk::Chunk(const Layer *layer, Range range) : d(new Private(layer, range)) {}

Chunk::~Chunk() {}

const Layer *Chunk::layer() const { return d->layer; }

void Chunk::setLayer(const Layer *layer) { d->layer = layer; }

Range Chunk::range() const { return d->range; }

void Chunk::setRange(const Range &range) { d->range = range; }
}
