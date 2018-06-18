#ifndef PLUGNODE_PLUGNEL_H
#define PLUGNODE_PLUGNEL_H

#include <stdint.h>
#include <string>
#include <vector>

extern "C" {
typedef uint32_t Token;
struct Context;
struct Layer;
struct Frame;
struct Session;
struct SessionProfile;
struct Filter;
struct Iter;

struct Range {
  uint32_t start;
  uint32_t end;
};

const char* plug_embedded_js();

void plug_str_free(char* data);

Token plug_token_get(const char* str);
char* plug_token_string(Token id);

void plug_context_close_stream(Context* context);
char* plug_context_get_config(Context* context, const char* str);
void plug_context_free(Context* context);

Layer* plug_layer_new(Token id);
Token plug_layer_id(const Layer* layer);
Range plug_layer_range(const Layer* layer);
void plug_layer_set_range(Layer* layer, Range range);
uint8_t plug_layer_worker(const Layer* layer);
void plug_layer_set_worker(Layer* layer, uint8_t worker);
void plug_layer_add_tag(Layer* layer, Token tag);
const Layer* plug_layer_children(const Layer* layer, size_t* len);
void plug_layer_add_child_move(Layer* layer, Layer* child);
void plug_layer_free(Layer* layer);

Iter* plug_layer_tags(const Layer* layer);
uint8_t plug_layer_tags_next(Iter* iter, Token* dst);

uint32_t plug_frame_index(const Frame* frame);
Layer* plug_frame_root_mut(Frame* frame);

SessionProfile* plug_session_profile_new();
uint32_t plug_session_profile_concurrency(const SessionProfile* SessionProfile);
void plug_session_profile_set_concurrency(SessionProfile* SessionProfile,
                                          uint32_t concurrency);
void plug_session_profile_add_link_layer(SessionProfile* SessionProfile,
                                         int32_t link,
                                         const char* id);
void plug_session_profile_set_config(SessionProfile* SessionProfile,
                                     const char* key,
                                     const char* value);
void plug_session_profile_free(SessionProfile* SessionProfile);

Session* plug_session_new(const SessionProfile* SessionProfile,
                          void (*callback)(void*, char*),
                          void* data);
Context* plug_session_context(Session* session);
void plug_session_push_frame(Session* session,
                             const char* data,
                             uint32_t len,
                             int32_t link);
void plug_session_frames(const Session* session,
                         uint32_t start,
                         uint32_t end,
                         size_t* len,
                         Frame const** dst);
void plug_session_filtered_frames(const Session* session,
                                  uint32_t id,
                                  uint32_t start,
                                  uint32_t end,
                                  size_t* len,
                                  Frame const** dst);
void plug_session_set_filter(Session* session, uint32_t id, Filter* filter);
uint32_t plug_session_len(const Session* session);
void plug_session_free(Session* session);
}

namespace genet_node {
class FilterIsolate;
}

struct FilterWorker;
struct Filter {
  FilterWorker* (*new_worker)(Filter*);
  void (*destroy)(Filter*);
  std::string data;
};

struct FilterWorker {
  uint8_t (*test)(FilterWorker*, const Frame*);
  void (*destroy)(FilterWorker*);
  genet_node::FilterIsolate* data;
};

#endif
