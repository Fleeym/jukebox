#include "list_cell.hpp"

bool JBListCell::init(CCLayer* layer, CCSize const& size) {
    m_width = size.width;
    m_height = size.height;
    m_layer = layer;
    this->setContentSize(size);
    this->setID("nong-list-cell");
    return true;
}

void JBListCell::draw() {
    reinterpret_cast<StatsCell*>(this)->StatsCell::draw();
}