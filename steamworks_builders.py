import json
from io import StringIO
from re import sub
from typing import Dict, List, Tuple

whitelisted_enum_names = [
    "EResult",
    "ELobbyType",
    "ESteamInputType",
    "ESteamInputGlyphSize",
    "EInputActionOrigin",
    "EResult",
    "ELobbyDistanceFilter",
    "ELobbyComparison",
    "EP2PSend",
    "EP2PSessionError",
    "EUGCMatchingUGCType",
    "EUGCQuery",
    "EUserUGCList",
    "EUserUGCListSortOrder",
    "EWorkshopFileType",
    "ERemoteStoragePublishedFileVisibility",
    "EItemPreviewType",
    "EGamepadTextInputMode",
    "EFloatingGamepadTextInputMode",
    "EGamepadTextInputLineMode",
    "EItemUpdateStatus",
    "EItemState",
    "EChatRoomEnterResponse",
    "EChatEntryType",
    "EChatMemberStateChange",
]

# Needed because godot can't convert unsigned long long
type_renames = {
    "unsigned long long": "uint64_t",
    "int32": "int32_t",
    "uint64": "uint64_t",
    "uint32": "uint32_t",
    "AppId_t": "uint32_t",
}

whitelisted_types = [
    "InputHandle_t",
    "HSteamPipe",
    "UGCHandle_t",
    "UGCQueryHandle_t",
    "UGCUpdateHandle_t",
    "PublishedFileId_t",
    "HAuthTicket",
]

bitfields = ["ItemState"]

whitelisted_constants = ["k_UGCQueryHandleInvalid", "k_UGCUpdateHandleInvalid"]

whitelisted_structs = ["SteamUGCDetails_t"]


def snake_case(s):
    return "_".join(sub("([A-Z][a-z]+)", r" \1", sub("([A-Z]+)", r" \1", s.replace("-", " "))).split()).upper()


class EnumInfo:
    enum_name: str
    enum_values: List[Tuple[str, str]]

    def __init__(self, enum_name: str):
        self.enum_name = enum_name
        self.enum_values = []


# Returns a dictionary of enum names with their renamed constant values inside
def process_enum_data(enums_data) -> List[EnumInfo]:
    enum_infos: List[EnumInfo] = []
    for enum in enums_data:
        enum_name = enum["enumname"]
        if enum_name not in whitelisted_enum_names:
            continue
        if enum_name.startswith("E"):
            enum_name = enum_name[1:]
        enum_info: EnumInfo = EnumInfo(enum_name)
        for value in enum["values"]:
            value_name = value["name"]
            if value_name.lower().startswith("k_e"):
                value_name = value_name[3:]
            value_name = value_name.replace("_", "")
            value_name = snake_case(value_name)
            if value_name.startswith("P2_P_"):
                # Hack to make P2P constants nicer
                value_name = "P2P_" + value_name[5:]
            enum_info.enum_values.append((value_name, value["value"]))
        enum_infos.append(enum_info)

    return enum_infos


def generate_enum_definitions(enums: List[EnumInfo]) -> StringIO:
    f = StringIO()
    for enum in enums:
        f.write("\tenum " + enum.enum_name + " {\n")
        for value_name, value in enum.enum_values:
            f.write("\t\t" + value_name + " = " + value + ",\n")
        f.write("\t};\n\n")
    return f


def generate_constants(constants: List[Dict]) -> StringIO:
    f = StringIO()
    for constant in constants:
        if constant["constname"] in whitelisted_constants:
            const_name = constant["constname"]
            const_type = constant["consttype"]

            const_val = constant["constval"]
            if const_name.startswith("k_"):
                if const_name[2].islower():
                    const_name = const_name[3:]
                else:
                    const_name = const_name[2:]
            f.write(f"\tstatic const {const_type} {snake_case(const_name)} = {const_val};\n")
    return f


def generate_structs(structs: List[Dict]) -> StringIO:
    f = StringIO()
    f.write("\t#if defined(LINUXBSD_ENABLED) || defined(MACOS_ENABLED)\n")
    f.write("\t#pragma pack( push, 4 )\n")
    f.write("\t#else\n")
    f.write("\t#pragma pack( push, 8 )\n")
    f.write("\t#endif\n\n")

    for struct in structs:
        if struct["struct"] not in whitelisted_structs:
            continue
        f.write(f"\tstruct {struct['struct']} " + "{\n")
        for field in struct["fields"]:
            field_type = field["fieldtype"]

            spl = field_type.split(" ")
            field_type_array = ""
            if len(spl) > 1:
                field_type_array = spl[1]
                field_type = spl[0]

            field_type = type_renames.get(field_type, field_type)

            if field_type.startswith("E"):
                field_type = field_type[1:]
            name_start_index = 0
            for idx, character in enumerate(field["fieldname"]):
                if character.isupper():
                    name_start_index = idx
                    break
            field_name = snake_case(field["fieldname"][name_start_index:]).lower()
            f.write(f"\t\t{field_type} {field_name}{field_type_array};\n")
        f.write("};\n\n")
    f.write("\t#pragma pack( pop )\n\n")
    return f


def generate_constants_file(target, source, env):
    json_f = json.load(open(source[0].path, "r"))

    enum_infos = process_enum_data(json_f["enums"])
    with open(target[0].path, "w") as f:
        f.write("#ifndef STEAMWORKS_CONSTANTS_GEN_H\n")
        f.write("#define STEAMWORKS_CONSTANTS_GEN_H\n\n")

        f.write('#include "core/variant/binder_common.h"\n')
        f.write('#include "core/object/object.h"\n\n')
        f.write('#include "core/object/class_db.h"\n\n')

        f.write("class SteamworksConstants : public Object {\n")
        f.write("GDCLASS(SteamworksConstants, Object);\n")
        f.write("public:\n")

        for typedef in json_f["typedefs"]:
            if typedef["typedef"] in whitelisted_types:
                typedef_name = typedef["typedef"]
                typedef_type = type_renames.get(typedef["type"], typedef["type"])
                f.write(f"\ttypedef {typedef_type} {typedef_name};\n")

        f.write("\n")
        f.write(generate_enum_definitions(enum_infos).getvalue())
        f.write("\n")
        f.write(generate_constants(json_f["consts"]).getvalue())
        f.write("\n")
        f.write(generate_structs(json_f["structs"]).getvalue())

        f.write("protected:\n")
        f.write("\tstatic void _bind_methods() {\n")
        for enum in enum_infos:
            for value_name, value in enum.enum_values:
                if enum.enum_name in bitfields:
                    f.write(f"\t\tBIND_BITFIELD_FLAG({value_name});\n")
                else:
                    f.write(f"\t\tBIND_ENUM_CONSTANT({value_name});\n")
        f.write("\t}\n")
        f.write("};\n\n")

        for enum in enum_infos:
            if enum.enum_name in bitfields:
                f.write(f"VARIANT_BITFIELD_CAST(SteamworksConstants::{enum.enum_name});\n")
            else:
                f.write(f"VARIANT_ENUM_CAST(SteamworksConstants::{enum.enum_name});\n")
        f.write("\n")

        f.write("using SWC = SteamworksConstants;\n\n")
        f.write("#endif // STEAMWORKS_CONSTANTS_GEN_H\n")
