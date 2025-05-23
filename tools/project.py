###
# decomp-toolkit project generator
# Generates build.ninja and objdiff.json.
#
# This generator is intentionally project-agnostic
# and shared between multiple projects. Any configuration
# specific to a project should be added to `configure.py`.
# But, we've modified it anyway to support asm_processor and
# make multiple versions more user-friendly.
###

import io
import json
import math
import os
import platform
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional, Set, Tuple, Union

from . import ninja_syntax
from .ninja_syntax import serialize_path

if sys.platform == "cygwin":
    sys.exit(
        f"Cygwin/MSYS2 is not supported."
        f"\nPlease use native Windows Python instead."
        f"\n(Current path: {sys.executable})"
    )

Library = Dict[str, Any]


class Object:
    def __init__(
        self, completed_versions: List[str], name: str, **options: Any
    ) -> None:
        self.name = name
        self.completed_versions = completed_versions
        self.options: Dict[str, Any] = {
            "add_to_all": True,
            "asflags": None,
            "asm_dir": None,
            "asm_processor": False,
            "cflags": None,
            "extra_asflags": None,
            "extra_cflags": None,
            "host": None,
            "lib": None,
            "mw_version": None,
            "progress_category": None,
            "shift_jis": None,
            "source": name,
            "src_dir": None,
            "mw_versions": None,
        }
        self.options.update(options)

        # Internal
        self.src_path: Optional[Path] = None
        self.asm_path: Optional[Path] = None
        self.src_obj_path: Optional[Path] = None
        self.asm_obj_path: Optional[Path] = None
        self.host_obj_path: Optional[Path] = None
        self.ctx_path: Optional[Path] = None

    def resolve(self, config: "ProjectConfig", lib: Library, version: str) -> "Object":
        # Use object options, then library options
        obj = Object(self.completed_versions, self.name, **lib)
        for key, value in self.options.items():
            if value is not None or key not in obj.options:
                obj.options[key] = value

        # Use default options from config
        def set_default(key: str, value: Any) -> None:
            if obj.options[key] is None:
                obj.options[key] = value

        set_default("add_to_all", True)
        set_default("asflags", config.asflags)
        set_default("asm_dir", config.asm_dir)
        set_default("host", False)
        if obj.options["mw_version"] is None and obj.options.get("mw_versions") is not None:
            obj.options["mw_version"] = obj.options["mw_versions"].get(version, config.linker_version)
        else:
            set_default("mw_version", config.linker_version)
        set_default("shift_jis", config.shift_jis)
        set_default("src_dir", config.src_dir)

        # Resolve paths
        build_dir = config.out_path(version)
        obj.src_path = Path(obj.options["src_dir"]) / obj.options["source"]
        if obj.options["asm_dir"] is not None:
            obj.asm_path = (
                Path(obj.options["asm_dir"]) / obj.options["source"]
            ).with_suffix(".s")
        base_name = Path(self.name).with_suffix("")
        obj.src_obj_path = build_dir / "src" / f"{base_name}.o"
        obj.asm_obj_path = build_dir / "mod" / f"{base_name}.o"
        obj.host_obj_path = build_dir / "host" / f"{base_name}.o"
        obj.ctx_path = build_dir / "src" / f"{base_name}.ctx"
        return obj

    def completed(self, config: "ProjectConfig", version: str) -> bool:
        complete = version in self.completed_versions
        # Don't consider asm_processor objects "complete" if asm_processor is disabled
        if self.options["asm_processor"] and not config.asm_processor:
            complete = False
        return complete


class ProgressCategory:
    def __init__(self, id: str, name: str) -> None:
        self.id = id
        self.name = name


class ProjectConfig:
    def __init__(self) -> None:
        # Paths
        self.build_dir: Path = Path("build")  # Output build files
        self.src_dir: Path = Path("src")  # C/C++/asm source files
        self.tools_dir: Path = Path("tools")  # Python scripts
        self.asm_dir: Optional[Path] = Path(
            "asm"
        )  # Override incomplete objects (for modding)

        # Tooling
        self.binutils_tag: Optional[str] = None  # Git tag
        self.binutils_path: Optional[Path] = None  # If None, download
        self.dtk_tag: Optional[str] = None  # Git tag
        self.dtk_path: Optional[Path] = None  # If None, download
        self.compilers_tag: Optional[str] = None  # 1
        self.compilers_path: Optional[Path] = None  # If None, download
        self.wibo_tag: Optional[str] = None  # Git tag
        self.wrapper: Optional[Path] = None  # If None, download wibo on Linux
        self.sjiswrap_tag: Optional[str] = None  # Git tag
        self.sjiswrap_path: Optional[Path] = None  # If None, download
        self.objdiff_tag: Optional[str] = None  # Git tag
        self.objdiff_path: Optional[Path] = None  # If None, download

        # Project config
        self.asm_processor: bool = True  # Enable asm_processor
        self.build_rels: bool = True  # Build REL files
        self.config_dir: Path = Path("config")  # Config directory
        self.debug: bool = False  # Build with debug info
        self.non_matching: bool = False  # Disable hash check (for modding)
        self.generate_map: bool = False  # Generate map file(s)
        self.asflags: Optional[List[str]] = None  # Assembler flags
        self.ldflags: Optional[List[str]] = None  # Linker flags
        self.libs: Optional[List[Dict[str, Any]]] = None  # List of libraries
        self.linker_version: Optional[str] = None  # mwld version
        self.versions: List[str] = []  # List of versions
        self.default_version: Optional[str] = None  # Default version name
        self.warn_missing_config: bool = False  # Warn on missing unit configuration
        self.warn_missing_source: bool = False  # Warn on missing source file
        self.rel_strip_partial: bool = True  # Generate PLFs with -strip_partial
        self.rel_empty_file: Optional[str] = (
            None  # Object name for generating empty RELs
        )
        self.shift_jis = (
            True  # Convert source files from UTF-8 to Shift JIS automatically
        )

        # Progress output and progress.json config
        self.progress_all: bool = True  # Include combined "all" category
        self.progress_modules: bool = True  # Include combined "modules" category
        self.progress_each_module: bool = (
            False  # Include individual modules, disable for large numbers of modules
        )
        self.progress_categories: List[ProgressCategory] = []  # Additional categories

        # Progress fancy printing
        self.progress_use_fancy: bool = False
        self.progress_code_fancy_frac: int = 0
        self.progress_code_fancy_item: str = ""
        self.progress_data_fancy_frac: int = 0
        self.progress_data_fancy_item: str = ""

    def validate(self) -> None:
        required_attrs = [
            "build_dir",
            "src_dir",
            "tools_dir",
            "config_dir",
            "ldflags",
            "linker_version",
            "libs",
            "default_version",
        ]
        for attr in required_attrs:
            if getattr(self, attr) is None:
                sys.exit(f"ProjectConfig.{attr} missing")

    # Creates a map of object names to Object instances
    # Options are fully resolved from the library and object
    def objects(self, version: str) -> Dict[str, Object]:
        out = {}
        for lib in self.libs or {}:
            objects: List[Object] = lib["objects"]
            for obj in objects:
                if obj.name in out:
                    sys.exit(f"Duplicate object name {obj.name}")
                out[obj.name] = obj.resolve(self, lib, version)
        return out

    def check_sha_path(self, version: str) -> Path:
        return self.config_dir / version / "build.sha1"

    def config_path(self, version: str) -> Path:
        return self.config_dir / version / "config.yml"

    def out_path(self, version: str) -> Path:
        return self.build_dir / version


