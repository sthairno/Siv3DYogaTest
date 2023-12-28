#include "WidgetTreeEditor.hpp"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <yoga/node/Node.h>
#include <yoga/enums/FlexDirection.h>
#include "Label.hpp"

using namespace facebook;

static std::optional<yoga::FloatOptional> CreateFloatOptionalInput(const char* label, const char* hint, yoga::FloatOptional value)
{
	std::string str = value.isDefined() ? fmt::format("{:.2f}", value.unwrap()) : "";
	if (ImGui::InputTextWithHint(
		label, hint, &str,
		ImGuiInputTextFlags_CharsDecimal |
		ImGuiInputTextFlags_EnterReturnsTrue))
	{
		if (str.empty())
		{
			return yoga::FloatOptional{ };
		}
		try
		{
			return yoga::FloatOptional{ std::stof(str) };
		}
		catch (std::invalid_argument)
		{ }
	}
	return none;
}

template<class Type>
static std::optional<Type> CreateEnumCombo(const char* label, Type value)
{
	using int_type = std::underlying_type_t<Type>;

	std::optional<Type> result;

	if (ImGui::BeginCombo(label, yoga::toString(value)))
	{
		for (int_type intChoice = 0; intChoice < yoga::ordinalCount<Type>(); intChoice++)
		{
			Type choice = static_cast<Type>(intChoice);
			bool selected = value == choice;

			if (ImGui::Selectable(yoga::toString(choice), selected))
			{
				result = choice;
			}
		}

		ImGui::EndCombo();
	}

	return result;
}

static std::string StyleLengthToString(yoga::StyleLength value)
{
	switch (value.unit())
	{
	case yoga::Unit::Undefined: return "";
	case yoga::Unit::Auto: return "auto";
	case yoga::Unit::Point: return fmt::format("{:.2f}", value.value().unwrap());
	case yoga::Unit::Percent: return fmt::format("{:.2f}%", value.value().unwrap() * 100);
	}
	return "";
}


static std::optional<yoga::StyleLength> TryParseStyleLength(std::string_view input)
{
	constexpr auto trimCharList = " \t\v\r\n";

	// 空白を取り除く(trim関数の代わり)
	{
		std::string::size_type left = input.find_first_not_of(trimCharList);
		if (left != std::string::npos)
		{
			std::string::size_type right = input.find_last_not_of(trimCharList);
			input = input.substr(left, right - left + 1);
		}
	}

	// Undefined
	if (input == "" || input == "undefined")
	{
		return yoga::StyleLength::undefined();
	}

	// Auto
	if (input == "auto")
	{
		return yoga::StyleLength::ofAuto();
	}

	// Percent
	if (input.ends_with('%'))
	{
		try
		{
			return yoga::StyleLength::percent(
				std::stof(std::string{ input.substr(0, input.size() - 1) }) / 100.f
			);
		}
		catch (std::invalid_argument)
		{
			return none;
		}
	}

	// Point
	try
	{
		return yoga::StyleLength::points(
			std::stof(std::string{ input })
		);
	}
	catch (std::invalid_argument)
	{
		return none;
	}
}

static std::optional<yoga::StyleLength> CreateStyleLengthInput(
	const char* label,
	const char* hint,
	yoga::StyleLength value)
{
	std::string str = StyleLengthToString(value);
	if (ImGui::InputTextWithHint(
		label, hint, &str,
		ImGuiInputTextFlags_CharsNoBlank |
		ImGuiInputTextFlags_EnterReturnsTrue))
	{
		return TryParseStyleLength(str);
	}
	return none;
}

