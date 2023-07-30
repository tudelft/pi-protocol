#!/usr/bin/env python3

import yaml
try:
    from yaml import CLoader as Loader, CDumper as Dumper
except ImportError:
    from yaml import Loader, Dumper

from jinja2 import Environment, FileSystemLoader
import os
import re
import semver

MSGS_DIR = "msgs"
CONFIG_FILE_STEM = "config"
CONFIG_FILE_REGEX = re.compile(rf"{CONFIG_FILE_STEM}.y[a]?ml")

# beun: parse ID and PAYLOAD limits from protocol.h.j2
for line in open("pi-protocol.h.j2").readlines():
    parts = line.split()
    if len(parts) != 3:
        continue

    if (parts[0] == "#define") and (parts[1] == "PI_MSG_MAX_ID"):
        # should automatically recognize base16 due to 0x
        PI_MSG_MAX_ID = int(parts[2], 0)
    elif (parts[0] =="#define" and (parts[1] == "PI_MSG_MAX_PAYLOAD_LEN")):
        PI_MSG_MAX_PAYLOAD_LEN = int(parts[2], 0)

DATATYPE_LENGTHS = {
    'uint8_t': 1,
    'uint16_t': 2,
    'uint32_t': 4,
    'uint64_t': 8,
    # 'short': 2, # may work on aarch64 with gcc: https://gcc.gnu.org/onlinedocs/gcc/Half-Precision.html
    'float': 4,
    'double': 8,
}

if __name__ == "__main__":
    # for checking duplicate ids
    id_list = [] 

    # jinja2 data structure
    data = {}

    # load msgList
    configCandidates = [x for x in os.listdir() \
                if CONFIG_FILE_REGEX.match(x)]
    if len(configCandidates) != 1:
        raise ValueError(f"Exactly one file {CONFIG_FILE_STEM}.yaml or {CONFIG_FILE_STEM}.yml must be present in root directory")
    config = yaml.load(open(configCandidates[0], 'r'), Loader)

    # parse version number string
    config['version'] = {}
    version = semver.Version.parse(config['protocol_version'])
    config['version']['major'] = version.major
    config['version']['minor'] = version.minor
    config['version']['patch'] = version.patch

    # check id range and duplicates
    for _, msg in config['include_messages'].items():
        if msg['id'] in id_list:
            raise ValueError(f"Your {CONFIG_FILE_STEM}.y(a)ml contains messages with duplicate ids!")
        id_list.append(msg['id'])
        if (msg['id'] < 0x00) or (msg['id'] > PI_MSG_MAX_ID):
            raise ValueError(f"Your {CONFIG_FILE_STEM}.y(a)ml contains messages with ids outside the range 0x00 and PI_MSG_MAX_ID ({PI_MSG_MAX_ID:#x})!")

    # load definition of all required messages and calculate payload length
    msgs = []
    for msgNAME in config['include_messages'].keys():
        print(f"Reading file {msgNAME}.y(a)ml")
        msgCandidates = [x for x in os.listdir(MSGS_DIR) \
                        if re.compile(rf'{msgNAME}.y[a]?ml').match(x)]

        if len(msgCandidates) != 1:
            raise ValueError(f"Your {CONFIG_FILE_STEM}.y(a)ml requires you to provide exactly one file {msgNAME}.yaml or {msgNAME}.yml in {MSGS_DIR}")

        # load yaml definition
        msgDefinition = yaml.load(
            open(os.path.join(MSGS_DIR, msgCandidates[0]), 'r'),
            Loader)

        # parse name
        msgDefinition['nameSNAKE_CAPS'] = msgNAME
        msgDefinition['nameCamelCase'] = \
            msgNAME.replace("_"," ").title().replace(" ","")

        # calculate payload name
        msgDefinition['payloadLen'] = 0
        for field, dtype in msgDefinition['fields'].items():
            msgDefinition['payloadLen'] += DATATYPE_LENGTHS[dtype]

        if msgDefinition['payloadLen'] > PI_MSG_MAX_PAYLOAD_LEN:
            raise ValueError(f"Sum of payload bytes of message definition {os.path.join(MSGS_DIR,msgNAME)}.y(a)ml exceed PI_MSG_MAX_PAYLOAD_LEN ({PI_MSG_MAX_PAYLOAD_LEN:#x})")

        msgs.append(msgDefinition)

    # combine to single data structure and give debug output
    data['config'] = config
    data['msgs'] = msgs
    print(f"Data structure read: {data}")

    # Setup jinja2
    env = Environment(loader = FileSystemLoader('./'),
                      trim_blocks=True,
                      lstrip_blocks=True,
                      )

    # Generate headers!
    # protocol: fill in global mode and version number
    # messages: generate all message data-types, packing and parsing logic
    protocol_template = env.get_template('pi-protocol.h.j2')
    messages_template = env.get_template('pi-messages.h.j2')

    with open("pi-protocol.h", 'w') as protocol_header:
        print("Writing out protocol header...")
        protocol_header.write(protocol_template.render(data))

    with open("pi-messages.h", 'w') as messages_header:
        print("Writing out messages header...")
        messages_header.write(messages_template.render(data))




