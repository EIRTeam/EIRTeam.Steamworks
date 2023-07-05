/**************************************************************************/
/*  steam_input.h                                                         */
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

#ifndef STEAM_INPUT_H
#define STEAM_INPUT_H

#include "scene/main/node.h"
#include "steamworks_constants.gen.h"

class ISteamInput;

class HBSteamInput : public Node {
	GDCLASS(HBSteamInput, Node);
	bool initialized = false;
	ISteamInput *steam_input = nullptr;

	struct DeviceInformation {
		String raw_name;
		int steam_input_idx = -1;
	};

	HashMap<int, DeviceInformation> devices;

	void _on_joy_connection_changed(int p_device, bool p_connected);

protected:
	static void _bind_methods();

public:
	void init_interface();
	bool is_valid() const;
	SWC::InputActionOrigin translate_action_origin(const SWC::SteamInputType &p_destination_input_type, const SWC::InputActionOrigin &p_input_action_origin) const;
	String get_glyph_png_for_action_origin(const SWC::InputActionOrigin &p_origin, const SWC::SteamInputGlyphSize &p_size, const uint32_t &p_flags) const;
	String get_glyph_svg_for_action_origin(const SWC::InputActionOrigin &p_origin, const uint32_t &p_flags) const;
	SWC::InputHandle_t get_controller_for_gamepad_index(const int &p_gamepad_index) const;
	SWC::SteamInputType get_input_type_for_handle(const SWC::InputHandle_t &p_input_handle) const;
	SWC::InputHandle_t get_joy_steam_input_handle(int p_device) const;
	void run_frame();
	void init(bool p_call_run_frame_automatically = true);
	~HBSteamInput();
};

#endif // STEAM_INPUT_H
