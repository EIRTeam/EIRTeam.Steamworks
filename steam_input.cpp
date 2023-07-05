/**************************************************************************/
/*  steam_input.cpp                                                       */
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

#include "steam_input.h"
#include "input_glyphs_steamworks.h"
#include "steam/steam_api_flat.h"
#include "steamworks.h"
#include "sw_error_macros.h"

void HBSteamInput::_on_joy_connection_changed(int p_device, bool p_connected) {
	if (devices.has(p_device)) {
		devices.erase(p_device);
	}
	if (p_connected) {
		DeviceInformation device_info;
		device_info.raw_name = Input::get_singleton()->get_joy_info(p_device).get("raw_name", "");
		device_info.steam_input_idx = Input::get_singleton()->get_joy_info(p_device).get("steam_input_index", -1);
		devices.insert(p_device, device_info);
	}
}

void HBSteamInput::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init", "call_run_frame_automatically"), &HBSteamInput::init, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("run_frame"), &HBSteamInput::run_frame);
}

void HBSteamInput::init_interface() {
	steam_input = SteamAPI_SteamInput();
	SW_ERR_FAIL_COND_MSG(steam_input == nullptr, "Steamworks: Failed to initialize Steam Input, something catastrophic must have happened");
}

bool HBSteamInput::is_valid() const {
	return steam_input != nullptr;
}

SWC::InputActionOrigin HBSteamInput::translate_action_origin(const SWC::SteamInputType &p_destination_input_type, const SWC::InputActionOrigin &p_source_origin) const {
	return (SWC::InputActionOrigin)SteamAPI_ISteamInput_TranslateActionOrigin(steam_input, (ESteamInputType)p_destination_input_type, (EInputActionOrigin)p_source_origin);
}

String HBSteamInput::get_glyph_png_for_action_origin(const SWC::InputActionOrigin &p_origin, const SWC::SteamInputGlyphSize &p_size, const uint32_t &p_flags) const {
	const char *glyph_path = SteamAPI_ISteamInput_GetGlyphPNGForActionOrigin(steam_input, (EInputActionOrigin)p_origin, (ESteamInputGlyphSize)p_size, p_flags);
	return String(glyph_path);
}

String HBSteamInput::get_glyph_svg_for_action_origin(const SWC::InputActionOrigin &p_origin, const uint32_t &p_flags) const {
	const char *glyph_path = SteamAPI_ISteamInput_GetGlyphSVGForActionOrigin(steam_input, (EInputActionOrigin)p_origin, p_flags);
	return String(glyph_path);
}

SWC::InputHandle_t HBSteamInput::get_controller_for_gamepad_index(const int &p_gamepad_index) const {
	return SteamAPI_ISteamInput_GetControllerForGamepadIndex(steam_input, p_gamepad_index);
}

SWC::SteamInputType HBSteamInput::get_input_type_for_handle(const SWC::InputHandle_t &p_input_handle) const {
	return (SWC::SteamInputType)SteamAPI_ISteamInput_GetInputTypeForHandle(steam_input, p_input_handle);
}

SWC::InputHandle_t HBSteamInput::get_joy_steam_input_handle(int p_device) const {
	if (!devices.has(p_device)) {
		return 0;
	}
	return SteamAPI_ISteamInput_GetControllerForGamepadIndex(steam_input, devices[p_device].steam_input_idx);
}

void HBSteamInput::run_frame() {
	SteamAPI_ISteamInput_RunFrame(steam_input, true);
}

void HBSteamInput::init(bool p_call_run_frame_automatically) {
	SW_ERR_FAIL_COND_MSG(!SteamAPI_ISteamInput_Init(steam_input, true), "Steam Input: Init returned false");
	initialized = true;
	run_frame();
	TypedArray<int> connected_joypads = Input::get_singleton()->get_connected_joypads();
	for (int i = 0; i < connected_joypads.size(); i++) {
		int device_id = connected_joypads[i];
		DeviceInformation device_info;
		device_info.raw_name = Input::get_singleton()->get_joy_info(device_id).get("raw_name", "");
		device_info.steam_input_idx = Input::get_singleton()->get_joy_info(device_id).get("steam_input_index", -1);
		devices.insert(device_id, device_info);
	}
	Input::get_singleton()->connect("joy_connection_changed", callable_mp(this, &HBSteamInput::_on_joy_connection_changed));
#ifdef MODULE_INPUT_GLYPHS_ENABLED
	HBSteamworksInputGlyphsSource::make_current();
#endif
}

HBSteamInput::~HBSteamInput() {
	if (initialized) {
		SteamAPI_ISteamInput_Shutdown(steam_input);
	}
}
