# use to more easily write variant visitors:

types = """
ExecBlock, ExecExprStmt, ExecBreakStmt, ExecIfStmt, ExecLoopStmt, ExecReturnStmt, ExecYieldStmt,
ExecExprIdentifier, ExecExprLiteral, ExecExprListLiteral, ExecExprAssignMove,
ExecExprAssignEqual, ExecExprIs, ExecExprMemberAccess, ExecExprPointerMemberAccess,
ExecExprBinary, ExecExprCast, ExecExprPreUnary, ExecExprPostUnary, ExecExprSubscript,
ExecExprFnCall, ExecExprBorrow, ExecExprDeref, ExecExprStructInit, ExecExprStructMemberInit,
ExecExprClosure, ExecExprVariantDecomp, ExecExprMatch, ExecExprMatchBranch
"""

RETURN_TYPE = "bool"

names = [t.strip() for t in types.replace("\n", "").split(",") if t.strip()]

template = """\
[&](const {name}& t) -> {ret} {{
    // TODO
}},"""

print("auto vs = Ovld{")
for n in names:
    print("    " + template.format(name=n, ret=RETURN_TYPE))
print("};")
