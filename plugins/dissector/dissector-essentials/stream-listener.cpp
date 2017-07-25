#include <nan.h>
#include <plugkit/listener.hpp>
#include <plugkit/chunk.hpp>
#include <plugkit/frame_view.hpp>
#include <plugkit/layer.hpp>
#include <plugkit/variant.hpp>

using namespace plugkit;

class StreamListener : public Listener {
public:
  StreamListener(const minins &ns, const minins &streamId);
  bool analyze(const FrameView *frame) override;
  std::vector<ChunkConstPtr> chunks() const override;

public:
  const Layer *layer;
  minins ns;
  minins streamId;
};

StreamListener::StreamListener(const minins &ns, const minins &streamId)
    : ns(ns), streamId(streamId) {}

bool StreamListener::analyze(const FrameView *frame) {
  layer = nullptr;
  if (frame->primaryLayer()->ns() == ns &&
      frame->primaryLayer()->streamId() == streamId) {
    layer = frame->primaryLayer();
  }
  return layer;
}

std::vector<ChunkConstPtr> StreamListener::chunks() const {
  auto chunk = std::make_shared<Chunk>(layer);
  chunk->setRange(std::make_pair(0ul, layer->payload().length()));
  return std::vector<ChunkConstPtr>{{chunk}};
}

class StreamListenerFactory : public ListenerFactory {
public:
  ListenerPtr create(const Variant &args,
                     const SessionContext &context) const override;
};

ListenerPtr StreamListenerFactory::create(const Variant &args,
                                          const SessionContext &context) const {
  minins ns(args["namespace"].string().c_str());
  minins streamId;
  const auto &slice = args["streamId"].slice();
  std::memcpy((char *)&streamId.id[0], slice.data(), slice.length());
  return ListenerPtr(new StreamListener(ns, streamId));
}

void Init(v8::Local<v8::Object> exports) {
  exports->Set(
      Nan::New("factory").ToLocalChecked(),
      ListenerFactory::wrap(std::make_shared<StreamListenerFactory>()));
}

NODE_MODULE(dissectorEssentials, Init);
