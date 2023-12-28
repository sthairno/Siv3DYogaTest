#pragma once 
#include <Siv3D.hpp>
#include <yoga/style/Style.h>
#include "LayoutResults.hpp"

class LayoutTree;
namespace facebook::yoga { class Node; }

class Widget
{
public:

	Widget()
		: m_id(RandomUint64())
	{ }

	Widget(const Widget& other) = delete;

	Widget(Widget&& other)
		: children(std::move(other.children))
	{
		other.m_id = RandomInt64();
	}

	Widget& operator=(Widget&& other)
	{
		children = std::move(other.children);
		other.m_id = RandomInt64();
	}

	Widget& operator=(const Widget&) = delete;

	static Widget* GetInstance(const facebook::yoga::Node& node);

	static Widget* GetInstance(const YGNodeConstRef node);

public:

	std::list<std::shared_ptr<Widget>> children;

	int64 id() const { return m_id; }

	facebook::yoga::Node* layoutNode() const { return m_node; }

	Optional<LayoutResults> layoutResults() const;

	facebook::yoga::Style& style();

	const facebook::yoga::Style& style() const;

	void setStyle(const facebook::yoga::Style& style);

public:

	void draw();

	void markLayoutDirty();

	virtual bool allowChildren() const { return true; }

protected:

	void drawChildren() const;

	virtual void drawContent(LayoutResults) const;

	virtual void onLayoutNodeAttach(facebook::yoga::Node&) { }

private:

	friend LayoutTree;

	intptr_t m_id;

	facebook::yoga::Node* m_node = nullptr;

	facebook::yoga::Style m_styleCache;

	void attachNode(facebook::yoga::Node& node);

	void detachNode();

public:

	virtual ~Widget() = default;
};
