#pragma once

namespace components
{
	class flashlight final : public component
	{
	public:
		flashlight();
		const char* get_name() override { return "flashlight"; };

		static void frame();

		static inline bool enabled = false;
	};
}
