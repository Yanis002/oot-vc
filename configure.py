#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object NotLinked status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import sys
import glob
from pathlib import Path
from typing import Any, Dict, List

from tools.project import (
    Object,
    ProgressCategory,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)

### Script's arguments

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "--non-matching",
    action="store_true",
    help="create non-matching build for modding",
)
parser.add_argument(
    "--no-asm-processor",
    action="store_true",
    help="disable asm_processor for progress calculation",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="DIR",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
    default=True,
)
parser.add_argument(
    "--no-asm",
    action="store_true",
    help="don't incorporate .s files from asm directory",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--progress-version",
    metavar="VERSION",
    help="version to print progress for",
)

args = parser.parse_args()

### Project configuration

config = ProjectConfig()


# Only configure versions for which an orig file exists
def version_exists(version: str) -> bool:
    return glob.glob(str(Path("orig") / version / "*")) != []
SM64 = [
    "sm64-j",
    "sm64-u",
    "sm64-e",
]

OOT = [
    "oot-j",
    "oot-u",
    "oot-e",
]

ALL_VERSIONS = [
    *SM64,
    *OOT,
]
config.versions = [
    version
    for version in ALL_VERSIONS
    if version_exists(version)
]

if not config.versions:
    sys.exit("Error: no orig files found for any version")

if "oot-j" in config.versions:
    config.default_version = "oot-j"
else:
    # Use the earliest version as default
    config.default_version = config.versions[0]

config.warn_missing_config = True
config.warn_missing_source = False
config.progress_all = True

config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.sjiswrap_path = args.sjiswrap
config.non_matching = args.non_matching
config.asm_processor = not args.no_asm_processor

if not is_windows():
    config.wrapper = args.wrapper

if args.no_asm:
    config.asm_dir = None

### Tool versions

config.binutils_tag = "2.42-1"
config.compilers_tag = "20240706"
config.dtk_tag = "v1.4.1"
config.objdiff_tag = "v2.7.1"
config.sjiswrap_tag = "v1.2.0"
config.wibo_tag = "0.6.11"
config.linker_version = "GC/3.0a5"

### Flags

config.asflags = [
    "-mgekko",
    "-I include",
    "-I libc",
]

config.ldflags = [
    "-fp hardware",
    "-nodefaults",
    "-warn off",
]

cflags_base = [
    "-proc gekko",
    "-fp hardware",
    "-fp_contract on",
    "-enum int",
    "-align powerpc",
    "-nosyspath",
    "-RTTI off",
    "-str reuse",
    "-inline auto",
    "-nodefaults",
    "-msgstyle gcc",
    "-sym on",
    "-i include",
    "-i libc",
    "-i libcpp",
    "-i src",
]

if config.non_matching:
    cflags_base.append("-DNON_MATCHING")

### Helper functions

def EmulatorLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_versions": {
            "oot-j": "GC/3.0a5",
            "oot-u": "GC/3.0a5",
            "oot-e": "GC/3.0a5",
        },
        "cflags": [*cflags_base, "-Cpp_exceptions off", "-O4,p", "-enc SJIS", "-ipa file"],
        "progress_category": "emulator",
        "objects": objects,
    }

def RevolutionLib(lib_name: str, objects: List[Object], cpp_exceptions: str = "off") -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/3.0a5",
        "cflags": [*cflags_base, f"-Cpp_exceptions {cpp_exceptions}", "-O4,p", "-ipa file", "-enc SJIS", "-fp_contract off"],
        "progress_category": "revolution",
        "objects": objects,
    }

def RevolutionHBMLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/3.0a5",
        "cflags": [*cflags_base, "-Cpp_exceptions off", "-O4,p", "-ipa file", "-enc SJIS", "-fp_contract off", "-lang c++", "-DHBM_ASSERT", "-sym off"],
        "progress_category": "hbm",
        "objects": objects,
    }

def LibC(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/3.0a5",
        "cflags": [*cflags_base, "-Cpp_exceptions on", "-O4,p", "-ipa file", "-rostr", "-use_lmw_stmw on", "-lang c", "-fp_contract off"],
        "progress_category": "libc",
        "objects": objects,
    }

