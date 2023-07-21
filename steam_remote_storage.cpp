/**************************************************************************/
/*  steam_remote_storage.cpp                                              */
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

#include "steam_remote_storage.h"
#include "steam/steam_api_flat.h"

void HBSteamRemoteStorage::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_cloud_enabled"), &HBSteamRemoteStorage::is_cloud_enabled);
	ClassDB::bind_method(D_METHOD("file_read", "file_name"), &HBSteamRemoteStorage::file_read);
	ClassDB::bind_method(D_METHOD("file_write", "file_name", "data"), &HBSteamRemoteStorage::file_write);
	ClassDB::bind_method(D_METHOD("file_exists", "file_name"), &HBSteamRemoteStorage::file_exists);
	ClassDB::bind_method(D_METHOD("get_file_size", "file_name"), &HBSteamRemoteStorage::get_file_size);
	ClassDB::bind_method(D_METHOD("is_valid"), &HBSteamRemoteStorage::is_valid);
}

bool HBSteamRemoteStorage::is_cloud_enabled() const {
	return SteamAPI_ISteamRemoteStorage_IsCloudEnabledForAccount(remote_storage) && SteamAPI_ISteamRemoteStorage_IsCloudEnabledForApp(remote_storage);
}

void HBSteamRemoteStorage::init_interface() {
	remote_storage = SteamAPI_SteamRemoteStorage();
}

bool HBSteamRemoteStorage::is_valid() const {
	return remote_storage != nullptr;
}

Vector<uint8_t> HBSteamRemoteStorage::file_read(const String &p_file_name) const {
	int32_t file_size = get_file_size(p_file_name);
	Vector<uint8_t> data;
	data.resize(file_size);
	int data_read_bytes = SteamAPI_ISteamRemoteStorage_FileRead(remote_storage, p_file_name.utf8().get_data(), data.ptrw(), data.size());
	data.resize(data_read_bytes);
	return data;
}

bool HBSteamRemoteStorage::file_write(const String &p_file_name, Vector<uint8_t> p_data) const {
	return SteamAPI_ISteamRemoteStorage_FileWrite(remote_storage, p_file_name.utf8(), p_data.ptr(), p_data.size());
}

bool HBSteamRemoteStorage::file_exists(const String &p_file_name) const {
	return SteamAPI_ISteamRemoteStorage_FileExists(remote_storage, p_file_name.utf8());
}

uint64_t HBSteamRemoteStorage::get_file_size(const String &p_file_name) const {
	return SteamAPI_ISteamRemoteStorage_GetFileSize(remote_storage, p_file_name.utf8().get_data());
}