def is_windows() -> bool:
    return os.name == "nt"


# On Windows, we need this to use && in commands
CHAIN = "cmd /c " if is_windows() else ""
# Native executable extension
EXE = ".exe" if is_windows() else ""


def make_flags_str(cflags: Union[str, List[str]]) -> str:
    if isinstance(cflags, list):
        return " ".join(cflags)
    else:
        return cflags


# Load decomp-toolkit generated config.json
def load_build_config(
    config: ProjectConfig, build_config_path: Path
) -> Optional[Dict[str, Any]]:
    if not build_config_path.is_file():
        return None

    def versiontuple(v: str) -> Tuple[int, ...]:
        return tuple(map(int, (v.split("."))))

    f = open(build_config_path, "r", encoding="utf-8")
    build_config: Dict[str, Any] = json.load(f)
    config_version = build_config.get("version")
    if not config_version:
        # Invalid config.json
        f.close()
        os.remove(build_config_path)
        return None

    dtk_version = str(config.dtk_tag)[1:]  # Strip v
    if versiontuple(config_version) < versiontuple(dtk_version):
        # Outdated config.json
        f.close()
        os.remove(build_config_path)
        return None

    f.close()
    return build_config


# Generate build.ninja and objdiff.json
def generate_build(config: ProjectConfig) -> None:
    config.validate()
    rebuild_configs = False
    version_objects = {}
    build_configs = {}

    for version in config.versions:
        objects = config.objects(version)
        build_config_path = config.out_path(version) / "config.json"
        build_config = load_build_config(config, build_config_path)
        if build_config is None:
            rebuild_configs = True
        else:
            version_objects[version] = objects
            build_configs[version] = build_config

    generate_build_ninja(config, version_objects, build_configs, rebuild_configs)
    generate_objdiff_config(config, version_objects, build_configs)


