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
#include "scene/resources/image_texture.h"
#include "scene/resources/placeholder_textures.h"
#include "steamworks.h"
#include "steam/isteaminput.h"
#include <iterator>

#ifdef MODULE_INPUT_GLYPHS_ENABLED

#include "modules/input_glyphs/input_glyph_svg_decode.h"

SWC::SteamInputType __input_type_lut[] = {
	SWC::SteamInputType::STEAM_INPUT_TYPE_UNKNOWN,
	SWC::SteamInputType::STEAM_INPUT_TYPE_STEAM_CONTROLLER,
	SWC::SteamInputType::STEAM_INPUT_TYPE_X_BOX360_CONTROLLER,
	SWC::SteamInputType::STEAM_INPUT_TYPE_X_BOX_ONE_CONTROLLER,
	SWC::SteamInputType::STEAM_INPUT_TYPE_GENERIC_GAMEPAD,
	SWC::SteamInputType::STEAM_INPUT_TYPE_PS3_CONTROLLER,
	SWC::SteamInputType::STEAM_INPUT_TYPE_PS4_CONTROLLER,
	SWC::SteamInputType::STEAM_INPUT_TYPE_PS5_CONTROLLER,
	SWC::SteamInputType::STEAM_INPUT_TYPE_SWITCH_PRO_CONTROLLER,
	SWC::SteamInputType::STEAM_INPUT_TYPE_STEAM_DECK_CONTROLLER,
	SWC::SteamInputType::STEAM_INPUT_TYPE_UNKNOWN, // Keyboard
};

static_assert(std::size(__input_type_lut) == InputGlyphsConstants::INPUT_TYPE_MAX);

SWC::InputActionOrigin __origin_to_teamworks_xbox_origin_lut[]{
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_A,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_B,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_X,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_Y,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_VIEW,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_MENU,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_LEFT_BUMPER,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_RIGHT_BUMPER,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_LEFT_TRIGGER_PULL,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_LEFT_TRIGGER_CLICK,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_RIGHT_TRIGGER_PULL,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_RIGHT_TRIGGER_CLICK,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_LEFT_STICK_MOVE,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_LEFT_STICK_CLICK,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_LEFT_STICK_D_PAD_NORTH,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_LEFT_STICK_D_PAD_SOUTH,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_LEFT_STICK_D_PAD_WEST,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_LEFT_STICK_D_PAD_EAST,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_RIGHT_STICK_MOVE,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_RIGHT_STICK_CLICK,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_RIGHT_STICK_D_PAD_NORTH,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_RIGHT_STICK_D_PAD_SOUTH,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_RIGHT_STICK_D_PAD_WEST,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_RIGHT_STICK_D_PAD_EAST,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_D_PAD_NORTH,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_D_PAD_SOUTH,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_D_PAD_WEST,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_D_PAD_EAST,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_D_PAD_MOVE,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_SHARE,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_LEFT_GRIP_UPPER,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_LEFT_GRIP_LOWER,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_RIGHT_GRIP_UPPER,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_X_BOX_ONE_RIGHT_GRIP_LOWER,
	SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_PS5_CENTER_PAD_CLICK,
};

static_assert(std::size(__origin_to_teamworks_xbox_origin_lut) == InputGlyphsConstants::INPUT_ORIGIN_COUNT);

SWC::InputActionOrigin HBSteamworksInputGlyphsSource::origin_to_steamworks_xbox_origin(const InputGlyphsConstants::InputOrigin &p_input_origin) {
	if (p_input_origin == InputGlyphsConstants::INPUT_ORIGIN_INVALID) {
		return SWC::InputActionOrigin::INPUT_ACTION_ORIGIN_NONE;
	}
	return __origin_to_teamworks_xbox_origin_lut[p_input_origin];
}

SWC::SteamInputType HBSteamworksInputGlyphsSource::input_type_to_steamworks_input_type(const InputGlyphsConstants::InputType &p_input_type) {
	// InputType matches ESteamInputType int eh steamworks SDK, so we just do a straight conversion
	ERR_FAIL_COND_V(p_input_type == InputGlyphsConstants::INPUT_TYPE_MAX, SWC::SteamInputType::STEAM_INPUT_TYPE_X_BOX_ONE_CONTROLLER);
	return __input_type_lut[p_input_type];
}

