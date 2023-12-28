#include "LayoutTree.hpp"
#include <yoga/node/Node.h>
#include <yoga/algorithm/CalculateLayout.h>
#include <yoga/enums/Direction.h>

using namespace facebook;

class LayoutTree::Impl
{
public:

	yoga::Config* config = nullptr;

	yoga::Node rootNode;

	std::vector<std::unique_ptr<yoga::Node>> unusedNodes;

	yoga::Node* newNode()
	{
		if (unusedNodes.empty())
		{
			return config == nullptr
				? new yoga::Node()
				: new yoga::Node(config);
		}

		yoga::Node* cachedNode = unusedNodes.back().release();
		unusedNodes.pop_back();

		cachedNode->reset();

		return cachedNode;
	}

	void releaseNode(yoga::Node* node)
	{
		for (auto child : node->getChildren())
		{
			releaseNode(child);
		}

		node->setOwner(nullptr);
		node->clearChildren();
		node->setContext(nullptr);

		unusedNodes.emplace_back(std::unique_ptr<yoga::Node>(node));
	}

	void construct(yoga::Node& node, Widget& widget)
	{
		assert(widget.children.empty() || widget.allowChildren());

		auto& children = node.getChildren();

		// IDが違うときはOwnerだけ残してリセット
		if (node.getContext() == nullptr ||
			Widget::GetInstance(node)->id() != widget.id())
		{
			auto owner = node.getOwner();
			auto children = node.getChildren();

			for (auto childNode : children)
			{
				releaseNode(childNode);
			}
			node.setOwner(nullptr);
			node.clearChildren();
			node.reset();

			node.setOwner(owner);
			node.setContext(&widget);
		}

		// 子要素がないとき全解放
		if (widget.children.empty())
		{
			for (auto childNode : children)
			{
				releaseNode(childNode);
			}
			node.clearChildren();
			node.markDirtyAndPropagate();

			// yoga::NodeとWidgetを紐づけ
			widget.attachNode(node);
			return;
		}

		// 多すぎる場合は解放
		for (int32 i = children.size() - 1; i >= static_cast<int32>(widget.children.size()); i--)
		{
			node.removeChild(i);
		}

		// 子ノードの追加
		for (auto [i, childWidget] : Indexed(widget.children))
		{
			if (i < children.size())
			{
				auto child = children[i];
				child->setOwner(&node);
			}
			else
			{
				// 足りない場合はノード作成
				auto newChild = newNode();
				newChild->setOwner(&node);
				newChild->setContext(childWidget.get());

				// 末尾に追加
				node.insertChild(newChild, i);

				// 更新を伝える
				node.markDirtyAndPropagate();
			}
		}

		// 子の更新
		for (auto [i, childWidget] : Indexed(widget.children))
		{
			construct(*children[i], *childWidget);
		}

		// yoga::NodeとWidgetを紐づけ
		widget.attachNode(node);
	}

};

LayoutTree::LayoutTree()
	: m_impl(new Impl()) { }

LayoutTree::LayoutTree(std::shared_ptr<Widget> root)
	: m_impl(new Impl())
{
	construct(root);
}

LayoutTree::~LayoutTree() { }

void LayoutTree::construct(std::shared_ptr<Widget> root)
{
	m_root = root;
	m_impl->construct(m_impl->rootNode, *m_root);
}

void LayoutTree::cleanCache()
{
	m_impl->unusedNodes.clear();
}

void LayoutTree::calculateLayout(float width, float height)
{
	yoga::calculateLayout(
		&(m_impl->rootNode),
		width,
		height,
		yoga::Direction::Inherit
	);

	updateLayoutResults({ 0, 0 }, *m_root);
}

void LayoutTree::updateLayoutResults(Vec2 offset, Widget& widget)
{
	auto& node = *widget.m_node;
	auto& layout = node.getLayout();

	if (not widget.m_layoutResults.has_value() || node.getHasNewLayout())
	{
		widget.m_layoutResults = LayoutResults{
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

	widget.m_layoutResults->offset = offset;

	offset += widget.m_layoutResults->localRect.pos;
	for (auto& child : widget.children)
	{
		updateLayoutResults(offset, *child);
	}
}
