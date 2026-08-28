from dataclasses import dataclass
import re
import json
from typing import List

# CMotive keywords are specified with capitalized spelling.  The lexer also
# accepts the earlier bootstrap lowercase spellings so existing conformance
# tests and examples continue to compile while the parser moves toward the
# formal CMotive grammar.
KEYWORD_KIND = {
    'Blend':'BLEND','Boolean':'BOOLEAN','Break':'BREAK','Block':'BLOCK','Case':'CASE',
    'Catch':'CATCH','Catchall':'CATCHALL','Char':'CHAR','Char16':'CHAR16','Char32':'CHAR32',
    'Class':'CLASS','Const':'CONST','Continue':'CONTINUE','Contains':'CONTAINS','Default':'DEFAULT',
    'Delete':'DELETE','Do':'DO','Double':'DOUBLE','Dynamic':'DYNAMIC','Elif':'ELIF','Else':'ELSE',
    'Enum':'ENUM','Extern':'EXTERN','False':'FALSE','Float':'FLOAT','For':'FOR','Fptr':'FPTR',
    'Get':'GET','Getall':'GETALL','Global':'GLOBAL','Goto':'GOTO','I16':'I16','I32':'I32',
    'I64':'I64','Int16':'I16','Int32':'I32','Int':'I64','If':'IF','Inherits':'INHERITS','Inline':'INLINE','Ldouble':'LDOUBLE',
    'Package':'PACKAGE','New':'NEW','Not':'NOT','Null':'NULL','Operation':'OPERATION',
    'Overridable':'OVERRIDABLE','Plugin':'PLUGIN','Plugcase':'PLUGCASE','Plugdefault':'PLUGDEFAULT',
    'Plugswitch':'PLUGSWITCH','Plugend':'PLUGEND','Private':'PRIVATE','Protected':'PROTECTED',
    'Public':'PUBLIC','Register':'REGISTER','Replace':'REPLACE','Return':'RETURN','Set':'SET',
    'Setall':'SETALL','Sizeof':'SIZEOF','Static':'STATIC','Struct':'STRUCT','Switch':'SWITCH',
    'Template':'TEMPLATE','Throw':'THROW','This':'THIS','Target':'TARGET','Hit':'HIT','Tstore':'TSTORE','ThreadStore':'TSTORE','True':'TRUE',
    'Try':'TRY','Type':'TYPE','Uchar':'UCHAR','U16':'U16','Uint16':'U16','U32':'U32','Uint32':'U32','U64':'U64','Uint':'U64',
    'Void':'VOID','Volatile':'VOLATILE','While':'WHILE',
    # Historical/bootstrap aliases.
    'class':'CLASS','extends':'INHERITS','func':'FUNC','return':'RETURN','var':'VAR',
    'if':'IF','else':'ELSE','while':'WHILE','new':'NEW','delete':'DELETE','virtual':'OVERRIDABLE',
    'package':'PACKAGE','plugin':'PLUGIN','template':'TEMPLATE','try':'TRY','catch':'CATCH',
    'throw':'THROW','target':'TARGET','hit':'HIT','void':'VOID','int':'I32','float':'FLOAT','double':'DOUBLE','const':'CONST',
    'break':'BREAK','continue':'CONTINUE','for':'FOR','switch':'SWITCH','case':'CASE','default':'DEFAULT',
}

MULTI_OPS = [
    '>>>','<<<','==','!=','<=','>=','->','::','++','--','&&','||','<<','>>','+=','-=','*=','/=',
    '%=','&=','|=','^=','##','...'
]
SINGLE_OPS = set('{}()[]<>;:,.=+-*/%&|^!~?$@')
ID_RE = re.compile(r'[A-Za-z_]\w*')
NUM_RE = re.compile(r'0[xX][0-9A-Fa-f]+|\d+(?:\.\d+)?(?:[eE][+-]?\d+)?[uUlLfF]*')

@dataclass
class Token:
    kind: str
    value: str
    line: int
    col: int

