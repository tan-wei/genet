#ifndef GENET_NODE_PLUGNEL_H
#define GENET_NODE_PLUGNEL_H

#include <stdint.h>
#include <string>
#include <vector>

extern "C" {
typedef uint32_t Token;
struct Context;
struct Layer;
struct Attr;
struct Frame;
struct Session;
struct SessionProfile;
struct Filter;
struct Iter;

struct Range {
  uint32_t start;
  uint32_t end;
};

enum VariantType {
  Error = -1,
  Nil = 0,
  Bool = 1,
  Int64 = 2,
  UInt64 = 3,
  Float64 = 4,
  String = 5,
  Buffer = 6,
  Slice = 7,
};

union VariantValue {
  uint64_t u64;
  int64_t i64;
  double f64;
};

struct Variant {
  int8_t type;
  VariantValue value;
  char *data;
};

const char *genet_embedded_js();

void genet_str_free(char *data);

Token genet_token_get(const char *str);
char *genet_token_string(Token id);

void genet_context_close_stream(Context *context);
char *genet_context_get_config(Context *context, const char *str);
void genet_context_free(Context *context);

Token genet_attr_id(const Attr *attr);
Token genet_attr_type(const Attr *attr);
void genet_attr_range(const Attr *attr, uint64_t *start, uint64_t *end);
Variant genet_attr_get(const Attr *attr, const Layer *layer);

Layer *genet_layer_new(Token id);
Token genet_layer_id(const Layer *layer);
const Attr *genet_layer_attr(const Layer *layer, Token id);
Attr const *const *genet_layer_attrs(const Layer *layer, uint64_t *len);
Attr const *const *genet_layer_headers(const Layer *layer, uint64_t *len);

uint32_t genet_frame_index(const Frame *frame);
Layer const *const *genet_frame_layers(const Frame *frame, uint32_t *len);
uint8_t *genet_frame_tree_indices(const Frame *frame, uint32_t *len);
const Attr *genet_frame_attr(const Frame *frame, Token id);

SessionProfile *genet_session_profile_new();
uint32_t
genet_session_profile_concurrency(const SessionProfile *SessionProfile);
void genet_session_profile_set_concurrency(SessionProfile *SessionProfile,
                                           uint32_t concurrency);
char *genet_session_profile_load_library(SessionProfile *SessionProfile,
                                         const char *path);
void genet_session_profile_set_config(SessionProfile *SessionProfile,
                                      const char *key,
                                      const char *value);
void genet_session_profile_free(SessionProfile *SessionProfile);

Session *genet_session_new(const SessionProfile *SessionProfile,
                           void (*callback)(void *, char *),
                           void *data);
Context *genet_session_context(Session *session);
void genet_session_push_frame(Session *session,
                              const char *data,
                              uint32_t len,
                              int32_t link);
void genet_session_frames(const Session *session,
                          uint32_t start,
                          uint32_t end,
                          uint32_t *len,
                          Frame const **dst);
void genet_session_filtered_frames(const Session *session,
                                   uint32_t id,
                                   uint32_t start,
                                   uint32_t end,
                                   uint32_t *len,
                                   Frame const **dst);
void genet_session_set_filter(Session *session, uint32_t id, Filter *filter);
uint32_t
genet_session_create_reader(Session *session, const char *id, const char *arg);
uint32_t
genet_session_create_writer(Session *session, const char *id, const char *arg);
void genet_session_close_reader(Session *session, uint32_t handle);
uint32_t genet_session_len(const Session *session);
void genet_session_free(Session *session);
}

namespace genet_node {
class FilterIsolate;
}

struct FilterWorker;
struct Filter {
  FilterWorker *(*new_worker)(Filter *);
  void (*destroy)(Filter *);
  std::string data;
};

struct FilterWorker {
  uint8_t (*test)(FilterWorker *, const Frame *);
  void (*destroy)(FilterWorker *);
  genet_node::FilterIsolate *data;
};

#endif
