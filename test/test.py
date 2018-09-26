import unittest
import subprocess

class TestDumpLC(unittest.TestCase):
    def test_spti1234(self):
        subprocess.check_call([
                "../dump_ihkilc",
                "mode=0",
                "start_time_utc=2008-03-19T05:35:00",
                "stop_time_utc=2008-03-19T05:35:02",
                "output=output.txt",
                "target=SPTI1234"
                ])

        output_ref=open("./output_ref.txt").read()
        output=open("./output.txt").read()

    def test_short(self):
        subprocess.check_call([
                "../dump_ihkilc",
                "mode=0",
                "start_time_utc=2008-03-19T05:35:00",
                "stop_time_utc=2008-03-19T05:35:02",
                "output=output.txt",
                "target=ACS"
                ])

        output_ref=open("./output_ref.txt").read()
        output=open("./output.txt").read()

        self.assertEqual(output,output_ref)

if __name__ == '__main__':
    unittest.main()
