#pragma once
#include <Siv3D.hpp>

struct Thickness
{
	double left = 0;

	double top = 0;

	double right = 0;

	double bottom = 0;
};

struct LayoutResults
{
	Vec2 offset{ 0, 0 };

	Thickness margin;

	RectF localRect;

	Thickness border;

	Thickness padding;

	RectF rect() const noexcept
	{
		return localRect.movedBy(offset);
	}

	RectF outerRect() const noexcept
	{
		return rect().stretched(
			margin.top,
			margin.right,
			margin.bottom,
			margin.left
		);
	}

	RectF rectWithoutBorder() const noexcept
	{
		return rect().stretched(
			-border.top,
			-border.right,
			-border.bottom,
			-border.left
		);
	}

	RectF innerRect() const noexcept
	{
		return rect().stretched(
			-border.top - padding.top,
			-border.right - padding.right,
			-border.bottom - padding.bottom,
			-border.left - padding.left
		);
	}
};
