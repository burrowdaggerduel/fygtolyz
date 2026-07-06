#include "std_include.hpp"

namespace components
{
	namespace
	{
		game::dvar_s* flashlight_color = nullptr;
		game::dvar_s* flashlight_intensity = nullptr;
		game::dvar_s* flashlight_radius = nullptr;
		game::dvar_s* flashlight_fov_inner = nullptr;
		game::dvar_s* flashlight_fov_outer = nullptr;
		game::dvar_s* flashlight_offset = nullptr;
	}

	// called from within CG_CalcViewValues (see radiant_livelink::CG_CalcViewValues_stub)
	// the scene is being populated at this point, so lights added here get culled and rendered within the current frame
	void flashlight::frame()
	{
		if (!flashlight::enabled)
		{
			return;
		}

		if (!game::clientUI || game::clientUI->connectionState != game::CA_ACTIVE)
		{
			return;
		}

		const auto scene = game::scene;
		if (!scene || scene->addedLightCount >= 32)
		{
			return;
		}

		// default dynamic-light attenuation def - a spot light is not rendered without one
		const auto def = game::rgp->dlightDef;
		if (!def)
		{
			return;
		}

		const auto ps = &game::cgs->predictedPlayerState;

		game::vec3_t fwd, rt, up;
		utils::vector::angle_vectors(ps->viewangles, fwd, rt, up);

		auto& light = scene->addedLight[scene->addedLightCount];
		std::memset(&light, 0, sizeof(game::GfxLight));

		light.type = 2; // GFX_LIGHT_TYPE_SPOT
		light.canUseShadowMap = 1;
		light.exponent = 1;
		light.def = def;

		const float intensity = flashlight_intensity->current.value;
		light.color[0] = flashlight_color->current.vector[0] * intensity;
		light.color[1] = flashlight_color->current.vector[1] * intensity;
		light.color[2] = flashlight_color->current.vector[2] * intensity;

		light.radius = flashlight_radius->current.value;

		float inner = flashlight_fov_inner->current.value;
		const float outer = flashlight_fov_outer->current.value;

		if (inner > outer)
		{
			inner = outer;
		}

		light.cosHalfFovInner = cosf(utils::vector::deg_to_rad(inner * 0.5f));
		light.cosHalfFovOuter = cosf(utils::vector::deg_to_rad(outer * 0.5f));

		// eye position + offset (forward / right / up) so the light sits "on the forehead"
		const auto* offset = flashlight_offset->current.vector;
		for (auto i = 0; i < 3; i++)
		{
			light.origin[i] = ps->origin[i] + fwd[i] * offset[0] + rt[i] * offset[1] + up[i] * offset[2];
			light.dir[i] = fwd[i];
		}

		light.origin[2] += ps->viewHeightCurrent;

		scene->addedLightCount++;
	}

	flashlight::flashlight()
	{
		flashlight_color = game::Dvar_RegisterVec3(
			/* name		*/ "flashlight_color",
			/* desc		*/ "color of the head-mounted flashlight (warm white by default)",
			/* x		*/ 1.0f,
			/* y		*/ 0.92f,
			/* z		*/ 0.78f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved);

		flashlight_intensity = game::Dvar_RegisterFloat(
			/* name		*/ "flashlight_intensity",
			/* desc		*/ "brightness (color multiplier) of the flashlight",
			/* default	*/ 3.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 100.0f,
			/* flags	*/ game::dvar_flags::saved);

		flashlight_radius = game::Dvar_RegisterFloat(
			/* name		*/ "flashlight_radius",
			/* desc		*/ "range of the flashlight in game units (~40 units = 1m)",
			/* default	*/ 2000.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 10000.0f,
			/* flags	*/ game::dvar_flags::saved);

		flashlight_fov_inner = game::Dvar_RegisterFloat(
			/* name		*/ "flashlight_fov_inner",
			/* desc		*/ "full angle of the bright beam core (hotspot) in degrees",
			/* default	*/ 24.0f,
			/* minVal	*/ 1.0f,
			/* maxVal	*/ 160.0f,
			/* flags	*/ game::dvar_flags::saved);

		flashlight_fov_outer = game::Dvar_RegisterFloat(
			/* name		*/ "flashlight_fov_outer",
			/* desc		*/ "full angle of the outer light cone (spill) in degrees",
			/* default	*/ 70.0f,
			/* minVal	*/ 1.0f,
			/* maxVal	*/ 160.0f,
			/* flags	*/ game::dvar_flags::saved);

		flashlight_offset = game::Dvar_RegisterVec3(
			/* name		*/ "flashlight_offset",
			/* desc		*/ "offset from the eye position (forward, right, up)",
			/* x		*/ 6.0f,
			/* y		*/ 0.0f,
			/* z		*/ 4.0f,
			/* minVal	*/ -100.0f,
			/* maxVal	*/ 100.0f,
			/* flags	*/ game::dvar_flags::saved);

		command::add("flashlighthead", "<0/1>", "toggle the head-mounted flashlight (no arg = toggle)", [](command::params params)
		{
			flashlight::enabled = params.length() > 1
				? atoi(params[1]) != 0
				: !flashlight::enabled;

			game::Com_PrintMessage(0, utils::va("flashlight %s\n", flashlight::enabled ? "^2on" : "^1off"), 0);
		});
	}
}