# Generate build.ninja
def generate_build_ninja(
    config: ProjectConfig,
    version_objects: Dict[str, Dict[str, Object]],
    build_configs: Dict[str, Dict[str, Any]],
    rebuild_configs: bool,
) -> None:
    out = io.StringIO()
    n = ninja_syntax.Writer(out)
    n.variable("ninja_required_version", "1.3")
    n.newline()

    configure_script = Path(os.path.relpath(os.path.abspath(sys.argv[0])))
    python_lib = Path(os.path.relpath(__file__))
    python_lib_dir = python_lib.parent
    n.comment("The arguments passed to configure.py, for rerunning it.")
    n.variable("configure_args", sys.argv[1:])
    n.variable("python", f'"{sys.executable}"')
    n.newline()

    ###
    # Variables
    ###
    n.comment("Variables")
    ldflags = " ".join(config.ldflags or [])
    if config.generate_map:
        ldflags += " -mapunused"
    if config.debug:
        ldflags += " -g"
    n.variable("ldflags", ldflags)
    if not config.linker_version:
        sys.exit("ProjectConfig.linker_version missing")
    n.variable("mw_version", Path(config.linker_version))
    n.newline()

    ###
    # Tooling
    ###
    n.comment("Tooling")

    build_tools_path = config.build_dir / "tools"
    download_tool = config.tools_dir / "download_tool.py"
    n.rule(
        name="download_tool",
        command=f"$python {download_tool} $tool $out --tag $tag",
        description="TOOL $out",
    )

    decompctx = config.tools_dir / "decompctx.py"
    n.rule(
        name="decompctx",
        command=f"$python {decompctx} $in -o $out -d $out.d",
        description="CTX $in",
        depfile="$out.d",
        deps="gcc",
    )

    cargo_rule_written = False

    def write_cargo_rule():
        nonlocal cargo_rule_written
        if not cargo_rule_written:
            n.pool("cargo", 1)
            n.rule(
                name="cargo",
                command="cargo build --release --manifest-path $in --bin $bin --target-dir $target",
                description="CARGO $bin",
                pool="cargo",
                depfile=Path("$target") / "release" / "$bin.d",
                deps="gcc",
            )
            cargo_rule_written = True

    if config.dtk_path is not None and config.dtk_path.is_file():
        dtk = config.dtk_path
    elif config.dtk_path is not None:
        dtk = build_tools_path / "release" / f"dtk{EXE}"
        write_cargo_rule()
        n.build(
            outputs=dtk,
            rule="cargo",
            inputs=config.dtk_path / "Cargo.toml",
            implicit=config.dtk_path / "Cargo.lock",
            variables={
                "bin": "dtk",
                "target": build_tools_path,
            },
        )
    elif config.dtk_tag:
        dtk = build_tools_path / f"dtk{EXE}"
        n.build(
            outputs=dtk,
            rule="download_tool",
            implicit=download_tool,
            variables={
                "tool": "dtk",
                "tag": config.dtk_tag,
            },
        )
    else:
        sys.exit("ProjectConfig.dtk_tag missing")

    if config.objdiff_path is not None and config.objdiff_path.is_file():
        objdiff = config.objdiff_path
    elif config.objdiff_path is not None:
        objdiff = build_tools_path / "release" / f"objdiff-cli{EXE}"
        write_cargo_rule()
        n.build(
            outputs=objdiff,
            rule="cargo",
            inputs=config.objdiff_path / "Cargo.toml",
            implicit=config.objdiff_path / "Cargo.lock",
            variables={
                "bin": "objdiff-cli",
                "target": build_tools_path,
            },
        )
    elif config.objdiff_tag:
        objdiff = build_tools_path / f"objdiff-cli{EXE}"
        n.build(
            outputs=objdiff,
            rule="download_tool",
            implicit=download_tool,
            variables={
                "tool": "objdiff-cli",
                "tag": config.objdiff_tag,
            },
        )
    else:
        sys.exit("ProjectConfig.objdiff_tag missing")

    if config.sjiswrap_path:
        sjiswrap = config.sjiswrap_path
    elif config.sjiswrap_tag:
        sjiswrap = build_tools_path / "sjiswrap.exe"
        n.build(
            outputs=sjiswrap,
            rule="download_tool",
            implicit=download_tool,
            variables={
                "tool": "sjiswrap",
                "tag": config.sjiswrap_tag,
            },
        )
    else:
        sys.exit("ProjectConfig.sjiswrap_tag missing")

    # Only add an implicit dependency on wibo if we download it
    wrapper = config.wrapper
    wrapper_implicit: Optional[Path] = None
    if (
        config.wibo_tag is not None
        and sys.platform == "linux"
        and platform.machine() in ("i386", "x86_64")
        and config.wrapper is None
    ):
        wrapper = build_tools_path / "wibo"
        wrapper_implicit = wrapper
        n.build(
            outputs=wrapper,
            rule="download_tool",
            implicit=download_tool,
            variables={
                "tool": "wibo",
                "tag": config.wibo_tag,
            },
        )
    if not is_windows() and wrapper is None:
        wrapper = Path("wine")
    wrapper_cmd = f"{wrapper} " if wrapper else ""

    compilers_implicit: Optional[Path] = None
    if config.compilers_path:
        compilers = config.compilers_path
    elif config.compilers_tag:
        compilers = config.build_dir / "compilers"
        compilers_implicit = compilers
        n.build(
            outputs=compilers,
            rule="download_tool",
            implicit=download_tool,
            variables={
                "tool": "compilers",
                "tag": config.compilers_tag,
            },
        )
    else:
        sys.exit("ProjectConfig.compilers_tag missing")

    binutils_implicit = None
    if config.binutils_path:
        binutils = config.binutils_path
    elif config.binutils_tag:
        binutils = config.build_dir / "binutils"
        binutils_implicit = binutils
        n.build(
            outputs=binutils,
            rule="download_tool",
            implicit=download_tool,
            variables={
                "tool": "binutils",
                "tag": config.binutils_tag,
            },
        )
    else:
        sys.exit("ProjectConfig.binutils_tag missing")

    n.newline()

    ###
    # Helper rule for downloading all tools
    ###
    n.comment("Download all tools")
    n.build(
        outputs="tools",
        rule="phony",
        inputs=[dtk, sjiswrap, wrapper, compilers, binutils, objdiff],
    )
    n.newline()

    ###
    # Build rules
    ###
    compiler_path = compilers / "$mw_version"

    # MWCC
    mwcc = compiler_path / "mwcceppc.exe"
    mwcc_cmd = f"{wrapper_cmd}{mwcc} $cflags -MMD -c $in -o $basedir"
    mwcc_implicit: List[Optional[Path]] = [compilers_implicit or mwcc, wrapper_implicit]

    # MWCC with UTF-8 to Shift JIS wrapper
    mwcc_sjis_cmd = f"{wrapper_cmd}{sjiswrap} {mwcc} $cflags -MMD -c $in -o $basedir"
    mwcc_sjis_implicit: List[Optional[Path]] = [*mwcc_implicit, sjiswrap]

    # MWLD
    mwld = compiler_path / "mwldeppc.exe"
    mwld_cmd = f"{wrapper_cmd}{mwld} $ldflags -o $out @$out.rsp"
    mwld_implicit: List[Optional[Path]] = [compilers_implicit or mwld, wrapper_implicit]

    # GNU as
    gnu_as = binutils / f"powerpc-eabi-as{EXE}"
    gnu_as_cmd = (
        f"{CHAIN}{gnu_as} $asflags -o $out $in -MD $out.d"
        + f" && {dtk} elf fixup $out $out"
    )
    gnu_as_implicit = [binutils_implicit or gnu_as, dtk]

    # MWCC with asm_processor
    objcopy = binutils / f"powerpc-eabi-objcopy{EXE}"
    mwcc_asm_processor_cmd = f'tools/asm_processor/compile.sh "{wrapper_cmd} {mwcc} $cflags -MMD" "{gnu_as} $asflags" {objcopy} $in $out'
    mwcc_asm_processor_implicit: List[Optional[Path]] = [
        *mwcc_implicit,
        binutils_implicit or gnu_as,
        Path("tools/asm_processor/compile.sh"),
        Path("tools/asm_processor/asm_processor.py"),
    ]

    # MWCC with asm_processor and UTF-8 to Shift JIS wrapper
    mwcc_asm_processor_sjis_cmd = f'tools/asm_processor/compile.sh "{wrapper_cmd}{sjiswrap} {mwcc} $cflags -MMD" "{gnu_as} $asflags" {objcopy} $in $out'
    mwcc_asm_processor_sjis_implicit: List[Optional[Path]] = [
        *mwcc_asm_processor_implicit,
        sjiswrap,
    ]

    if os.name != "nt":
        transform_dep = config.tools_dir / "transform_dep.py"
        mwcc_cmd += f" && $python {transform_dep} $basefile.d $basefile.d"
        mwcc_sjis_cmd += f" && $python {transform_dep} $basefile.d $basefile.d"
        mwcc_asm_processor_cmd += f" && $python {transform_dep} $basefile.d $basefile.d"
        mwcc_asm_processor_sjis_cmd += (
            f" && $python {transform_dep} $basefile.d $basefile.d"
        )
        mwcc_implicit.append(transform_dep)
        mwcc_sjis_implicit.append(transform_dep)
        mwcc_asm_processor_implicit.append(transform_dep)
        mwcc_asm_processor_sjis_implicit.append(transform_dep)
    else:
        mwcc_asm_processor_cmd = "sh " + mwcc_asm_processor_cmd
        mwcc_asm_processor_sjis_cmd = "sh " + mwcc_asm_processor_sjis_cmd

    n.comment("Link ELF file")
    n.rule(
        name="link",
        command=mwld_cmd,
        description="LINK $out",
        rspfile="$out.rsp",
        rspfile_content="$in_newline",
    )
    n.newline()

    n.comment("Generate DOL")
    n.rule(
        name="elf2dol",
        command=f"{dtk} elf2dol $in $out",
        description="DOL $out",
    )
    n.newline()

    n.comment("MWCC build")
    n.rule(
        name="mwcc",
        command=mwcc_cmd,
        description="MWCC $out",
        depfile="$basefile.d",
        deps="gcc",
    )
    n.newline()

    n.comment("MWCC build (with UTF-8 to Shift JIS wrapper)")
    n.rule(
        name="mwcc_sjis",
        command=mwcc_sjis_cmd,
        description="MWCC $out",
        depfile="$basefile.d",
        deps="gcc",
    )
    n.newline()

    n.comment("MWCC build (with asm_processor)")
    n.rule(
        name="mwcc_asm_processor",
        command=mwcc_asm_processor_cmd,
        description="MWCC $out",
        depfile="$basefile.d",
        deps="gcc",
    )
    n.newline()

    n.comment("MWCC build (with asm_processor and UTF-8 to Shift JIS wrapper)")
    n.rule(
        name="mwcc_asm_processor_sjis",
        command=mwcc_asm_processor_sjis_cmd,
        description="MWCC $out",
        depfile="$basefile.d",
        deps="gcc",
    )
    n.newline()

    n.comment("Assemble asm")
    n.rule(
        name="as",
        command=gnu_as_cmd,
        description="AS $out",
        depfile="$out.d",
        deps="gcc",
    )
    n.newline()

    n.comment("Split DOL into relocatable objects")
    n.rule(
        name="split",
        command=f"{dtk} dol split $in $out_dir",
        description="SPLIT $in",
        depfile="$out_dir/dep",
        deps="gcc",
    )
    n.newline()

    n.comment("Check hash")
    n.rule(
        name="check",
        command=f"{dtk} shasum -c $in -o $out",
        description="CHECK $in",
    )
    n.newline()

    n.comment("Progress")
    n.rule(
        name="progress",
        command=f"$python {configure_script} $configure_args progress --progress-version $version",
        description="PROGRESS $version",
    )
    n.newline()

    n.comment("Report")
    n.rule(
        name="report",
        command=f"{objdiff} report generate -o $out",
        description="REPORT",
    )
    n.newline()

    ###
    # Source files
    ###
    n.comment("Source files")

    def map_path(path: Path) -> Path:
        return path.parent / (path.name + ".MAP")

    class LinkStep:
        def __init__(self, config: Dict[str, Any]) -> None:
            self.name: str = config["name"]
            self.module_id: int = config["module_id"]
            self.ldscript: Optional[Path] = Path(config["ldscript"])
            self.entry = config["entry"]
            self.inputs: List[str] = []

        def add(self, obj: Path) -> None:
            self.inputs.append(serialize_path(obj))

        def output(self) -> Path:
            if self.module_id == 0:
                return build_path / f"{self.name}.dol"
            else:
                return build_path / self.name / f"{self.name}.rel"

        def partial_output(self) -> Path:
            if self.module_id == 0:
                return build_path / f"{self.name}.elf"
            else:
                return build_path / self.name / f"{self.name}.plf"

        def write(self, n: ninja_syntax.Writer) -> None:
            n.comment(f"Link {self.name}")
            if self.module_id == 0:
                elf_path = build_path / f"{self.name}.elf"
                dol_path = build_path / f"{self.name}.dol"
                elf_ldflags = f"$ldflags -lcf {serialize_path(self.ldscript)}"
                if config.generate_map:
                    elf_map = map_path(elf_path)
                    elf_ldflags += f" -map {serialize_path(elf_map)}"
                else:
                    elf_map = None
                n.build(
                    outputs=elf_path,
                    rule="link",
                    inputs=self.inputs,
                    implicit=[self.ldscript, *mwld_implicit],
                    implicit_outputs=elf_map,
                    variables={"ldflags": elf_ldflags},
                )
                n.build(
                    outputs=dol_path,
                    rule="elf2dol",
                    inputs=elf_path,
                    implicit=dtk,
                )
            else:
                preplf_path = build_path / self.name / f"{self.name}.preplf"
                plf_path = build_path / self.name / f"{self.name}.plf"
                preplf_ldflags = "$ldflags -sdata 0 -sdata2 0 -r"
                plf_ldflags = f"$ldflags -sdata 0 -sdata2 0 -r1 -lcf {serialize_path(self.ldscript)}"
                if self.entry:
                    plf_ldflags += f" -m {self.entry}"
                    # -strip_partial is only valid with -m
                    if config.rel_strip_partial:
                        plf_ldflags += " -strip_partial"
                if config.generate_map:
                    preplf_map = map_path(preplf_path)
                    preplf_ldflags += f" -map {serialize_path(preplf_map)}"
                    plf_map = map_path(plf_path)
                    plf_ldflags += f" -map {serialize_path(plf_map)}"
                else:
                    preplf_map = None
                    plf_map = None
                n.build(
                    outputs=preplf_path,
                    rule="link",
                    inputs=self.inputs,
                    implicit=mwld_implicit,
                    implicit_outputs=preplf_map,
                    variables={"ldflags": preplf_ldflags},
                )
                n.build(
                    outputs=plf_path,
                    rule="link",
                    inputs=self.inputs,
                    implicit=[self.ldscript, preplf_path, *mwld_implicit],
                    implicit_outputs=plf_map,
                    variables={"ldflags": plf_ldflags},
                )
            n.newline()

    report_path = config.build_dir / "report.json"
    source_inputs: List[Path] = []

    for version, build_config in build_configs.items():
        build_path = config.out_path(version)
        progress_path = build_path / "progress.json"

        link_steps: List[LinkStep] = []
        used_compiler_versions: Set[str] = set()
        host_source_inputs: List[Path] = []
        source_added: Set[Path] = set()

        def c_build(obj: Object, src_path: Path) -> Optional[Path]:
            asflags_str = make_flags_str(obj.options["asflags"])
            cflags_str = make_flags_str(obj.options["cflags"])
            if obj.options["extra_cflags"] is not None:
                extra_cflags_str = make_flags_str(obj.options["extra_cflags"])
                cflags_str += " " + extra_cflags_str
            cflags_str += f" -i build/{version}/include -DVERSION={version.upper().replace('-', '_')}"
            used_compiler_versions.add(obj.options["mw_version"])

            # Avoid creating duplicate build rules
            if obj.src_obj_path is None or obj.src_obj_path in source_added:
                return obj.src_obj_path
            source_added.add(obj.src_obj_path)

            # Add MWCC build rule
            lib_name = obj.options["lib"]
            n.comment(f"{obj.name}: {lib_name} (linked {obj.completed(config, version)})")
            if config.asm_processor and obj.options["asm_processor"]:
                n.build(
                    outputs=obj.src_obj_path,
                    rule=(
                        "mwcc_asm_processor_sjis"
                        if obj.options["shift_jis"]
                        else "mwcc_asm_processor"
                    ),
                    inputs=src_path,
                    variables={
                        "mw_version": Path(obj.options["mw_version"]),
                        "asflags": asflags_str,
                        "cflags": cflags_str,
                        "basedir": os.path.dirname(obj.src_obj_path),
                        "basefile": obj.src_obj_path.with_suffix(""),
                    },
                    implicit=(
                        mwcc_asm_processor_sjis_implicit
                        if obj.options["shift_jis"]
                        else mwcc_asm_processor_implicit
                    ),
                )
            else:
                n.build(
                    outputs=obj.src_obj_path,
                    rule="mwcc_sjis" if obj.options["shift_jis"] else "mwcc",
                    inputs=src_path,
                    variables={
                        "mw_version": Path(obj.options["mw_version"]),
                        "cflags": cflags_str,
                        "basedir": os.path.dirname(obj.src_obj_path),
                        "basefile": obj.src_obj_path.with_suffix(""),
                    },
                    implicit=(
                        mwcc_sjis_implicit
                        if obj.options["shift_jis"]
                        else mwcc_implicit
                    ),
                )

            # Add ctx build rule
            if obj.ctx_path is not None:
                n.build(
                    outputs=obj.ctx_path,
                    rule="decompctx",
                    inputs=src_path,
                    implicit=decompctx,
                )

            # Add host build rule
            if obj.options["host"] and obj.host_obj_path is not None:
                n.build(
                    outputs=obj.host_obj_path,
                    rule="host_cc" if src_path.suffix == ".c" else "host_cpp",
                    inputs=src_path,
                    variables={
                        "basedir": os.path.dirname(obj.host_obj_path),
                        "basefile": obj.host_obj_path.with_suffix(""),
                    },
                )
                if obj.options["add_to_all"]:
                    host_source_inputs.append(obj.host_obj_path)
            n.newline()

            if obj.options["add_to_all"]:
                source_inputs.append(obj.src_obj_path)

            return obj.src_obj_path

        def asm_build(
            obj: Object, src_path: Path, obj_path: Optional[Path]
        ) -> Optional[Path]:
            if obj.options["asflags"] is None:
                sys.exit("ProjectConfig.asflags missing")
            asflags_str = make_flags_str(obj.options["asflags"])
            if obj.options["extra_asflags"] is not None:
                extra_asflags_str = make_flags_str(obj.options["extra_asflags"])
                asflags_str += " " + extra_asflags_str

            # Avoid creating duplicate build rules
            if obj_path is None or obj_path in source_added:
                return obj_path
            source_added.add(obj_path)

            # Add assembler build rule
            lib_name = obj.options["lib"]
            n.comment(f"{obj.name}: {lib_name} (linked {obj.completed(config, version)})")
            n.build(
                outputs=obj_path,
                rule="as",
                inputs=src_path,
                variables={"asflags": asflags_str},
                implicit=gnu_as_implicit,
            )
            n.newline()

            if obj.options["add_to_all"]:
                source_inputs.append(obj_path)

            return obj_path

        def add_unit(build_obj, link_step: LinkStep):
            obj_path, obj_name = build_obj["object"], build_obj["name"]
            obj = version_objects[version].get(obj_name)
            if obj is None:
                if config.warn_missing_config and not build_obj["autogenerated"]:
                    print(f"({version}) Missing configuration for {obj_name}")
                link_step.add(obj_path)
                return

            link_built_obj = obj.completed(config, version)
            built_obj_path: Optional[Path] = None
            if obj.src_path is not None and obj.src_path.exists():
                if obj.src_path.suffix in (".c", ".cp", ".cpp"):
                    # Add MWCC & host build rules
                    built_obj_path = c_build(obj, obj.src_path)
                elif obj.src_path.suffix == ".s":
                    # Add assembler build rule
                    built_obj_path = asm_build(obj, obj.src_path, obj.src_obj_path)
                else:
                    sys.exit(f"Unknown source file type {obj.src_path}")
            else:
                if config.warn_missing_source or obj.completed(config, version):
                    print(f"Missing source file {obj.src_path}")
                link_built_obj = False

            # Assembly overrides
            if obj.asm_path is not None and obj.asm_path.exists():
                link_built_obj = True
                built_obj_path = asm_build(obj, obj.asm_path, obj.asm_obj_path)

            if link_built_obj and built_obj_path is not None:
                # Use the source-built object
                link_step.add(built_obj_path)
            elif obj_path is not None:
                # Use the original (extracted) object
                link_step.add(obj_path)
            else:
                lib_name = obj.options["lib"]
                sys.exit(
                    f"Missing object for {obj_name}: {obj.src_path} {lib_name} {obj}"
                )

        # Add DOL link step
        link_step = LinkStep(build_config)
        for unit in build_config["units"]:
            add_unit(unit, link_step)
        link_steps.append(link_step)

        if config.build_rels:
            # Add REL link steps
            for module in build_config["modules"]:
                module_link_step = LinkStep(module)
                for unit in module["units"]:
                    add_unit(unit, module_link_step)
                # Add empty object to empty RELs
                if len(module_link_step.inputs) == 0:
                    if config.rel_empty_file is None:
                        sys.exit("ProjectConfig.rel_empty_file missing")
                    add_unit(
                        {
                            "object": None,
                            "name": config.rel_empty_file,
                            "autogenerated": True,
                        },
                        module_link_step,
                    )
                link_steps.append(module_link_step)
        n.newline()

        # Check if all compiler versions exist
        for mw_version in used_compiler_versions:
            mw_path = compilers / mw_version / "mwcceppc.exe"
            if config.compilers_path and not os.path.exists(mw_path):
                sys.exit(f"Compiler {mw_path} does not exist")

        # Check if linker exists
        mw_path = compilers / str(config.linker_version) / "mwldeppc.exe"
        if config.compilers_path and not os.path.exists(mw_path):
            sys.exit(f"Linker {mw_path} does not exist")

        ###
        # Link
        ###
        for step in link_steps:
            step.write(n)
        n.newline()

        ###
        # Check hash
        ###
        ok_path = build_path / "ok"
        n.build(
            outputs=ok_path,
            rule="check",
            inputs=config.check_sha_path(version),
            implicit=[dtk, *map(lambda step: step.output(), link_steps)],
        )
        n.newline()

        ###
        # Calculate progress
        ###
        n.build(
            outputs=progress_path,
            rule="progress",
            variables={"version": version},
            implicit=[
                ok_path,
                configure_script,
                python_lib,
                config.config_path(version),
            ],
        )
        n.newline()

        ###
        # Version target
        ###
        n.comment(f"Version {version}")
        if config.non_matching:
            n.build(
                outputs=version,
                rule="phony",
                inputs=[*map(lambda step: step.output(), link_steps)],
            )
        else:
            n.build(
                outputs=f"{version}",
                rule="phony",
                inputs=progress_path,
            )
        n.newline()

    ###
    # Split DOL
    ###
    n.comment("Split DOL into relocatable objects")
    for version in config.versions:
        build_path = config.out_path(version)
        build_config_path = build_path / "config.json"
        n.build(
            inputs=config.config_path(version),
            outputs=build_config_path,
            rule="split",
            implicit=dtk,
            variables={"out_dir": build_path},
        )
        n.build(
            outputs=f"split-{version}",
            rule="phony",
            inputs=build_config_path,
        )
    n.build(
        outputs="split-all",
        rule="phony",
        inputs=[f"split-{version}" for version in build_configs.keys()],
    )
    n.newline()

    ###
    # Helper rule for building all source files
    ###
    n.comment("Build all source files")
    n.build(
        outputs="all_source",
        rule="phony",
        inputs=source_inputs,
    )
    n.newline()

    ###
    # Generate progress report
    ###
    n.comment("Generate progress report")
    report_implicit: List[str | Path] = [objdiff, "all_source"]
    n.build(
        outputs=report_path,
        rule="report",
        implicit=report_implicit,
    )
    n.newline()

    ###
    # Regenerate on change
    ###
    n.comment("Reconfigure on change")
    n.rule(
        name="configure",
        command=f"$python {configure_script} $configure_args",
        generator=True,
        description=f"RUN {configure_script}",
    )
    n.build(
        outputs="build.ninja",
        rule="configure",
        implicit=[
            *[config.out_path(version) / "config.json" for version in config.versions],
            configure_script,
            python_lib,
            python_lib_dir / "ninja_syntax.py",
        ],
    )
    n.newline()

    ###
    # Default rule
    ###
    n.comment("Default rules")
    if rebuild_configs:
        n.default("split-all")
    elif config.default_version is not None:
        n.default(config.default_version)

    n.comment("Build All Versions")
    n.build(
        outputs="all",
        rule="phony",
        inputs=list(build_configs.keys()),
    )
    n.newline()

    # Write build.ninja
    with open("build.ninja", "w", encoding="utf-8") as f:
        f.write(out.getvalue())
    out.close()


