"""
Simple SysEx tester for MMM device on Windows using pyserial.
Sends SetDistributor (0x50) for distributor 0 with a 24-byte payload, then sends GetDistributor (0x40) and captures responses.
Saves raw bytes to test/sysex_log.txt

Usage: (requires pyserial)
    python test\sysex_test.py COM3

Note: Script runs at 115200 baud and waits briefly for device to reboot after upload.
"""
import sys
import time
import serial

# Addresses and constants (must match Constants.h)
SYSEX_START = 0xF0
SYSEX_END = 0xF7
SYSEX_ID = 0x7D

# Helper: build 14-bit id into MSB/LSB (7-bit each)
def id_to_bytes(id14):
    msb = (id14 >> 7) & 0x7F
    lsb = id14 & 0x7F
    return [msb, lsb]

# Build midi SysEx: F0, SYSEX_ID, srcMSB, srcLSB, dstMSB, dstLSB, cmd, payload..., F7
def build_sysex(src_id, dst_id, cmd, payload_bytes):
    data = [SYSEX_START, SYSEX_ID]
    data += id_to_bytes(src_id)
    data += id_to_bytes(dst_id)
    data.append(cmd & 0x7F)
    # payload must be 7-bit bytes
    data += [b & 0x7F for b in payload_bytes]
    data.append(SYSEX_END)
    return bytes(data)

# Expected platform defaults
BAUD = 115200

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('Usage: sysex_test.py <COM_PORT> [DEST_HEX]')
        print('Example: sysex_test.py COM3 0x42')
        sys.exit(1)

    port = sys.argv[1]
    # Optional destination id (hex or decimal). Default uses Constants.h device id (0x0001)
    if len(sys.argv) >= 3:
        dst = int(sys.argv[2], 0)
    print('Opening', port)
    ser = serial.Serial(port, BAUD, timeout=1)
    time.sleep(0.2)

    # Use host/source id = 0x3FFF (server) per V1 tests
    src = 0x3FFF
    # dst may have been overridden by CLI; otherwise default to 0x0001
    try:
        dst
    except NameError:
        dst = 0x0001  # device default in Constants.h

    # Build SetDistributor (SYSEX_SetDistributorConstruct = 0x50) for distributor 0
    # Payload: [ID MSB, ID LSB, then 22 bytes of config] => total 24 bytes
    set_cmd = 0x50
    dist_id = 0
    payload = id_to_bytes(dist_id)
    # Append 22 arbitrary config bytes (within 0-127)
    payload += [10] * 22

    set_msg = build_sysex(src, dst, set_cmd, payload)
    print('Sending SetDistributor (len=%d): %s' % (len(set_msg), set_msg.hex()))
    ser.write(set_msg)
    ser.flush()
    time.sleep(0.5)

    # Now send GetDistributor (SYSEX_GetDistributorConstruct = 0x40) for id 0
    get_cmd = 0x40
    get_payload = [0]  # distributor id
    get_msg = build_sysex(src, dst, get_cmd, get_payload)
    print('Sending GetDistributor (len=%d): %s' % (len(get_msg), get_msg.hex()))
    ser.write(get_msg)
    ser.flush()

    # Read for a short window to capture response
    start = time.time()
    responses = []
    print('Listening for responses (2s)...')
    while time.time() - start < 2.0:
        b = ser.read(1)
        if not b:
            continue
        # read until F7
        if b[0] == SYSEX_START:
            buf = bytearray([b[0]])
            while True:
                nb = ser.read(1)
                if not nb:
                    break
                buf.append(nb[0])
                if nb[0] == SYSEX_END:
                    break
            print('Received SysEx:', buf.hex())
            responses.append(bytes(buf))
    
    # Save log
    with open('test/sysex_log.txt', 'wb') as f:
        for r in responses:
            f.write(r + b'\n')

    print('Done. Captured %d SysEx responses. Log: test/sysex_log.txt' % len(responses))
    ser.close()
