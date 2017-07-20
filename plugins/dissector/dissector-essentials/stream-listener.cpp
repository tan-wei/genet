#include <nan.h>
#include <plugkit/listener.hpp>
#include <plugkit/chunk.hpp>
#include <plugkit/frame_view.hpp>

using namespace plugkit;

class StreamListener : public Listener {
public:
  StreamListener();
  bool analyze(const FrameView *frame) override;
  std::vector<ChunkConstPtr> chunks() const override;

public:
  const Layer *layer;
};

StreamListener::StreamListener() {}

bool StreamListener::analyze(const FrameView *frame) {
  layer = frame->primaryLayer();
  return true;
}

std::vector<ChunkConstPtr> StreamListener::chunks() const {
  return std::vector<ChunkConstPtr>{{std::make_shared<Chunk>(layer)}};
}

class StreamListenerFactory : public ListenerFactory {
public:
  ListenerPtr create(const Variant &args,
                     const SessionContext &context) const override;
};

ListenerPtr StreamListenerFactory::create(const Variant &args,
                                          const SessionContext &context) const {
  return ListenerPtr(new StreamListener());
}

void Init(v8::Local<v8::Object> exports) {
  exports->Set(
      Nan::New("factory").ToLocalChecked(),
      ListenerFactory::wrap(std::make_shared<StreamListenerFactory>()));
}

NODE_MODULE(dissectorEssentials, Init);
