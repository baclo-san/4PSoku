"""Stand-in spectator for testing the relay's broadcast mode without the game.

    python fake_spectator.py make <file.rep.4p>     write a synthetic version-4 replay
    python fake_spectator.py run <port>             talk to a relay on 127.0.0.1:<port>
    python fake_spectator.py live <port>            four fake players feed a live match
                                                    to a relay (no --broadcast); then watch it
"""
import socket, struct, sys, time, zlib

GAME_ID = bytes([0x41, 0xD0, 0x3B, 0x30, 0x64, 0x41, 0x74, 0xC8, 0xC6, 0x24, 0x8C, 0xA4, 0x15, 0x44, 0x32, 0x9B])
FRAMES = 600


def player(chr_, pal):
    cards = list(range(100, 120))
    return struct.pack("<4B20H", chr_, pal, 0, 20, *cards)


def frame_input(f, p):
    return (f * 7 + p * 3) & 0xFFF


def make(path):
    hdr = b"4PR2" + struct.pack("<II", 4, FRAMES) + player(3, 1) + player(9, 2)
    setup = player(7, 0) + player(1, 3) + bytes([5, 6, 0, 0, 0, 0]) + struct.pack("<H", 0) + struct.pack("<I", 0xDEADBEEF)
    assert len(hdr) == 100 and len(setup) == 100
    body = b"".join(struct.pack("<4H", frame_input(f, 2), frame_input(f, 3), frame_input(f, 0), frame_input(f, 1)) for f in range(FRAMES))
    open(path, "wb").write(hdr + setup + body)
    print("wrote", path)


