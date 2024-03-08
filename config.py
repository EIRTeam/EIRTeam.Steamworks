def can_build(env, platform):
    env.module_add_dependencies("steamworks", ["input_glyphs"], True)
    return platform == "windows" or platform == "linuxbsd"


def configure(env):
    pass


def get_doc_path():
    return "doc_classes"


def get_doc_classes():
    return [
        "Steamworks",
        "HBSteamInput",
        "HBSteamFriends",
        "HBSteamFriend",
        "HBSteamLobby",
        "HBSteamApps",
        "HBSteamUGC",
        "HBSteamUGCItem",
        "HBSteamUserStats",
        "HBSteamUGCEditor",
        "HBSteamUGCAdditionalPreview",
        "HBSteamUGCQuery",
        "HBSteamUGCQueryPageResult",
        "HBLobbyListQuery",
        "HBSteamMatchmaking",
        "HBSteamNetworking",
        "HBSteamRemoteStorage",
        "HBSteamUtils",
        "HBSteamUser",
        "HBAuthTicketForWebAPI",
        "SteamP2PPacket",
        "SteamworksConstants",
        "HBSteamUGCItemUpdateProgress",
        "HBSteamUGCUserItemVoteResult",
    ]
