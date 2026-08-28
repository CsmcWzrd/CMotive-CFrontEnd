import re
from .ast import (
    Program, Param, Function, ClassDecl, Field, PackageDecl, PluginDecl,
    TemplateDecl, BlendDecl, DynamicStructDecl, DynamicStructExpand, VarDecl, Return, ExprStmt, If, While, DoWhile,
    For, Break, Continue, Throw, TryCatch, RawStmt, TargetStmt, RawDecl
)

TYPE_KINDS = {
    'BOOLEAN','CHAR','CHAR16','CHAR32','DOUBLE','FLOAT','I16','I32','I64','LDOUBLE',
    'UCHAR','U16','U32','U64','VOID','STRUCT','DYNAMIC','TYPE','TSTORE','ID'
}
DECORATORS = {'INLINE','EXTERN','STATIC','OVERRIDABLE','REGISTER','TSTORE','VOLATILE','CONST','GLOBAL','FPTR'}
VISIBILITY = {'PUBLIC':'Public','PRIVATE':'Private','PROTECTED':'Protected'}

class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.i = 0

    def peek(self, offset=0):
        j = self.i + offset
        if j >= len(self.tokens):
            return self.tokens[-1]
        return self.tokens[j]

    def peek_non_eol(self, offset=0):
        j = self.i
        found = 0
        while j < len(self.tokens):
            if self.tokens[j].kind != 'EOL':
                if found == offset:
                    return self.tokens[j]
                found += 1
            j += 1
        return self.tokens[-1]

    def skip_eol(self):
        while self.peek().kind == 'EOL':
            self.i += 1

    def take(self, kind=None, value=None):
        t = self.peek()
        if kind and t.kind != kind:
            raise SyntaxError(f'expected {kind}, got {t.kind} ({t.value!r}) at {t.line}:{t.col}')
        if value and t.value != value:
            raise SyntaxError(f'expected {value!r}, got {t.value!r} at {t.line}:{t.col}')
        self.i += 1
        return t

    def accept(self, value=None, kind=None):
        t = self.peek()
        if kind and t.kind != kind:
            return None
        if value and t.value != value:
            return None
        self.i += 1
        return t

    def accept_kind(self, kind):
        if self.peek().kind == kind:
            return self.take()
        return None

    def token_name(self, t):
        return t.value

    def is_name(self, t=None):
        t = t or self.peek()
        return t.kind == 'ID' or t.kind in TYPE_KINDS or t.kind in {'VOID'}

    def is_type_start(self, t=None):
        t = t or self.peek()
        return t.kind in TYPE_KINDS or t.kind == 'ID'

    def parse(self):
        decls = []
        while self.peek().kind != 'EOF':
            self.skip_eol()
            if self.peek().kind == 'EOF':
                break
            if self.accept(';'):
                continue
            k = self.peek().kind
            if k == 'GLOBAL':
                decls.append(self.var_decl(global_decl=True)); continue
            if k == 'PACKAGE':
                decls.append(self.package_decl()); continue
            if k == 'PLUGIN':
                decls.append(self.plugin_decl()); continue
            if k == 'PLUGSWITCH':
                decls.append(self.skip_plugswitch()); continue
            if k == 'TEMPLATE':
                decls.append(self.template_decl()); continue
            if k == 'DYNAMIC' and self.peek_non_eol(1).kind == 'STRUCT':
                decls.append(self.dynamic_struct_decl()); continue
            if k == 'BLEND' or k == 'ENUM':
                decls.append(self.blend_decl()); continue
            if k == 'HIT':
                decls.append(self.hit_decorated_declaration()); continue
            if k == 'CLASS':
                decls.append(self.class_decl()); continue
            if self.peek().value == '$':
                decls.append(self.out_of_class_method()); continue
            if self.looks_like_var_decl():
                decls.append(self.var_decl(global_decl=True)); continue
            if k == 'FUNC' or k in DECORATORS or self.is_type_start():
                try:
                    decls.append(self.function())
                except SyntaxError:
                    decls.append(self.raw_decl_until(';'))
                continue
            decls.append(self.raw_decl_until(';'))
        self.assign_packages(decls)
        return Program(decls)

    def assign_packages(self, decls):
        current = 'StartPackage'
        for d in decls:
            if isinstance(d, PackageDecl):
                current = d.name or 'StartPackage'
                continue
            self.apply_package(d, current)

    def apply_package(self, node, package):
        if isinstance(node, Function):
            node.package = package
            for s in getattr(node, 'body', []) or []:
                self.apply_package(s, package)
        elif isinstance(node, ClassDecl):
            node.package = package
            for m in node.methods:
                self.apply_package(m, package)
            for n in node.nested:
                self.apply_package(n, package)
        elif isinstance(node, TemplateDecl):
            node.package = package
            if node.body_node is not None:
                self.apply_package(node.body_node, package)
        elif isinstance(node, DynamicStructDecl):
            node.package = package
        elif isinstance(node, DynamicStructExpand):
            pass
        elif isinstance(node, VarDecl):
            node.package = package
        elif isinstance(node, If):
            for s in node.then_body:
                self.apply_package(s, package)
            for s in node.else_body:
                self.apply_package(s, package)
        elif isinstance(node, While):
            for s in node.body:
                self.apply_package(s, package)
        elif isinstance(node, DoWhile):
            for s in node.body:
                self.apply_package(s, package)
        elif isinstance(node, For):
            for s in node.body:
                self.apply_package(s, package)
        elif isinstance(node, TryCatch):
            for s in node.try_body:
                self.apply_package(s, package)
            for _, body in node.catches:
                for s in body:
                    self.apply_package(s, package)

    def dotted_name_until_eol_or_semicolon(self):
        parts = []
        while self.peek().kind not in {'EOF','EOL'} and self.peek().value != ';':
            parts.append(self.take().value)
        self.accept(';')
        return self.compact_tokens(parts)

    def package_decl(self):
        self.take('PACKAGE')
        return PackageDecl(self.dotted_name_until_eol_or_semicolon())

    def plugin_decl(self):
        self.take('PLUGIN')
        return PluginDecl(self.dotted_name_until_eol_or_semicolon())

    def split_top_level_colons(self, text):
        parts, cur, depth, in_str, esc = [], [], 0, False, False
        for ch in str(text or ''):
            if in_str:
                cur.append(ch)
                if esc:
                    esc = False
                elif ch == '\\':
                    esc = True
                elif ch == '"':
                    in_str = False
                continue
            if ch == '"':
                in_str = True; cur.append(ch); continue
            if ch in '([{<': depth += 1
            elif ch in ')]}>': depth -= 1
            if ch == ':' and depth == 0:
                parts.append(''.join(cur).strip()); cur = []
            else:
                cur.append(ch)
        parts.append(''.join(cur).strip())
        return parts

    def parse_hit_prefix(self):
        self.take('HIT')
        spec = self.expr_until_eol_or({';'})
        self.accept(';')
        parts = self.split_top_level_colons(spec)
        if len(parts) == 1:
            sender, hit_id = '', parts[0]
        else:
            sender, hit_id = parts[0], parts[-1]
        return sender.strip(), hit_id.strip()

    def hit_decorated_declaration(self):
        sender, hit_id = self.parse_hit_prefix()
        self.skip_eol()
        if self.peek().value == '$':
            fn = self.out_of_class_method()
        else:
            fn = self.function()
        if not isinstance(fn, Function):
            raise SyntaxError('Hit must prefix a function or method declaration')
        fn.hit_sender = sender
        fn.hit_id = hit_id
        return fn

    def skip_plugswitch(self):
        parts = []
        depth = 0
        while self.peek().kind != 'EOF':
            t = self.take(); parts.append(t.value)
            if t.kind == 'PLUGSWITCH': depth += 1
            elif t.kind == 'PLUGEND':
                depth -= 1
                if depth <= 0: break
        return RawDecl(' '.join(parts))

    def template_decl(self):
        self.take('TEMPLATE')
        params = []
        # Formal CMotive template parameter lines: Name : Type
        while True:
            self.skip_eol()
            if self.peek().kind in {'EOF', 'CLASS', 'FUNC'} or self.peek().value in {'{', ';'}:
                break
            # A non-parameter line starts a function template signature.
            if not (self.is_name() and self.peek_non_eol(1).value == ':'):
                break
            save = self.i
            name = self.take().value
            self.take(value=':')
            typ = self.parse_type_until_line_or({',', '{', ';', '('})
            if (typ or '').strip() != 'Type':
                # This is likely the first function parameter, not a template parameter.
                self.i = save
                break
            params.append(Param(name, typ or 'Type'))
            self.accept(',')
        self.skip_eol()
        if self.peek().kind == 'CLASS':
            cls = self.class_decl()
            return TemplateDecl(cls.name, params, f'class {cls.name}', 'class', cls)
        # Function templates use the same line-oriented function grammar after
        # the Template parameter block.  They are instantiated by codegen only
        # when a concrete Foo<T...>(...) call is seen.
        try:
            fn = self.function()
            return TemplateDecl(fn.name, params, f'function {fn.name}', 'function', fn)
        except SyntaxError:
            body = []
            if self.peek().value == '{':
                body = self.collect_balanced('{','}')
                self.accept(';')
            else:
                while self.peek().kind != 'EOF' and self.peek().value != ';':
                    body.append(self.take().value)
                self.accept(';')
            return TemplateDecl('', params, self.compact_tokens(body), 'raw', None)

    def blend_decl(self):
        name = self.take().value
        body = []
        if self.peek().kind == 'ID':
            name = self.take().value
        self.skip_eol()
        if self.peek().value == '{':
            body = self.collect_balanced('{','}')
            self.accept(';')
        return BlendDecl(name, self.compact_tokens(body))

    def dynamic_struct_decl(self):
        self.take('DYNAMIC')
        self.skip_eol()
        self.take('STRUCT')
        self.skip_eol()
        name = self.take().value
        self.skip_eol()
        fields = self.dynamic_struct_fields()
        self.accept(';')
        return DynamicStructDecl(name, fields)

    def dynamic_struct_expand_stmt(self):
        name = self.take().value
        if self.peek().value != 'Expand':
            raise SyntaxError('expected Expand after dynamic struct name')
        self.take()
        self.skip_eol()
        fields = self.dynamic_struct_fields()
        self.accept(';')
        return DynamicStructExpand(name, fields)

    def dynamic_struct_fields(self):
        self.take(value='{')
        fields = []
        while self.peek().kind != 'EOF' and self.peek().value != '}':
            self.skip_eol()
            if self.peek().value == '}':
                break
            if self.looks_like_var_decl():
                v = self.var_decl(global_decl=False)
                fields.append(Field(v.name, v.type_name, v.value, 'Public'))
                continue
            parts = []
            depth = 0
            while self.peek().kind != 'EOF':
                t = self.peek()
                if depth == 0 and t.value in {';', '}'}:
                    break
                if t.value in {'(', '[', '{'}:
                    depth += 1
                elif t.value in {')', ']', '}'}:
                    depth -= 1
                parts.append(self.take().value)
            self.accept(';')
            clean = [x for x in parts if x not in {',', '\n'}]
            if len(clean) >= 2:
                name = clean[-1]
                typ = self.compact_tokens(clean[:-1])
                fields.append(Field(name, typ or 'I32', None, 'Public'))
        self.take(value='}')
        return fields

    def raw_decl_until(self, term):
        parts = []
        while self.peek().kind != 'EOF':
            if self.peek().value == term:
                parts.append(self.take().value); break
            if self.peek().value == '{':
                parts += self.collect_balanced('{','}')
                self.accept(';')
                break
            parts.append(self.take().value)
        return RawDecl(self.compact_tokens(parts))

    def class_decl(self):
        self.take('CLASS')
        self.skip_eol()
        name = self.take().value
        bases = []
        self.skip_eol()
        if self.peek().kind == 'INHERITS':
            self.take('INHERITS')
            while self.peek().kind != 'EOF' and self.peek().value != '{':
                self.skip_eol()
                if self.peek().value == '{': break
                if not self.is_name(): break
                bname = self.take().value
                self.skip_eol()
                vis = 'Public'
                if self.peek().kind in VISIBILITY:
                    vis = VISIBILITY[self.take().kind]
                bases.append((bname, vis))
                self.skip_eol()
        self.take(value='{')
        fields = []
        methods = []
        nested = []
        has_virtuals = False
        current_visibility = 'Public'
        while self.peek().kind != 'EOF' and self.peek().value != '}':
            self.skip_eol()
            if self.peek().value == '}': break
            if self.peek().kind in VISIBILITY:
                current_visibility = VISIBILITY[self.take().kind]
                self.skip_eol()
                if self.accept('{'):
                    while self.peek().kind != 'EOF' and self.peek().value != '}':
                        self.parse_class_item(name, current_visibility, fields, methods, nested)
                    self.take(value='}')
                continue
            self.parse_class_item(name, current_visibility, fields, methods, nested)
        self.take(value='}')
        self.accept(';')
        return ClassDecl(name, bases[0][0] if bases else None, methods, fields, any(m.decorators and 'Overridable' in m.decorators for m in methods), bases, nested)

    def parse_class_item(self, class_name, visibility, fields, methods, nested):
        self.skip_eol()
        if self.peek().kind == 'EOF' or self.peek().value == '}':
            return
        if self.peek().kind == 'HIT':
            sender, hit_id = self.parse_hit_prefix()
            self.skip_eol()
            fn = self.function(in_class=class_name)
            fn.hit_sender = sender
            fn.hit_id = hit_id
            methods.append(fn)
            return
        if self.peek().kind == 'CLASS':
            nested.append(self.class_decl()); return
        if self.peek().kind == 'VAR':
            self.take('VAR')
            fname = self.take().value
            fields.append(Field(fname, 'I32', None, visibility))
            while self.peek().kind != 'EOF' and self.peek().value != ';': self.take()
            self.accept(';')
            return
        if self.looks_like_var_decl():
            f = self.var_decl(global_decl=False)
            fld = Field(f.name, f.type_name, f.value, visibility)
            fld.bit_fields = getattr(f, 'bit_fields', [])
            fld.block_getset = getattr(f, 'block_getset', False)
            fields.append(fld); return
        if self.peek().kind in DECORATORS or self.peek().kind == 'FUNC' or self.peek().value == '~' or self.is_type_start() or self.peek().value == class_name:
            try:
                fn = self.function(in_class=class_name)
                methods.append(fn)
                if 'Overridable' in fn.decorators or fn.pure_virtual:
                    fn.decorators.append('Overridable') if 'Overridable' not in fn.decorators else None
                return
            except SyntaxError:
                # Continue as raw class member if it is not a method after all.
                pass
        while self.peek().kind != 'EOF' and self.peek().value not in {';','}'}:
            if self.peek().value == '{':
                self.collect_balanced('{','}')
            else:
                self.take()
        self.accept(';')

    def looks_like_var_decl(self):
        first = self.peek_non_eol(0)
        if first.kind == 'GLOBAL':
            return self.is_name(self.peek_non_eol(1)) and self.peek_non_eol(2).value == ':'
        return self.is_name(first) and self.peek_non_eol(1).value == ':'

    def strip_storage_from_type(self, typ):
        t = typ or 'I32'
        is_global = bool(re.search(r'\bGlobal\b', t))
        t = re.sub(r'\bGlobal\b', '', t).strip()
        t = re.sub(r'\s+', ' ', t).strip()
        return (t or 'I32'), is_global

    def var_decl(self, global_decl=False):
        prefix_global = False
        if self.peek().kind == 'GLOBAL':
            prefix_global = True
            self.take('GLOBAL')
            self.skip_eol()
        name = self.take().value
        self.take(value=':')
        typ = self.parse_type_until_line_or({'=',';','{','BLOCK'})
        typ, type_global = self.strip_storage_from_type(typ)
        bit_fields = []
        if self.peek().value == '{':
            bit_fields = self.parse_bit_fields()
        val = '0'
        if self.accept('='):
            val = self.expr_until(';', stop_at_eol=False)
        block = False
        if self.peek().kind == 'BLOCK':
            block = True; self.take()
        self.accept(';')
        if self.peek().kind == 'BLOCK':
            block = True; self.take(); self.accept(';')
        # Store bit field metadata in a side-car string for VarDecl users. Class
        # members are represented as Field and populated by caller when needed.
        vd = VarDecl(name, val or '0', typ or 'I32', bool(global_decl or prefix_global or type_global))
        vd.bit_fields = bit_fields
        vd.block_getset = block
        return vd

    def parse_bit_fields(self):
        bits = []
        self.take(value='{')
        while self.peek().kind != 'EOF' and self.peek().value != '}':
            self.skip_eol()
            if self.peek().value == '}': break
            count = self.take().value
            self.take(value=':')
            name = self.take().value
            try: count_int = int(count, 0)
            except Exception: count_int = 1
            bits.append((count_int, name))
            self.accept(',')
        self.take(value='}')
        return bits

    def out_of_class_method(self):
        self.take(value='$')
        cls = self.take().value
        self.skip_eol()
        decorators = []
        while self.peek().kind in DECORATORS:
            decorators.append(self.take().value)
            self.skip_eol()
        # CMotive permits out-of-class method bodies as:
        #   $ClassName
        #   MethodName
        #   arg : Type
        #   ()
        #   { ... }
        # where the return type is taken from the declaration in the .HMOT file.
        # When a concrete return type is provided, fall back to normal function parsing.
        if self.peek().kind == 'ID' and (self.peek_non_eol(1).value == '(' or self.peek_non_eol(2).value == ':'):
            name = self.take().value
            params = []
            self.skip_eol()
            while self.peek().kind != 'EOF':
                self.skip_eol()
                if self.peek().value == '(' and self.peek_non_eol(1).value == ')':
                    self.take(value='('); self.skip_eol(); self.take(value=')'); break
                if self.peek().value == '(':
                    params = self.parse_old_paren_params(); break
                if self.peek().value in {'{',';','='}: break
                if not self.is_name() or self.peek_non_eol(1).value != ':':
                    break
                pname = self.take().value; self.take(value=':')
                ptype = self.parse_type_until_line_or({'=',',','{',';','('})
                default = None
                if self.accept('='):
                    default = self.expr_until_eol_or({',','('})
                params.append(Param(pname, ptype or 'I32', default))
                self.accept(',')
            self.skip_eol()
            body = self.block() if self.peek().value == '{' else []
            self.accept(';')
            return Function(name, params, body, 'Void', cls, decorators)
        # Rewind decorators by reparsing through the normal routine is awkward,
        # so construct the method directly when decorators were already consumed.
        if decorators:
            if not self.is_type_start():
                raise SyntaxError(f'expected method return type or name at {self.peek().line}:{self.peek().col}')
            return_type = self.parse_type_name_for_signature()
            self.skip_eol()
            if self.peek().kind == 'OPERATION':
                name = self.parse_operation_name_after_keyword()
            else:
                name = self.take().value
            self.skip_eol()
            params = []
            if self.peek().value == '(':
                params = self.parse_old_paren_params()
            else:
                while self.peek().kind != 'EOF':
                    self.skip_eol()
                    if self.peek().value == '(' and self.peek_non_eol(1).value == ')':
                        self.take(value='('); self.skip_eol(); self.take(value=')'); break
                    if self.peek().value in {'{',';','='}: break
                    if not self.is_name() or self.peek_non_eol(1).value != ':': break
                    pname = self.take().value; self.take(value=':')
                    ptype = self.parse_type_until_line_or({'=',',','{',';','('})
                    default = self.expr_until_eol_or({',','('}) if self.accept('=') else None
                    params.append(Param(pname, ptype or 'I32', default))
                    self.accept(',')
            self.skip_eol()
            body = self.block() if self.peek().value == '{' else []
            self.accept(';')
            return Function(name, params, body, return_type, cls, decorators)
        fn = self.function(in_class=cls, force_method_of=cls)
        return fn


    def operation_symbol_name(self, op):
        table = {
            '+':'Plus', '-':'Minus', '*':'Multiply', '/':'Divide', '%':'Modulo',
            '==':'Equal', '!=':'NotEqual', '<':'Less', '>':'Greater', '<=':'LessEqual', '>=':'GreaterEqual',
            '[]':'Index', '()':'Call', '=':'Assign', '+=':'PlusAssign', '-=':'MinusAssign',
            '*=':'MultiplyAssign', '/=':'DivideAssign', '%=':'ModuloAssign', '<<':'LeftShift', '>>':'RightShift',
            '>>>':'RightRotate', '<<<':'LeftRotate', '&':'BitAnd', '|':'BitOr', '^':'BitXor', '!':'Not'
        }
        return table.get(str(op).strip(), re.sub(r'\W+', '_', str(op).strip()).strip('_') or 'Unknown')

    def parse_operation_name_after_keyword(self):
        self.take('OPERATION')
        parts = []
        depth = 0
        while self.peek().kind != 'EOF':
            t = self.peek()
            if depth == 0 and (t.kind == 'EOL' or t.value in {';', '{'}):
                break
            if depth == 0 and t.value == '(':
                # Keep Operation() distinct from the parameter-list opener only when it is written as a symbol.
                if parts:
                    break
            if depth == 0 and self.is_name(t) and parts:
                break
            if t.value in {'(', '[', '{'}: depth += 1
            elif t.value in {')', ']', '}'}: depth -= 1
            parts.append(self.take().value)
            # Operators are one logical token except [] and ().
            if depth == 0 and parts:
                if ''.join(parts) not in {'[', '('}:
                    break
        op = ''.join(parts).strip()
        if op == '[' and self.peek().value == ']':
            self.take(value=']'); op = '[]'
        elif op == '(' and self.peek().value == ')':
            self.take(value=')'); op = '()'
        return 'Operation__' + self.operation_symbol_name(op)

    def function(self, in_class=None, force_method_of=None):
        self.skip_eol()
        decorators = []
        while self.peek().kind in DECORATORS:
            decorators.append(self.take().value)
            self.skip_eol()
        if self.peek().kind == 'FUNC':
            return self.old_function(in_class, force_method_of, decorators)

        constructor = False; destructor = False
        return_type = 'I32'
        if in_class and self.peek().value == '~':
            self.take(value='~'); name = self.take().value; return_type = 'Void'; destructor = True
            if name.lower() != str(in_class).lower():
                raise SyntaxError(f'destructor ~{name} does not match class {in_class} at {self.peek().line}:{self.peek().col}')
        elif in_class and self.peek().value.lower() == str(in_class).lower():
            name = self.take().value; return_type = 'Void'; constructor = True
        else:
            if not self.is_type_start():
                raise SyntaxError(f'expected type at {self.peek().line}:{self.peek().col}')
            return_type = self.parse_type_name_for_signature()
            self.skip_eol()
            if self.peek().kind == 'OPERATION':
                name = self.parse_operation_name_after_keyword()
            else:
                name = self.take().value
        self.skip_eol()
        params = []
        if self.peek().value == '(':
            params = self.parse_old_paren_params()
        else:
            while self.peek().kind != 'EOF':
                self.skip_eol()
                if self.peek().value == '(' and self.peek_non_eol(1).value == ')':
                    self.take(value='('); self.skip_eol(); self.take(value=')'); break
                if self.peek().value == '(':
                    params = self.parse_old_paren_params(); break
                if self.peek().value in {'{',';','='}: break
                if not self.is_name() or self.peek_non_eol(1).value != ':':
                    break
                pname = self.take().value; self.take(value=':')
                ptype = self.parse_type_until_line_or({'=',',','{',';','('})
                default = None
                if self.accept('='):
                    default = self.expr_until_eol_or({',','('})
                params.append(Param(pname, ptype or 'I32', default))
                self.accept(',')
        self.skip_eol()
        pure_virtual = False
        body = []
        if self.accept('='):
            self.expr_until(';')
            self.accept(';')
            pure_virtual = True
        elif self.peek().value == '{':
            body = self.block()
        elif self.accept(';'):
            body = []
        else:
            raise SyntaxError(f'expected function body for {name} at {self.peek().line}:{self.peek().col}')
        return Function(name, params, body, return_type, force_method_of or in_class, decorators, constructor, destructor, pure_virtual)

    def old_function(self, in_class=None, force_method_of=None, decorators=None):
        decorators = decorators or []
        self.take('FUNC')
        name = self.take().value
        params = self.parse_old_paren_params()
        ret = 'I32'
        if self.accept(':'):
            ret = self.parse_type_name_for_signature()
        self.skip_eol()
        body = self.block() if self.peek().value == '{' else []
        self.accept(';')
        return Function(name, params, body, ret, force_method_of or in_class, decorators)

    def parse_old_paren_params(self):
        params = []
        self.take(value='(')
        while self.peek().kind != 'EOF' and self.peek().value != ')':
            self.skip_eol()
            if self.peek().value == ')': break
            if self.is_name():
                pname = self.take().value
                ptype = 'I32'
                if self.accept(':'):
                    ptype = self.parse_type_until_line_or({',',')','='})
                if self.accept('='):
                    default = self.expr_until_eol_or({',',')'})
                else:
                    default = None
                params.append(Param(pname, ptype or 'I32', default))
            else:
                self.take()
            self.accept(',')
        self.take(value=')')
        return params

    def parse_type_name_for_signature(self):
        parts = [self.take().value]
        while self.peek().value in {'*','&'} or self.peek().kind == 'CONST' or self.peek().value == '::':
            parts.append(self.take().value)
            if parts[-1] == '::' and self.is_name():
                parts.append(self.take().value)
        return self.compact_tokens(parts)

    def parse_type_until_line_or(self, stops):
        parts = []
        depth = 0
        while self.peek().kind != 'EOF':
            t = self.peek()
            if depth == 0 and (t.value in stops or t.kind in stops):
                break
            if depth == 0 and t.kind == 'EOL':
                break
            if t.value == '(' or t.value == '[': depth += 1
            elif t.value == ')' or t.value == ']': depth -= 1
            parts.append(self.take().value)
        self.skip_eol()
        return self.compact_tokens(parts)

    def block(self):
        self.take(value='{')
        body = []
        while self.peek().kind != 'EOF' and self.peek().value != '}':
            self.skip_eol()
            if self.peek().value == '}': break
            stmt = self.statement()
            if stmt is not None:
                body.append(stmt)
        self.take(value='}')
        return body

    def statement(self):
        self.skip_eol()
        k = self.peek().kind
        if self.accept(';'):
            return None
        if k == 'ID' and self.peek_non_eol(1).value == 'Expand':
            return self.dynamic_struct_expand_stmt()
        if k == 'TARGET':
            return self.target_stmt()
        if k == 'RETURN':
            self.take('RETURN')
            val = '' if self.peek().value == ';' else self.expr_until(';')
            self.accept(';')
            return Return(val)
        if k == 'BREAK':
            self.take('BREAK'); self.accept(';'); return Break()
        if k == 'CONTINUE':
            self.take('CONTINUE'); self.accept(';'); return Continue()
        if k == 'IF':
            self.take('IF')
            cond = self.paren_expr_or_until_block()
            self.skip_eol()
            then = self.block()
            else_body = []
            self.skip_eol()
            if self.peek().kind == 'ELIF':
                self.take('ELIF')
                c2 = self.paren_expr_or_until_block()
                self.skip_eol()
                b2 = self.block()
                else_body = [If(c2, b2, [])]
            self.skip_eol()
            if self.peek().kind == 'ELSE':
                self.take('ELSE')
                self.skip_eol()
                final_else = self.block()
                if else_body and isinstance(else_body[0], If):
                    else_body[0].else_body = final_else
                else:
                    else_body = final_else
            return If(cond, then, else_body)
        if k == 'WHILE':
            self.take('WHILE')
            cond = self.paren_expr_or_until_block()
            self.skip_eol()
            return While(cond, self.block())
        if k == 'DO':
            self.take('DO')
            self.skip_eol()
            body = self.block()
            self.skip_eol()
            if self.peek().kind == 'WHILE': self.take('WHILE')
            cond = self.paren_expr_or_until_semicolon()
            self.accept(';')
            return DoWhile(body, cond)
        if k == 'FOR':
            self.take('FOR')
            header = self.paren_expr_or_until_block()
            self.skip_eol()
            return For(header, self.block())
        if k == 'THROW':
            self.take('THROW')
            val = self.expr_until(';')
            self.accept(';')
            return Throw(val)
        if k == 'TRY':
            self.take('TRY')
            self.skip_eol()
            tb = self.block()
            self.skip_eol()
            catches = []
            while self.peek().kind in {'CATCH','CATCHALL'}:
                ck = self.take().kind
                spec = 'all'
                if self.peek().value == '(':
                    spec = self.paren_expr_or_until_block()
                self.skip_eol()
                catches.append((spec, self.block()))
                self.skip_eol()
            return TryCatch(tb, catches)
        if k == 'SWITCH':
            return self.raw_keyword_block_statement()
        if k == 'GLOBAL':
            return self.var_decl(global_decl=True)
        if k == 'VAR':
            self.take('VAR')
            name = self.take().value
            typ = 'I32'
            if self.accept(':'):
                typ = self.parse_type_until_line_or({'=',';'})
            val = '0'
            if self.accept('='):
                val = self.expr_until(';')
            self.accept(';')
            return VarDecl(name, val, typ, False)
        if self.looks_like_var_decl():
            return self.var_decl(global_decl=False)
        if self.peek().value == '{':
            return RawStmt(self.compact_tokens(self.collect_balanced('{','}')))
        expr = self.expr_until(';')
        self.accept(';')
        return ExprStmt(expr)

    def target_stmt(self):
        self.take('TARGET')
        spec = self.expr_until_eol_or({';'})
        self.accept(';')
        parts = self.split_top_level_colons(spec)
        while len(parts) < 4:
            parts.append('')
        sender = parts[0]
        object_expr = parts[1]
        args = ':'.join(parts[2:-1]).strip() if len(parts) > 3 else ''
        hit_id = parts[-1]
        return TargetStmt(sender.strip(), object_expr.strip(), args.strip(), hit_id.strip())

    def raw_keyword_block_statement(self):
        parts = [self.take().value]
        if self.peek().value == '(':
            parts += self.collect_balanced('(',')')
        self.skip_eol()
        if self.peek().value == '{':
            parts += self.collect_balanced('{','}')
        else:
            while self.peek().kind != 'EOF' and self.peek().value != ';':
                parts.append(self.take().value)
            self.accept(';')
        return RawStmt(self.compact_tokens(parts))

    def paren_expr_or_until_block(self):
        self.skip_eol()
        if self.peek().value == '(':
            inner = self.collect_balanced('(',')')[1:-1]
            return self.compact_tokens(inner)
        parts = []
        while self.peek().kind != 'EOF' and self.peek().value != '{':
            parts.append(self.take().value)
        return self.compact_tokens(parts)

    def paren_expr_or_until_semicolon(self):
        self.skip_eol()
        if self.peek().value == '(':
            inner = self.collect_balanced('(',')')[1:-1]
            return self.compact_tokens(inner)
        return self.expr_until(';')

    def expr_until(self, term, stop_at_eol=False):
        parts = []
        depth = 0
        while self.peek().kind != 'EOF':
            t = self.peek()
            if depth == 0 and t.value == term:
                break
            if stop_at_eol and depth == 0 and t.kind == 'EOL':
                break
            if t.value in {'(', '[', '{'}: depth += 1
            elif t.value in {')', ']', '}'}: depth -= 1
            parts.append(self.take().value)
        return self.compact_tokens(parts)

    def expr_until_eol_or(self, stops):
        parts = []
        depth = 0
        while self.peek().kind != 'EOF':
            t = self.peek()
            if depth == 0 and (t.value in stops or t.kind == 'EOL'):
                break
            if t.value in {'(', '[', '{'}: depth += 1
            elif t.value in {')', ']', '}'}: depth -= 1
            parts.append(self.take().value)
        self.skip_eol()
        return self.compact_tokens(parts)

    def collect_balanced(self, openv, closev):
        parts = []
        depth = 0
        while self.peek().kind != 'EOF':
            t = self.take(); parts.append(t.value)
            if t.value == openv:
                depth += 1
            elif t.value == closev:
                depth -= 1
                if depth == 0:
                    break
        return parts

    def compact_tokens(self, values):
        values = [v for v in values if v != '\n']
        if not values:
            return ''
        out = ''
        prev = ''
        no_space_before = {')',']','}',',',';','.', '::'}
        no_space_after = {'(','[','{','.', '::', '~', '$'}
        ops_no_space = {'*','&'}
        for v in values:
            if not out:
                out = v
            elif v in no_space_before or prev in no_space_after:
                out += v
            elif v in ops_no_space and prev not in {')',']'}:
                out += v
            elif prev in ops_no_space:
                out += v
            else:
                out += ' ' + v
            prev = v
        return out
