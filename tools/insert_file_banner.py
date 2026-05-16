#!/usr/bin/env python3
# PROJECT:     LibreNT
# LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
# PURPOSE:     LibreNT source file
# COPYRIGHT:   Copyright 2026 Panoc95

"""Insert a standard file banner into source files.

Usage:
    python tools/insert_file_banner.py --dry-run
    python tools/insert_file_banner.py --apply
"""

import argparse
import pathlib
import re
import sys
from typing import Sequence, Tuple, Optional

BANNER_BODY_PREFIX = [
    "PROJECT:     LibreNT",
    "LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)",
    "COPYRIGHT:   Copyright 2026 Panoc95",
]

COMMENT_WRAPPERS = {
    # C-like source and header files
    ".c": ("/*", "*/"),
    ".cc": ("/*", "*/"),
    ".cpp": ("/*", "*/"),
    ".cxx": ("/*", "*/"),
    ".h": ("/*", "*/"),
    ".hpp": ("/*", "*/"),
    ".hh": ("/*", "*/"),
    ".hxx": ("/*", "*/"),
    ".rc": ("/*", "*/"),
    ".idl": ("/*", "*/"),
    ".m": ("/*", "*/"),
    ".mm": ("/*", "*/"),
    ".rs": ("/*", "*/"),
    ".cs": ("/*", "*/"),
    ".java": ("/*", "*/"),
    ".kt": ("/*", "*/"),
    ".swift": ("/*", "*/"),
    ".go": ("/*", "*/"),
    ".js": ("/*", "*/"),
    ".ts": ("/*", "*/"),
    ".tsx": ("/*", "*/"),
    ".jsx": ("/*", "*/"),
    ".hlsl": ("/*", "*/"),
    # XML-style files
    ".xml": ("<!--", "-->"),
    ".xaml": ("<!--", "-->"),
    ".vcxproj": ("<!--", "-->"),
    ".csproj": ("<!--", "-->"),
    # Shell/python/toml files
    ".sh": ("#", ""),
    ".py": ("#", ""),
    ".ps1": ("#", ""),
    ".toml": ("#", ""),
    ".yml": ("#", ""),
    ".yaml": ("#", ""),
    ".md": ("<!--", "-->"),
    ".txt": ("#", ""),
}

BANNER_SIGNATURE_RE = re.compile(
    r"PROJECT:\s*(LibreNT|ReactOS Kernel)|LICENSE:\s*(BSD-3-Clause|GPL-2.0-or-later)"
)


def purpose_for_file(path: pathlib.Path) -> str:
    lower = str(path).replace("\\", "/").lower()
    ext = path.suffix.lower()

    if path.name == "build.rs":
        return "Cargo build script for native components"
    if path.name == "Cargo.toml":
        return "Cargo workspace manifest"
    if ext == ".rs":
        if "winelf" in lower:
            return "WinELF ELF loader and runtime"
        if "libredaemon" in lower:
            return "LibreNT daemon implementation"
        if "libreplatformbuilder" in lower:
            return "LibreNT platform builder utility"
        if "tests" in lower:
            return "Rust test support"
        return "LibreNT Rust component"
    if ext in {".c", ".cc", ".cpp", ".cxx", ".h", ".hpp", ".hh", ".hxx", ".rc"}:
        if "driver" in lower:
            return "Windows driver or kernel support"
        if "ntos2nd" in lower and "posix" in lower:
            return "POSIX subsystem compatibility layer"
        if "winelf" in lower:
            return "WinELF native runtime integration"
        if "mswindows" in lower:
            return "Windows native helper component"
        if "adk" in lower:
            return "Windows installer or deployment component"
        if "tests" in lower:
            return "Native test support"
        return "Native LibreNT system component"
    if ext in {".cs", ".xaml", ".csproj", ".vcxproj"}:
        if "shell" in lower:
            return "Windows shell component"
        if "welcome" in lower:
            return "Windows welcome application"
        return "Windows user interface component"
    if ext in {".md", ".txt"}:
        return "Project documentation"
    if ext in {".sh", ".ps1"}:
        return "Project maintenance script"
    if ext in {".toml", ".yml", ".yaml"}:
        return "Project configuration file"
    if ext == ".xml":
        return "XML configuration or resource file"
    return "LibreNT source file"


