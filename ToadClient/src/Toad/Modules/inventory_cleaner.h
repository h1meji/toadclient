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

	void UpdateSlotPosOffsets();

private:
	void SetupDropPath();

	POINT get_middle_of_screen() const;

private:
	static inline std::array<POINT, 40> m_slotPosOffset{};

	std::array<std::string, 40> m_inventoryContents{};

	// ends with -1
	std::deque<int> m_indexDropPath{};

	std::mutex m_mutex;

	bool m_isCtrlDown = false;

	std::string extractAfterX(const std::string & s);
};

};