static bool CreateEdgeInput(
	const char* id,
	const char* label,
	yoga::Style& style,
	yoga::StyleLength(yoga::Style::* getter)(yoga::Edge) const,
	void (yoga::Style::* setter)(yoga::Edge, yoga::StyleLength))
{
	bool valueChanged = false;

	if (ImGui::BeginTable(id, 3))
	{
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (auto v = CreateStyleLengthInput("##top", "", (style.*getter)(yoga::Edge::Top)))
		{
			(style.*setter)(yoga::Edge::Top, *v);
			valueChanged = true;
		}

		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (auto v = CreateStyleLengthInput("##left", "", (style.*getter)(yoga::Edge::Left)))
		{
			(style.*setter)(yoga::Edge::Left, *v);
			valueChanged = true;
		}

		ImGui::TableSetColumnIndex(1);
		ImGui::Text(label);

		ImGui::TableSetColumnIndex(2);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (auto v = CreateStyleLengthInput("##right", "", (style.*getter)(yoga::Edge::Right)))
		{
			(style.*setter)(yoga::Edge::Right, *v);
			valueChanged = true;
		}

		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (auto v = CreateStyleLengthInput("##bottom", "", (style.*getter)(yoga::Edge::Bottom)))
		{
			(style.*setter)(yoga::Edge::Bottom, *v);
			valueChanged = true;
		}

		ImGui::EndTable();
	}

	return valueChanged;
}

static bool ShowStyleEditor(yoga::Style& style)
{
	bool valueChanged = false;

	if (ImGui::BeginTabBar("type"))
	{
		if (ImGui::BeginTabItem("Flex"))
		{
			if (auto v = CreateEnumCombo("Direction", style.direction()))
			{
				style.setDirection(*v);
				valueChanged = true;
			}

			if (auto v = CreateEnumCombo("Flex Direction", style.flexDirection()))
			{
				style.setFlexDirection(*v);
				valueChanged = true;
			}

			if (auto v = CreateStyleLengthInput("Basis", "auto", style.flexBasis()))
			{
				style.setFlexBasis(*v);
				valueChanged = true;
			}

			if (auto v = CreateFloatOptionalInput("Grow", "0", style.flexGrow()))
			{
				style.setFlexGrow(*v);
				valueChanged = true;
			}

			if (auto v = CreateFloatOptionalInput("Shrink", "1", style.flexShrink()))
			{
				style.setFlexShrink(*v);
				valueChanged = true;
			}

			if (auto v = CreateEnumCombo("Flex Wrap", style.flexWrap()))
			{
				style.setFlexWrap(*v);
				valueChanged = true;
			}

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Alignment"))
		{
			if (auto v = CreateEnumCombo("Justify Content", style.justifyContent()))
			{
				style.setJustifyContent(*v);
				valueChanged = true;
			}

			if (auto v = CreateEnumCombo("Align Items", style.alignItems()))
			{
				style.setAlignItems(*v);
				valueChanged = true;
			}

			if (auto v = CreateEnumCombo("Align Self", style.alignSelf()))
			{
				style.setAlignSelf(*v);
				valueChanged = true;
			}

			if (auto v = CreateEnumCombo("Align Content", style.alignContent()))
			{
				style.setAlignContent(*v);
				valueChanged = true;
			}

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Layout"))
		{
			ImGui::Text("Width x Height");
			if (ImGui::BeginTable("wh", 2))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::SetNextItemWidth(-FLT_MIN);
				if (auto v = CreateStyleLengthInput("#w", "auto", style.dimension(yoga::Dimension::Width)))
				{
					style.setDimension(yoga::Dimension::Width, *v);
					valueChanged = true;
				}

				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(-FLT_MIN);
				if (auto v = CreateStyleLengthInput("#h", "auto", style.dimension(yoga::Dimension::Height)))
				{
					style.setDimension(yoga::Dimension::Height, *v);
					valueChanged = true;
				}

				ImGui::EndTable();
			}

			ImGui::Text("Max-Width x Max-Height");
			if (ImGui::BeginTable("maxwh", 2))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::SetNextItemWidth(-FLT_MIN);
				if (auto v = CreateStyleLengthInput("#w", "none", style.maxDimension(yoga::Dimension::Width)))
				{
					style.setMaxDimension(yoga::Dimension::Width, *v);
					valueChanged = true;
				}

				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(-FLT_MIN);
				if (auto v = CreateStyleLengthInput("#h", "none", style.maxDimension(yoga::Dimension::Height)))
				{
					style.setMaxDimension(yoga::Dimension::Height, *v);
					valueChanged = true;
				}

				ImGui::EndTable();
			}

			ImGui::Text("Min-Width x Min-Height");
			if (ImGui::BeginTable("minwh", 2))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::SetNextItemWidth(-FLT_MIN);
				if (auto v = CreateStyleLengthInput("#w", "0", style.minDimension(yoga::Dimension::Width)))
				{
					style.setMinDimension(yoga::Dimension::Width, *v);
					valueChanged = true;
				}

				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(-FLT_MIN);
				if (auto v = CreateStyleLengthInput("#h", "0", style.minDimension(yoga::Dimension::Height)))
				{
					style.setMinDimension(yoga::Dimension::Height, *v);
					valueChanged = true;
				}

				ImGui::EndTable();
			}

			ImGui::Separator();

			if (auto v = CreateFloatOptionalInput("Aspect Ratio", "auto", style.aspectRatio()))
			{
				style.setAspectRatio(*v);
				valueChanged = true;
			}

			ImGui::Separator();

			valueChanged |= CreateEdgeInput("padding", "Padding", style, &yoga::Style::padding, &yoga::Style::setPadding);

			valueChanged |= CreateEdgeInput("border", "Border", style, &yoga::Style::border, &yoga::Style::setBorder);

			valueChanged |= CreateEdgeInput("margin", "Margin", style, &yoga::Style::margin, &yoga::Style::setMargin);

			ImGui::Separator();

			if (auto v = CreateEnumCombo("Position Type", style.positionType()))
			{
				style.setPositionType(*v);
				valueChanged = true;
			}

			CreateEdgeInput("position", "Position", style, &yoga::Style::position, &yoga::Style::setPosition);

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	return valueChanged;
}

bool RenderWidgetTreeNode(std::shared_ptr<Widget> widget, std::shared_ptr<Widget>& selectedWidget)
{
	bool treeChanged = false;
	auto id = widget->id();
	bool isSelected = widget == selectedWidget;
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	if (isSelected)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	bool showChildren = ImGui::TreeNodeEx(reinterpret_cast<void*>(id), flags, "0x%08x", id);

	if (not showChildren)
	{
		ImGui::TreePush();
	}

	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		selectedWidget = widget;
		treeChanged = true;
	}

	if (showChildren)
	{
		for (auto& child : widget->children)
		{
			treeChanged |= RenderWidgetTreeNode(child, selectedWidget);
		}
	}

	ImGui::TreePop();

	return treeChanged;
}

