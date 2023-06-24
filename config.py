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
    ]
