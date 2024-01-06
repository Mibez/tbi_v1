#!/usr/bin/python3
import argparse
import json
import os

VERBOSE = False
OUT_PATH = "./generated/messagespec.h"

TYPES = {
    0: "timediff_s  ",
    1: "timediff_ms ",
    2: "uint8_t     ",
    3: "int8_t      ",
    4: "uint16_t    ",
    5: "int16_t     ",
    6: "uint32_t    ",
    7: "int32_t     ",
}

def debug(line: str):
    if VERBOSE:
        print(line)

def generate_begin(path: str = OUT_PATH) -> bool:
    """ 
        Generate file header with include guards, includes
    """
    try:
        debug("Removing existing files...")
        if os.path.exists(path) and os.path.isfile(path):
            os.unlink(path)

        debug("Adding include guards...")
        with open(path, "w") as f:
            f.write("""/* Auto-generated file - DO NOT EDIT! */\n
#ifndef __MESSAGESPEC_H
#define __MESSAGESPEC_H

#include <stdint.h>
#include <stdlib.h>
#include "tbi_types.h"
#include "tbi.h"\n\n""")
    except Exception as e:
        print(f"Error initializing messagespec file: {repr(e)}")
        return False
    return True

def generate_enum(spec: dict, path: str = OUT_PATH) -> bool:
    """
        Generate enum that maps to msg ID for easy referring, as well as
        list of message types that shall be sent as a DCB
    """
    debug("Generating message type enumerations...")
    try:
        with open(path, "a") as f:
            f.write("/** @brief Message name enumeration, values are IDs */\n")
            f.write("typedef enum {\n")
            bundles: list = []
            k: str
            v: dict
            for k, v in spec.items():
                id_num = v.get("id", -1)
                if id_num < 0:
                    print(f"Error: No id field for messagespec {k}")
                    return False
                f.write(f"  {k.upper()} = {id_num},\n")
                if v.get("bundle", False):
                    bundles.append(k.upper())
            f.write("} msgspec_types_t;\n\n")
            f.write(f"const int msgspec_types_len = {len(spec.keys())};\n")
            f.write(f"const int msgspec_bundle_types_len = {len(bundles)};\n")
            f.write(f"const uint8_t msgspec_bundle_types = {{ {','.join(bundles)} }};\n")

    except Exception as e:
        print(f"Error in generate_enum: {repr(e)}")
        return False
    return True

def generate_structs(spec: dict, path: str = OUT_PATH) -> bool:
    """
        Generate a data structure for each message spec type
    """
    debug("Generating type definitions...")
    try:
        with open(path, "a") as f:
        
            f.write("\n/** @brief Convenience structs for sending telemetry */\n")
            k: str
            v: dict
            for k,v in spec.items():
                f.write("typedef struct {\n")
                for typename, datatype in v.get("data_types", {}).items():
                    if datatype not in TYPES:
                        print(f"Error in struct generation for {k}: unknown type {datatype}")
                        return False
                    f.write(f"   {TYPES[datatype]} {typename};\n")
                f.write(f"}} msgspec_{k}_t;\n\n")

    except Exception as e:
        print(f"Error in generate_structs: {repr(e)}")
        return False
    return True


def generate_functions(spec: dict, path: str = OUT_PATH) -> bool:
    """
        Generate function declarations for sending the telemetry
    """
    debug("Generating function declarations...")
    try:
        with open(path, "a") as f:
            f.write("/** @brief conviniency functions for sending telemetry */")
            k: str
            for k in spec.keys():
                f.write(f"\nint tbi_send_{k}(tbi_ctx_t* tbi, msgspec_{k}_t *value)\n")
                f.write("{\n")
                f.write(f"\treturn tbi_telemetry_schedule(tbi, {k.upper()}, (void*)value);\n")
                f.write("}\n")

    except Exception as e:
        print(f"Error in generate_structs: {repr(e)}")
        return False
    return True

def generate_machine_format_arrays(spec: dict, path: str = OUT_PATH) -> bool:
    """
        Generate machine readable format arrays
    """
    debug("Generating format arrays...")
    try:
        with open(path, "a") as f:
            f.write("\n/** @brief Machine understandable format for message specs */")
            k: str
            v: dict
            for k, v in spec.items():
                datatypes = []
                for _, datatype in v.get("data_types", {}).items():
                    if datatype not in TYPES:
                        print(f"Error in format array generation for {k}: unknown type {datatype}")
                        return False
                    datatypes.append(str(datatype))
                
                f.write(f"\nconst uint8_t msgspec_binary_{k}[] = {{ {', '.join(datatypes)} }};")
    except Exception as e:
        print(f"Error in generate_machine_format_arrays: {repr(e)}")
        return False
    return True