bool WidgetTreeEditor::update()
{
	// 選択中のウィジェットを編集
	showSelectedWidgetEditor();

	// mouseOver中のウィジェットを検索
	std::shared_ptr<Widget> hoveredWidget, hoveredParentWidget;
	if (!ImGui::GetIO().WantCaptureMouse)
	{
		mouseOverTest(m_root, hoveredWidget, hoveredParentWidget);
	}

	// m_selectedWidgetを切り替え
	if (hoveredWidget && MouseL.down())
	{
		if (m_selectedWidget == hoveredWidget)
		{
			m_selectedWidget.reset();
			m_selectedWidgetParent.reset();
		}
		else
		{
			m_selectedWidget = hoveredWidget;
			m_selectedWidgetParent = hoveredParentWidget;
		}
	}

	// -----描画-----

	// LayoutResultsを描画
	if (hoveredWidget)
	{
		drawLayoutResults(*hoveredWidget->layoutResults());
	}
	else if (m_selectedWidget)
	{
		drawLayoutResults(*m_selectedWidget->layoutResults());
	}


	// 選択中のウィジェットを表示
	if (m_selectedWidget)
	{
		if (auto layout = m_selectedWidget->layoutResults())
		{
			layout->rect().drawFrame(0.5, 1.5, SelectedWidgetFrameColor);
		}
	}

	return m_treeChanged;
}

