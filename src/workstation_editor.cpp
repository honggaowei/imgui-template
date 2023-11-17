#define IMGUI_DEFINE_MATH_OPERATORS
#include "workstation_editor.h"
#include "imnodes.h"
#include "imgui.h"

#include <algorithm>
#include <vector>
#include <iostream>
#include <string>
#include <map>


namespace dms_editor

{
	namespace
	{


		struct Node
		{
			int   id;
			float value;
			std::string title;
			Node() = default;
			Node(int id, float value, const std::string& title)
				: id(id), value(value), title(title)
			{
			}
		};

		struct Link
		{
			int id;
			int start_attr, end_attr;
		};

		struct Editor
		{
			ImNodesEditorContext* context = nullptr;
			std::vector<Node>     nodes;
			std::vector<Link>     links;
			int                   current_id = 0;
		};

		struct TextFilters
		{
			// Modify character input by altering 'data->Eventchar' (ImGuiInputTextFlags_CallbackCharFilter callback)
			static int FilterCasingSwap(ImGuiInputTextCallbackData* data)
			{
				//if (data->EventChar >= 'a' && data->EventChar <= 'z') { data->EventChar -= 'a' - 'A'; } // Lowercase becomes uppercase
				if (data->EventChar >= 'A' && data->EventChar <= 'Z') { data->EventChar += 'a' - 'A'; } // Uppercase becomes lowercase
				return 0;
			}

			// Return 0 (pass) if the character is 'i' or 'm' or 'g' or 'u' or 'i', otherwise return 1 (filter out)
			static int FilterImGuiLetters(ImGuiInputTextCallbackData* data)
			{
				if (data->EventChar < 256 && strchr("imgui", (char)data->EventChar))
					return 0;
				return 1;
			}
		};

		//展示编辑器
		void show_editor(const char* editor_name, bool* is_open, Editor& editor)
		{
			int *select_ids = new int(editor.nodes.size());
			ImNodes::EditorContextSet(editor.context);
			ImGui::Begin(editor_name, is_open);
			ImNodes::BeginNodeEditor();
			ImNodes::MiniMap();

			if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
				ImNodes::IsEditorHovered() && ImGui::IsKeyDown(ImGuiKey_Delete))
			{
				ImNodes::GetSelectedNodes(select_ids);
			
				for (int i = 0 ; i < sizeof(select_ids); i++)
				{
					auto it = std::find_if(editor.nodes.begin(), editor.nodes.end(), [select_ids,i](const Node& node) -> bool {return node.id == select_ids[i]; });
					if (it != editor.nodes.end())
					{
						editor.nodes.erase(it);
					}
				}
			}

			if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
				ImNodes::IsEditorHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup(u8"add_new_node");
			}

			if (ImGui::BeginPopup(u8"add_new_node"))
			{
				ImGui::SeparatorText(u8"新增节点");
				ImGui::Spacing();
				if (ImGui::Selectable("http"))
				{
					ImGui::Spacing();
					const int node_id = ++editor.current_id;
					ImNodes::SetNodeScreenSpacePos(node_id, ImGui::GetMousePos());
					Node node = Node(node_id, 0.f, "http");
					editor.nodes.push_back(node);
				};
				if (ImGui::Selectable("tcp"))
				{
					ImGui::Spacing();
					const int node_id = ++editor.current_id;
					ImNodes::SetNodeScreenSpacePos(node_id, ImGui::GetMousePos());
					Node node = Node(node_id, 0.f, "tcp");
					editor.nodes.push_back(node);
				};
				if (ImGui::Selectable("serial"))
				{
					ImGui::Spacing();
					const int node_id = ++editor.current_id;
					ImNodes::SetNodeScreenSpacePos(node_id, ImGui::GetMousePos());
					Node node = Node(node_id, 0.f, "serial");
					editor.nodes.push_back(node);
				};
				ImGui::EndPopup();
			};

			for (Node& node : editor.nodes)
			{
				ImNodes::BeginNode(node.id);
				ImNodes::BeginNodeTitleBar();
				ImGui::Text(u8"node name");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(200);
				//ImGui::InputText("", node.title.c_str(), node.title.size(), ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterCasingSwap);
				ImGui::TextUnformatted(node.title.c_str());
				ImNodes::EndNodeTitleBar();

				ImNodes::BeginInputAttribute(node.id << 8);
				ImGui::TextUnformatted("input");
				ImNodes::EndInputAttribute();

				ImNodes::BeginStaticAttribute(node.id << 16);
				ImGui::PushItemWidth(120.0f);
				ImGui::DragFloat("value", &node.value, 0.01f);
				ImGui::PopItemWidth();
				ImNodes::EndStaticAttribute();

				ImNodes::BeginOutputAttribute(node.id << 24);
				const float text_width = ImGui::CalcTextSize("output").x;
				ImGui::Indent(120.f + ImGui::CalcTextSize("value").x - text_width);
				ImGui::TextUnformatted("output");
				ImNodes::EndOutputAttribute();

				ImNodes::EndNode();
			}

			for (const Link& link : editor.links)
			{
				ImNodes::Link(link.id, link.start_attr, link.end_attr);
			}

			ImNodes::EndNodeEditor();

			{
				Link link;
				if (ImNodes::IsLinkCreated(&link.start_attr, &link.end_attr))
				{
					link.id = ++editor.current_id;
					editor.links.push_back(link);
				}
			}

			{
				int link_id;
				if (ImNodes::IsLinkDestroyed(&link_id))
				{
					auto iter = std::find_if(
						editor.links.begin(), editor.links.end(), [link_id](const Link& link) -> bool {
							return link.id == link_id;
						});
					assert(iter != editor.links.end());
					editor.links.erase(iter);
				}
			}

			ImGui::End();
		}
		static Editor editor1;

	}

	void NodeEditorInitialize()
	{
		editor1.context = ImNodes::EditorContextCreate();
		ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);

		ImNodesIO& io = ImNodes::GetIO();
		io.LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;
		io.MultipleSelectModifier.Modifier = &ImGui::GetIO().KeyCtrl;

		ImNodesStyle& style = ImNodes::GetStyle();
		//style.Flags |= ImNodesStyleFlags_GridLinesPrimary | ImNodesStyleFlags_GridSnapping;
	}

	void NodeEditorShow(const char* name, bool* p_open)
	{
		show_editor(name, p_open, editor1);
	}

	void NodeEditorShutdown()
	{
		ImNodes::PopAttributeFlag();
		ImNodes::EditorContextFree(editor1.context);
	}


}