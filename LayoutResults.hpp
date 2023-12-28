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
	Thickness margin;

	RectF localRect;

	Thickness border;

	Thickness padding;

	RectF outerRect() const noexcept
	{
		return localRect.stretched(
			margin.top,
			margin.right,
			margin.bottom,
			margin.left
		);
	}

	RectF rectWithoutBorder() const noexcept
	{
		return localRect.stretched(
			-border.top,
			-border.right,
			-border.bottom,
			-border.left
		);
	}

	RectF innerRect() const noexcept
	{
		return localRect.stretched(
			-border.top - padding.top,
			-border.right - padding.right,
			-border.bottom - padding.bottom,
			-border.left - padding.left
		);
	}
};
