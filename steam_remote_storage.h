/**************************************************************************/
/*  steam_remote_storage.h                                                */
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

#ifndef STEAM_REMOTE_STORAGE_H
#define STEAM_REMOTE_STORAGE_H

#include "core/object/ref_counted.h"

class ISteamRemoteStorage;
class HBSteamRemoteStorage;

class HBSteamRemoteStorage : public RefCounted {
	GDCLASS(HBSteamRemoteStorage, RefCounted);
	ISteamRemoteStorage *remote_storage = nullptr;

protected:
	static void _bind_methods();

public:
	bool is_cloud_enabled() const;
	void init_interface();
	bool is_valid() const;
	uint64_t get_file_size(const String &p_file_name) const;
	Vector<uint8_t> file_read(const String &p_file_name) const;
	bool file_write(const String &p_file_name, Vector<uint8_t> p_data) const;
	bool file_exists(const String &p_file_name) const;
};

#endif // STEAM_REMOTE_STORAGE_H
