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

		LOGDEBUG("[inventory cleaner] enabled");
	}
	void CInventoryCleaner::OnImGuiRender(ImDrawList* draw)
	{
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
}