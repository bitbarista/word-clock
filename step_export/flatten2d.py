# Flatten all 2D geometry in an OpenSCAD .csg file: every
# linear_extrude body (arbitrary 2D booleans/offsets that OpenCASCADE
# chokes on) is rendered by OpenSCAD's own exact Clipper-based 2D
# engine (SVG export) and replaced with a `polyset(id = K);` node whose
# loops land in a JSON sidecar. Arcs tessellate with the node's own
# $fa/$fs, i.e. identically to the STL exports.
import json, re, subprocess, sys, tempfile, os

src_path, out_path, json_path = sys.argv[1:4]
lines = open(src_path).read().splitlines()

out = []
polysets = {}
i = 0
while i < len(lines):
    line = lines[i]
    m = re.match(r"(\s*)linear_extrude\((.*)\)\s*\{\s*$", line)
    if not m:
        out.append(line)
        i += 1
        continue
    indent = m.group(1)
    depth = 1
    j = i + 1
    while depth > 0:
        s = lines[j].strip()
        depth += s.endswith("{") - (s == "}")
        j += 1
    body = "\n".join(lines[i + 1:j - 1])
    k = len(polysets)
    polysets[k] = body
    out.append(line)
    out.append(indent + "\tpolyset(id = %d);" % k)
    out.append(indent + "}")
    i = j

def svg_loops(svg):
    loops = []
    for d in re.findall(r'd="([^"]*)"', svg):
        for sub in re.split(r"[zZ]", d):
            pts = [[float(x), -float(y)] for x, y in
                   re.findall(r"[ML]\s*([-\d.e]+),([-\d.e]+)", sub)]
            if len(pts) >= 3:
                loops.append(pts)
    return loops

tmp = tempfile.mkdtemp(prefix="flat2d_",
                       dir=os.path.dirname(os.path.abspath(src_path)))
loop_data = {}
for k, body in polysets.items():
    scad = os.path.join(tmp, "m%d.scad" % k)
    svg = os.path.join(tmp, "m%d.svg" % k)
    open(scad, "w").write(body + "\n")
    r = subprocess.run(["openscad", "-o", svg, scad],
                       capture_output=True, text=True)
    if not os.path.exists(svg):
        print("FAIL polyset", k, r.stderr[-500:])
        sys.exit(1)
    loop_data[k] = svg_loops(open(svg).read())

open(out_path, "w").write("\n".join(out) + "\n")
json.dump(loop_data, open(json_path, "w"))
print("flattened %d extrudes -> %s (+ %s)" % (len(polysets), out_path, json_path))
