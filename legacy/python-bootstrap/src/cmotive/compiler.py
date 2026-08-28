from pathlib import Path
from .lexer import Lexer
from .parser import Parser
from .semantic import SemanticAnalyzer
from .codegen import NativeCodegen
from .preprocessor import Preprocessor

class CompilerPipeline:
    def __init__(self, include_dirs=None, target_arch='native'):
        self.include_dirs = [str(p) for p in (include_dirs or [])]
        self.target_arch = target_arch
        self.root = Path(__file__).resolve().parents[2]

    def compile_to_c(self, src_path):
        src_path = Path(src_path)
        include_dirs = [src_path.parent, self.root / 'lib'] + [Path(p) for p in self.include_dirs]
        pp = Preprocessor(include_dirs=include_dirs)
        text = pp.process(src_path)
        ast = Parser(Lexer().tokenize(text)).parse()
        sema = SemanticAnalyzer().analyze(ast)
        unit = NativeCodegen(self.target_arch).emit(ast, sema)
        for rec in getattr(unit, 'debug_symbols', []) or []:
            rec.setdefault('source', str(src_path))
            if not rec.get('source'):
                rec['source'] = str(src_path)
        return unit
