import json


def generate_enums(target, source, env):
    json_f = json.load(open(source[0].path, "r"))
    with open(target[0].path, "w") as f:
        f.write("#ifndef STEAMWORKS_ENUMS_GEN_H\n")
        f.write("#define STEAMWORKS_ENUMS_GEN_H\n\n")

        for enum in json_f["enums"]:
            enum_name = enum["enumname"]
            f.write("enum " + enum_name + " {\n")
            for value in enum["values"]:
                f.write("   " + value["name"] + " = " + value["value"] + ",\n")
            f.write("};\n\n")

        f.write("#endif\n")
