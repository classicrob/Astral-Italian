import io
import json
from pathlib import Path
import tempfile
import unittest

from dolphin_collect import ObserverError
from kpad_observe import decode_sample, ObservedInput, ENTRY, PROBE, CONTROL_BLOCKS, contract


class ObserveTests(unittest.TestCase):
    def test_named_sample_roundtrip_and_nonfinite_rejection(self):
        for connected in (True, False):
            raw = bytes(contract.encode_sample(contract.idle_sample(connected)))
            self.assertEqual(bytes(contract.encode_sample(decode_sample(raw))), raw)
        for sentinel in (252,255):
            sample = contract.idle_sample(False)
            sample['dev_type'] = sentinel
            raw = bytes(contract.encode_sample(sample))
            self.assertEqual(decode_sample(raw)['dev_type'], sentinel)
            self.assertEqual(bytes(contract.encode_sample(decode_sample(raw))), raw)
        raw = bytearray(raw)
        raw[12:16] = bytes.fromhex('7fc00000')
        with self.assertRaises(ObserverError): decode_sample(raw)

    def test_connection_query_covers_original_status_device_ready_branches(self):
        with tempfile.TemporaryDirectory() as directory:
            inputs = ObservedInput(Path(directory)/'inputs.jsonl')
            class Remote:
                status = 0
                device = 1
                ready = 1
                def read(self, address, length):
                    return {CONTROL_BLOCKS:(0x80006000).to_bytes(4,'big'),
                            0x80006900:(self.status & 0xffffffff).to_bytes(4,'big'),
                            0x80006905:bytes([self.device]),
                            0x80006920:self.ready.to_bytes(4,'big')}[address]
            remote = Remote()
            for status, device, ready, expected in ((0,1,1,0),(0,1,0,-2),(0,253,1,-1),(-1,1,1,-1),(-3,1,1,-3)):
                remote.status,remote.device,remote.ready=status,device,ready
                self.assertEqual(inputs.connection(remote,0),dict(probe_error=expected,device_type=device))
            inputs.close(None)

    def test_observed_transition_is_retained_and_post_read_connection_is_exported(self):
        with tempfile.TemporaryDirectory() as directory:
            inputs=ObservedInput(Path(directory)/'inputs.jsonl')
            class Remote:
                regs={3:1,64:0x80005000}
                raw=bytes(contract.encode_sample(contract.idle_sample(True)))
                def reg(self,number): return self.regs[number]
                def ok(self,command):
                    if not command.startswith(('Z0,','z0,')): raise AssertionError('guest write')
                def stop(self,_): self.regs[64]+=4
                def read(self,address,length):
                    return {CONTROL_BLOCKS:(0x80006000).to_bytes(4,'big'),
                            0x80006900:bytes(4),0x80006905:b'\x01',0x80006920:(1).to_bytes(4,'big'),
                            0x80002000:bytes(4),0x80004000:self.raw}[address][:length]
            before=dict(probe_error=-2,device_type=1)
            inputs.pending=[dict(channel=0,pointer=0x80004000,lr=0x80005000,capacity=120,
                                 error=0x80002000,connection=before)]
            inputs.handle(Remote(),0x80005000,536)
            inputs.close(None,True)
            row=json.loads(inputs.output.read_text())
            self.assertEqual(row['connection'],dict(probe_error=0,device_type=1))
            self.assertEqual(inputs.connection_transitions[0]['before'],before)
            self.assertEqual(inputs.connection_transitions[0]['frame'],536)

    def test_tampered_probe_function_fails_before_installing_breakpoints(self):
        with tempfile.TemporaryDirectory() as directory:
            inputs = ObservedInput(Path(directory)/'inputs.jsonl')
            class Remote:
                def read(self,address,length):
                    return bytes.fromhex('38e000014800000c') if address==ENTRY else bytes(length)
                def ok(self,_): raise AssertionError('breakpoint installed before signature check')
            with self.assertRaisesRegex(ObserverError,'complete function signature'):
                inputs.install(Remote())
            inputs.close(None)

    def test_verified_connection_and_kpad_are_observed_without_any_guest_write(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory)/'inputs.jsonl'
            inputs = ObservedInput(output)
            class Remote:
                regs = {3:0, 4:0x80001000, 5:120, 6:0x80002000, 64:PROBE, 67:0x80003000}
                memory = {CONTROL_BLOCKS:(0x80006000).to_bytes(4,'big'),
                          0x80006900:bytes(4), 0x80006905:b'\x01', 0x80006920:(1).to_bytes(4,'big')}
                def reg(self, number): return self.regs[number]
                def ok(self, command):
                    if not command.startswith(('Z0,','z0,')):
                        raise AssertionError('observer issued guest write')
                def stop(self, _): self.regs[64] += 4
                def read(self, address, length): return self.memory[address][:length]
            remote=Remote()
            inputs.breakpoints={ENTRY}
            remote.regs.update({3:0,4:0x80004000,64:ENTRY,67:0x80005000})
            inputs.handle(remote,ENTRY,0)
            raw=bytes(contract.encode_sample(contract.idle_sample(True)))
            remote.memory.update({0x80004000:raw,0x80002000:bytes(4)})
            remote.regs.update({3:1,64:0x80005000})
            inputs.handle(remote,0x80005000,0)
            inputs.end_frame()
            inputs.close(remote,True)
            row=json.loads(output.read_text())
            self.assertEqual(row['connection'],dict(probe_error=0,device_type=1))
            self.assertEqual((row['frame'],row['read_ordinal'],row['returned_count']), (0,0,1))
            tape=io.BytesIO()
            contract.encode_rows([row],1,tape)
            self.assertTrue(tape.getvalue().startswith(b'SMG2KPD1'))


if __name__=='__main__': unittest.main()
