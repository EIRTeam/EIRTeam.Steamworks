/**************************************************************************/
/*  input_glyphs_steamworks.cpp                                           */
/**************************************************************************/
/*                         This file is part of:                          */
/*                           EIRTeam.Steamworks                           */
/*                         https://ph.eirteam.moe                         */
/**************************************************************************/
/* Copyright (c) 2023-present Álex Román (EIRTeam) & contributors.        */
/*                                                                        */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "input_glyphs_steamworks.h"
#include "core/error/error_macros.h"
#include "core/io/dir_access.h"
#include "core/io/image.h"
#include "core/io/json.h"
#include "steamworks.h"
#include "thirdparty/steamworks/public/steam/isteaminput.h"
#include <iterator>

#ifdef MODULE_INPUT_GLYPHS_ENABLED

ESteamInputType __input_type_lut[] = {
	ESteamInputType::k_ESteamInputType_Unknown,
	ESteamInputType::k_ESteamInputType_SteamController,
	ESteamInputType::k_ESteamInputType_XBox360Controller,
	ESteamInputType::k_ESteamInputType_XBoxOneController,
	ESteamInputType::k_ESteamInputType_GenericGamepad,
	ESteamInputType::k_ESteamInputType_PS3Controller,
	ESteamInputType::k_ESteamInputType_PS4Controller,
	ESteamInputType::k_ESteamInputType_PS5Controller,
	ESteamInputType::k_ESteamInputType_SwitchProController,
	ESteamInputType::k_ESteamInputType_SteamDeckController
};

static_assert(std::size(__input_type_lut) == HBInputType::INPUT_TYPE_MAX);

EInputActionOrigin __origin_to_teamworks_xbox_origin_lut[]{
	k_EInputActionOrigin_XBoxOne_A,
	k_EInputActionOrigin_XBoxOne_B,
	k_EInputActionOrigin_XBoxOne_X,
	k_EInputActionOrigin_XBoxOne_Y,
	k_EInputActionOrigin_XBoxOne_View,
	k_EInputActionOrigin_XBoxOne_Menu,
	k_EInputActionOrigin_XBoxOne_LeftBumper,
	k_EInputActionOrigin_XBoxOne_RightBumper,
	k_EInputActionOrigin_XBoxOne_LeftTrigger_Pull,
	k_EInputActionOrigin_XBoxOne_LeftTrigger_Click,
	k_EInputActionOrigin_XBoxOne_RightTrigger_Pull,
	k_EInputActionOrigin_XBoxOne_RightTrigger_Click,
	k_EInputActionOrigin_XBoxOne_LeftStick_Move,
	k_EInputActionOrigin_XBoxOne_LeftStick_Click,
	k_EInputActionOrigin_XBoxOne_LeftStick_DPadNorth,
	k_EInputActionOrigin_XBoxOne_LeftStick_DPadSouth,
	k_EInputActionOrigin_XBoxOne_LeftStick_DPadWest,
	k_EInputActionOrigin_XBoxOne_LeftStick_DPadEast,
	k_EInputActionOrigin_XBoxOne_RightStick_Move,
	k_EInputActionOrigin_XBoxOne_RightStick_Click,
	k_EInputActionOrigin_XBoxOne_RightStick_DPadNorth,
	k_EInputActionOrigin_XBoxOne_RightStick_DPadSouth,
	k_EInputActionOrigin_XBoxOne_RightStick_DPadWest,
	k_EInputActionOrigin_XBoxOne_RightStick_DPadEast,
	k_EInputActionOrigin_XBoxOne_DPad_North,
	k_EInputActionOrigin_XBoxOne_DPad_South,
	k_EInputActionOrigin_XBoxOne_DPad_West,
	k_EInputActionOrigin_XBoxOne_DPad_East,
	k_EInputActionOrigin_XBoxOne_DPad_Move,
	k_EInputActionOrigin_XBoxOne_Share,
	k_EInputActionOrigin_XBoxOne_LeftGrip_Upper,
	k_EInputActionOrigin_XBoxOne_LeftGrip_Lower,
	k_EInputActionOrigin_XBoxOne_RightGrip_Upper,
	k_EInputActionOrigin_XBoxOne_RightGrip_Lower,
	k_EInputActionOrigin_PS5_CenterPad_Click,
};

static_assert(std::size(__origin_to_teamworks_xbox_origin_lut) == HBInputOrigin::INPUT_ORIGIN_COUNT);