# Generate objdiff.json
def generate_objdiff_config(
    config: ProjectConfig,
    version_objects: Dict[str, Dict[str, Object]],
    build_configs: Dict[str, Dict[str, Any]],
) -> None:
    objdiff_config: Dict[str, Any] = {
        "$schema": "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json",
        "min_version": "2.0.0-alpha.5",
        "custom_make": "ninja",
        "build_target": False,
        "watch_patterns": [
            "*.c",
            "*.cp",
            "*.cpp",
            "*.h",
            "*.hpp",
            "*.inc",
            "*.py",
            "*.yml",
            "*.txt",
            "*.json",
        ],
        "units": [],
        "progress_categories": [],
    }

    # decomp.me compiler name mapping
    # Commented out versions have not been added to decomp.me yet
    COMPILER_MAP = {
        "GC/1.0": "mwcc_233_144",
        "GC/1.1": "mwcc_233_159",
        "GC/1.2.5": "mwcc_233_163",
        "GC/1.2.5e": "mwcc_233_163e",
        "GC/1.2.5n": "mwcc_233_163n",
        "GC/1.3": "mwcc_242_53",
        "GC/1.3.2": "mwcc_242_81",
        "GC/1.3.2r": "mwcc_242_81r",
        "GC/2.0": "mwcc_247_92",
        "GC/2.5": "mwcc_247_105",
        "GC/2.6": "mwcc_247_107",
        "GC/2.7": "mwcc_247_108",
        "GC/3.0a3": "mwcc_41_51213",
        "GC/3.0a3.2": "mwcc_41_60126",
        "GC/3.0a3.3": "mwcc_41_60209",
        "GC/3.0a3.4": "mwcc_42_60308",
        "GC/3.0a5": "mwcc_42_60422",
        "GC/3.0a5.2": "mwcc_41_60831",
        "GC/3.0": "mwcc_41_60831",
        "Wii/1.0RC1": "mwcc_42_140",
        "Wii/0x4201_127": "mwcc_42_142",
        "Wii/1.0a": "mwcc_42_142",
        "Wii/1.0": "mwcc_43_145",
        "Wii/1.1": "mwcc_43_151",
        "Wii/1.3": "mwcc_43_172",
        "Wii/1.5": "mwcc_43_188",
        "Wii/1.6": "mwcc_43_202",
        "Wii/1.7": "mwcc_43_213",
    }

    def add_unit(
        build_obj: Dict[str, Any], version: str, progress_categories: List[str]
    ) -> None:
        obj_path, obj_name = build_obj["object"], build_obj["name"]
        base_object = Path(obj_name).with_suffix("")
        unit_config: Dict[str, Any] = {
            "name": Path(version) / base_object,
            "target_path": obj_path,
            "metadata": {
                "auto_generated": build_obj["autogenerated"],
                "progress_categories": progress_categories,
            },
        }

        obj = version_objects[version].get(obj_name)
        if obj is None or not obj.src_path or not obj.src_path.exists():
            objdiff_config["units"].append(unit_config)
            return

        cflags = obj.options["cflags"]
        reverse_fn_order = False
        if type(cflags) is list:
            for flag in cflags:
                if not flag.startswith("-inline "):
                    continue
                for value in flag.split(" ")[1].split(","):
                    if value == "deferred":
                        reverse_fn_order = True
                    elif value == "nodeferred":
                        reverse_fn_order = False

            # Filter out include directories
            def keep_flag(flag):
                return not flag.startswith("-i ") and not flag.startswith("-I ")

            cflags = list(filter(keep_flag, cflags))

            # Add appropriate lang flag
            if obj.src_path.suffix in (".cp", ".cpp"):
                cflags.insert(0, "-lang=c++")
            else:
                cflags.insert(0, "-lang=c")

        unit_config["base_path"] = obj.src_obj_path
        compiler_version = COMPILER_MAP.get(obj.options["mw_version"])
        if compiler_version is None:
            print(f"Missing scratch compiler mapping for {obj.options['mw_version']}")
        else:
            cflags_str = make_flags_str(cflags)
            if obj.options["extra_cflags"] is not None:
                extra_cflags_str = make_flags_str(obj.options["extra_cflags"])
                cflags_str += " " + extra_cflags_str
            unit_config["scratch"] = {
                "platform": "gc_wii",
                "compiler": compiler_version,
                "c_flags": cflags_str,
                "ctx_path": obj.ctx_path,
                "build_ctx": True,
            }
        category_opt: List[str] | str = obj.options["progress_category"]
        if isinstance(category_opt, list):
            progress_categories.extend(map(lambda x: f"{version}.{x}", category_opt))
        elif category_opt is not None:
            progress_categories.append(f"{version}.{category_opt}")
        unit_config["metadata"].update(
            {
                "complete": obj.completed(config, version),
                "reverse_fn_order": reverse_fn_order,
                "source_path": obj.src_path,
                "progress_categories": progress_categories,
            }
        )
        objdiff_config["units"].append(unit_config)

    for version, build_config in build_configs.items():
        # Add DOL units
        for unit in build_config["units"]:
            progress_categories = [version]
            # Only include a "dol" category if there are any modules
            # Otherwise it's redundant with the global report measures
            if len(build_config["modules"]) > 0:
                progress_categories.append(f"{version}.dol")
            add_unit(unit, version, progress_categories)

        # Add REL units
        for module in build_config["modules"]:
            for unit in module["units"]:
                progress_categories = [version]
                if config.progress_modules:
                    progress_categories.append(f"{version}.modules")
                if config.progress_each_module:
                    progress_categories.append(f"{version}.{module['name']}")
                add_unit(unit, version, progress_categories)

        # Add progress categories
        def add_category(id: str, name: str):
            objdiff_config["progress_categories"].append(
                {
                    "id": id,
                    "name": name,
                }
            )

        add_category(version, version)
        if len(build_config["modules"]) > 0:
            if config.progress_modules:
                add_category(f"{version}.modules", "Modules")
            if config.progress_each_module:
                for module in build_config["modules"]:
                    add_category(f"{version}.{module['name']}", module["name"])
        for category in config.progress_categories:
            add_category(f"{version}.{category.id}", category.name)

    # Write objdiff.json
    with open("objdiff.json", "w", encoding="utf-8") as w:

        def unix_path(input: Any) -> str:
            return str(input).replace(os.sep, "/") if input else ""

        json.dump(objdiff_config, w, indent=4, default=unix_path)


