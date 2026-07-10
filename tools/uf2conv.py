#!/usr/bin/env python3

import argparse
import struct
import sys


UF2_MAGIC_START0 = 0x0A324655
UF2_MAGIC_START1 = 0x9E5D5157
UF2_MAGIC_END = 0x0AB16F30
UF2_FLAG_FAMILY_ID_PRESENT = 0x00002000
UF2_PAYLOAD_SIZE = 256


def parse_int(value):
    return int(value, 0)


def parse_hex_line(line, line_number):
    line = line.strip()
    if not line:
        return None
    if not line.startswith(":"):
        raise ValueError(f"line {line_number}: missing ':'")

    try:
        raw = bytes.fromhex(line[1:])
    except ValueError as exc:
        raise ValueError(f"line {line_number}: invalid hex") from exc

    if len(raw) < 5:
        raise ValueError(f"line {line_number}: record is too short")

    count = raw[0]
    if len(raw) != count + 5:
        raise ValueError(f"line {line_number}: record length mismatch")

    if (sum(raw) & 0xFF) != 0:
        raise ValueError(f"line {line_number}: checksum mismatch")

    address = (raw[1] << 8) | raw[2]
    record_type = raw[3]
    payload = raw[4:4 + count]
    return record_type, address, payload


def read_ihex(filename):
    data = {}
    upper_address = 0

    with open(filename, "r", encoding="ascii") as hex_file:
        for line_number, line in enumerate(hex_file, 1):
            record = parse_hex_line(line, line_number)
            if record is None:
                continue

            record_type, address, payload = record
            if record_type == 0x00:
                absolute = upper_address + address
                for offset, value in enumerate(payload):
                    data[absolute + offset] = value
            elif record_type == 0x01:
                break
            elif record_type == 0x02:
                upper_address = int.from_bytes(payload, "big") << 4
            elif record_type == 0x04:
                upper_address = int.from_bytes(payload, "big") << 16
            elif record_type in (0x03, 0x05):
                continue
            else:
                raise ValueError(f"line {line_number}: unsupported record type {record_type:#x}")

    if not data:
        raise ValueError("input HEX contains no data records")

    return data


def build_blocks(data):
    blocks = {}
    for address, value in data.items():
        block_address = address & ~(UF2_PAYLOAD_SIZE - 1)
        if block_address not in blocks:
            blocks[block_address] = bytearray([0xFF] * UF2_PAYLOAD_SIZE)
        blocks[block_address][address - block_address] = value

    return sorted(blocks.items())


def write_uf2(blocks, family_id, output):
    total = len(blocks)
    with open(output, "wb") as uf2_file:
        for index, (address, payload) in enumerate(blocks):
            header = struct.pack(
                "<IIIIIIII",
                UF2_MAGIC_START0,
                UF2_MAGIC_START1,
                UF2_FLAG_FAMILY_ID_PRESENT,
                address,
                UF2_PAYLOAD_SIZE,
                index,
                total,
                family_id,
            )
            block = header + bytes(payload)
            block += bytes(512 - len(block) - 4)
            block += struct.pack("<I", UF2_MAGIC_END)
            uf2_file.write(block)


def main(argv):
    parser = argparse.ArgumentParser(description="Convert an Intel HEX file to UF2.")
    parser.add_argument("-f", "--family", type=parse_int, required=True, help="UF2 family ID")
    parser.add_argument("-o", "--output", required=True, help="output UF2 file")
    parser.add_argument("input", help="input Intel HEX file")
    args = parser.parse_args(argv)

    data = read_ihex(args.input)
    blocks = build_blocks(data)
    write_uf2(blocks, args.family, args.output)
    print(f"Wrote {args.output} ({len(blocks)} UF2 blocks)")


if __name__ == "__main__":
    main(sys.argv[1:])
