/// @file
/// Binary sequence
#ifndef PLUGKIT_VIEW_H
#define PLUGKIT_VIEW_H

extern "C" {

namespace plugkit {

struct View {
  const char *begin;
  const char *end;
};

size_t View_length(View view);
View View_slice(View view, size_t offset, size_t length);
}
}

#endif
