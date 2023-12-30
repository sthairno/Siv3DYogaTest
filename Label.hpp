#pragma once
#include <Siv3D.hpp>
#include "Widget.hpp"

class Label : public Widget
{
	struct LayoutCache;

public:

	Label();

public:

	const String& text() const { return m_text; }

	void setText(const StringView text);

	Font font() const { return m_font; }

	void setFont(Font font);

	ColorF color() const { return m_color; }

	void setColor(ColorF color) { m_color = color; }

private:

	String m_text = U"";

	Font m_font = SimpleGUI::GetFont();

	ColorF m_color = Palette::White;

	Array<Glyph> m_glyphCache;

	void drawContent(const LayoutResults& layout) const override;

	void onLayoutNodeAttach(facebook::yoga::Node& node) override;

	YGSize measureCallback(YGNodeConstRef, float, YGMeasureMode, float, YGMeasureMode);

	float baselineCallback(YGNodeConstRef, float, float);

	bool allowChildren() const override { return false; }
};
