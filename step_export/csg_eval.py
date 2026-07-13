# Minimal OpenSCAD-CSG -> BRep evaluator for FreeCAD (freecadcmd).
# Replaces importCSG for this project: expects a .csg pre-processed by
# flatten2d.py (all linear_extrude bodies are single `polyset` nodes,
# loops in a JSON sidecar), so the only nodes left are group/union/
# difference/multmatrix/color/linear_extrude/polyset/cylinder.
# Transforms accumulate top-down and apply at leaves (rigid only, so
# primitive surfaces stay exact); booleans use fuzzy tolerance; faces
# with holes are built with FaceMakerBullseye. color() nodes tag their
# fully-transformed subtree as a named part (assembly marker colors).
import json, os, re
import FreeCAD
import Part
import Import
from FreeCAD import Vector

csg_path = os.environ["CSG_IN"]
loops_path = os.environ["LOOPS_JSON"]
out_path = os.environ["STEP_OUT"]
part_name = os.environ.get("PART_NAME", "part")
log = open(csg_path + ".eval.log", "w", buffering=1)

def say(*a):
    log.write(" ".join(str(x) for x in a) + "\n")

LOOPS = {int(k): v for k, v in json.load(open(loops_path)).items()}

COLOR_NAMES = {  # both palettes of true_colors (see wordclock.scad)
    (0x45, 0x4b, 0x58): "faceplate", (0xe8, 0x73, 0x4d): "faceplate",
    (0x56, 0x5f, 0x70): "shell",     (0x4a, 0x90, 0xd9): "shell",
    (0x36, 0x3c, 0x48): "stand_feet", (0x7c, 0xb3, 0x42): "stand_feet",
}

FUZZ = 1e-6

class Node:
    def __init__(self, name, args, term):
        self.name, self.args, self.kids = name, args, []

def parse(path):
    root = Node("root", None, "{")
    stack = [root]
    pat = re.compile(r"^\s*([a-z_]+)\((.*)\)\s*([{;])\s*$")
    for ln in open(path):
        s = ln.strip()
        if not s:
            continue
        if s == "}":
            stack.pop()
            continue
        m = pat.match(s)
        assert m, "unparsed line: %r" % s
        name, argstr, term = m.groups()
        argstr = (argstr.replace("$", "").replace("true", "True")
                  .replace("false", "False").replace("undef", "None"))
        if "=" in argstr:
            args = eval("dict(%s)" % argstr, {"__builtins__": {}, "dict": dict})
        elif argstr.strip():
            args = eval("(%s)" % argstr, {"__builtins__": {}})
        else:
            args = None
        node = Node(name, args, term)
        stack[-1].kids.append(node)
        if term == "{":
            stack.append(node)
    return root

tagged = []   # (name, shape)

def combine(shapes):
    shapes = [s for s in shapes if s is not None]
    if not shapes:
        return None
    if len(shapes) == 1:
        return shapes[0]
    return shapes[0].multiFuse(shapes[1:], FUZZ)

def build(node, M):
    n = node.name
    if n in ("group", "union"):
        return combine([build(k, M) for k in node.kids])
    if n == "color":
        sh = combine([build(k, M) for k in node.kids])
        if sh is not None:
            rgb = tuple(int(round(c * 255)) for c in node.args[:3])
            tagged.append((COLOR_NAMES.get(rgb, "part_%02x%02x%02x" % rgb), sh))
        return None
    if n == "difference":
        kids = [build(k, M) for k in node.kids]
        base, tools = kids[0], [k for k in kids[1:] if k is not None]
        if base is None:
            return None
        return base.cut(tools, FUZZ) if tools else base
    if n == "multmatrix":
        rows = [[float(v) for v in row] for row in node.args]
        # OpenSCAD prints matrices with 6 significant digits, so pure
        # rotations arrive slightly non-orthonormal; transformShape
        # would then fall back to geometry distortion and mating faces
        # across sibling subtrees stop lining up. Re-orthonormalize
        # (Gram-Schmidt) when the 3x3 is within rounding of a rotation.
        r = [rows[i][:3] for i in range(3)]
        def dot(a, b): return sum(x*y for x, y in zip(a, b))
        det = (r[0][0]*(r[1][1]*r[2][2] - r[1][2]*r[2][1])
             - r[0][1]*(r[1][0]*r[2][2] - r[1][2]*r[2][0])
             + r[0][2]*(r[1][0]*r[2][1] - r[1][1]*r[2][0]))
        if det > 0.5 and all(abs(dot(r[i], r[j]) - (i == j)) < 1e-4
                             for i in range(3) for j in range(3)):
            n0 = dot(r[0], r[0]) ** 0.5
            r[0] = [x / n0 for x in r[0]]
            d = dot(r[1], r[0])
            r[1] = [x - d*y for x, y in zip(r[1], r[0])]
            n1 = dot(r[1], r[1]) ** 0.5
            r[1] = [x / n1 for x in r[1]]
            r[2] = [r[0][1]*r[1][2] - r[0][2]*r[1][1],
                    r[0][2]*r[1][0] - r[0][0]*r[1][2],
                    r[0][0]*r[1][1] - r[0][1]*r[1][0]]
            rows = [r[i] + [rows[i][3]] for i in range(3)] + [rows[3]]
        m2 = FreeCAD.Matrix(*[v for row in rows for v in row])
        return combine([build(k, M.multiply(m2)) for k in node.kids])
    if n == "linear_extrude":
        assert node.args.get("scale", [1, 1]) == [1, 1] and \
               not node.args.get("twist"), "unsupported extrude " + str(node.args)
        (kid,) = node.kids
        assert kid.name == "polyset", kid.name
        loops = LOOPS[kid.args["id"]]
        if not loops:
            return None
        wires = []
        for lp in loops:
            pts = [Vector(x, y, 0) for x, y in lp]
            wires.append(Part.makePolygon(pts + [pts[0]]))
        face = Part.makeFace(wires, "Part::FaceMakerBullseye")
        h = float(node.args["height"])
        sol = face.extrude(Vector(0, 0, h))
        if node.args.get("center"):
            sol.translate(Vector(0, 0, -h / 2))
        sol = sol.copy()
        sol.transformShape(M, False, True)
        return sol
    if n == "cylinder":
        r1, r2 = float(node.args["r1"]), float(node.args["r2"])
        h = float(node.args["h"])
        sol = Part.makeCylinder(r1, h) if r1 == r2 else \
              Part.makeCone(r1, r2, h)
        if node.args.get("center"):
            sol.translate(Vector(0, 0, -h / 2))
        sol.transformShape(M, False, True)
        return sol
    raise ValueError("unhandled node: " + n)

root = parse(csg_path)
rest = combine([build(k, FreeCAD.Matrix()) for k in root.kids])
if rest is not None and not tagged:
    tagged.append((part_name, rest))

doc = FreeCAD.newDocument("out")
objs = []
for name, sh in tagged:
    try:
        ref = sh.removeSplitter()
        if ref.isValid() and abs(ref.Volume - sh.Volume) < 1e-6 * max(1, sh.Volume):
            sh = ref
    except Exception as e:
        say(name, "removeSplitter failed:", repr(e))
    o = doc.addObject("Part::Feature", name)
    o.Label = name
    o.Shape = sh
    objs.append(o)
    say("%-12s valid=%s solids=%d faces=%d vol=%.4f" %
        (name, sh.isValid(), len(sh.Solids), len(sh.Faces), sh.Volume))
if objs:
    Import.export(objs, out_path)
    say("EXPORTED", out_path, os.path.getsize(out_path))
else:
    say("FAILED: nothing to export")
log.close()
