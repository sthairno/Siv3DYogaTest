#include "Widget.hpp"
#include <yoga/node/Node.h>
#include <yoga/event/event.h>

using namespace facebook;

Widget* Widget::GetInstance(const yoga::Node& node)
{
	return reinterpret_cast<Widget*>(node.getContext());
}

Widget* Widget::GetInstance(const YGNodeConstRef node)
{
	return reinterpret_cast<Widget*>(YGNodeGetContext(node));
}

Optional<LayoutResults> Widget::layoutResults() const
{
	auto node = layoutNode();

	if (node == nullptr)
	{
		return none;
	}

	auto layout = node->getLayout();
	return LayoutResults{
		.margin = {
			layout.margin(facebook::yoga::Edge::Left),
			layout.margin(facebook::yoga::Edge::Top),
			layout.margin(facebook::yoga::Edge::Right),
			layout.margin(facebook::yoga::Edge::Bottom),
		},
		.localRect = {
			layout.position(facebook::yoga::Edge::Left),
			layout.position(facebook::yoga::Edge::Top),
			layout.dimension(facebook::yoga::Dimension::Width),
			layout.dimension(facebook::yoga::Dimension::Height)
		},
		.border = {
			layout.border(facebook::yoga::Edge::Left),
			layout.border(facebook::yoga::Edge::Top),
			layout.border(facebook::yoga::Edge::Right),
			layout.border(facebook::yoga::Edge::Bottom),
		},
		.padding = {
			layout.padding(facebook::yoga::Edge::Left),
			layout.padding(facebook::yoga::Edge::Top),
			layout.padding(facebook::yoga::Edge::Right),
			layout.padding(facebook::yoga::Edge::Bottom),
		}
	};
}

yoga::Style& Widget::style()
{
	if (m_node)
	{
		return m_node->getStyle();
	}
	else
	{
		return m_styleCache;
	}
}

const facebook::yoga::Style& Widget::style() const
{
	if (m_node)
	{
		return m_node->getStyle();
	}
	else
	{
		return m_styleCache;
	}
}

void Widget::setStyle(const yoga::Style& style)
{
	if (m_node)
	{
		m_node->setStyle(style);
	}
	else
	{
		m_styleCache = style;
	}
}

std::shared_ptr<Widget> Widget::query(const StringView value)
{
	auto result = queryAll(value, 1);
	return result.empty() ? nullptr : result.at(0);
}

Array<std::shared_ptr<Widget>> Widget::queryAll(const StringView value, size_t limit)
{
	Array<std::shared_ptr<Widget>> result;
	queryAll(value, result, limit);
	return result;
}

void Widget::queryAll(const StringView value, Array<std::shared_ptr<Widget>>& result, size_t limit)
{
	for (auto& child : children)
	{
		child->queryAll(value, result, limit);

		if (result.size() >= limit)
		{
			return;
		}
	}

	if (result.size() >= limit)
	{
		return;
	}

	if (name == value)
	{
		result.push_back(shared_from_this());
	}
}

void Widget::draw()
{
	drawContent(*layoutResults());
}

void Widget::markLayoutDirty()
{
	if (m_node)
	{
		m_node->markDirtyAndPropagate();
	}
}

void Widget::drawChildren() const
{
	if (auto result = layoutResults())
	{
		Transformer2D t(Mat3x2::Translate(result->localRect.pos));
		for (auto child : children)
		{
			child->draw();
		}
	}
}

void Widget::drawContent(LayoutResults layout) const
{
	layout.localRect.drawFrame(1, Palette::Black);
	drawChildren();
}

void Widget::attachNode(facebook::yoga::Node& node)
{
	if (m_node)
	{
		detachNode();
	}

	m_node = &node;
	m_node->setStyle(m_styleCache);
	
	onLayoutNodeAttach(*m_node);
}

void Widget::detachNode()
{
	if (!m_node)
	{
		return;
	}

	m_styleCache = m_node->getStyle();
	m_node = nullptr;
}
