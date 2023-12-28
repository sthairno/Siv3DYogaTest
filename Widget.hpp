#pragma once 
#include <Siv3D.hpp>
#include <yoga/style/Style.h>
#include "LayoutResults.hpp"

class LayoutTree;
namespace facebook::yoga { class Node; }

class Widget : private std::enable_shared_from_this<Widget>
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

	String name;

	std::list<std::shared_ptr<Widget>> children;

	int64 id() const { return m_id; }

	facebook::yoga::Node* layoutNode() const { return m_node; }

	const Optional<LayoutResults>& layoutResults() const { return m_layoutResults; }

	facebook::yoga::Style& style();

	const facebook::yoga::Style& style() const;

	void setStyle(const facebook::yoga::Style& style);

public:

	std::shared_ptr<Widget> query(const StringView value);

	Array<std::shared_ptr<Widget>> queryAll(const StringView value, size_t limit = Largest<size_t>);

	void queryAll(const StringView value, Array<std::shared_ptr<Widget>>& result, size_t limit = Largest<size_t>);

	void draw();

	void markLayoutDirty();

	virtual bool allowChildren() const { return true; }

protected:

	void drawChildren() const;

	virtual void drawContent(const LayoutResults&) const;

	virtual void onLayoutNodeAttach(facebook::yoga::Node&) { }

private:

	friend LayoutTree;

	intptr_t m_id;

	facebook::yoga::Node* m_node = nullptr;

	facebook::yoga::Style m_styleCache;

	Optional<LayoutResults> m_layoutResults;

	void attachNode(facebook::yoga::Node& node);

	void detachNode();

public:

	virtual ~Widget() = default;
};