class Lexer:
    def tokenize(self, text: str) -> List[Token]:
        toks: List[Token] = []
        i = 0
        line = 1
        col = 1
        n = len(text)

        def add(kind: str, value: str, ln=None, co=None):
            toks.append(Token(kind, value, line if ln is None else ln, col if co is None else co))

        def advance(s: str):
            nonlocal line, col
            parts = s.split('\n')
            if len(parts) > 1:
                line += len(parts) - 1
                col = len(parts[-1]) + 1
            else:
                col += len(s)

        while i < n:
            ch = text[i]
            start_line, start_col = line, col
            if ch in ' \t\v\f':
                j = i + 1
                while j < n and text[j] in ' \t\v\f':
                    j += 1
                advance(text[i:j]); i = j; continue
            if ch == '\r' or ch == '\n':
                if ch == '\r' and i + 1 < n and text[i+1] == '\n':
                    add('EOL', '\n', start_line, start_col); advance('\r\n'); i += 2
                else:
                    add('EOL', '\n', start_line, start_col); advance(ch); i += 1
                continue
            if text.startswith('//', i):
                j = text.find('\n', i)
                if j < 0:
                    advance(text[i:]); break
                advance(text[i:j]); i = j; continue
            if text.startswith('/*', i):
                j = text.find('*/', i + 2)
                j = n - 2 if j < 0 else j
                frag = text[i:j+2]
                advance(frag); i = j + 2; continue
            if ch == '"':
                j = i + 1; esc = False
                while j < n:
                    c = text[j]
                    if esc:
                        esc = False
                    elif c == '\\':
                        esc = True
                    elif c == '"':
                        j += 1; break
                    j += 1
                val = text[i:j]
                add('STRING', val, start_line, start_col); advance(val); i = j; continue
            if ch == "'":
                j = i + 1; esc = False
                while j < n:
                    c = text[j]
                    if esc:
                        esc = False
                    elif c == '\\':
                        esc = True
                    elif c == "'":
                        j += 1; break
                    j += 1
                val = text[i:j]
                add('CHAR_LITERAL', val, start_line, start_col); advance(val); i = j; continue
            m = ID_RE.match(text, i)
            if m:
                value = m.group(0)
                # Contains { ... } is a CMotive raw string form.  Fold it into one
                # STRING token so the parser can use it anywhere an initializer or
                # expression string literal is valid.
                if value == 'Contains':
                    j = m.end(); k = j
                    while k < n and text[k] in ' \t\v\f\r\n':
                        k += 1
                    if k < n and text[k] == '{':
                        depth = 1; p = k + 1; esc = False
                        while p < n and depth:
                            c = text[p]
                            if esc:
                                esc = False
                            elif c in ('\\','/'):
                                esc = True
                            elif c == '{':
                                depth += 1
                            elif c == '}':
                                depth -= 1
                                if depth == 0:
                                    break
                            p += 1
                        raw = text[k+1:p]
                        # Preserve raw content, but make it a C-compatible literal.
                        literal = json.dumps(raw)
                        add('STRING', literal, start_line, start_col)
                        consumed = text[i:p+1]
                        advance(consumed); i = p + 1; continue
                kind = KEYWORD_KIND.get(value, 'ID')
                add(kind, value, start_line, start_col)
                advance(value); i = m.end(); continue
            m = NUM_RE.match(text, i)
            if m:
                value = m.group(0)
                add('NUMBER', value, start_line, start_col); advance(value); i = m.end(); continue
            matched = None
            for op in MULTI_OPS:
                if text.startswith(op, i):
                    matched = op; break
            if matched:
                add('OP', matched, start_line, start_col); advance(matched); i += len(matched); continue
            if ch in SINGLE_OPS:
                add('OP', ch, start_line, start_col); advance(ch); i += 1; continue
            raise SyntaxError(f'unexpected character {ch!r} at {start_line}:{start_col}')
        toks.append(Token('EOF', '', line, col))
        return toks
