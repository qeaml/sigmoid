"""Plugin to automatically pack bundles"""

import bip

g_src: bip.Path
g_out: bip.Path

def configure(settings: dict) -> bool:
  if "src" not in settings:
    bip.err("No bundle source directory is defined.",
            "Define it via the 'src' key.")
    return False

  if "out" not in settings:
    bip.err("No output bundle is defined.",
            "Define it via the 'out' key.")
    return False

  global g_src
  global g_out
  global g_exe

  g_src = bip.Path(settings["src"]).resolve()
  g_out = bip.Path(settings["out"]).resolve()

  if not g_out.parent.exists():
    g_out.parent.mkdir(parents=True)

  if not g_src.exists():
    bip.err(f"Bundle source directory `{g_src}` does not exist.",
             "Make sure you haven't made a typo.")
    return False

  return True

def clean() -> bool:
  if g_out.exists():
    g_out.unlink()
  return True

def want_run() -> bool:
  if not g_out.exists():
    return True

  bndlmt = g_out.stat().st_mtime
  for bndlfile in g_src.iterdir():
    filemt = bndlfile.stat().st_mtime
    if filemt > bndlmt:
      return True
  return False

def run() -> bool:
  if not bip.cmd("nwgebndl", ["create", f"{g_src}", f"{g_out}"]):
    return False

  return True
