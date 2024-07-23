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
	void SetupMovePath();

	POINT get_middle_of_screen() const;

private:
	static inline std::array<POINT, 40> m_slotPosOffset{};

	std::array<std::string, 40> m_inventoryContents{};

	// ends with -1
	std::deque<int> m_indexDropPath{};
	std::deque<std::array<int, 2>> m_indexMovePath{};

	std::mutex m_mutex;

	bool m_isCtrlDown = false;

	static inline void right_mouse_down(const POINT & pt);
	static inline void right_mouse_up(const POINT & pt);
};

};