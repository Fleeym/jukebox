#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class JBListCell : public CCLayer, public FLAlertLayerProtocol {
protected:
    float m_width;
    float m_height;
    CCLayer* m_layer;

    bool init(CCLayer* layer, CCSize const& size);
    void draw() override;
};