def build_banner(wrapper: Tuple[str, str], purpose: str) -> str:
    lines = BANNER_BODY_PREFIX[:2] + [f"PURPOSE:     {purpose}"] + BANNER_BODY_PREFIX[2:]
    start, end = wrapper
    if start == "#":
        formatted = [f"{start} {line}" if line else start for line in lines]
        return "\n".join(formatted) + "\n\n"
    if start == "<!--":
        formatted = [f" {line}" for line in lines]
        text = "\n".join(formatted)
        return f"{start}\n{text}\n{end}\n\n"
    formatted = [f" * {line}" for line in lines]
    text = "\n".join(formatted)
    return f"{start}\n{text}\n {end}\n\n"


def extract_top_banner(text: str, wrapper: Tuple[str, str]) -> str:
    start, end = wrapper
    if start == "#":
        lines = []
        rest = text
        if rest.startswith("#!"):
            first_line, rest = rest.split("\n", 1)
            lines.append(first_line + "\n")
        for line in rest.splitlines(keepends=True):
            if line.startswith("#") or line.strip() == "":
                lines.append(line)
            else:
                break
        return "".join(lines)
    if start == "<!--":
        if not text.lstrip().startswith("<!--"):
            return ""
        idx = text.find("-->")
        return text[: idx + 3] if idx != -1 else text
    if start == "/*":
        if not text.lstrip().startswith("/*"):
            return ""
        idx = text.find("*/")
        return text[: idx + 2] if idx != -1 else text
    return ""


def is_our_banner_block(block: str) -> bool:
    return bool(BANNER_SIGNATURE_RE.search(block))


def replace_existing_banner(text: str, wrapper: Tuple[str, str], banner: str) -> str:
    block = extract_top_banner(text, wrapper)
    if not block:
        return prepend_banner(text, banner)
    if wrapper[0] == "#" and text.startswith("#!"):
        first_line, rest = text.split("\n", 1)
        banner_block = extract_top_banner(rest, wrapper)
        return f"{first_line}\n{banner}{rest[len(banner_block):]}" if banner_block else f"{first_line}\n{banner}{rest}"
    return banner + text[len(block) :]


def prepend_banner(content: str, banner: str) -> str:
    if content.startswith("#!"):
        first_line, rest = content.split("\n", 1)
        return f"{first_line}\n{banner}{rest}"
    return f"{banner}{content}"


def find_files(root: pathlib.Path) -> Sequence[pathlib.Path]:
    ext_patterns = [f"*{ext}" for ext in COMMENT_WRAPPERS.keys()]
    files = []
    for pattern in ext_patterns:
        files.extend(root.rglob(pattern))
    return sorted(set(files))


def main() -> int:
    parser = argparse.ArgumentParser(description="Insert a standard file banner into source files.")
    parser.add_argument("--apply", action="store_true", help="Apply banner additions to files.")
    parser.add_argument("--dry-run", action="store_true", help="Display files that would be modified without changing them.")
    parser.add_argument("--root", default=".", help="Root path to search for source files.")
    args = parser.parse_args()

    root = pathlib.Path(args.root).resolve()
    if not root.exists():
        print(f"Root path does not exist: {root}", file=sys.stderr)
        return 1

    files = find_files(root)
    candidates = []
    banners = {}

    for path in files:
        if not path.is_file():
            continue
        wrapper = COMMENT_WRAPPERS.get(path.suffix.lower())
        if not wrapper:
            continue

        text = path.read_text(encoding="utf-8", errors="ignore")
        purpose = purpose_for_file(path)
        banner = banners.setdefault((wrapper, purpose), build_banner(wrapper, purpose))
        existing_block = extract_top_banner(text, wrapper)

        if existing_block:
            if existing_block.strip() == banner.strip():
                continue
            if not is_our_banner_block(existing_block):
                continue

        candidates.append((path, text, wrapper, banner))

    if not candidates:
        print("No files require banner insertion or update.")
        return 0

    print(f"Found {len(candidates)} files to add or update.")
    for path, _, _, _ in candidates[:20]:
        print(f"  {path.relative_to(root)}")
    if len(candidates) > 20:
        print(f"  ... and {len(candidates)-20} more files")

    if not args.apply:
        if args.dry_run:
            print("Dry run: no files were modified.")
        else:
            print("Run with --apply to modify files.")
        return 0

    for path, text, wrapper, banner in candidates:
        updated = replace_existing_banner(text, wrapper, banner)
        path.write_text(updated, encoding="utf-8")

    print(f"Applied banner to {len(candidates)} files.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
