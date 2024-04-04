#pragma once

#include <Geode/binding/FLAlertLayerProtocol.hpp>
#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h>

using namespace geode::prelude;

namespace jukebox {

class JBListCell : public CCLayer, public FLAlertLayerProtocol {
protected:
    float m_width;
    float m_height;
    CCLayer* m_layer;

    bool init(CCLayer* layer, CCSize const& size);
    void draw() override;
};

}