def MathLibC(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/3.0a5",
        "cflags": [*cflags_base, "-Cpp_exceptions off", "-O4,p", "-rostr", "-use_lmw_stmw on", "-lang c", "-fp_contract off"],
        "progress_category": "libc",
        "objects": objects,
    }

def RuntimeLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/3.0a5",
        "cflags": [*cflags_base, "-Cpp_exceptions off", "-O4,p", "-rostr", "-use_lmw_stmw on", "-enc SJIS"],
        "progress_category": "runtime",
        "objects": objects,
    }

def MetroTRKLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/2.7",
        "cflags": [*cflags_base, "-Cpp_exceptions off", "-O4,p", "-rostr", "-use_lmw_stmw on", "-lang c", "-inline on,deferred", "-func_align 4", "-sdata 0", "-sdata2 0"],
        "progress_category": "metrotrk",
        "objects": objects,
    }


### Link order

# Not matching for any version
NotLinked: List[str] = []

# Linked for all versions
Linked = config.versions

# Linked for specific versions
def LinkedFor(*versions):
    return versions

config.libs = [
    EmulatorLib(
        "emulator",
        [
            Object(LinkedFor(*OOT), "emulator/vc64_RVL.c"),
            Object(LinkedFor(*OOT), "emulator/system.c"),
            Object(LinkedFor(*OOT), "emulator/ai.c"),
            Object(LinkedFor(*OOT), "emulator/vi.c"),
            Object(LinkedFor(*OOT), "emulator/si.c"),
            Object(LinkedFor(*OOT), "emulator/pi.c"),
            Object(LinkedFor(*OOT), "emulator/mi.c"),
            Object(LinkedFor(*OOT), "emulator/disk.c"),
            Object(NotLinked, "emulator/cpu.c"),
            Object(LinkedFor(*OOT), "emulator/pif.c"),
            Object(LinkedFor(*OOT), "emulator/ram.c"),
            Object(LinkedFor(*OOT), "emulator/rom.c"),
            Object(LinkedFor(*OOT), "emulator/rdb.c"),
            Object(LinkedFor(*OOT), "emulator/pak.c"),
            Object(LinkedFor(*OOT), "emulator/sram.c"),
            Object(LinkedFor(*OOT), "emulator/flash.c"),
            Object(LinkedFor(*OOT), "emulator/_frameGCNcc.c"),
            Object(NotLinked, "emulator/_buildtev.c"),
            Object(NotLinked, "emulator/frame.c"),
            Object(LinkedFor(*OOT), "emulator/library.c"),
            Object(LinkedFor(*OOT), "emulator/codeRVL.c"),
            Object(LinkedFor(*OOT), "emulator/helpRVL.c"),
            Object(LinkedFor(*OOT), "emulator/soundRVL.c"),
            Object(LinkedFor(*OOT), "emulator/video.c"),
            Object(LinkedFor(*OOT), "emulator/storeRVL.c"),
            Object(LinkedFor(*OOT), "emulator/controller.c", mw_version="GC/3.0a5.2"),
            Object(LinkedFor(*OOT), "emulator/errordisplay.c"),
            Object(LinkedFor("oot-j", "oot-u"), "emulator/banner.c"),
            Object(LinkedFor(*OOT), "emulator/stringtable.c"),
            Object(NotLinked, "emulator/rsp.c"),
            Object(NotLinked, "emulator/rdp.c"),
            Object(LinkedFor(*OOT), "emulator/xlCoreRVL.c"),
            Object(LinkedFor(*OOT), "emulator/xlPostRVL.c"),
            Object(LinkedFor(*OOT), "emulator/xlFileRVL.c"),
            Object(LinkedFor(*OOT), "emulator/xlText.c"),
            Object(LinkedFor(*OOT), "emulator/xlList.c"),
            Object(LinkedFor(*OOT), "emulator/xlHeap.c"),
            Object(LinkedFor(*OOT), "emulator/xlFile.c"),
            Object(LinkedFor(*OOT), "emulator/xlObject.c"),
        ]
    ),
    RevolutionLib(
        "NdevExi2AD",
        [
            Object(LinkedFor(*OOT), "revolution/NdevExi2AD/DebuggerDriver.c"),
            Object(LinkedFor(*OOT), "revolution/NdevExi2AD/exi2.c"),
        ]
    ),
    RevolutionLib(
        "vcmv",
        [
            Object(NotLinked, "revolution/vcmv/code_80083070.cpp"),
        ],
        cpp_exceptions="on"
    ),
    RevolutionLib(
        "base",
        [
            Object(LinkedFor(*OOT), "revolution/base/PPCArch.c"),
        ]
    ),
    RevolutionLib(
        "os",
        [
            Object(LinkedFor(*OOT), "revolution/os/OS.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSAlarm.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSAlloc.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSArena.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSAudioSystem.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSCache.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSContext.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSError.c"),
            Object(NotLinked, "revolution/os/OSExec.c", cflags=[*cflags_base, "-Cpp_exceptions off", "-O4,p", "-ipa off"]),
            Object(LinkedFor(*OOT), "revolution/os/OSFatal.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSFont.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSInterrupt.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSLink.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSMessage.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSMemory.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSMutex.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSReboot.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSReset.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSRtc.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSSync.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSThread.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSTime.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSUtf.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSIpc.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSStateTM.c"),
            Object(LinkedFor(*OOT), "revolution/os/time.dolphin.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSPlayRecord.c"),
            Object(LinkedFor(*OOT), "revolution/os/OSStateFlags.c"),
            Object(LinkedFor(*OOT), "revolution/os/__start.c"),
            Object(LinkedFor(*OOT), "revolution/os/__ppc_eabi_init.c"),
        ]
    ),
    RevolutionLib(
        "exi",
        [
            Object(LinkedFor(*OOT), "revolution/exi/EXIBios.c", cflags=[*cflags_base, "-Cpp_exceptions off", "-O3,p", "-ipa file"]),
            Object(LinkedFor(*OOT), "revolution/exi/EXIUart.c"),
            Object(LinkedFor(*OOT), "revolution/exi/EXICommon.c"),
        ]
    ),
    RevolutionLib(
        "si",
        [
            Object(LinkedFor(*OOT), "revolution/si/SIBios.c"),
            Object(LinkedFor(*OOT), "revolution/si/SISamplingRate.c"),
        ]
    ),
    RevolutionLib(
        "db",
        [
            Object(LinkedFor(*OOT), "revolution/db/db.c"),
        ]
    ),
    RevolutionLib(
        "vi",
        [
            Object(LinkedFor(*OOT), "revolution/vi/vi.c"),
            Object(LinkedFor(*OOT), "revolution/vi/i2c.c"),
            Object(LinkedFor(*OOT), "revolution/vi/vi3in1.c"),
        ]
    ),
    RevolutionLib(
        "mtx",
        [
            Object(LinkedFor(*OOT), "revolution/mtx/mtx.c"),
            Object(LinkedFor(*OOT), "revolution/mtx/mtxvec.c"),
            Object(LinkedFor(*OOT), "revolution/mtx/mtx44.c"),
        ]
    ),
    RevolutionLib(
        "gx",
        [
            Object(LinkedFor(*OOT), "revolution/gx/GXInit.c"),
            Object(LinkedFor(*OOT), "revolution/gx/GXFifo.c"),
            Object(LinkedFor(*OOT), "revolution/gx/GXAttr.c"),
            Object(LinkedFor(*OOT), "revolution/gx/GXMisc.c"),
            Object(LinkedFor(*OOT), "revolution/gx/GXGeometry.c"),
            Object(LinkedFor(*OOT), "revolution/gx/GXFrameBuf.c"),
            Object(LinkedFor(*OOT), "revolution/gx/GXLight.c"),
            Object(NotLinked, "revolution/gx/GXTexture.c"),
            Object(LinkedFor(*OOT), "revolution/gx/GXBump.c"),
            Object(LinkedFor(*OOT), "revolution/gx/GXTev.c"),
            Object(LinkedFor(*OOT), "revolution/gx/GXPixel.c"),
            Object(LinkedFor(*OOT), "revolution/gx/GXTransform.c"),
            Object(LinkedFor(*OOT), "revolution/gx/GXPerf.c"),
        ]
    ),
    RevolutionLib(
        "dvd",
        [
            Object(LinkedFor(*OOT), "revolution/dvd/dvdfs.c"),
            Object(LinkedFor(*OOT), "revolution/dvd/dvd.c"),
            Object(LinkedFor(*OOT), "revolution/dvd/dvdqueue.c"),
            Object(LinkedFor(*OOT), "revolution/dvd/dvderror.c"),
            Object(LinkedFor(*OOT), "revolution/dvd/dvdidutils.c"),
            Object(LinkedFor(*OOT), "revolution/dvd/dvdFatal.c"),
            Object(LinkedFor(*OOT), "revolution/dvd/dvd_broadway.c"),
        ]
    ),
    RevolutionLib(
        "demo",
        [
            Object(LinkedFor(*OOT), "revolution/demo/DEMOInit.c"),
            Object(LinkedFor(*OOT), "revolution/demo/DEMOPuts.c", cflags=[*cflags_base, "-Cpp_exceptions off", "-O4,p"]),
        ]
    ),
    RevolutionLib(
        "ai",
        [
            Object(LinkedFor(*OOT), "revolution/ai/ai.c"),
        ]
    ),
    RevolutionLib(
        "ax",
        [
            Object(LinkedFor(*OOT), "revolution/ax/AX.c"),
            Object(LinkedFor(*OOT), "revolution/ax/AXAlloc.c"),
            Object(LinkedFor(*OOT), "revolution/ax/AXAux.c"),
            Object(LinkedFor(*OOT), "revolution/ax/AXCL.c"),
            Object(LinkedFor(*OOT), "revolution/ax/AXOut.c"),
            Object(LinkedFor(*OOT), "revolution/ax/AXSPB.c"),
            Object(NotLinked, "revolution/ax/AXVPB.c"),
            Object(LinkedFor(*OOT), "revolution/ax/AXComp.c"),
            Object(LinkedFor(*OOT), "revolution/ax/DSPCode.c"),
            Object(LinkedFor(*OOT), "revolution/ax/AXProf.c"),
        ]
    ),
    RevolutionLib(
        "axfx",
        [
            Object(LinkedFor(*OOT), "revolution/axfx/AXFXReverbHi.c"),
            Object(NotLinked, "revolution/axfx/AXFXReverbHiExp.c"),
            Object(LinkedFor(*OOT), "revolution/axfx/AXFXHooks.c"),
        ]
    ),
    RevolutionLib(
        "mem",
        [
            Object(LinkedFor(*OOT), "revolution/mem/mem_heapCommon.c"),
            Object(LinkedFor(*OOT), "revolution/mem/mem_expHeap.c"),
            Object(LinkedFor(*OOT), "revolution/mem/mem_frameHeap.c"),
            Object(LinkedFor(*OOT), "revolution/mem/mem_unitHeap.c"),
            Object(LinkedFor(*OOT), "revolution/mem/mem_allocator.c"),
            Object(LinkedFor(*OOT), "revolution/mem/mem_list.c"),
        ]
    ),
    RevolutionLib(
        "code_800B17A8",
        [
            Object(NotLinked, "revolution/code_800B17A8.c")
        ]
    ),
    RevolutionLib(
        "dsp",
        [
            Object(LinkedFor(*OOT), "revolution/dsp/dsp.c"),
            Object(LinkedFor(*OOT), "revolution/dsp/dsp_debug.c"),
            Object(LinkedFor(*OOT), "revolution/dsp/dsp_task.c"),
        ]
    ),
    RevolutionLib(
        "nand",
        [
            Object(LinkedFor(*OOT), "revolution/nand/nand.c"),
            Object(LinkedFor(*OOT), "revolution/nand/NANDOpenClose.c"),
            Object(LinkedFor(*OOT), "revolution/nand/NANDCore.c"),
            Object(LinkedFor(*OOT), "revolution/nand/NANDCheck.c"),
        ]
    ),
    RevolutionLib(
        "sc",
        [
            Object(LinkedFor(*OOT), "revolution/sc/scsystem.c"),
            Object(LinkedFor(*OOT), "revolution/sc/scapi.c"),
            Object(LinkedFor(*OOT), "revolution/sc/scapi_prdinfo.c"),
        ]
    ),
    RevolutionLib(
        "wenc",
        [
            Object(LinkedFor(*OOT), "revolution/wenc/wenc.c"),
        ]
    ),
    RevolutionLib(
        "arc",
        [
            Object(LinkedFor(*OOT), "revolution/arc/arc.c"),
        ]
    ),
    RevolutionLib(
        "ipc",
        [
            Object(LinkedFor(*OOT), "revolution/ipc/ipcMain.c"),
            Object(LinkedFor(*OOT), "revolution/ipc/ipcclt.c"),
            Object(LinkedFor(*OOT), "revolution/ipc/memory.c"),
            Object(LinkedFor(*OOT), "revolution/ipc/ipcProfile.c"),
        ]
    ),
    RevolutionLib(
        "fs",
        [
            Object(LinkedFor(*OOT), "revolution/fs/fs.c"),
        ]
    ),
    RevolutionLib(
        "pad",
        [
            Object(LinkedFor(*OOT), "revolution/pad/Padclamp.c"),
            Object(LinkedFor(*OOT), "revolution/pad/Pad.c"),
        ]
    ),
    RevolutionLib(
        "wpad",
        [
            Object(NotLinked, "revolution/wpad/WPAD.c"),
            Object(NotLinked, "revolution/wpad/WPADHIDParser.c"),
            Object(NotLinked, "revolution/wpad/WPADEncrypt.c"),
            Object(NotLinked, "revolution/wpad/debug_msg.c"),
        ]
    ),
    RevolutionLib(
        "kpad",
        [
            Object(NotLinked, "revolution/kpad/KPAD.c"),
        ]
    ),
    RevolutionLib(
        "usb",
        [
            Object(LinkedFor(*OOT), "revolution/usb/usb.c"),
        ]
    ),
    RevolutionLib(
        "wud",
        [
            Object(NotLinked, "revolution/wud/WUD.c"),
            Object(NotLinked, "revolution/wud/WUDHidHost.c"),
            Object(NotLinked, "revolution/wud/debug_msg.c"),
        ]
    ),
    RevolutionLib(
        "code_800D1134",
        [
            Object(NotLinked, "revolution/code_800D1134.c"),
        ]
    ),
    RevolutionLib(
        "bte",
        [
            Object(NotLinked, "revolution/bte/code_800D1614.c"),
        ]
    ),
    RevolutionLib(
        "cnt",
        [
            Object(NotLinked, "revolution/cnt/cnt.c"),
        ]
    ),
    RevolutionLib(
        "esp",
        [
            Object(LinkedFor(*OOT), "revolution/esp/esp.c"),
        ]
    ),
    RevolutionLib(
        "rso",
        [
            Object(NotLinked, "revolution/rso/RSOLink.c"),
        ]
    ),
    RevolutionLib(
        "tpl",
        [
            Object(LinkedFor(*OOT), "revolution/tpl/TPL.c"),
        ]
    ),
    RevolutionHBMLib(
        "hbm",
        [
            Object(LinkedFor(*OOT), "revolution/hbm/homebutton/HBMBase.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/homebutton/HBMAnmController.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/homebutton/HBMFrameController.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/homebutton/HBMGUIManager.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/homebutton/HBMController.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/homebutton/HBMRemoteSpk.cpp"),
        ],
    ),
    RevolutionHBMLib(
        "nw4hbm/db",
        [
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/db/db_assert.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/db/db_console.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/db/db_directPrint.cpp"),
            Object(NotLinked, "revolution/hbm/nw4hbm/db/db_mapFile.cpp"),
        ],
    ),
    RevolutionHBMLib(
        "nw4hbm/lyt",
        [
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/lyt/lyt_animation.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/lyt/lyt_arcResourceAccessor.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/lyt/lyt_bounding.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/lyt/lyt_common.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/lyt/lyt_drawInfo.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/lyt/lyt_group.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/lyt/lyt_layout.cpp"),
            Object(NotLinked, "revolution/hbm/nw4hbm/lyt/lyt_material.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/lyt/lyt_pane.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/lyt/lyt_picture.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/lyt/lyt_resourceAccessor.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/lyt/lyt_textBox.cpp", mw_version="GC/3.0a5.2"),
            Object(NotLinked, "revolution/hbm/nw4hbm/lyt/lyt_window.cpp", mw_version="GC/3.0a5.2"),
        ],
    ),
    RevolutionHBMLib(
        "nw4hbm/math",
        [
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/math/math_triangular.cpp"),
        ],
    ),
    RevolutionHBMLib(
        "nw4hbm/snd",
        [
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_AxManager.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_AxVoice.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_Bank.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_BankFile.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_BasicSound.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_Channel.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_DisposeCallbackManager.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_DvdSoundArchive.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_EnvGenerator.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_ExternalSoundPlayer.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_FrameHeap.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_InstancePool.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_Lfo.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_MemorySoundArchive.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_MidiSeqPlayer.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_MidiSeqTrack.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_MmlParser.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_MmlSeqTrack.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_MmlSeqTrackAllocator.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_NandSoundArchive.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_RemoteSpeaker.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_RemoteSpeakerManager.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_SeqFile.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_SeqPlayer.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_SeqSound.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_SeqSoundHandle.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_SeqTrack.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_SoundArchive.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_SoundArchiveFile.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_SoundArchiveLoader.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_SoundArchivePlayer.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_SoundHandle.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_SoundHeap.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_SoundPlayer.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_SoundStartable.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_SoundSystem.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_SoundThread.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_StrmChannel.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_StrmFile.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_StrmPlayer.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_StrmSound.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_StrmSoundHandle.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_TaskManager.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_TaskThread.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_Util.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_WaveFile.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_WavePlayer.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_WaveSound.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_WaveSoundHandle.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_WsdFile.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_WsdPlayer.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/snd/snd_WsdTrack.cpp"),
        ],
    ),
    RevolutionHBMLib(
        "nw4hbm/ut",
        [
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/ut/ut_binaryFileFormat.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/ut/ut_CharStrmReader.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/ut/ut_CharWriter.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/ut/ut_DvdFileStream.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/ut/ut_FileStream.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/ut/ut_Font.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/ut/ut_IOStream.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/ut/ut_LinkList.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/ut/ut_list.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/ut/ut_NandFileStream.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/ut/ut_ResFont.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/ut/ut_ResFontBase.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/ut/ut_TagProcessorBase.cpp"),
            Object(LinkedFor(*OOT), "revolution/hbm/nw4hbm/ut/ut_TextWriterBase.cpp"),
        ]
    ),
    RuntimeLib(
        "runtime",
        [
            Object(LinkedFor(*OOT), "runtime/__mem.c"),
            Object(LinkedFor(*OOT), "runtime/__va_arg.c"),
            Object(LinkedFor(*OOT), "runtime/global_destructor_chain.c"),
            Object(LinkedFor(*OOT), "runtime/New.cpp", extra_cflags=["-lang c++", "-Cpp_exceptions on", "-RTTI on"]),
            Object(LinkedFor(*OOT), "runtime/NMWException.cpp",  extra_cflags=["-lang c++", "-Cpp_exceptions on", "-RTTI on"]),
            Object(LinkedFor(*OOT), "runtime/ptmf.c"),
            Object(LinkedFor(*OOT), "runtime/runtime.c"),
            Object(LinkedFor(*SM64, *OOT), "runtime/__init_cpp_exceptions.cpp"),
            Object(LinkedFor(*OOT), "runtime/Gecko_setjmp.c"),
            Object(NotLinked, "runtime/Gecko_ExceptionPPC.cpp",  extra_cflags=["-lang c++", "-Cpp_exceptions on", "-RTTI on"]),
            Object(LinkedFor(*OOT), "runtime/GCN_mem_alloc.c"),
        ]
    ),
    LibC(
        "libc",
        [
            Object(LinkedFor(*OOT), "libc/alloc.c"),
            Object(LinkedFor(*OOT), "libc/ansi_files.c"),
            Object(LinkedFor(*OOT), "libc/ansi_fp.c"),
            Object(LinkedFor(*OOT), "libc/arith.c"),
            Object(LinkedFor(*OOT), "libc/bsearch.c"),
            Object(LinkedFor(*OOT), "libc/buffer_io.c"),
            Object(LinkedFor(*OOT), "libc/direct_io.c"),
            Object(LinkedFor(*OOT), "libc/ctype.c"),
            Object(LinkedFor(*OOT), "libc/errno.c"),
            Object(LinkedFor(*OOT), "libc/file_io.c"),
            Object(LinkedFor(*OOT), "libc/file_pos.c"),
            Object(LinkedFor(*OOT), "libc/float.c"),
            Object(LinkedFor(*OOT), "libc/locale.c"),
            Object(LinkedFor(*OOT), "libc/mbstring.c"),
            Object(LinkedFor(*OOT), "libc/mem.c"),
            Object(LinkedFor(*OOT), "libc/mem_funcs.c"),
            Object(LinkedFor(*OOT), "libc/math_api.c"),
            Object(LinkedFor(*OOT), "libc/misc_io.c"),
            Object(LinkedFor(*OOT), "libc/printf.c", mw_version="GC/3.0a5.2"),
            Object(LinkedFor(*OOT), "libc/qsort.c"),
            Object(LinkedFor(*OOT), "libc/rand.c"),
            Object(LinkedFor(*OOT), "libc/scanf.c"),
            Object(LinkedFor(*OOT), "libc/signal.c"),
            Object(LinkedFor(*OOT), "libc/string.c"),
            Object(LinkedFor(*OOT), "libc/strtold.c"),
            Object(LinkedFor(*OOT), "libc/strtoul.c"),
            Object(LinkedFor(*OOT), "libc/time.c"),
            Object(LinkedFor(*OOT), "libc/wstring.c"),
            Object(LinkedFor(*OOT), "libc/wchar_io.c"),
            Object(LinkedFor(*OOT), "libc/wctype.c"),
            Object(LinkedFor(*OOT), "libc/sysenv.c"),
            Object(LinkedFor(*OOT), "libc/uart_console_io.c"),
            Object(LinkedFor(*OOT), "libc/abort_exit_ppc_eabi.c"),
            Object(LinkedFor(*OOT), "libc/extras.c"),
        ]
    ),
    MathLibC(
        "libm",
        [
            Object(LinkedFor(*OOT), "libc/math/e_acos.c"),
            Object(LinkedFor(*OOT), "libc/math/e_asin.c"),
            Object(LinkedFor(*OOT), "libc/math/e_atan2.c"),
            Object(LinkedFor(*OOT), "libc/math/e_exp.c"),
            Object(LinkedFor(*OOT), "libc/math/e_fmod.c"),
            Object(LinkedFor(*OOT), "libc/math/e_log.c"),
            Object(LinkedFor(*OOT), "libc/math/e_log10.c"),
            Object(LinkedFor(*OOT), "libc/math/e_pow.c"),
            Object(LinkedFor(*OOT), "libc/math/e_rem_pio2.c"),
            Object(LinkedFor(*OOT), "libc/math/k_cos.c"),
            Object(LinkedFor(*OOT), "libc/math/k_rem_pio2.c"),
            Object(LinkedFor(*OOT), "libc/math/k_sin.c"),
            Object(LinkedFor(*OOT), "libc/math/k_tan.c"),
            Object(LinkedFor(*OOT), "libc/math/s_atan.c"),
            Object(LinkedFor(*OOT), "libc/math/s_ceil.c"),
            Object(LinkedFor(*OOT), "libc/math/s_copysign.c"),
            Object(LinkedFor(*OOT), "libc/math/s_cos.c"),
            Object(LinkedFor(*OOT), "libc/math/s_floor.c"),
            Object(LinkedFor(*OOT), "libc/math/s_frexp.c"),
            Object(LinkedFor(*OOT), "libc/math/s_ldexp.c"),
            Object(LinkedFor(*OOT), "libc/math/s_sin.c"),
            Object(LinkedFor(*OOT), "libc/math/s_tan.c"),
            Object(LinkedFor(*OOT), "libc/math/w_acos.c"),
            Object(LinkedFor(*OOT), "libc/math/w_asin.c"),
            Object(LinkedFor(*OOT), "libc/math/w_atan2.c"),
            Object(LinkedFor(*OOT), "libc/math/w_exp.c"),
            Object(LinkedFor(*OOT), "libc/math/w_fmod.c"),
            Object(LinkedFor(*OOT), "libc/math/w_log.c"),
            Object(LinkedFor(*OOT), "libc/math/w_log10f.c"),
            Object(LinkedFor(*OOT), "libc/math/w_pow.c"),
            Object(LinkedFor(*OOT), "libc/math/e_sqrt.c"),
            Object(LinkedFor(*OOT), "libc/math/math_ppc.c"),
            Object(LinkedFor(*OOT), "libc/math/w_sqrt.c"),
        ]
    ),
    MetroTRKLib(
        "metrotrk",
        [
            Object(LinkedFor(*OOT), "metrotrk/mainloop.c"),
            Object(LinkedFor(*OOT), "metrotrk/nubevent.c"),
            Object(LinkedFor(*OOT), "metrotrk/nubinit.c"),
            Object(LinkedFor(*OOT), "metrotrk/msg.c"),
            Object(LinkedFor(*OOT), "metrotrk/msgbuf.c"),
            Object(LinkedFor(*OOT), "metrotrk/serpoll.c", extra_cflags=["-sdata 8"]),
            Object(LinkedFor(*OOT), "metrotrk/usr_put.c"),
            Object(LinkedFor(*OOT), "metrotrk/dispatch.c"),
            Object(LinkedFor(*OOT), "metrotrk/msghndlr.c"),
            Object(LinkedFor(*OOT), "metrotrk/support.c"),
            Object(LinkedFor(*OOT), "metrotrk/mutex_TRK.c"),
            Object(LinkedFor(*OOT), "metrotrk/notify.c"),
            Object(LinkedFor(*OOT), "metrotrk/flush_cache.c"),
            Object(LinkedFor(*OOT), "metrotrk/mem_TRK.c"),
            Object(LinkedFor(*OOT), "metrotrk/string_TRK.c"),
            Object(LinkedFor(*OOT), "metrotrk/__exception.s"),
            Object(LinkedFor(*OOT), "metrotrk/targimpl.c"),
            Object(LinkedFor(*OOT), "metrotrk/targsupp.c", extra_cflags=["-func_align 16"]),
            Object(LinkedFor(*OOT), "metrotrk/mpc_7xx_603e.c"),
            Object(LinkedFor(*OOT), "metrotrk/mslsupp.c"),
            Object(LinkedFor(*OOT), "metrotrk/dolphin_trk.c"),
            Object(LinkedFor(*OOT), "metrotrk/main_TRK.c"),
            Object(LinkedFor(*OOT), "metrotrk/dolphin_trk_glue.c"),
            Object(LinkedFor(*OOT), "metrotrk/targcont.c"),
            Object(LinkedFor(*OOT), "metrotrk/target_options.c"),
            Object(LinkedFor(*OOT), "metrotrk/cc_udp.c"),
            Object(LinkedFor(*OOT), "metrotrk/cc_gdev.c", extra_cflags=["-sdata 8"]),
            Object(LinkedFor(*OOT), "metrotrk/CircleBuffer.c"),
            Object(LinkedFor(*OOT), "metrotrk/MWCriticalSection_gc.c"),
        ]
    )
]

# Optional extra categories for progress tracking
# Adjust as desired for your project
config.progress_categories = [
    ProgressCategory("emulator", "Emulator"),
    ProgressCategory("revolution", "Revolution SDK"),
    ProgressCategory("hbm", "Home Button Menu"),
    ProgressCategory("libc", "Libc"),
    ProgressCategory("runtime", "Runtime"),
    ProgressCategory("metrotrk", "MetroTRK"),
]

### Execute mode

if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress and write progress.json
    calculate_progress(config, args.progress_version)
else:
    sys.exit("Unknown mode: " + args.mode)
