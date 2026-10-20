import socket
import struct
import threading
import unittest
import re

from dolphin_collect import COLUMNS, Memory, ObserverError, ROOT, RSP, Snapshot, private_path, game_private_path, REPO


class FakeRAM:
    def __init__(self):
        self.data = bytearray(0x10000)
        self.reads = []

    def read(self, address, length):
        self.reads.append((address, length))
        if address <= ROOT and ROOT + 4 <= address + length:
            data = bytearray(length)
            data[ROOT - address:ROOT - address + 4] = struct.pack(">I", 0x80001000)
            return bytes(data)
        offset = address - 0x80000000
        if not 0 <= offset <= len(self.data) - length:
            raise ObserverError("unmapped fake read")
        return bytes(self.data[offset:offset + length])

    def put(self, address, data):
        offset = address - 0x80000000
        self.data[offset:offset + len(data)] = data

    def word(self, address, value):
        self.put(address, struct.pack(">I", value))


class SnapshotTests(unittest.TestCase):
    def setUp(self):
        self.ram = FakeRAM()
        self.ram.word(0x80001020, 0x80002000)
        self.ram.word(0x80001024, 0x80003000)
        self.ram.word(0x80002034, 0xDEADBEEF)
        self.ram.put(0x80003000, b"Title\0")
        self.observer = Snapshot(self.ram)

    def player(self):
        r = self.ram
        r.put(0x80003000, b"Game\0")
        r.put(0x80003040, b"TestGalaxy\0")
        r.word(0x80003130, 6)
        for address, pointer in ((0x8000312C, 0x80004000), (0x80004010, 0x80004100),
                                 (0x80004154, 0x80004500), (0x80004514, 0x80005000),
                                 (0x80005584, 0x80006000), (0x80006138, 0x80008000),
                                 (0x80006880 + 0x100, 0x80009000)):
            r.word(address, pointer)
        # State stack address is P+0x980.
        r.word(0x80006980, 0x80009000)
        r.word(0x8000900C, 0)
        r.put(0x80005014, struct.pack(">fff", 1.25, -2.5, 3.0))
        r.word(0x800056B0, 3)

    def test_title_preserves_loop_and_rng_with_invalid_player(self):
        row = self.observer.take(7)
        self.assertEqual((row["frame"], row["scene"], row["rng_seed"]), (7, "Title", 0xDEADBEEF))
        self.assertEqual(row["scene_valid"], 1)
        self.assertEqual(row["player_valid"], 0)
        self.assertEqual(row["x"], "")
        self.assertEqual(row["observer_error"], "")

    def test_uninitialized_noticed_callback_scratch_is_unavailable_at_frame_seam(self):
        self.player()
        self.ram.word(0x80006984, 0x00780620)
        row = self.observer.take(0)
        self.assertEqual(row["observer_error"], "")
        self.assertEqual(row["player_valid"], 1)
        self.assertEqual(row["state_ids"], "[0]")
        self.assertEqual(row["pending_state_id"], "")

    def test_player_zero_state_id_is_distinct_from_no_pending_state(self):
        self.player()
        row = self.observer.take(0)
        self.assertEqual(row["observer_error"], "")
        self.assertEqual(row["player_valid"], 1)
        self.assertEqual(row["state_ids"], "[0]")
        self.assertEqual(row["pending_state_id"], "")
        self.assertEqual(row["x"], "1.25")
        self.assertEqual(row["x_bits"], "0x3fa00000")
        self.assertEqual(row["hp"], 3)

    def test_snapshot_pages_are_reused_only_until_the_next_frame(self):
        self.player()
        first = self.observer.take(0)
        self.assertEqual(first['observer_error'], '')
        self.assertEqual(len(self.ram.reads), len(set(self.ram.reads)))
        self.assertLess(len(self.ram.reads), 12)
        self.ram.put(0x80005014, struct.pack('>f', 4.5))
        second = self.observer.take(1)
        self.assertEqual(second['x'], '4.5')
        self.assertEqual(second['observer_error'], '')

    def test_cycle_emits_error_instead_of_silently_truncating(self):
        self.player()
        self.ram.word(0x80009008, 0x80009000)
        row = self.observer.take(2)
        self.assertIn("cyclic", row["observer_error"])
        self.assertEqual(row["player_valid"], 0)

    def test_nonfinite_is_observer_error(self):
        self.player()
        self.ram.put(0x80005014, struct.pack(">f", float("nan")))
        self.assertIn("nonfinite", self.observer.take(0)["observer_error"])

    def test_null_root_and_loading_are_not_errors(self):
        self.ram.word(0x80001024, 0)
        row = self.observer.take(4)
        self.assertEqual(row["scene_valid"], 0)
        self.assertEqual(row["observer_error"], "")

    def test_invalid_nonnull_pointer_fails(self):
        self.ram.word(0x80001024, 1)
        self.assertIn("bad guest pointer", self.observer.take(0)["observer_error"])

    def test_scene_epoch_does_not_change_each_frame(self):
        first = self.observer.take(0)
        self.assertEqual(first["scene_epoch"], self.observer.take(1)["scene_epoch"])
        self.ram.put(0x80003000, b"Game\0")
        self.assertEqual(self.observer.take(2)["scene_epoch"], first["scene_epoch"] + 1)

    def test_bad_actor_counts_fail_before_dereferencing_array(self):
        self.player()
        self.ram.word(0x80004100 + 4 * 0x39, 0x8000A000)
        self.ram.word(0x8000A014, 2)
        self.ram.word(0x8000A018, 3)
        self.assertIn("actor count", self.observer.take(0)["observer_error"])

    def test_actor_dead_byte_is_boolean(self):
        self.player()
        self.ram.put(0x80005070, b"\x02")
        self.assertIn("nonboolean", self.observer.take(0)["observer_error"])

    def test_actor_array_counts_live_members_and_rejects_null_entries(self):
        self.player()
        r = self.ram
        r.word(0x80004100 + 4 * 0x39, 0x8000A000)
        r.word(0x8000A014, 2)
        r.word(0x8000A018, 2)
        r.word(0x8000A01C, 0x8000B000)
        r.word(0x8000B000, 0x8000C000)
        r.word(0x8000B004, 0x8000D000)
        r.put(0x8000D070, b"\x01")
        row = self.observer.take(0)
        self.assertEqual(row["observer_error"], "")
        self.assertEqual((row["actor_registered"], row["actor_living"]), (2, 1))
        r.word(0x8000B004, 0)
        self.assertIn("bad guest pointer", self.observer.take(1)["observer_error"])

    def test_private_outputs_cannot_be_inside_repo(self):
        with self.assertRaises(ObserverError):
            private_path(str(REPO / "build/game-data/trace.csv"))
        with self.assertRaises(ObserverError):
            game_private_path("~/MacioGalaxy2-team/scratch-port/solc/dolphin-user")

    def test_column_contract_matches_native_harness(self):
        columns = []
        for kind, name in re.findall(r"^(TEXT|FLOAT)\((\w+)\)$", (REPO / "native/app/headless/trace_fields.inc").read_text(), re.M):
            columns.append(name)
            if kind == "FLOAT":
                columns.append(name + "_bits")
        self.assertEqual(COLUMNS, columns)

    def test_animation_name_can_have_byte_alignment(self):
        self.player()
        self.ram.word(0x80005588, 0x8000B000)
        self.ram.word(0x8000B114, 0x8000C001)
        self.ram.put(0x8000C001, b"Wait\0")
        row = self.observer.take(0)
        self.assertEqual(row["observer_error"], "")
        self.assertEqual((row["animation_valid"], row["anim_name"]), (1, "Wait"))


