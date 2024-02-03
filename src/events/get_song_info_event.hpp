#pragma once

#include "Geode/binding/SongInfoObject.hpp"
#include "Geode/cocos/base_nodes/CCNode.h"
#include "Geode/loader/Event.hpp"
#include "Geode/utils/MiniFunction.hpp"

using namespace geode::prelude;

class GetSongInfoEvent : public Event {
protected:
    SongInfoObject* m_songInfo;
public:
    GetSongInfoEvent(SongInfoObject* object)
        : m_songInfo(object) {}
    SongInfoObject* getObject() { return m_songInfo; }
};

class GetSongInfoEventFilter : public EventFilter<GetSongInfoEvent> {
protected:
    int m_songID;
    CCNode* m_target;
public:
    int getSongID() { return m_songID; }
    using Callback = ListenerResult(SongInfoObject* obj);
    ListenerResult handle(MiniFunction<Callback> fn, GetSongInfoEvent* event);
    GetSongInfoEventFilter(CCNode* target, int songID)
        : m_songID(songID), m_target(target) {}
};