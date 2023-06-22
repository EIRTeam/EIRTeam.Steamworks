/**************************************************************************/
/*  steamworks.h                                                          */
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

#ifndef STEAMWORKS_H
#define STEAMWORKS_H

#include "scene/main/node.h"
#include "steam/steam_api_flat.h"
#include "steam_input.h"

class Steamworks : public Node {
	GDCLASS(Steamworks, Node);
	HSteamPipe steam_pipe;
	ISteamClient *steam_client;
	static Steamworks *singleton;
	bool initialized = false;
	int app_id;

	HBSteamInput *input = nullptr;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	static String last_error;
	static String get_last_error() { return last_error; };
	static Steamworks *get_singleton() { return singleton; }

	bool init(int p_app_id, bool p_run_callbacks_automatically = true);
	bool is_valid() const { return initialized; };

	void run_callbacks();

	HBSteamInput *get_input() const;
	Steamworks();
	~Steamworks();
};

#endif // STEAMWORKS_H
