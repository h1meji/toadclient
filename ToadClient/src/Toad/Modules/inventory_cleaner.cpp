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

		static bool cleaning = false;
		static int index = -1;
		static int step_counter = 0;
		static bool stopped = false;
		static bool dropped = false;

		if (stopped)
		{
			if (m_isCtrlDown)
			{
				send_key(VK_CONTROL, false);
				m_isCtrlDown = false;
			}

			if (!GetAsyncKeyState(inventory_cleaner::key))
				stopped = false;

			SLEEP(100);
			return;
		}

		static Timer aim_timer;
		static POINT current_pos{};

		if ((GetAsyncKeyState(inventory_cleaner::key) & 0x8000 && CVarsUpdater::IsInGui) || cleaning)
		{
			UpdateSlotPosOffsets();

			if (!cleaning)
			{
				{
					std::lock_guard lock(m_mutex);

					std::shared_ptr<c_Entity> localPlayer = MC->getLocalPlayer();
					m_inventoryContents = localPlayer->getInventory();

					if (m_inventoryContents.empty())
					{
						LOGDEBUG("[inventory cleaner] empty inventory");
						SLEEP(100);
						return;
					}

				}

				// setup mouse path
				SetupDropPath();
				LOGDEBUG("[inventory cleaner] dropping {} items", m_indexDropPath.size() - 2);

				LOGDEBUG("inventory size: {}", m_inventoryContents.size());
				LOGDEBUG("drop path size: {}", m_indexDropPath.size());

				step_counter = 0;

				// get the first index from the path
				index = m_indexDropPath.front();
				m_indexDropPath.pop_front();
				step_counter++;

				// start holding shift and wait a bit
				send_key(VK_CONTROL);
				m_isCtrlDown = true;
				SLEEP(rand_int(30, 50));

				// start timer and set starting mouse position
				aim_timer.Start();
				GetCursorPos(&current_pos);
			}
		
			if (!dropped && index != -1)
			{
				if (step_counter > 1 && ((GetAsyncKeyState(inventory_cleaner::key) & 0x8000) || GetAsyncKeyState(0x45 /*E*/) & 0x8000 || GetAsyncKeyState(VK_ESCAPE) & 0x8000))
				{
					step_counter = 0;
					index = -1;
					cleaning = false;
					dropped = false;

					stopped = true;

					return;
				}

				const std::string& name = m_inventoryContents[index];
				POINT pos = m_slotPosOffset[index];
				const POINT middle = get_middle_of_screen();

				pos.x += middle.x;
				pos.y += middle.y;

				if (aim_timer.Elapsed() > inventory_cleaner::delay)
				{
					// make sure mouse is on slot and right click
					SetCursorPos(pos.x, pos.y);
					SLEEP(rand_int(50, 60));

					/*right_mouse_down(pos);
					SLEEP(rand_int(35, 70));
					right_mouse_up(pos);*/

					// send "Q"
					send_key(0x51);
					SLEEP(rand_int(35, 70));
					send_key(0x51, false);

					current_pos = pos;

					// get next in the path
					index = m_indexDropPath.front();
					m_indexDropPath.pop_front();
					step_counter++;

					// restart timer
					aim_timer.Start();
				}
				else
				{
					float t = std::clamp(aim_timer.Elapsed() / inventory_cleaner::delay, 0.f, 1.f);
					int x_lerp = (int)slerp((float)current_pos.x, (float)pos.x, t);
					int y_lerp = (int)slerp((float)current_pos.y, (float)pos.y, t);
					SetCursorPos(x_lerp, y_lerp);
				}

				if (index == -1)
					dropped = true;
			}
			else if (dropped && index != -1)
			{
				LOGDEBUG("MOVING ITEMS!");
			}
			else
			{
				if (m_isCtrlDown)
				{
					send_key(VK_CONTROL, false);
					m_isCtrlDown = false;
				}

				cleaning = false;
				dropped = false;
			}

			//LOGDEBUG("looting = true");
			// do it here so we don't check toggle key press at same time for disabling
			cleaning = true;
		}
		else
		{
			if (m_isCtrlDown)
			{
				send_key(VK_CONTROL, false);
				m_isCtrlDown = false;
			}

			cleaning = false;
			dropped = false;
		}

		if (index == -1)
		{
			if (m_isCtrlDown)
			{
				send_key(VK_CONTROL, false);
				m_isCtrlDown = false;
			}

			cleaning = false;
			dropped = false;
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

		//// for now it just randomizes the path
		//static auto rng = std::default_random_engine{};
		//std::ranges::shuffle(m_indexDropPath, rng);

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

	POINT CInventoryCleaner::get_middle_of_screen() const
	{
		RECT window_rect{};
		if (!GetWindowRect(g_hWnd, &window_rect))
		{
			return { -1, -1 };
		}

		POINT pt{};
		pt.x = window_rect.left + g_screen_width / 2;
		pt.y = window_rect.top + g_screen_height / 2;
		return pt;
	}
}