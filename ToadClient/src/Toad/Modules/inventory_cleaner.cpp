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
}