EInputActionOrigin HBSteamworksInputGlyphsSource::origin_to_steamworks_xbox_origin(const HBInputOrigin &p_input_origin) {
	if (p_input_origin == HBInputOrigin::INPUT_ORIGIN_INVALID) {
		return EInputActionOrigin::k_EInputActionOrigin_None;
	}
	return __origin_to_teamworks_xbox_origin_lut[p_input_origin];
}

ESteamInputType HBSteamworksInputGlyphsSource::input_type_to_steamworks_input_type(const HBInputType &p_input_type) {
	// InputType matches ESteamInputType int eh steamworks SDK, so we just do a straight conversion
	ERR_FAIL_COND_V(p_input_type == HBInputType::INPUT_TYPE_MAX, ESteamInputType::k_ESteamInputType_XBoxOneController);
	return __input_type_lut[p_input_type];
}

HBInputType HBSteamworksInputGlyphsSource::steamworks_input_type_to_input_type(const ESteamInputType &p_steam_input_type) {
	for (int i = 0; i < HBInputType::INPUT_TYPE_MAX; i++) {
		if (__input_type_lut[i] == p_steam_input_type) {
			return (HBInputType)i;
		}
	}
	return HBInputType::UNKNOWN;
}

ESteamInputGlyphSize input_glph_size_to_steamworks(const HBInputGlyphSize &p_glyph_size) {
	return (ESteamInputGlyphSize)p_glyph_size;
}

Ref<Texture2D> HBSteamworksInputGlyphsSource::get_input_glyph(const HBInputType &p_input_type, const HBInputOrigin &p_input_origin, const int &p_glyphs_style, const HBInputGlyphSize &p_size) {
	HBSteamInput *input = Steamworks::get_singleton()->get_input();
	// Convert from xbox 360 reference origin to the destination input type
	EInputActionOrigin steamworks_origin = origin_to_steamworks_xbox_origin(p_input_origin);
	ESteamInputType steamworks_input_type = input_type_to_steamworks_input_type(p_input_type);
	ESteamInputGlyphSize glyph_size = input_glph_size_to_steamworks(p_size);

	EInputActionOrigin translated_origin = input->translate_action_origin(steamworks_input_type, steamworks_origin);
	String glyph_path = input->get_glyph_png_for_action_origin(translated_origin, glyph_size, p_glyphs_style);

	if (glyph_path.is_empty()) {
		Ref<PlaceholderTexture2D> placeholder;
		placeholder.instantiate();
		placeholder->set_size(Vector2(32, 32));
		return placeholder;
	}

	Ref<Image> image = Image::load_from_file(glyph_path);
	Ref<Texture2D> tex = ImageTexture::create_from_image(image);
	tex->set_meta("glyph_path", glyph_path);
	return tex;
}

HBInputType HBSteamworksInputGlyphsSource::identify_joy(int p_device) const {
	InputHandle_t input_handle = Steamworks::get_singleton()->get_input()->get_joy_steam_input_handle(p_device);
	if (input_handle == 0) {
		return HBInputType::UNKNOWN;
	}
	ESteamInputType steam_input_type = Steamworks::get_singleton()->get_input()->get_input_type_for_handle(input_handle);
	return steamworks_input_type_to_input_type(steam_input_type);
}

