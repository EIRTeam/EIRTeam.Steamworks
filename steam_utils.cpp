/**************************************************************************/
/*  steam_utils.cpp                                                       */
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

#include "steam_utils.h"
#include "steam/steam_api_flat.h"
#include "sw_error_macros.h"

void HBSteamUtils::_on_gamepad_text_input_dismissed(Ref<SteamworksCallbackData> p_callback) {
	const GamepadTextInputDismissed_t *input = p_callback->get_data<GamepadTextInputDismissed_t>();
	if (!input->m_bSubmitted) {
		emit_signal("gamepad_text_input_dismissed", false, "");
	}
	Vector<uint8_t> text_input;
	text_input.resize(SteamAPI_ISteamUtils_GetEnteredGamepadTextLength(steam_utils));
	SteamAPI_ISteamUtils_GetEnteredGamepadTextInput(steam_utils, (char *)text_input.ptrw(), text_input.size());
	emit_signal("gamepad_text_input_dismissed", true, String::utf8((char *)text_input.ptr(), text_input.size()));
}

void HBSteamUtils::_on_floating_gamepad_text_input_dismissed(Ref<SteamworksCallbackData> p_callback) {
}

void HBSteamUtils::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_in_big_picture_mode"), &HBSteamUtils::is_in_big_picture_mode);
	ClassDB::bind_method(D_METHOD("is_on_steam_deck"), &HBSteamUtils::is_on_steam_deck);
	ClassDB::bind_method(D_METHOD("show_gamepad_text_input", "input_mode", "line_input_mode", "description", "existing_text", "max_text"), &HBSteamUtils::show_gamepad_text_input);
	ClassDB::bind_method(D_METHOD("show_floating_gamepad_text_input", "input_mode", "text_field_rect"), &HBSteamUtils::show_floating_gamepad_text_input);
	ADD_SIGNAL(MethodInfo("gamepad_text_input_dismissed", PropertyInfo(Variant::BOOL, "submitted"), PropertyInfo(Variant::STRING, "text")));
	ADD_SIGNAL(MethodInfo("floating_gamepad_text_input_dismissed"));
}

bool HBSteamUtils::is_in_big_picture_mode() const {
	return SteamAPI_ISteamUtils_IsSteamInBigPictureMode(steam_utils);
}

bool HBSteamUtils::is_on_steam_deck() const {
	return SteamAPI_ISteamUtils_IsSteamRunningOnSteamDeck(steam_utils);
}

bool HBSteamUtils::show_gamepad_text_input(SWC::GamepadTextInputMode p_input_mode, SWC::GamepadTextInputLineMode p_line_input_mode, String p_description, String p_existing_text, uint32_t p_max_text) const {
	return SteamAPI_ISteamUtils_ShowGamepadTextInput(
			steam_utils,
			(EGamepadTextInputMode)p_input_mode,
			(EGamepadTextInputLineMode)p_line_input_mode,
			p_description.utf8().get_data(),
			p_max_text,
			p_existing_text.utf8().get_data());
}

bool HBSteamUtils::show_floating_gamepad_text_input(SWC::FloatingGamepadTextInputMode p_input_mode, Rect2i p_text_field_rect) const {
	return SteamAPI_ISteamUtils_ShowFloatingGamepadTextInput(
			steam_utils,
			(EFloatingGamepadTextInputMode)p_input_mode,
			p_text_field_rect.position.x,
			p_text_field_rect.position.y,
			p_text_field_rect.size.x,
			p_text_field_rect.size.y);
}

void HBSteamUtils::init_interface() {
	steam_utils = SteamAPI_SteamUtils();
	SW_ERR_FAIL_COND_MSG(steam_utils == nullptr, "Steamworks: Failed to initialize Steam Utils, something catastrophic must have happened");
}

ISteamUtils *HBSteamUtils::get_interface() {
	return steam_utils;
}

bool HBSteamUtils::is_valid() const {
	return steam_utils != nullptr;
}

HBSteamUtils::HBSteamUtils() {
	Steamworks::get_singleton()->add_callback(GamepadTextInputDismissed_t::k_iCallback, callable_mp(this, &HBSteamUtils::_on_gamepad_text_input_dismissed));
	Steamworks::get_singleton()->add_callback(FloatingGamepadTextInputDismissed_t::k_iCallback, callable_mp(this, &HBSteamUtils::_on_floating_gamepad_text_input_dismissed));
}
