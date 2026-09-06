import copy
import json
import unittest
from pathlib import Path
from roadmap import validate, overlaps, suggest_wave, task_conflicts, normalized
ROOT=Path(__file__).resolve().parents[1]

class RoadmapTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data=json.loads((ROOT/'backlog/tasks.json').read_text())
    def fresh(self):return copy.deepcopy(self.data)
    def test_200_valid_goals(self):
        self.assertEqual(len(self.data['tasks']),200);self.assertEqual(validate(self.data),[])
    def test_duplicate_id(self):
        d=self.fresh();d['tasks'][1]['id']=d['tasks'][0]['id'];self.assertTrue(any('duplicate ID' in x for x in validate(d)))
    def test_missing_dependency(self):
        d=self.fresh();d['tasks'][0]['depends_on']=['VG-NOT-999'];self.assertTrue(any('missing dependency' in x for x in validate(d)))
    def test_cycle(self):
        d=self.fresh();d['tasks'][0]['depends_on']=['VG-GOV-002'];self.assertTrue(any('cycle' in x for x in validate(d)))
    def test_self_edge(self):
        d=self.fresh();d['tasks'][0]['depends_on']=['VG-GOV-001'];self.assertTrue(any('self-dependency' in x for x in validate(d)))
    def test_empty_outcome(self):
        d=self.fresh();d['tasks'][0]['outcome']='';self.assertTrue(validate(d))
    def test_ready_requires_stamp(self):
        d=self.fresh();d['tasks'][0]['state']='READY';errors=validate(d);self.assertTrue(any('base_sha' in x for x in errors));self.assertTrue(any('commands' in x for x in errors))
    def test_no_actual_ready_wave(self):self.assertEqual(suggest_wave(self.data)['selected'],[])
    def test_planning_is_labelled(self):self.assertIn('NOT_CLAIMABLE',suggest_wave(self.data,planning=True)['mode'])
    def test_assumptions_require_planning(self):
        with self.assertRaises(ValueError):suggest_wave(self.data,assumed={'VG-GOV-001'})
    def test_assumptions_must_close_dependencies(self):
        with self.assertRaises(ValueError):suggest_wave(self.data,planning=True,assumed={'VG-GOV-002'})
    def test_unknown_assumed_id(self):
        with self.assertRaises(ValueError):suggest_wave(self.data,planning=True,assumed={'VG-NOT-999'})
    def test_path_casefold(self):self.assertTrue(overlaps('Native/Client/Main.cpp','native/client/main.cpp'))
    def test_directory_overlap(self):self.assertTrue(overlaps('native/client','native/client/main.cpp'))
    def test_glob_overlap(self):self.assertTrue(overlaps('native/**','native/client/main.cpp'))
    def test_uncertain_globs_conservative(self):self.assertTrue(overlaps('native/*/core.*','native/src/*.cpp'))
    def test_distinct_files(self):self.assertFalse(overlaps('native/client/a.hpp','native/client/b.hpp'))
    def test_unsafe_paths(self):
        for p in ['../escape','/absolute','C:/unsafe']:
            with self.assertRaises(ValueError):normalized(p)
    def test_shared_resource_conflict(self):
        a={'owned_paths':['a.cpp'],'exclusive_resources':['schema:profile']}
        b={'owned_paths':['b.cpp'],'exclusive_resources':['schema:profile']}
        self.assertTrue(task_conflicts(a,b))
    def test_projection_read_only(self):
        before=json.dumps(self.data,sort_keys=True);suggest_wave(self.data,planning=True,assumed={'VG-GOV-001'});self.assertEqual(before,json.dumps(self.data,sort_keys=True))

if __name__=='__main__':unittest.main(verbosity=2)
