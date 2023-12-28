#include <Siv3D.hpp> // Siv3D v0.6.13

#include "imgui_impl_s3d/DearImGuiAddon.hpp"

#include "Widget.hpp"
#include "LayoutTree.hpp"
#include "WidgetTreeEditor.hpp"

#include "Label.hpp"

void Main()
{
	Addon::Register<DearImGuiAddon>(U"ImGui");
	Scene::SetBackground(Palette::White);
	Window::SetStyle(WindowStyle::Sizable);

	constexpr int32 Padding = 50;

	std::shared_ptr<Widget> rootWidget = std::make_shared<Widget>();

	WidgetTreeEditor editor(rootWidget);

	LayoutTree tree;
	tree.construct(rootWidget);

	while (System::Update())
	{
		Rect rect = Scene::Rect().stretched(-Padding);

		rect.drawFrame(0, 10, Palette::Lightgray);

		{
			Transformer2D tf{ Mat3x2::Translate(rect.pos), TransformCursor::Yes };

			tree.calculateLayout(rect.size);

			rootWidget->draw();

			if (editor.update())
			{
				tree.construct(rootWidget);
			}
		}
	}
}
