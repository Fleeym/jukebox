#pragma once

#include <functional>

#include "Geode/cocos/base_nodes/CCNode.h"

#include "spitfire/state.hpp"

namespace spitfire {

template <typename T>
class ReactiveNode : public cocos2d::CCNode {
protected:
    spitfire::State<T>* m_state = nullptr;

    bool init(T initialState,
              ChangeDetection changeDetectionType = ChangeDetection::ON_FRAME) {
        if (!cocos2d::CCNode::init()) {
            return false;
        }

        m_state = spitfire::State<T>::create(
            initialState, changeDetectionType,
            [this](const T& state) { this->onChanges(state); });
        if (!m_state) {
            return false;
        }

        m_state->retain();

        return true;
    }

    ~ReactiveNode() {
        m_state->release();
        m_state = nullptr;
    }

    virtual void onChanges(const T& state) {}

public:
    const T& state() { return m_state->value(); }

    void updateState(std::function<void(T& current)> modifier) {
        m_state->updateState(modifier);
    }

    static ReactiveNode<T>* create(T initialState) {
        auto ret = new ReactiveNode<T>();

        if (ret->init(initialState)) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
};

}  // namespace spitfire
