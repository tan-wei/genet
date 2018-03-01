#ifndef PLUGKIT_SANDBOX_HPP
#define PLUGKIT_SANDBOX_HPP

namespace plugkit {
namespace Sandbox {
enum Profile { PROFILE_FILTER, PROFILE_DISSECTOR };
void activate(Profile profile);
}; // namespace Sandbox
} // namespace plugkit

#endif