def run(port):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.settimeout(0.5)
    addr = ("127.0.0.1", port)
    name = b"tester"
    s.sendto(bytes([5]) + GAME_ID + bytes(8) + bytes([0, len(name)]) + name, addr)

    def recv_until(pred, what, timeout=3):
        end = time.time() + timeout
        while time.time() < end:
            try:
                d, _ = s.recvfrom(4096)
            except socket.timeout:
                continue
            if pred(d):
                return d
        raise SystemExit("FAIL: no " + what)

    d = recv_until(lambda d: d[0] == 6, "INIT_SUCCESS")
    names = [d[13 + 32 * i:13 + 32 * i + 32].split(b"\0")[0] for i in range(2)]
    print("INIT_SUCCESS, names", names, "size", len(d))
    assert d[5] & 1, "INIT_SUCCESS does not flag the client as a spectator"

    # Stale match id: the relay must answer with GAME_MATCH.
    s.sendto(bytes([0x0E, 0x0B]) + struct.pack("<IB", 0, 0), addr)
    d = recv_until(lambda d: d[0] == 0x0D and d[1] == 4, "GAME_MATCH")
    off = 2
    chars = []
    for _ in range(4):
        c, pal, deck, n = d[off:off + 4]
        chars.append((c, pal, n))
        off += 4 + 2 * n + 1
    stage, music = d[off], d[off + 1]
    seed, match = struct.unpack_from("<IB", d, off + 2)
    print("GAME_MATCH chars", chars, "stage", stage, "music", music, "seed %08x" % seed, "match", match)
    assert [c[0] for c in chars] == [7, 1, 3, 9] and seed == 0xDEADBEEF and stage == 5, "setup wrong"

    time.sleep(1.0)  # let ~60 frames be released
    s.sendto(bytes([0x0E, 0x0B]) + struct.pack("<IB", 10, match), addr)
    d = recv_until(lambda d: d[0] == 0x0D and d[1] == 9, "GAME_REPLAY")
    size = d[2]
    body = zlib.decompress(d[3:3 + size])
    after, end, m, n = struct.unpack_from("<IIBB", body)
    inputs = struct.unpack_from("<%dH" % n, body, 10)
    print("GAME_REPLAY after", after, "end", end, "match", m, "count", n)
    for k, v in enumerate(inputs):
        entry = after - 1 - k
        want = frame_input(entry // 2, entry % 2) & 0x3FF
        assert v == want, ("entry", entry, v, want)
    assert after == 10 + n

    s.sendto(bytes([0x11, match]) + struct.pack("<I", 0), addr)
    d = recv_until(lambda d: d[0] == 0x10, "SPECTATOR_INPUTS")
    m, first, count, ended = struct.unpack_from("<BIHB", d, 1)
    vals = struct.unpack_from("<%dH" % (4 * count), d, 9)
    for f in range(count):
        for p in range(4):
            assert vals[4 * f + p] == frame_input(first + f, p), (f, p)
    print("SPECTATOR_INPUTS match", m, "first", first, "count", count, "ended", ended)
    print("ALL OK")


def recv_from(s, pred, what, timeout=3):
    end = time.time() + timeout
    while time.time() < end:
        try:
            d, _ = s.recvfrom(4096)
        except socket.timeout:
            continue
        if pred(d):
            return d
    raise SystemExit("FAIL: no " + what)


def live(port):
    addr = ("127.0.0.1", port)
    chars = [7, 1, 3, 9]
    players = []
    for i in range(4):
        p = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        p.settimeout(0.2)
        name = b"player%d" % (i + 1)
        # The game sends its name field zero-padded to 32 bytes.
        p.sendto(bytes([5]) + GAME_ID + bytes(8) + bytes([1, len(name)]) + name.ljust(32, bytes(1)), addr)
        recv_from(p, lambda d: d[0] == 6, "INIT_SUCCESS for player %d" % (i + 1))
        p.sendto(bytes([0x0A, i]), addr)  # PLAYER_JOIN_ACK: take slot i
        recv_from(p, lambda d: d[0] == 0x0A and d[1] == i, "slot %d" % i)
        players.append(p)
    for i, p in enumerate(players):
        deck = list(range(100, 120))
        p.sendto(bytes([0x0F, chars[i], 5, 6, i, 0, 0, 20]) + struct.pack("<20H", *deck), addr)
    seed = None
    for i, p in enumerate(players):
        p.sendto(bytes([0x0E, 5]), addr)  # GAME_MATCH_ACK
        d = recv_from(p, lambda d: d[0] == 0x0D and d[1] == 4, "GAME_MATCH for player %d" % (i + 1))
        off = 2
        for _ in range(4):
            off += 4 + 2 * d[off + 3] + 1
        seed = struct.unpack_from("<I", d, off + 2)[0]
    print("players in the match, seed %08x" % seed)

    def feed(p, first, count):
        body = b"".join(struct.pack("<4H", *(frame_input(f, k) for k in range(4))) for f in range(first, first + count))
        p.sendto(bytes([0x12]) + struct.pack("<IIH", seed, first, count) + body, addr)

    feed(players[0], 0, 32)
    feed(players[1], 64, 32)   # a gap: 32..63 not yet in
    feed(players[2], 16, 32)   # overlaps, fills 32..47
    feed(players[3], 0, 5)
    feed(players[0], 0, 8)     # wrong seed below must be ignored
    players[0].sendto(bytes([0x12]) + struct.pack("<IIH", seed ^ 1, 48, 16) + bytes(16 * 8), addr)
    time.sleep(0.3)

    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.settimeout(0.2)
    s.sendto(bytes([5]) + GAME_ID + bytes(8) + bytes([0, 6]) + b"tester", addr)
    d = recv_from(s, lambda d: d[0] == 6, "INIT_SUCCESS for the spectator")
    assert d[5] & 1, "not flagged as a spectator"
    names = [d[o:o + 32].split(bytes(1))[0] for o in (13, 45, 81, 113)]
    print("spectator in, names", names)
    assert names == [b"player1", b"player2", b"player3", b"player4"], names
    s.sendto(bytes([0x0E, 0x0B]) + struct.pack("<IB", 0xFFFFFFFF, 0), addr)
    d = recv_from(s, lambda d: d[0] == 0x0D and d[1] == 4, "GAME_MATCH for the spectator")
    match = d[-1]
    print("spectator got the match, id", match)
    assert match != 0

    s.sendto(bytes([0x11, match]) + struct.pack("<I", 0), addr)
    d = recv_from(s, lambda d: d[0] == 0x10, "SPECTATOR_INPUTS")
    m, first, count, ended = struct.unpack_from("<BIHB", d, 1)
    print("SPECTATOR_INPUTS match", m, "first", first, "count", count, "(48 expected: 0..47, 48..63 missing)")
    assert (m, first, count) == (match, 0, 48), (m, first, count)
    vals = struct.unpack_from("<%dH" % (4 * count), d, 9)
    for f in range(count):
        for k in range(4):
            assert vals[4 * f + k] == frame_input(f, k), (f, k)

    feed(players[3], 40, 32)   # closes the gap
    time.sleep(0.2)
    s.sendto(bytes([0x11, match]) + struct.pack("<I", 48), addr)
    d = recv_from(s, lambda d: d[0] == 0x10, "SPECTATOR_INPUTS after the gap")
    m, first, count, ended = struct.unpack_from("<BIHB", d, 1)
    print("after the gap closed: first", first, "count", count)
    assert (first, count) == (48, 48), (first, count)
    print("ALL OK")


if __name__ == "__main__":
    {"make": lambda: make(sys.argv[2]), "run": lambda: run(int(sys.argv[2])), "live": lambda: live(int(sys.argv[2]))}[sys.argv[1]]()
