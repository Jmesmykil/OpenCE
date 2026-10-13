"""tools/android_asm_convert.py: the guest's Mach-O assembly made ELF's."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import android_asm_convert


def converted(text):
    converter = android_asm_convert.Converter(text.splitlines())
    converter.run()
    return "\n".join(converter.out)


def test_strings_keep_their_underscores():
    # (a string is text: "_%s" named maps for their hash, and lost its underscore)
    out = converted('\t.section\t__TEXT,__cstring,cstring_literals\nl_.str:\n\t.asciz\t"_%s"\n\t.ascii\t"_de"\n')
    assert '.asciz "_%s"' in out.replace("\t", " ").replace("  ", " ")
    assert '.ascii "_de"' in out.replace("\t", " ").replace("  ", " ")


def test_symbols_lose_the_mach_o_underscore():
    out = converted('\t.section\t__TEXT,__text,regular,pure_instructions\n_f:\n\tbl\t_csprintf\n\tret\n')
    assert "f:" in out and "bl csprintf" in out.replace("\t", " ")
