#include "get_song_info_event.hpp"
#include "Geode/loader/Event.hpp"
#include "Geode/utils/MiniFunction.hpp"

ListenerResult GetSongInfoEventFilter::handle(MiniFunction<Callback> fn, GetSongInfoEvent* event) {
    if (event->getObject() == nullptr) {
        return ListenerResult::Stop;
    }

    if (event->getObject()->m_songID == m_songID) {
        fn(event->getObject());
        return ListenerResult::Stop;
    }
    return ListenerResult::Propagate;
}