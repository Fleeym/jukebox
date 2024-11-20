#pragma once

#include <functional>

#include "Geode/cocos/CCScheduler.h"
#include "Geode/cocos/cocoa/CCObject.h"

namespace spitfire {

enum class ChangeDetection { NONE, ON_UPDATE, ON_FRAME };

template <typename T>
class State final : public cocos2d::CCObject {
private:
    bool m_isDirty = false;
    ChangeDetection m_cdType;
    T m_state;
    std::function<void(const T&)> m_notify;

    bool init(T state, ChangeDetection changeDetectionType,
              std::function<void(const T&)> notify) {
        m_state = state;
        m_cdType = changeDetectionType;
        m_notify = notify;

        cocos2d::CCScheduler::get()->scheduleUpdateForTarget(this, 0, false);

        return true;
    }

public:
    void update(float dt) {
        if (m_cdType != ChangeDetection::ON_FRAME) {
            return;
        }

        this->runChangeDetection();
    }

    void runChangeDetection() {
        if (!m_isDirty) {
            return;
        }

        if (m_notify) {
            m_notify(m_state);
        }

        m_isDirty = false;
    }

    void updateState(std::function<void(T& state)> callback) {
        callback(m_state);
        m_isDirty = true;

        if (m_cdType == ChangeDetection::ON_UPDATE) {
            this->runChangeDetection();
        }
    }

    static State* create(T state, ChangeDetection changeDetectionType,
                         std::function<void(const T&)> notify) {
        auto ret = new State<T>;
        if (ret->init(state, changeDetectionType, notify)) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }

    const T& value() { return m_state; }
    ChangeDetection changeDetectionType() const { return m_cdType; }
    bool isDirty() const { return m_isDirty; }
};

}  // namespace spitfire
