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

	// WidgetTree
	auto rootWidget = std::make_shared<Widget>();
	{
		// labelWidgetを中央に配置
		rootWidget->style().setJustifyContent(facebook::yoga::Justify::Center);
		rootWidget->style().setAlignItems(facebook::yoga::Align::Center);

		// labelWidgetをrootWidgetに追加
		auto labelWidget = std::make_shared<Label>(U"Siv3DYogaTest", Palette::Black);
		rootWidget->children.emplace_back(std::move(labelWidget));
	}

	// WidgetTreeを編集するエディタ
	WidgetTreeEditor editor{ rootWidget };

	// WidgetTreeからLayoutTreeを構築
	LayoutTree tree{ rootWidget };

	while (System::Update())
	{
		// 表示する領域のRect
		Rect rect = Scene::Rect().stretched(-Padding);
		rect.drawFrame(0, 10, Palette::Lightgray);

		{
			Transformer2D tf{ Mat3x2::Translate(rect.pos), TransformCursor::Yes };

			// レイアウトを計算
			tree.calculateLayout(rect.size);

			// ウィジェットを描画
			rootWidget->draw();

			// WidgetTreeを編集
			if (editor.update())
			{
				// 変更があったらWidgetTreeを再構築
				tree.construct(rootWidget);
			}
		}
	}
}
