#include "null_logger.hpp"

namespace plugkit {

NullLogger::NullLogger() {}

NullLogger::~NullLogger() {}

void NullLogger::log(MessagePtr &&msg) {}

void NullLogger::log(Level level, const std::string &message,
                     const std::string &domain,
                     const std::string &resourceName) {}

void NullLogger::logTrivial(Level level, const std::string &message,
                            const std::string &domain,
                            const std::string &resourceName) {}
}
