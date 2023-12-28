#pragma once 
#include "Widget.hpp"

class LayoutTree
{
public:

	class Impl;

	LayoutTree();

	LayoutTree(std::shared_ptr<Widget> root);

	void construct(std::shared_ptr<Widget> root);

	void cleanCache();

	void calculateLayout(Size size) { calculateLayout(size.x, size.y); }

	void calculateLayout(float width, float height);

private:

	std::unique_ptr<Impl> m_impl;

	std::shared_ptr<Widget> m_root;

public:

	~LayoutTree();
};