class ProtocolTests(unittest.TestCase):
    def exchange(self, response, operation, corrupt=False):
        client, server = socket.socketpair()
        client.settimeout(1)
        received = []

        def peer():
            command = bytearray()
            while not command.endswith(b"#"):
                command += server.recv(1)
            command += server.recv(2)
            received.append(bytes(command))
            data = response.encode()
            packet = b"$" + data + b"#" + f"{sum(data) & 255:02x}".encode()
            if corrupt:
                server.sendall(b"+$" + data + b"#00")
                received.append(server.recv(1))
            for char in b"+" + packet:
                server.sendall(bytes([char]))
            received.append(server.recv(1))

        thread = threading.Thread(target=peer)
        thread.start()
        try:
            value = operation(RSP(client))
        finally:
            thread.join(2)
            client.close()
            server.close()
        return value, received

    def test_large_coalesced_packets_are_buffered_without_bytewise_receives(self):
        def packet(text):
            data = text.encode()
            return b'$' + data + b'#' + f'{sum(data) & 255:02x}'.encode()
        class Stream:
            calls = 0
            def __init__(self):
                self.data = b'+' + packet('ab' * 4096) + packet('T05')
            def recv(self, size):
                self.calls += 1
                data, self.data = self.data[:size], self.data[size:]
                return data
            def sendall(self, _): pass
        stream = Stream()
        remote = RSP(stream)
        self.assertEqual(remote.receive(), 'ab' * 4096)
        self.assertEqual(remote.receive(), 'T05')
        self.assertEqual(stream.calls, 1)

    def test_fragmented_reply_and_rejected_checksum(self):
        value, traffic = self.exchange("1234abcd", lambda r: r.reg(64), corrupt=True)
        self.assertEqual(value, 0x1234ABCD)
        self.assertEqual(traffic[1:], [b"-", b"+"])
        self.assertEqual(traffic[0], b"$p40#d4")

    def test_read_error_is_not_decoded_as_guest_data(self):
        with self.assertRaises(ObserverError):
            self.exchange("E00", lambda r: r.read(0x80000000, 4))

    def test_short_memory_reply_is_not_copied_into_error_log(self):
        try:
            self.exchange("abcdef", lambda r: r.read(0x80000000, 4))
        except ObserverError as exc:
            self.assertNotIn("abcdef", str(exc))
        else:
            self.fail("short memory reply was accepted")

    def test_unexpected_stop_signal_fails(self):
        with self.assertRaises(ObserverError):
            self.exchange("T0b", lambda r: r.stop("c"))

    def test_startup_sigterm_is_accepted_only_at_startup(self):
        self.exchange("T0f40:80004050;", lambda r: r.stop("?", startup=True))
        with self.assertRaises(ObserverError):
            self.exchange("T0f", lambda r: r.stop("c"))


if __name__ == "__main__":
    unittest.main()
