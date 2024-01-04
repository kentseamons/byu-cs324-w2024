#!/usr/bin/python3

import argparse
import hashlib
import random
import re
import socket
import subprocess
import struct
import sys
import threading
import time

NUM_LEVELS = 5
SEEDS = [7719, 33833, 20468, 19789, 59455]
CLIENT = './treasure_hunter'
BYTES_MINUS_CHUNK = 8
TIMEOUT = 20

LEVEL_SCORES = { 0: 50, 1: 15, 2: 15, 3: 15, 4: 5 }

SUMS = ['127624217659f4ba97d5457391edc8f60758714b',
        '2483b89fefaee5a83c25ba92dda9bd004357d6b1',
        '285e8e43bf9d8b7204f6972a3be88b8a599b068d',
        '3334488d5b819492e13335105df59164acbf98a0',
        '384835f2469dbb37a6eb0bbf6f66e45677f61423',
        '3f362e32653be4ab829fd7b9838fdfe71c01385d',
        '5bc7244d527b79d2625d51ab514f0412d22acc2c',
        '7514a7a267acdfed8de2bcf0729a2037035c3acd',
        'b156795f7b657f2fe33639b4f0bb3f6193960f79',
        'c2f7e1078c91ab8ce0674680e7d2e7ab9a335a06',
        'c41d89fe9ebac2cf06fc8e3f0f0f8335ec9dce8f',
        'c657aeed6d2c2eeb40382898cdd4f3d25182c719',
        'd29e5524dda590d2eaf097e9e32b53cb9770e965',
        'dec427a457cc1cba9533630a2c511cd5f21aa1f0',
        'fee33676b4c20e5b267ba40b2eba3c2c7ee3d260',
        'ffb54044122cb1883513849d396cf144af3e0ed4']

RECV_RE = re.compile('^recv(from)?.* = (\d+)$')

level_seed_result = (False, False, None, None)

def tmp_server(port):
    global level_seed_result
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.settimeout(1)
    s.bind(('127.0.0.1', port))

    try:
        (buf, addr) = s.recvfrom(65536)
    except socket.timeout:
        return
    if len(buf) != 8:
        level_seed_result = (True, False, None, None)
        return
    level, userid, seed = struct.unpack('!HIH', buf[:8])
    level_seed_result = (True, True, level, seed)

def test_level_seed(level, seed):
    global level_seed_result
    port = random.randint(1024, 65535)
    level_seed_result = (None, None)
    t = threading.Thread(target=tmp_server, args=(port,))
    t.start()
    cmd = [CLIENT, '127.0.0.1', str(port), str(level), str(seed)]
    p = subprocess.Popen(cmd, stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL)
    t.join()
    p.kill()
    if not level_seed_result[0]:
        return 'Port provided on command line is not used by client'
    if not level_seed_result[1]:
        return 'Initial message length invalid'
    if level_seed_result[2] != level:
        return 'Level provided on command line is not sent by client'
    if level_seed_result[3] != seed:
        return 'Seed provided on command line is not sent by client'
    return ''

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('server', action='store', type=str)
    parser.add_argument('port', action='store', type=int)
    parser.add_argument('level', action='store', type=int,
        nargs='?', choices=range(NUM_LEVELS))
    args = parser.parse_args(sys.argv[1:])

    if args.level is None:
        levels = range(NUM_LEVELS)
    else:
        levels = [args.level]

    score = 0
    max_score = 0
    for level in levels:
        sys.stdout.write(f'Testing level {level}:\n')
        for seed in SEEDS:
            max_score += LEVEL_SCORES[level] / len(SEEDS)
            sys.stdout.write(f'    Seed %5d:' % (seed))
            sys.stdout.flush()

            warn_msg = test_level_seed(level, seed)

            cmd = ['strace', '-e', 'trace=%network',
                    CLIENT, args.server, str(args.port), str(level), str(seed)]
            try:
                p = subprocess.run(cmd,
                        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                        timeout=TIMEOUT)
            except subprocess.TimeoutExpired:
                treasure = b''
                strace_output = b''
                h = ''
            else:
                treasure = p.stdout
                strace_output = p.stderr
                h = hashlib.sha1(treasure).hexdigest()

            tot_bytes = 0
            output = strace_output.decode('utf-8').strip()
            for line in output.splitlines():
                # skip DNS lookups
                if 'htons(53)' in line:
                    continue
                m = RECV_RE.search(line)
                if m is not None:
                    received_bytes = int(m.group(2))
                    if received_bytes > 1:
                        tot_bytes += received_bytes - BYTES_MINUS_CHUNK
            
            allowed_lengths = [tot_bytes]
            if treasure and treasure[-1] == b'\n':
                allowed_lengths += 1

            if h in SUMS and tot_bytes in allowed_lengths:
                score += LEVEL_SCORES[level] / len(SEEDS)
                sys.stdout.write(f' PASSED')
            else:
                sys.stdout.write(f' FAILED')
            if warn_msg:
                sys.stdout.write(f' (warning: {warn_msg})')
            sys.stdout.write('\n')
            
    print(f'Score: {score}/{max_score}')
            
if __name__ == '__main__':
    main()
