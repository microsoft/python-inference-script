# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import ast
from textwrap import dedent
import inspect
from typing import List


class Variable:
    def __init__(self, id, t) -> None:
        self.id = id
        self.t = t


class Stmt:
    def __init__(self) -> None:
        self.obj = None
        self.op = None
        self.inputs: List[Variable] = []
        self.outputs: List[Variable] = []


class IfBlock(Stmt):
    def __init__(self) -> None:
        super().__init__()


class ForBlock(Stmt):
    def __init__(self) -> None:
        super().__init__()


class SSAGraph:
    def __init__(self) -> None:
        self.stmts = []

    def add_stmt(self, stmt: Stmt):
        self.stmts.append(stmt)

class SSAPass(ast.NodeTransformer):

    def __init__(self, module, func):
        self.ssa_graph = SSAGraph()
        self.source = dedent(inspect.getsource(func))
        self.ast = ast.parse(self.source)
        self.visit(self.ast)
    
    def visit_BinOp(self, node: ast.BinOp):
        stmt = Stmt()
        stmt.op = node.op
        stmt.inputs = [
            Variable(node.left.id, int), 
            Variable(node.right.id, int)]
        return stmt

    def visit_Assign(self, node: ast.Assign):
        stmt = self.visit(node.value)
        stmt.outputs = node.targets
        self.ssa_graph.add_stmt(stmt)
