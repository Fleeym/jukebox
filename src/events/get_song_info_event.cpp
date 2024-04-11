#include <Geode/loader/Event.hpp>
#include <Geode/utils/MiniFunction.hpp>

#include "get_song_info_event.hpp"

ListenerResult GetSongInfoEventFilter::handle(MiniFunction<Callback> fn, GetSongInfoEvent* event) {
    if (event->getObject() == nullptr) {
        return ListenerResult::Propagate;
    }

    if (event->getObject()->m_songID == m_songID) {
        fn(event->getObject());
        return ListenerResult::Propagate;
    }
    return ListenerResult::Propagate;
}