void HBSteamworksInputGlyphDumpTool::_dump_input_type(InputDumpInfo &p_dump_info, HBInputType p_input_type, Ref<HBSteamworksInputGlyphsSource> p_source) {
	HBSteamInput *input = Steamworks::get_singleton()->get_input();

	ESteamInputType steam_input_type = p_source->input_type_to_steamworks_input_type(p_input_type);

	for (int size_i = 0; size_i < HBInputGlyphSize::GLYPH_SIZE_MAX; size_i++) {
		HBInputGlyphSize size = (HBInputGlyphSize)size_i;
		for (int theme_i = 0; theme_i < HBInputGlyphStyle::GLYPH_STYLE_THEME_COUNT; theme_i++) {
			int base_style = theme_i;
			const int ABXY_STYLES[] = {
				base_style | HBInputGlyphStyle::GLYPH_STYLE_NEUTRAL_COLOR_ABXY,
				base_style | HBInputGlyphStyle::GLYPH_STYLE_SOLID_ABXY,
				base_style | HBInputGlyphStyle::GLYPH_STYLE_SOLID_ABXY | HBInputGlyphStyle::GLYPH_STYLE_NEUTRAL_COLOR_ABXY,
			};
			static_assert(std::size(ABXY_STYLES) - 1 < HBInputGlyphStyle::GLYPH_STYLE_THEME_COUNT);
			// First get the different ABXY variations
			for (int i = 0; i < 4 * 3; i++) {
				int style_modifiers = ABXY_STYLES[i / 4];
				int style = style_modifiers | base_style;
				HBInputOrigin origin = (HBInputOrigin)(i % 4);
				EInputActionOrigin input_origin = HBSteamworksInputGlyphsSource::origin_to_steamworks_xbox_origin(origin);
				input_origin = input->translate_action_origin(steam_input_type, input_origin);

				Ref<Texture2D> tex = p_source->get_input_glyph(p_input_type, origin, style, size);
				String svg_path = input->get_glyph_svg_for_action_origin(input_origin, style);
				p_dump_info.themes[theme_i].file_names.push_back(svg_path.get_file());
				p_dump_info.themes[theme_i].filename_to_idx_map.insert(svg_path.get_file(), p_dump_info.themes[theme_i].file_names.size() - 1);
				p_dump_info.themes[theme_i].texture_map_abxy_overrides[i] = p_dump_info.themes[theme_i].file_names.size() - 1;
			}

			// Now do the rest
			for (int i = 0; i < HBInputOrigin::INPUT_ORIGIN_COUNT; i++) {
				HBInputOrigin origin = (HBInputOrigin)i;
				HBInputGlyphStyle glyph_style = (HBInputGlyphStyle)base_style;

				EInputActionOrigin input_origin = HBSteamworksInputGlyphsSource::origin_to_steamworks_xbox_origin(origin);
				input_origin = input->translate_action_origin(steam_input_type, input_origin);

				Ref<Texture2D> tex = p_source->get_input_glyph(p_input_type, origin, glyph_style, size);
				String svg_path = input->get_glyph_svg_for_action_origin(input_origin, base_style);
				p_dump_info.themes[theme_i].file_names.push_back(svg_path.get_file());
				p_dump_info.themes[theme_i].filename_to_idx_map.insert(svg_path.get_file(), p_dump_info.themes[theme_i].file_names.size() - 1);
				p_dump_info.themes[theme_i].texture_map[i] = p_dump_info.themes[theme_i].file_names.size() - 1;
			}
		}
	}
}

void HBSteamworksInputGlyphDumpTool::dump(const String &p_module_dir_path) {
	const String glyphs_path = p_module_dir_path.path_join("resources");
	if (!DirAccess::dir_exists_absolute(glyphs_path)) {
		DirAccess::make_dir_recursive_absolute(glyphs_path);
	}

	Ref<HBSteamworksInputGlyphsSource> source = HBSteamworksInputGlyphsSource::_create_current();

	HBSteamInput *input = Steamworks::get_singleton()->get_input();

	InputDumpInfo info[HBInputType::INPUT_TYPE_MAX - 1];

	Dictionary out_dict;
	Array input_types_out;

	for (int i = 0; i < HBInputType::INPUT_TYPE_MAX - 1; i++) {
		HBInputType input_type = (HBInputType)(i + 1);
		_dump_input_type(info[i], input_type, source);
		Dictionary input_type_dict;
		input_type_dict["input_type"] = input_type;
		Array themes;
		for (int j = 0; j < GLYPH_STYLE_THEME_COUNT; j++) {
			Array graphics_abxy;
			Array graphics;

			for (int k = 0; k < 4 * 3; k++) {
				int str_idx = info[i].themes[j].texture_map_abxy_overrides[k];
				graphics_abxy.push_back(info[i].themes[j].file_names[str_idx]);
			}
			for (int k = 0; k < HBInputOrigin::INPUT_ORIGIN_COUNT; k++) {
				int str_idx = info[i].themes[j].texture_map[k];
				graphics.push_back(info[i].themes[j].file_names[str_idx]);
			}
			Dictionary theme_dict;
			theme_dict["graphics"] = graphics;
			theme_dict["graphics_abxy_overrides"] = graphics_abxy;
			themes.append(theme_dict);
		}
		input_type_dict["themes"] = themes;
		input_types_out.push_back(input_type_dict);
	}

	out_dict["input_types"] = input_types_out;

	Ref<FileAccess> fa = FileAccess::open(glyphs_path.path_join("info.json"), FileAccess::WRITE);

	fa->store_string(JSON::stringify(out_dict, "  "));
	SceneTree::get_singleton()->quit();
}

#endif // MODULE_INPUT_GLYPHS_ENABLED
