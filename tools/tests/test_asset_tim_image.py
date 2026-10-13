import pathlib
import shutil
import subprocess
import tempfile
import sys
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/tests"))
from csource import host_unit  # noqa: E402


class AssetTimImageTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cc"), "host C compiler unavailable")
    def test_uploads_image_then_optional_clut(self):
        source = host_unit("Asset_LoadTimImage")
        harness = source + r'''
#include <assert.h>
static RECT *rects[2];
static int *payloads[2], calls;
int LoadImage(RECT *rect, void *pixels) {
    rects[calls] = rect;
    payloads[calls++] = pixels;
    return 0;
}
int main(void) {
    int raw[12] = {0};
    TimFile *tim = (TimFile *)raw;
    TimBlock *image;
    tim->flags = 0;
    assert(Asset_LoadTimImage(tim) == tim->first_block.header.pixels);
    assert(calls == 1 && rects[0] == &tim->first_block.header.rect);
    calls = 0;
    tim->flags = 8;
    tim->first_block.header.length = 16;
    image = (TimBlock *)&tim->first_block.bytes[16];
    Asset_LoadTimImage(tim);
    assert(calls == 2 && rects[0] == &image->header.rect && payloads[0] == image->header.pixels);
    assert(rects[1] == &tim->first_block.header.rect && payloads[1] == tim->first_block.header.pixels);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            exe = pathlib.Path(directory) / "gpu-tim-image-test"
            result = subprocess.run(
                ["cc", "-std=gnu11", "-O2", "-x", "c", "-", "-o", str(exe)],
                input=harness, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            subprocess.run([str(exe)], check=True, timeout=10)


if __name__ == "__main__":
    unittest.main()