InputGlyphsConstants::InputType HBSteamworksInputGlyphsSource::steamworks_input_type_to_input_type(const SWC::SteamInputType &p_steam_input_type) {
	for (int i = 0; i < InputGlyphsConstants::INPUT_TYPE_MAX; i++) {
		if (__input_type_lut[i] == p_steam_input_type) {
			return (InputGlyphsConstants::InputType)i;
		}
	}
	return InputGlyphsConstants::UNKNOWN;
}

ESteamInputGlyphSize input_glph_size_to_steamworks(const InputGlyphSize &p_glyph_size) {
	return (ESteamInputGlyphSize)p_glyph_size;
}

Ref<Texture2D> HBSteamworksInputGlyphsSource::get_input_glyph(const InputGlyphsConstants::InputType &p_input_type, const InputGlyphsConstants::InputOrigin &p_input_origin, const BitField<InputGlyphStyle> &p_glyphs_style, const InputGlyphSize &p_size) {
	HBSteamInput *input = Steamworks::get_singleton()->get_input();
	// Convert from xbox 360 reference origin to the destination input type
	SWC::InputActionOrigin steamworks_origin = (SWC::InputActionOrigin)origin_to_steamworks_xbox_origin(p_input_origin);
	SWC::SteamInputType steamworks_input_type = (SWC::SteamInputType)input_type_to_steamworks_input_type(p_input_type);

	SWC::InputActionOrigin translated_origin = input->translate_action_origin(steamworks_input_type, steamworks_origin);
	String glyph_path = input->get_glyph_svg_for_action_origin(translated_origin, p_glyphs_style);

	Vector2i size = Vector2i(32, 32);

	switch (p_size) {
		case GLYPH_SIZE_LARGE: {
			size = Vector2i(256, 256);
		} break;
		case GLYPH_SIZE_MEDIUM: {
			size = Vector2i(128, 128);
		} break;
		default:
		case GLYPH_SIZE_SMALL: {
			size = Vector2i(32, 32);
		} break;
	}

	if (glyph_path.is_empty()) {
		Ref<PlaceholderTexture2D> placeholder = memnew(PlaceholderTexture2D);
		placeholder->set_size(size);
		return placeholder;
	}

	Error err;
	Ref<FileAccess> file = FileAccess::open(glyph_path, FileAccess::ModeFlags::READ, &err);
	if (glyph_path.is_empty() || err != OK) {
		Ref<PlaceholderTexture2D> placeholder = memnew(PlaceholderTexture2D);
		placeholder->set_size(size);
		return placeholder;
	}

	Ref<Image> out_image;
	out_image.instantiate();
	String svg_str = file->get_as_utf8_string();
	PackedByteArray pba = svg_str.to_utf8_buffer();
	if (InputGlyphSVGDecode::render_svg(out_image, pba, size) != OK) {
		Ref<PlaceholderTexture2D> placeholder = memnew(PlaceholderTexture2D);
		placeholder->set_size(size);
		return placeholder;
	}

	Ref<Texture2D> tex = ImageTexture::create_from_image(out_image);
	tex->set_meta("glyph_path", glyph_path);
	return tex;
}

InputGlyphsConstants::InputType HBSteamworksInputGlyphsSource::identify_joy(int p_device) const {
	InputHandle_t input_handle = Steamworks::get_singleton()->get_input()->get_joy_steam_input_handle(p_device);
	if (input_handle == 0) {
		return InputGlyphsConstants::UNKNOWN;
	}
	SWC::SteamInputType steam_input_type = Steamworks::get_singleton()->get_input()->get_input_type_for_handle(input_handle);
	return steamworks_input_type_to_input_type(steam_input_type);
}

