#pragma once

namespace toadll
{

class CInventoryCleaner SET_MODULE_CLASS(CInventoryCleaner)
{
public:
	CInventoryCleaner();

public:
	void PreUpdate() override;
	void Update(const std::shared_ptr<LocalPlayer>&lPlayer) override;
	void OnImGuiRender(ImDrawList * draw) override;
};

};