def generate_message_type_contexts(spec: dict, path: str = OUT_PATH) -> bool:
    """
        Generate type context structs
    """
    debug("Generating message type contexts...")
    try:
        with open(path, "a") as f:
            f.write("\n\n/** @brief Initialize message context for each type */\n")
            f.write("tbi_msg_ctx_t msgspec_ctxs[] = {\n")
            k: str
            v: dict
            for k, v in spec.items():
                f.write("\t{\n")
                f.write(f"\t\t.msgtype      = {k.upper()},\n")
                f.write(f"\t\t.dcb          = {'true' if bool(v.get('bundle', False)) else 'false'},\n")
                f.write(f"\t\t.raw_size     = sizeof(msgspec_{k}_t),\n")
                f.write(f"\t\t.format_len   = sizeof(msgspec_binary_{k}) / sizeof(uint8_t),\n")
                f.write(f"\t\t.format       = &msgspec_binary_{k}[0],\n")
                f.write(f"\t\t.buflen       = 0,\n")
                f.write(f"\t\t.head         = NULL,\n")
                f.write("\t},\n")
            
            f.write("};\n")
            f.write(f"const int msgspec_ctxs_len = {len(spec.keys())};\n")

    except Exception as e:
        print(f"Error in generate_message_type_contexts: {repr(e)}")
        return False
    return True

def generate_register_msgspec_fn(spec: dict, path: str = OUT_PATH) -> bool:
    """
        Generate tbi_register_msgspec function
    """
    debug("Generating tbi_register_msgspec function...")
    try:
        with open(path, "a") as f:
            # This fn is just constant at least for now
            f.write("\n\n/** @brief register message spec with tbi context */\n")
            f.write("int tbi_register_msgspec(tbi_ctx_t* tbi)\n")
            f.write("{\n")
            f.write("\ttbi->msg_ctxs = &msgspec_ctxs[0];\n")
            f.write("\ttbi->msg_ctxs_len = msgspec_ctxs_len;\n")
            f.write("\treturn 0;\n")
            f.write("}\n")

    except Exception as e:
        print(f"Error in generate_register_msgspec_fn: {repr(e)}")
        return False
    return True

def generate_finalize(path: str = OUT_PATH) -> bool:
    try:
        debug("Closing include guards...")
        with open(path, "a") as f:
            f.write("""
#endif /* __MESSAGESPEC_H */    
        """)
    except Exception as e:
        print(f"Error initializing messagespec file: {repr(e)}")
        return False
    return True

def compose(path: str) -> bool:
    debug(f"Running compose for file {path}")

    try:
        with open(path, "r") as f:
            contents: str = f.read()
            contents_json: dict = json.loads(contents)

    except Exception as e:
        print(f"Error parsing file: {repr(e)}")
        return False


    if not generate_begin():
        debug("File header generation failed")
        return False

    if not generate_enum(contents_json):
        debug("Message type enum generation failed")
        return False

    if not generate_structs(contents_json):
        debug("Message type data structure generation failed")
        return False

    if not generate_functions(contents_json):
        debug("Message type send function generation failed")
        return False

    if not generate_machine_format_arrays(contents_json):
        debug("Machine readable format spec generation failed")
        return False

    if not generate_message_type_contexts(contents_json):
        debug("Message type context generation failed")
        return False
    
    if not generate_register_msgspec_fn(contents_json):
        debug("tbi_register_msgspec function generation failed")
        return False

    if not generate_finalize():
        debug("File wrap-up failed")
        return False

    return True


if __name__ == "__main__":

    p = argparse.ArgumentParser(description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    
    p.add_argument("path_to_file",
                   help="desc")
    p.add_argument("-v", "--verbose", action="store_true",
                   help="Enable verbose mode")
                   
    cmdline_args = p.parse_args()
    VERBOSE = cmdline_args.verbose

    success = compose(cmdline_args.path_to_file)
    if not success:
        print("ERROR! Run with -v to debug")
    else:
        print("Message spec composition done!")