void HBSteamworksInputGlyphDumpTool::_dump_input_type(InputDumpInfo &p_dump_info, InputGlyphsConstants::InputType p_input_type, Ref<HBSteamworksInputGlyphsSource> p_source) {
	HBSteamInput *input = Steamworks::get_singleton()->get_input();

	SWC::SteamInputType steam_input_type = p_source->input_type_to_steamworks_input_type(p_input_type);
	for (int size_i = 0; size_i < InputGlyphSize::GLYPH_SIZE_MAX; size_i++) {
		InputGlyphSize size = (InputGlyphSize)size_i;
		for (int theme_i = 0; theme_i < InputGlyphStyle::GLYPH_STYLE_THEME_COUNT; theme_i++) {
			const char *THEME_NAMES[] = {
				"knockout",
				"light",
				"dark"
			};

			String theme_name = THEME_NAMES[theme_i];
			String theme_path = p_dump_info.module_path.path_join(theme_name);
			Ref<DirAccess> dir_fs = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
			if (!dir_fs->dir_exists(theme_path)) {
				dir_fs->make_dir_recursive(theme_path);
			}
			int base_style = theme_i;
			const int ABXY_STYLES[] = {
				base_style | InputGlyphStyle::GLYPH_STYLE_NEUTRAL_COLOR_ABXY,
				base_style | InputGlyphStyle::GLYPH_STYLE_SOLID_ABXY,
				base_style | InputGlyphStyle::GLYPH_STYLE_SOLID_ABXY | InputGlyphStyle::GLYPH_STYLE_NEUTRAL_COLOR_ABXY,
			};
			static_assert(std::size(ABXY_STYLES) - 1 < InputGlyphStyle::GLYPH_STYLE_THEME_COUNT);
			// First get the different ABXY variations
			for (int i = 0; i < 4 * 3; i++) {
				int style_modifiers = ABXY_STYLES[i / 4];
				int style = style_modifiers | base_style;
				InputGlyphsConstants::InputOrigin origin = (InputGlyphsConstants::InputOrigin)(i % 4);
				SWC::InputActionOrigin input_origin = HBSteamworksInputGlyphsSource::origin_to_steamworks_xbox_origin(origin);
				input_origin = input->translate_action_origin(steam_input_type, input_origin);

				Ref<Texture2D> tex = p_source->get_input_glyph(p_input_type, origin, style, size);
				String svg_path = input->get_glyph_svg_for_action_origin(input_origin, style);
				String new_file_path = theme_name + "/" + svg_path.get_file();
				if (!svg_path.is_empty()) {
					new_file_path = theme_name + "/" + svg_path.get_file();
					dir_fs->copy(svg_path, theme_path.path_join(svg_path.get_file()));
				}
				p_dump_info.themes[theme_i].file_names.push_back(new_file_path);
				p_dump_info.themes[theme_i].filename_to_idx_map.insert(new_file_path, p_dump_info.themes[theme_i].file_names.size() - 1);
				p_dump_info.themes[theme_i].texture_map_abxy_overrides[i] = p_dump_info.themes[theme_i].file_names.size() - 1;
			}

			// Now do the rest
			for (int i = 0; i < InputGlyphsConstants::INPUT_ORIGIN_COUNT; i++) {
				InputGlyphsConstants::InputOrigin origin = (InputGlyphsConstants::InputOrigin)i;
				InputGlyphStyle glyph_style = (InputGlyphStyle)base_style;

				SWC::InputActionOrigin input_origin = HBSteamworksInputGlyphsSource::origin_to_steamworks_xbox_origin(origin);
				input_origin = input->translate_action_origin(steam_input_type, input_origin);

				Ref<Texture2D> tex = p_source->get_input_glyph(p_input_type, origin, glyph_style, size);
				String svg_path = input->get_glyph_svg_for_action_origin(input_origin, base_style);
				String new_file_path;
				if (!svg_path.is_empty()) {
					new_file_path = theme_name + "/" + svg_path.get_file();
					dir_fs->copy(svg_path, theme_path.path_join(svg_path.get_file()));
				}
				p_dump_info.themes[theme_i].file_names.push_back(new_file_path);
				p_dump_info.themes[theme_i].filename_to_idx_map.insert(new_file_path, p_dump_info.themes[theme_i].file_names.size() - 1);
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

	InputDumpInfo info[InputGlyphsConstants::INPUT_TYPE_MAX];

	Dictionary out_dict;
	Array input_types_out;

	for (int i = 0; i < InputGlyphsConstants::INPUT_TYPE_MAX; i++) {
		InputGlyphsConstants::InputType input_type = (InputGlyphsConstants::InputType)(i);
		info[i].module_path = glyphs_path;
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
			for (int k = 0; k < InputGlyphsConstants::INPUT_ORIGIN_COUNT; k++) {
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
