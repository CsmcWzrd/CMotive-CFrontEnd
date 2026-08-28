from dataclasses import dataclass, field
from typing import Any, List, Optional, Tuple

@dataclass
class Node: pass

@dataclass
class Program(Node):
    declarations: List[Node] = field(default_factory=list)

@dataclass
class Param(Node):
    name: str
    type_name: str = 'I32'
    default: Optional[str] = None

@dataclass
class Function(Node):
    name: str
    params: List[Param]
    body: List[Node]
    return_type: str = 'I32'
    method_of: Optional[str] = None
    decorators: List[str] = field(default_factory=list)
    constructor: bool = False
    destructor: bool = False
    pure_virtual: bool = False
    package: str = 'StartPackage'
    hit_sender: Optional[str] = None
    hit_id: Optional[str] = None

@dataclass
class Field(Node):
    name: str
    type_name: str = 'I32'
    initial: Optional[str] = None
    visibility: str = 'Public'
    block_getset: bool = False
    bit_fields: List[Tuple[int, str]] = field(default_factory=list)

@dataclass
class DynamicStructDecl(Node):
    name: str
    fields: List[Field] = field(default_factory=list)
    package: str = 'StartPackage'

@dataclass
class DynamicStructExpand(Node):
    name: str
    fields: List[Field] = field(default_factory=list)

@dataclass
class ClassDecl(Node):
    name: str
    base: Optional[str] = None
    methods: List[Function] = field(default_factory=list)
    fields: List[Field] = field(default_factory=list)
    has_virtuals: bool = False
    bases: List[Tuple[str, str]] = field(default_factory=list)
    nested: List['ClassDecl'] = field(default_factory=list)
    package: str = 'StartPackage'

@dataclass
class PackageDecl(Node): name: str
@dataclass
class PluginDecl(Node): name: str
@dataclass
class TemplateDecl(Node):
    name: str = ''
    params: List[Param] = field(default_factory=list)
    body: str = ''
    kind: str = 'raw'
    body_node: Optional[Any] = None
    package: str = 'StartPackage'
@dataclass
class BlendDecl(Node): name: str = ''; body: str = ''
@dataclass
class VarDecl(Node): name: str; value: str = '0'; type_name: str = 'I32'; global_decl: bool = False; package: str = 'StartPackage'
@dataclass
class Return(Node): value: str = ''
@dataclass
class ExprStmt(Node): value: str
@dataclass
class TargetStmt(Node):
    sender: str = ''
    object_expr: str = ''
    args: str = ''
    hit_id: str = ''
@dataclass
class If(Node): condition: str; then_body: List[Node]; else_body: List[Node] = field(default_factory=list)
@dataclass
class While(Node): condition: str; body: List[Node]
@dataclass
class DoWhile(Node): body: List[Node]; condition: str
@dataclass
class For(Node): header: str; body: List[Node]
@dataclass
class Break(Node): pass
@dataclass
class Continue(Node): pass
@dataclass
class Throw(Node): value: str
@dataclass
class TryCatch(Node): try_body: List[Node]; catches: List[Tuple[str, List[Node]]] = field(default_factory=list)
@dataclass
class RawStmt(Node): value: str
@dataclass
class RawDecl(Node): value: str
