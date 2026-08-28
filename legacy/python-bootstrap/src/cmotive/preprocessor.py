import os
import platform
import re
import sys as _sys
from pathlib import Path

DEFINE_RE = re.compile(r'^\s*#\s*define\s+(\w+)\s*(.*)$')
UNDEF_RE = re.compile(r'^\s*#\s*undef\s+(\w+)\s*$')
INCLUDE_RE = re.compile(r'^\s*#\s*include\s+["<]([^">]+)[">]\s*$')
REPLACE_RE = re.compile(r'^\s*Replace\s+(\w+)\s*(.*?)\s*;?\s*$')
PLUGIN_RE = re.compile(r'^\s*Plugin\s+([^;\r\n]+)\s*;?\s*$')
PACKAGE_EXTS = ('.HMOT', '.HMTV', '.CMOT', '.CMTV', '.hmot', '.hmtv', '.cmot', '.cmtv')
TOKEN_RE = re.compile(r'\b[A-Za-z_]\w*\b')

class Preprocessor:
    def __init__(self, include_dirs=None, defines=None):
        self.include_dirs = [Path(p) for p in (include_dirs or [])]
        self.macros = dict(defines or {})
        self.seen = []
        self.platform_macros = self._platform_macros()

    def _platform_macros(self):
        system_name = platform.system().lower()
        machine = platform.machine().lower()
        oses = {'UNIX'}
        if system_name == 'windows': oses |= {'WIN32','WIN64'}
        elif system_name == 'darwin': oses |= {'MACOS','UNIX'}
        elif system_name == 'linux': oses |= {'LINUX','UNIX'}
        procs = set()
        if machine in {'x86_64','amd64'}: procs |= {'X64','X86_64'}
        elif machine in {'i386','i686','x86'}: procs |= {'X86'}
        elif machine in {'aarch64','arm64'}: procs |= {'ARM64','AARCH64','ARM'}
        elif machine.startswith('arm'): procs |= {'ARM'}
        endian = 'LITTLE' if _sys.byteorder == 'little' else 'BIG'
        return {'OS': oses, 'PROCESSOR': procs, 'ENDIAN': {endian}}

    def resolve(self, name, current):
        name = Path(name)
        candidates = [current.parent / name] + [d / name for d in self.include_dirs]
        for c in candidates:
            if c.exists():
                return c.resolve()
        raise FileNotFoundError('include not found: ' + str(name))

    def resolve_plugin(self, plugin_name, current):
        # Plugin Abc::Def resolves to Abc/Def.<CMotive ext>. A bare Plugin X
        # resolves to X.<CMotive ext>.  Directory packages may provide package.HMOT
        # or package.CMOT.  This makes packages real translation-unit inputs rather
        # than inert parser declarations.
        logical = plugin_name.strip().rstrip(';')
        logical = re.sub(r'\s+', '', logical)
        rel = Path(*[p for p in logical.split('::') if p])
        bases = [current.parent] + self.include_dirs
        candidates = []
        for base in bases:
            for ext in PACKAGE_EXTS:
                candidates.append(base / (str(rel) + ext))
            for ext in PACKAGE_EXTS:
                candidates.append(base / rel / ('package' + ext))
                candidates.append(base / rel / ('Package' + ext))
        for c in candidates:
            if c.exists():
                return c.resolve()
        raise FileNotFoundError('plugin/package not found: ' + plugin_name)

    def expand(self, line):
        return TOKEN_RE.sub(lambda m: self.macros.get(m.group(0), m.group(0)), line)

    def process(self, path):
        path = Path(path).resolve()
        if path in self.seen:
            return ''
        self.seen.append(path)
        text = path.read_text(encoding='utf-8')
        return self.process_text(text, path)

    def process_text(self, text, current_path, current_package='StartPackage'):
        lines = text.splitlines(True)
        out = []
        i = 0
        while i < len(lines):
            raw = lines[i]
            pkg_match = re.match(r'^\s*Package\s+([^;\r\n]+)\s*;?\s*$', raw)
            if pkg_match:
                current_package = re.sub(r'\s+', '', pkg_match.group(1).strip()) or 'StartPackage'
            if re.match(r'^\s*Plugswitch\b', raw):
                block = []
                depth = 1; i += 1
                while i < len(lines) and depth:
                    line = lines[i]
                    if re.match(r'^\s*Plugswitch\b', line): depth += 1
                    if re.match(r'^\s*Plugend\b', line):
                        depth -= 1
                        if depth == 0:
                            i += 1; break
                    block.append(line); i += 1
                out.append(self.process_plugswitch(block, current_path))
                continue
            m = INCLUDE_RE.match(raw)
            if m:
                out.append(self.process(self.resolve(m.group(1), current_path)))
                i += 1; continue
            m = PLUGIN_RE.match(raw)
            if m:
                plugin_name = m.group(1).strip()
                try:
                    loaded = self.process(self.resolve_plugin(plugin_name, current_path))
                    out.append('\n/* Plugin ' + plugin_name + ' loaded. */\n')
                    out.append(loaded)
                    out.append('\n/* End Plugin ' + plugin_name + '. */\n')
                    # Plugin/package files may declare their own Package. Restore
                    # the importing translation unit's package so imported code
                    # does not accidentally retag following declarations.
                    out.append('Package ' + current_package + ';\n')
                except FileNotFoundError:
                    # Preserve unresolved Plugin lines as AST metadata so front-end
                    # diagnostics and future external package managers can still see it.
                    out.append(raw)
                i += 1; continue
            m = DEFINE_RE.match(raw)
            if m:
                self.macros[m.group(1)] = m.group(2).strip() or '1'
                i += 1; continue
            m = UNDEF_RE.match(raw)
            if m:
                self.macros.pop(m.group(1), None)
                i += 1; continue
            m = REPLACE_RE.match(raw)
            if m:
                self.macros[m.group(1)] = m.group(2).strip() or '1'
                i += 1; continue
            out.append(self.expand(raw))
            i += 1
        return ''.join(out)

    def process_plugswitch(self, lines, current_path):
        cases = []
        default = []
        active_cond = None
        active_lines = []
        def flush():
            nonlocal active_cond, active_lines, default
            if active_cond is None:
                return
            if active_cond == 'DEFAULT': default = active_lines[:]
            else: cases.append((active_cond, active_lines[:]))
            active_cond = None; active_lines = []
        for line in lines:
            m = re.match(r'^\s*Plugcase\s+(.*?)\s*$', line)
            if m:
                flush(); active_cond = m.group(1).strip(); active_lines = []; continue
            if re.match(r'^\s*Plugdefault\b', line):
                flush(); active_cond = 'DEFAULT'; active_lines = []; continue
            active_lines.append(line)
        flush()
        chosen = None
        for cond, body in cases:
            if self.eval_plugcase(cond):
                chosen = body; break
        if chosen is None:
            chosen = default
        return self.process_text(''.join(chosen), current_path)

    def eval_plugcase(self, cond):
        c = cond.strip()
        m = re.search(r'OS\s*:\s*([A-Za-z0-9_]+)', c, re.I)
        if m: return m.group(1).upper() in self.platform_macros['OS']
        m = re.search(r'PROCESSOR\s*:\s*([A-Za-z0-9_]+)', c, re.I)
        if m: return m.group(1).upper() in self.platform_macros['PROCESSOR']
        m = re.search(r'ENDIAN\s*:\s*([A-Za-z0-9_]+)', c, re.I)
        if m: return m.group(1).upper() in self.platform_macros['ENDIAN']
        m = re.search(r'DEFINED\s*:\s*(.*)', c, re.I)
        if m:
            expr = m.group(1)
            for k, v in self.macros.items():
                expr = re.sub(r'\b' + re.escape(k) + r'\b', str(v), expr)
            expr = expr.replace('&&', ' and ').replace('||', ' or ')
            try:
                return bool(eval(expr, {'__builtins__': {}}, {}))
            except Exception:
                return False
        return False
