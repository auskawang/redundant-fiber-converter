import unittest
from .engine import Engine


def advance(e, seconds):
    for _ in range(round(seconds * 10)):
        e.step(.1)
        for board in e.boards.values():
            assert board['uplink'] in (None, 0, 1)


class SimulationTests(unittest.TestCase):
    def test_healthy_traffic(self):
        e = Engine(); advance(e, 3)
        self.assertEqual(e.traffic['throughput_mbps'], 900)
        self.assertEqual(e.failovers, 0)

    def test_transient_does_not_switch(self):
        e = Engine(); e.links[0]['cut'] = True; advance(e, .2)
        e.links[0]['cut'] = False; advance(e, 3)
        self.assertEqual(e.active, 0)
        self.assertEqual(e.failovers, 0)

    def test_directional_fade_persistence_and_handshake(self):
        e = Engine(); e.links[0]['attenuation']['B'] = 10
        advance(e, 1)
        self.assertEqual(e.links[0]['health'], 'SUSPECT')
        self.assertIsNone(e.transaction)
        advance(e, 3)
        self.assertEqual(e.active, 1)
        self.assertEqual([b['uplink'] for b in e.boards.values()], [1, 1])
        self.assertEqual(e.failovers, 1)
        self.assertTrue(any('QUIESCE' in v['message'] for v in e.events))

    def test_hysteresis_and_recovery(self):
        e = Engine(); e.links[0]['attenuation']['B'] = 10; advance(e, 4)
        e.links[0]['attenuation']['B'] = 7; advance(e, 6)
        self.assertEqual(e.links[0]['health'], 'FAILED')
        e.links[0]['attenuation']['B'] = 0; advance(e, 4)
        self.assertEqual(e.links[0]['health'], 'RECOVERING')
        advance(e, 2)
        self.assertEqual(e.links[0]['health'], 'HEALTHY')
        self.assertEqual(e.active, 1)  # nonrevertive default

    def test_unhealthy_backup_not_selected(self):
        e = Engine(); e.links[1]['cut'] = True; advance(e, 1)
        e.links[0]['attenuation']['B'] = 10; advance(e, 5)
        self.assertEqual(e.failovers, 0)

    def test_partition_fails_closed_and_recovers(self):
        e = Engine(); e.partition = True; advance(e, 3)
        self.assertTrue(all(b['uplink'] is None for b in e.boards.values()))
        self.assertFalse(e.traffic['connected'])
        e.partition = False; advance(e, 3)
        self.assertTrue(e.traffic['connected'])

    def test_write_failure_no_optimistic_commit(self):
        e = Engine(); e.boards['B']['write_fault'] = True
        e.begin_switch(1, 'test'); advance(e, 3)
        self.assertEqual(e.phase, 'TIMEOUT')
        self.assertEqual(e.failovers, 0)
        self.assertEqual([b['uplink'] for b in e.boards.values()], [0, 0])

    def test_drop_after_quiesce_blocks_both(self):
        e = Engine(); e.begin_switch(1, 'test')
        while e.phase != 'COMMIT': advance(e, .1)
        e.drop_messages = True; advance(e, 3)
        self.assertEqual(e.failovers, 0)
        self.assertTrue(all(b['uplink'] is None for b in e.boards.values()))

    def test_stale_dom(self):
        e = Engine(); e.links[0]['stale']['B'] = True; advance(e, 6)
        self.assertEqual(e.active, 1)

    def test_invalid_inputs_atomic(self):
        e = Engine()
        for values in ({'trip_dbm': -10, 'recover_dbm': -15}, {'auto': 'false'}, {'persistence_s': float('nan')}):
            with self.assertRaises(ValueError): e.configure(values)
        self.assertEqual(e.policy.trip_dbm, -16)
        with self.assertRaises(ValueError): e.command({'action': 'inject', 'field': 'cut', 'value': 'false'})

    def test_revertive_hold_down(self):
        e = Engine(); e.policy.revertive = True
        e.links[0]['cut'] = True; advance(e, 3)
        self.assertEqual(e.active, 1)
        e.links[0]['cut'] = False; advance(e, 5)
        self.assertEqual(e.active, 1)
        advance(e, 5)
        self.assertEqual(e.active, 0)

    def test_deterministic(self):
        a, b = Engine(42), Engine(42)
        advance(a, 10); advance(b, 10)
        self.assertEqual(a.snapshot(), b.snapshot())


if __name__ == '__main__': unittest.main()