bool WidgetTreeEditor::mouseOverTest(
	std::shared_ptr<Widget> widget,
	std::shared_ptr<Widget>& hoveredWidget,
	std::shared_ptr<Widget>& hoveredParentWidget)
{
	auto layout = widget->layoutResults();

	if (!layout.has_value())
	{
		return false;
	}

	for (auto [i, child] : ReverseIndexedRef(widget->children))
	{
		if (mouseOverTest(child, hoveredWidget, hoveredParentWidget))
		{
			if (!hoveredParentWidget)
			{
				hoveredParentWidget = widget;
			}
			return true;
		}
	}

	if (layout->rect().mouseOver())
	{
		hoveredWidget = widget;
		return true;
	}

	return false;
}

void WidgetTreeEditor::drawLayoutResults(LayoutResults layout)
{
	for (auto polygon : Geometry2D::Subtract(layout.outerRect(), layout.rect().asPolygon()))
	{
		polygon.draw(MarginColor);
	}

	for (auto polygon : Geometry2D::Subtract(layout.rect(), layout.rectWithoutBorder().asPolygon()))
	{
		polygon.draw(BorderColor);
	}

	for (auto polygon : Geometry2D::Subtract(layout.rectWithoutBorder(), layout.innerRect().asPolygon()))
	{
		polygon.draw(PaddingColor);
	}

	layout.innerRect().draw(InnerRectColor);
}

void WidgetTreeEditor::showPropertyEditor(Widget& widget)
{
	if (auto label = dynamic_cast<Label*>(&widget))
	{
		std::string buffer = Unicode::ToUTF8(label->text());
		if (ImGui::InputTextMultiline("Text", &buffer))
		{
			label->setText(Unicode::FromUTF8(buffer));
		}

		Float4 color = label->color().toFloat4();
		if (ImGui::ColorEdit4("TextColor", color.getPointer()))
		{
			label->setColor(ColorF{ color });
		}
	}
}

void WidgetTreeEditor::showSelectedWidgetEditor()
{
	bool isItemSelected = true;

	if (m_selectedWidget && ImGui::Begin("Selected Widget", &isItemSelected, ImGuiWindowFlags_NoCollapse))
	{
		ImGui::Text("ID: 0x%08x (%s)", m_selectedWidget->id(), typeid(*m_selectedWidget.get()).name());

		if (ImGui::CollapsingHeader("Tree"))
		{
			ImGui::BeginDisabled(!m_selectedWidget->allowChildren());
			if (ImGui::Button("[+] Add Child Widget"))
			{
				auto newChild = std::make_shared<Widget>();
				{
					newChild->style().setDimension(yoga::Dimension::Width, yoga::Style::Length::points(100));
					newChild->style().setDimension(yoga::Dimension::Height, yoga::Style::Length::points(100));
				}
				m_selectedWidget->children.emplace_back(std::move(newChild));
				m_treeChanged = true;
			}
			if (ImGui::Button("[+] Add Child Label"))
			{
				m_selectedWidget->children.emplace_back(std::make_shared<Label>(U"Hoge", Palette::Black));
				m_treeChanged = true;
			}
			ImGui::EndDisabled();

			ImGui::Spacing();

			ImGui::BeginDisabled(!m_selectedWidgetParent);
			if (ImGui::Button("[-] Remove Widget") || (m_selectedWidgetParent && KeyDelete.down()))
			{
				m_selectedWidgetParent->children.remove(m_selectedWidget);
				isItemSelected = false;
				m_treeChanged = true;
			}
			ImGui::EndDisabled();
		}

		if (ImGui::CollapsingHeader("Property"))
		{
			showPropertyEditor(*m_selectedWidget);
		}

		auto node = m_selectedWidget->layoutNode();

		if (node)
		{
			if (ImGui::CollapsingHeader("Style"))
			{
				if (ShowStyleEditor(node->getStyle()))
				{
					node->markDirtyAndPropagate();
				}
			}
		}

		ImGui::End();
	}

	if (!isItemSelected)
	{
		m_selectedWidget.reset();
		m_selectedWidgetParent.reset();
	}
}
