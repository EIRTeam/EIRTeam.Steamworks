/**************************************************************************/
/*  steam_friends.h                                                       */
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

#ifndef STEAM_FRIENDS_H
#define STEAM_FRIENDS_H

#include "core/object/ref_counted.h"
#include "scene/resources/texture.h"

class ISteamFriends;

class HBSteamFriend : public RefCounted {
	GDCLASS(HBSteamFriend, RefCounted);

private:
	Ref<Texture2D> avatar;
	uint64_t steam_id;

protected:
	static void _bind_methods();

public:
	String get_persona_name() const;
	Ref<Texture2D> get_avatar() const;
	uint64_t get_steam_id() const;
	static Ref<HBSteamFriend> from_steam_id(uint64_t p_steam_id);
};

class HBSteamFriends : public RefCounted {
	GDCLASS(HBSteamFriends, RefCounted);
	ISteamFriends *steam_friends = nullptr;

public:
	void init_interface();
	bool is_valid() const;
	ISteamFriends *get_interface() const;
};

#endif // STEAM_FRIENDS_H
