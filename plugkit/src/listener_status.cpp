#include "listener_status.hpp"
#include <mutex>

namespace plugkit {

class ListenerStatus::Private {
public:
};

ListenerStatus::ListenerStatus() : d(new Private()) {}

ListenerStatus::~ListenerStatus() {}
}
/*
std::map<miniid, Property *> properties;
std::list<Chunk *> chunks;
*/
