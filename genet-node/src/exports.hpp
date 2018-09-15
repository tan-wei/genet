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
  ByteSlice = 7,
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

struct Payload {
  char *data;
  uint64_t len;
  Token id;
  Token type;
};

const char *genet_embedded_js();
uint64_t genet_abi_version();

void genet_str_free(char *data);

Token genet_token_get(const char *str);
char *genet_token_string(Token id);

Token genet_attr_id(const Attr *attr);
Token genet_attr_type(const Attr *attr);
void genet_attr_range(const Attr *attr, uint64_t *start, uint64_t *end);
Variant genet_attr_get(const Attr *attr, const Layer *layer);

Token genet_layer_id(const Layer *layer);
const Attr *genet_layer_attr(const Layer *layer, Token id);
char *genet_layer_data(const Layer *layer, uint64_t *len);
Attr const *const *genet_layer_attrs(const Layer *layer, uint32_t *len);
Attr const *const *genet_layer_headers(const Layer *layer, uint32_t *len);
const Payload *genet_layer_payloads(const Layer *layer, uint32_t *len);

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
                                   uint32_t *dst);
void genet_session_set_filter(Session *session,
                              uint32_t id,
                              const char *filter);
uint32_t
genet_session_create_reader(Session *session, const char *id, const char *arg);
uint32_t genet_session_create_writer(Session *session,
                                     const char *id,
                                     const char *arg,
                                     const char *filter);
void genet_session_close_reader(Session *session, uint32_t handle);
uint32_t genet_session_len(const Session *session);
void genet_session_free(Session *session);
}

#endif
