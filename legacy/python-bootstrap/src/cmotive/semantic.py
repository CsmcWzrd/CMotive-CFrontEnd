from .ast import Function, ClassDecl, VarDecl

class SemanticError(Exception): pass

class SemanticAnalyzer:
    def analyze(self, program):
        funcs = set(); classes = {}; globals_ = set()
        for d in program.declarations:
            if isinstance(d, Function):
                key = (getattr(d, 'package', 'StartPackage'), d.method_of or '', d.name, tuple(p.type_name for p in d.params), d.constructor, d.destructor)
                if key in funcs and not d.constructor:
                    raise SemanticError('duplicate function: ' + ('%s::%s' % (d.method_of or '', d.name)))
                funcs.add(key)
            elif isinstance(d, ClassDecl):
                self.collect_class(d, classes)
            elif isinstance(d, VarDecl) and d.global_decl:
                if d.name in globals_:
                    raise SemanticError('duplicate global variable: ' + d.name)
                globals_.add(d.name)
        for k, c in list(classes.items()):
            if isinstance(k, tuple):
                self.validate_class(c, classes)
        self.detect_inheritance_cycles(classes)
        return {'functions': funcs, 'classes': classes, 'globals': globals_}

    def collect_class(self, cls, classes):
        key = (getattr(cls, 'package', 'StartPackage'), cls.name)
        if key in classes:
            raise SemanticError('duplicate class: ' + getattr(cls, 'package', 'StartPackage') + '::' + cls.name)
        # The current implementation still resolves type names by bare class name,
        # but records package-qualified duplicates for diagnostics and ABI metadata.
        if cls.name in classes:
            raise SemanticError('duplicate class name across packages is not yet supported: ' + cls.name)
        classes[key] = cls
        classes[cls.name] = cls
        for nested in cls.nested:
            self.collect_class(nested, classes)

    def validate_class(self, c, classes):
        if len(c.bases) > 1:
            raise SemanticError('CMotive supports single inheritance; class %s declares %d bases' % (c.name, len(c.bases)))
        if c.base and c.base not in classes:
            raise SemanticError('undefined base class %s for class %s' % (c.base, c.name))
        seen_methods = set()
        for m in c.methods:
            if m.constructor:
                if m.name.lower() != c.name.lower():
                    raise SemanticError('constructor %s must match class %s' % (m.name, c.name))
                key = ('ctor', tuple(p.type_name for p in m.params))
            elif m.destructor:
                raw = m.name[1:] if m.name.startswith('~') else m.name
                if raw.lower() != c.name.lower():
                    raise SemanticError('destructor %s must match class %s' % (m.name, c.name))
                key = ('dtor', ())
            else:
                key = (m.name, tuple(p.type_name for p in m.params))
            if key in seen_methods and not m.constructor:
                raise SemanticError('duplicate method %s in class %s' % (m.name, c.name))
            seen_methods.add(key)

    def detect_inheritance_cycles(self, classes):
        visiting = set(); visited = set()
        def visit(name, chain):
            if name in visited:
                return
            if name in visiting:
                raise SemanticError('inheritance cycle: ' + ' -> '.join(chain + [name]))
            visiting.add(name)
            base = classes[name].base
            if base:
                visit(base, chain + [name])
            visiting.remove(name)
            visited.add(name)
        for name in [k for k in classes.keys() if isinstance(k, str)]:
            visit(name, [])
