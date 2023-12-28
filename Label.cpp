#include "Label.hpp"
#include <yoga/node/Node.h>

static void CalculatePos(Vec2& penPos, Vec2& renderSize, const Glyph& glyph, const Font font, double maxWidth)
{
	if (glyph.codePoint == U'\n')
	{
		renderSize.y = Max(renderSize.y, penPos.y + font.height() + font.height());
		penPos.x = 0;
		penPos.y += font.height();
	}
	else if (glyph.codePoint == U' ' || glyph.codePoint == U'\t')
	{
		renderSize.y = Max(renderSize.y, penPos.y + font.height());
		penPos.x += font.spaceWidth();
	}
	else
	{
		renderSize.x = Max(renderSize.x, penPos.x + glyph.xAdvance);
		renderSize.y = Max(renderSize.y, penPos.y + font.height());
		penPos.x += glyph.xAdvance;
	}

	if (penPos.x > maxWidth)
	{
		penPos.x = 0;
		penPos.y += font.height();
	}
}

Label::Label(const StringView text, ColorF color, Font font)
	: m_text(text)
	, m_color(color)
	, m_font(font)
{
	m_glyphCache = m_font.getGlyphs(m_text);
}

void Label::setText(const StringView text)
{
	m_text = text;
	m_glyphCache = m_font.getGlyphs(m_text);
	markLayoutDirty();
}

void Label::setFont(Font font)
{
	m_font = font;
	m_glyphCache = m_font.getGlyphs(m_text);
	markLayoutDirty();
}

void Label::drawContent(const LayoutResults& layout) const
{
	m_font(m_text).draw(layout.innerRect(), m_color);
}

void Label::onLayoutNodeAttach(facebook::yoga::Node& node)
{
	node.setMeasureFunc([](YGNodeConstRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) -> YGSize
		{ return static_cast<Label*>(Widget::GetInstance(node))->measureCallback(node, width, widthMode, height, heightMode); }
	);
	node.setBaselineFunc([](YGNodeConstRef node, float width, float height) -> float
		{ return static_cast<Label*>(Widget::GetInstance(node))->baselineCallback(node, width, height); }
	);
}

YGSize Label::measureCallback(
	YGNodeConstRef,
	float width,
	YGMeasureMode widthMode,
	float height,
	YGMeasureMode heightMode)
{
	if (widthMode == YGMeasureModeExactly && heightMode == YGMeasureModeExactly)
	{
		return { width, height };
	}

	float maxWidth = widthMode == YGMeasureModeUndefined ? Math::InfF : width;

	Vec2 penPos{ 0, 0 };
	Vec2 renderSize{ 0, 0 };
	for (auto& glyph : m_glyphCache)
	{
		CalculatePos(penPos, renderSize, glyph, m_font, maxWidth);
	}

	float measuredWidth = 0, measuredHeight = 0;

	switch (widthMode)
	{
	case YGMeasureModeUndefined: measuredWidth = static_cast<float>(renderSize.x); break;
	case YGMeasureModeExactly: measuredWidth = width; break;
	case YGMeasureModeAtMost: measuredWidth = Math::Min(width, static_cast<float>(renderSize.x)); break;
	}

	switch (heightMode)
	{
	case YGMeasureModeUndefined: measuredHeight = static_cast<float>(renderSize.y); break;
	case YGMeasureModeExactly: measuredHeight = height; break;
	case YGMeasureModeAtMost: measuredHeight = Math::Min(height, static_cast<float>(renderSize.y)); break;
	}

	return { measuredWidth, measuredHeight };
}

float Label::baselineCallback(YGNodeConstRef, float width, float height)
{
	return m_text.empty() ? 0.0f : m_font.ascender();
}
