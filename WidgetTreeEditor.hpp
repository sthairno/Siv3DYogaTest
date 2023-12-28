#pragma once
#include "Widget.hpp"

class WidgetTreeEditor
{
public:

	WidgetTreeEditor(std::shared_ptr<Widget> root)
		: m_root(root) { }

public:

	Color MarginColor{ 176, 131, 84, 180 };
	Color BorderColor{ 228, 196, 130, 180 };
	Color PaddingColor{ 184, 196, 128, 180 };
	Color InnerRectColor{ 136, 178, 189, 180 };

	Color SelectedWidgetFrameColor{ 86, 117, 9, 200 };

	bool update();

	bool isTreeChanged() const { return m_treeChanged; }

private:

	std::shared_ptr<Widget> m_root;

	std::shared_ptr<Widget> m_selectedWidget;

	std::shared_ptr<Widget> m_selectedWidgetParent;

	bool m_treeChanged = false;

	bool mouseOverTest(std::shared_ptr<Widget> widget, std::shared_ptr<Widget>& hoveredWidget, std::shared_ptr<Widget>& hoveredParentWidget);

	void drawLayoutResults(LayoutResults layout);

	void showPropertyEditor(Widget& widget);

	void showSelectedWidgetEditor();
};
