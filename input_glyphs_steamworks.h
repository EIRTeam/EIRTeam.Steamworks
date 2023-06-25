/**************************************************************************/
/*  input_glyphs_steamworks.h                                             */
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

#ifndef INPUT_GLYPHS_STEAMWORKS_H
#define INPUT_GLYPHS_STEAMWORKS_H

#include "modules/modules_enabled.gen.h"

#ifdef MODULE_INPUT_GLYPHS_ENABLED

#include "modules/input_glyphs/input_glyphs.h"
#include "modules/input_glyphs/input_glyphs_source.h"
#include "thirdparty/steamworks/public/steam/isteaminput.h"

class HBSteamworksInputGlyphDumpTool;

class HBSteamworksInputGlyphsSource : public InputGlyphsSource {
	GDCLASS(HBSteamworksInputGlyphsSource, InputGlyphsSource);

protected:
	static EInputActionOrigin origin_to_steamworks_xbox_origin(const HBInputOrigin &p_input_origin);
	static ESteamInputType input_type_to_steamworks_input_type(const HBInputType &p_input_type);

public:
	static HBInputType steamworks_input_type_to_input_type(const ESteamInputType &p_steam_input_type);
	static Ref<InputGlyphsSource> _create_current() {
		Ref<HBSteamworksInputGlyphsSource> ref;
		ref.instantiate();
		return ref;
	}

	static void make_current() {
		_create_func = _create_current;
	}

	virtual Ref<Texture2D> get_input_glyph(const HBInputType &p_input_type, const HBInputOrigin &p_input_origin, const int &p_glyphs_style, const HBInputGlyphSize &p_size) override;
	virtual HBInputType identify_joy(int p_controller_idx) const override;
	friend class HBSteamworksInputGlyphDumpTool;
};

class HBSteamworksInputGlyphDumpTool {
	struct InputDumpInfo {
		String module_path;
		struct ThemeInfo {
			HashMap<String, int> filename_to_idx_map;
			Vector<String> file_names;
			int texture_map_abxy_overrides[4 * 3];
			int texture_map[HBInputOrigin::INPUT_ORIGIN_COUNT];
		} themes[HBInputGlyphStyle::GLYPH_STYLE_THEME_COUNT];
	};

public:
	static void _dump_input_type(InputDumpInfo &p_dump_info, HBInputType p_input_type, Ref<HBSteamworksInputGlyphsSource> p_source);
	static void dump(const String &p_module_dir_path);
};

#endif // MODULE_INPUT_GLYPHS_ENABLED

#endif // INPUT_GLYPHS_STEAMWORKS_H