# Calculate, print and write progress to progress.json
def calculate_progress(config: ProjectConfig, version: str) -> None:
    config.validate()
    objects = config.objects(version)
    out_path = config.out_path(version)
    build_config = load_build_config(config, out_path / "config.json")
    if not build_config:
        return

    class ProgressUnit:
        def __init__(self, name: str) -> None:
            self.name: str = name
            self.code_total: int = 0
            self.code_fancy_frac: int = config.progress_code_fancy_frac
            self.code_fancy_item: str = config.progress_code_fancy_item
            self.code_progress: int = 0
            self.data_total: int = 0
            self.data_fancy_frac: int = config.progress_data_fancy_frac
            self.data_fancy_item: str = config.progress_data_fancy_item
            self.data_progress: int = 0
            self.objects_progress: int = 0
            self.objects_total: int = 0
            self.objects: Set[Object] = set()

        def add(self, build_obj: Dict[str, Any]) -> None:
            self.code_total += build_obj["code_size"]
            self.data_total += build_obj["data_size"]

            # Avoid counting the same object in different modules twice
            include_object = build_obj["name"] not in self.objects
            if include_object:
                self.objects.add(build_obj["name"])
                self.objects_total += 1

            if build_obj["autogenerated"]:
                # Skip autogenerated objects
                return

            obj = objects.get(build_obj["name"])
            if obj is None or not obj.completed(config, version):
                return

            self.code_progress += build_obj["code_size"]
            self.data_progress += build_obj["data_size"]
            if include_object:
                self.objects_progress += 1

        def code_frac(self) -> float:
            return self.code_progress / self.code_total

        def data_frac(self) -> float:
            return self.data_progress / self.data_total

    progress_units: Dict[str, ProgressUnit] = {}
    if config.progress_all:
        progress_units["all"] = ProgressUnit("All")
    if len(build_config["modules"]) > 0:
        if config.progress_modules:
            progress_units["modules"] = ProgressUnit("Modules")
    if len(config.progress_categories) > 0:
        for category in config.progress_categories:
            progress_units[category.id] = ProgressUnit(category.name)
    if config.progress_each_module:
        for module in build_config["modules"]:
            progress_units[module["name"]] = ProgressUnit(module["name"])

    def add_unit(id: str, unit: Dict[str, Any]) -> None:
        progress = progress_units.get(id)
        if progress is not None:
            progress.add(unit)

    # Add DOL units
    for unit in build_config["units"]:
        add_unit("all", unit)
        add_unit("dol", unit)
        obj = objects.get(unit["name"])
        if obj is not None:
            category_opt = obj.options["progress_category"]
            if isinstance(category_opt, list):
                for id in category_opt:
                    add_unit(id, unit)
            elif category_opt is not None:
                add_unit(category_opt, unit)

    # Add REL units
    for module in build_config["modules"]:
        for unit in module["units"]:
            add_unit("all", unit)
            add_unit("modules", unit)
            add_unit(module["name"], unit)
            obj = objects.get(unit["name"])
            if obj is not None:
                category_opt = obj.options["progress_category"]
                if isinstance(category_opt, list):
                    for id in category_opt:
                        add_unit(id, unit)
                elif category_opt is not None:
                    add_unit(category_opt, unit)

    # Print human-readable progress
    print(f"{version} progress:")

    def print_category(unit: Optional[ProgressUnit]) -> None:
        if unit is None:
            return

        code_frac = unit.code_frac()
        data_frac = unit.data_frac()
        print(
            f"  {unit.name}: {code_frac:.2%} code, {data_frac:.2%} data ({unit.objects_progress} / {unit.objects_total} files)"
        )
        print(f"    Code: {unit.code_progress} / {unit.code_total} bytes")
        print(f"    Data: {unit.data_progress} / {unit.data_total} bytes")
        if config.progress_use_fancy:
            print(
                "\nYou have {} out of {} {} and collected {} out of {} {}.".format(
                    math.floor(code_frac * unit.code_fancy_frac),
                    unit.code_fancy_frac,
                    unit.code_fancy_item,
                    math.floor(data_frac * unit.data_fancy_frac),
                    unit.data_fancy_frac,
                    unit.data_fancy_item,
                )
            )

    for progress in progress_units.values():
        # don't print the progress of the libraries
        if progress.name in {"All", "Emulator"}:
            print_category(progress)

    # Generate and write progress.json
    progress_json: Dict[str, Any] = {}

    def add_category(category: str, unit: ProgressUnit) -> None:
        progress_json[category] = {
            "code": unit.code_progress,
            "code/total": unit.code_total,
            "data": unit.data_progress,
            "data/total": unit.data_total,
        }

    for id, progress in progress_units.items():
        add_category(id, progress)
    with open(out_path / "progress.json", "w", encoding="utf-8") as w:
        json.dump(progress_json, w, indent=4)
