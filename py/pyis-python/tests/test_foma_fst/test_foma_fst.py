import unittest
from pyis.python import ops
import os
import sys

class TestFomaFst(unittest.TestCase):
    def test_basic(self):
        fst = ops.FomaFst(os.path.join(os.path.dirname(__file__), 'recognize_latin.fst'))
        query = "avoid writing ad hoc code"
        expected_res = "avoid writing <latin>ad hoc</latin> code"
        self.assertEqual(expected_res, fst.apply_down(query))

    def test_compile(self):
        os.makedirs("./tmp/", exist_ok = True)
        save_path = str(Path('./tmp/recognize_latin_saved.fst'))
        script_str = r"""
regex "ad hoc" -> %<latin%> ... %<%/latin%> ;
apply down avoid writing ad hoc code
save stack $path
"""
        script_str = script_str.replace("$path", save_path)
        print(script_str)
        ops.FomaFst.compile_from_str(script_str)
        fst = ops.FomaFst(save_path)
        query = "avoid writing ad hoc code"
        expected_res = "avoid writing <latin>ad hoc</latin> code"
        self.assertEqual(expected_res, fst.apply_down(query))

if __name__ == "__main__":
    unittest.main()
