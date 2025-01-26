#include <jukebox/events/nong_deleted.hpp>

namespace jukebox {

namespace event {

NongDeleted::NongDeleted(const std::string& uniqueId, int gdId)
    : m_uniqueId(uniqueId), m_gdId(gdId) {}

std::string NongDeleted::uniqueId() const { return m_uniqueId; }
int NongDeleted::gdId() const { return m_gdId; }

}  // namespace event

}  // namespace jukebox
