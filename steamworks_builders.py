import json
from re import sub
from io import StringIO
from typing import List, Dict, Tuple

whitelisted_enum_names = [
    "EResult",
    "ELobbyType",
    "ESteamInputType",
    "ESteamInputGlyphSize",
    "EInputActionOrigin",
    "EResult",
    "ELobbyDistanceFilter",
    "ELobbyComparison",
]

# Needed because godot can't convert unsigned long long
type_renames = {"unsigned long long": "uint64_t", "int32": "int32_t"}

whitelisted_types = ["InputHandle_t", "HSteamPipe"]


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
        if not enum_name in whitelisted_enum_names:
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


def generate_constants(target, source, env):
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

        f.write("protected:\n")
        f.write("\tstatic void _bind_methods() {\n")
        for enum in enum_infos:
            for value_name, value in enum.enum_values:
                f.write(f"\t\tBIND_ENUM_CONSTANT({value_name});\n")
        f.write("\t}\n")
        f.write("};\n\n")

        for enum in enum_infos:
            f.write(f"VARIANT_ENUM_CAST(SteamworksConstants::{enum.enum_name});\n")
        f.write("\n")

        f.write("using SWC = SteamworksConstants;\n\n")
        f.write("#endif // STEAMWORKS_CONSTANTS_GEN_H\n")
