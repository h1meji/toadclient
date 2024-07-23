#include "pch.h"
#include "Toad/toadll.h"
#include "inventory_cleaner.h"

using namespace toad;

namespace toadll
{
	CInventoryCleaner::CInventoryCleaner()
	{
		Enabled = &inventory_cleaner::enabled;
	}

	void CInventoryCleaner::PreUpdate()
	{
		WaitIsEnabled();
		WaitIsVerified();
	}

	void CInventoryCleaner::Update(const std::shared_ptr<LocalPlayer>& lPlayer)
	{
		if (!*Enabled)
		{
			SLEEP(100);
			return;
		}
	}
	void CInventoryCleaner::OnImGuiRender(ImDrawList* draw)
	{
		if (CVarsUpdater::IsInGui && inventory_cleaner::show_slot_positions)
		{
			UpdateSlotPosOffsets();

			ImVec2 middle = ImGui::GetMainViewport()->Size;
			middle.x /= 2;
			middle.y /= 2;

			// imgui coordinates? 
			middle.x -= 12;
			middle.y -= 40;

			RECT desktop;
			GetWindowRect(GetDesktopWindow(), &desktop);
			int horizontal = (int)desktop.right;
			int vertical = (int)desktop.bottom;

			if (g_screen_height == vertical && g_screen_width == horizontal)
			{
				// imgui coordinates? 

				middle.y += 34;
				middle.x += 9;
			}

			for (int i = 0; i < 40; i++)
			{
				POINT pos = m_slotPosOffset[i];

				pos.x += (int)middle.x;
				pos.y += (int)middle.y;

				//const std::string& name = m_chestContents[i];
				//const char* text = !name.empty() ? name.c_str() : std::to_string(i).c_str();
				draw->AddText({ (float)pos.x, (float)pos.y }, IM_COL32(255, 0, 0, 255), "T");
			}
		}
	}

	void CInventoryCleaner::UpdateSlotPosOffsets()
	{
		int index = 0;
		ChestStealerSlotLocationInfo* info = nullptr;

		if (inventory_cleaner::slot_info.res_x == g_screen_width && inventory_cleaner::slot_info.res_y == g_screen_height)
		{
			info = &inventory_cleaner::slot_info;
			return;
		}

		if (inventory_cleaner::slot_info.res_x == -1 || inventory_cleaner::slot_info.res_y == -1)
		{
			info = &inventory_cleaner::slot_info;
		}

		if (!info)
			return;

		// hotbar
		int hotbar_y = info->begin_y + (info->space_y * 3) + (info->space_y * 0.22) + 1;
		for (int x = info->begin_x, slotx = 0; slotx < 9; x += info->space_x, slotx++)
			m_slotPosOffset[index++] = { x, hotbar_y };

		
		// inv
		for (int y = info->begin_y, sloty = 0; sloty < 3; y += info->space_y, sloty++)
			for (int x = info->begin_x, slotx = 0; slotx < 9; x += info->space_x, slotx++)
				m_slotPosOffset[index++] = { x, y };


		// armor
		int armor_y = info->begin_y - (info->space_y * 0.22) - 1 - info->space_y;
		for (int y = armor_y, sloty = 0; sloty < 4; y -= info->space_y, sloty++)
			m_slotPosOffset[index++] = { info->begin_x, y };
	}

	void CInventoryCleaner::SetupDropPath()
	{
		m_indexDropPath.clear();

		for (int i = 0; i < m_inventoryContents.size(); i++)
		{
			const std::string& name = extractAfterX(m_inventoryContents[i]);
			if (name.empty())
			{
				continue;
			}

			if (!inventory_cleaner::inventory_layout.empty())
			{
				bool found = false;
				for (const auto& item : inventory_cleaner::inventory_layout)
				{
					if (name.find(extractItemName(item)) != std::string::npos)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					m_indexDropPath.emplace_back(i);
				}
			}
		}

		// for now it just randomizes the path
		static auto rng = std::default_random_engine{};
		std::ranges::shuffle(m_indexDropPath, rng);

		m_indexDropPath.emplace_back(-1);
	}

	int CInventoryCleaner::extractSlotNumber(const std::string& s) {
		if (s.empty())
			return -1;
		std::stringstream ss(s);
		std::string slot;
		std::getline(ss, slot, '|');
		return std::stoi(slot);
	}

	std::string CInventoryCleaner::extractItemName(const std::string& s) {
		if (s.empty())
			return "";
		std::size_t pos = s.find('|');
		if (pos != std::string::npos) {
			return s.substr(pos + 1);
		}
		return "";
	}

	std::string CInventoryCleaner::extractAfterX(const std::string& s) {
		if (s.empty())
			return "";
		std::size_t pos = s.find('x');
		if (pos != std::string::npos) {
			return s.substr(pos + 1);
		}
		return "";
	}
}