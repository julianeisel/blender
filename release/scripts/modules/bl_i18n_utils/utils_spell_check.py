# ##### BEGIN GPL LICENSE BLOCK #####
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software Foundation,
#  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# ##### END GPL LICENSE BLOCK #####

# <pep8 compliant>

import enchant
import os
import pickle
import re


class SpellChecker:
    """
    A basic spell checker.
    """

    # These must be all lower case for comparisons
    uimsgs = {
        # OK words
        "aren",  # aren't
        "betweens",  # yuck! in-betweens!
        "boolean", "booleans",
        "couldn",  # couldn't
        "decrement",
        "derivate",
        "doesn",  # doesn't
        "equi",  # equi-angular, etc.
        "fader",
        "globbing",
        "hasn",  # hasn't
        "hetero",
        "hoc",  # ad-hoc
        "indices",
        "iridas",
        "isn",  # isn't
        "iterable",
        "kyrgyz",
        "latin",
        "merchantability",
        "mplayer",
        "pong",  # ping pong
        "teleport", "teleporting",
        "vertices",

        # Merged words
        "addon", "addons",
        "antialiasing",
        "arcsine", "arccosine", "arctangent",
        "autoclip",
        "autocomplete",
        "autoexec",
        "autoexecution",
        "autoname",
        "autopack",
        "autosave",
        "autoscale",
        "autosmooth",
        "autosplit",
        "backface", "backfacing",
        "backimage",
        "backscattered",
        "bandnoise",
        "bindcode",
        "bitflag", "bitflags",
        "bitrate",
        "blackbody",
        "blendfile",
        "blendin",
        "bonesize",
        "boundbox",
        "boxpack",
        "buffersize",
        "builtin", "builtins",
        "bytecode",
        "chunksize",
        "customdata",
        "dataset", "datasets",
        "de",
        "deconstruct",
        "defocus",
        "denoise",
        "deselect", "deselecting", "deselection",
        "despill", "despilling",
        "editcurve",
        "editmesh",
        "filebrowser",
        "filelist",
        "filename", "filenames",
        "filepath", "filepaths",
        "forcefield", "forcefields",
        "fulldome", "fulldomes",
        "fullscreen",
        "gridline",
        "hemi",
        "inbetween",
        "inscatter", "inscattering",
        "libdata",
        "lightless",
        "lineset",
        "linestyle",
        "localview",
        "lookup", "lookups",
        "mathutils",
        "midlevel",
        "midground",
        "mixdown",
        "multi",
        "multifractal",
        "multires", "multiresolution",
        "multisampling",
        "multitexture",
        "multiuser",
        "namespace",
        "keyconfig",
        "online",
        "playhead",
        "popup", "popups",
        "pre",
        "precache", "precaching",
        "precalculate",
        "prefetch",
        "premultiply", "premultiplied",
        "prepass",
        "prepend",
        "preprocess", "preprocessing",
        "preseek",
        "promillage",
        "pushdown",
        "raytree",
        "readonly",
        "realtime",
        "rekey",
        "remesh",
        "reprojection",
        "resize",
        "restpose",
        "retarget", "retargets", "retargeting", "retargeted",
        "rigidbody",
        "ringnoise",
        "rolloff",
        "runtime",
        "screencast", "screenshot", "screenshots",
        "selfcollision",
        "shadowbuffer", "shadowbuffers",
        "singletexture",
        "spellcheck", "spellchecking",
        "startup",
        "stateful",
        "starfield",
        "subflare", "subflares",
        "subframe", "subframes",
        "subclass", "subclasses", "subclassing",
        "subdirectory", "subdirectories", "subdir", "subdirs",
        "submodule", "submodules",
        "subpath",
        "subsize",
        "substep", "substeps",
        "targetless",
        "textbox", "textboxes",
        "tilemode",
        "timestamp", "timestamps",
        "timestep", "timesteps",
        "todo",
        "un",
        "unbake",
        "uncomment",
        "unculled",
        "undeformed",
        "undistort", "undistortion",
        "ungroup", "ungrouped",
        "unhide",
        "unindent",
        "unkeyed",
        "unpremultiply",
        "unprojected",
        "unreacted",
        "unregister",
        "unselected", "unselectable",
        "unsubdivided", "unsubdivide",
        "unshadowed",
        "unspill",
        "unstitchable",
        "vectorscope",
        "whitespace", "whitespaces",
        "worldspace",
        "workflow",

        # Neologisms, slangs
        "affectable",
        "animatable",
        "automagic", "automagically",
        "blobby",
        "blockiness", "blocky",
        "collider", "colliders",
        "deformer", "deformers",
        "determinator",
        "editability",
        "keyer",
        "lacunarity",
        "numerics",
        "occluder", "occluders",
        "passepartout",
        "perspectively",
        "pixelate",
        "pointiness",
        "polycount",
        "polygonization", "polygonalization",  # yuck!
        "selectability",
        "stitchable",
        "symmetrize",
        "trackability",
        "transmissivity",
        "rasterized", "rasterization", "rasterizer",
        "renderer", "renderable", "renderability",

        # Really bad!!!
        "convertor",

        # Abbreviations
        "aero",
        "amb",
        "anim",
        "bool",
        "calc",
        "config", "configs",
        "const",
        "coord", "coords",
        "degr",
        "diff",
        "dof",
        "dupli", "duplis",
        "eg",
        "esc",
        "expr",
        "fac",
        "fra",
        "frs",
        "grless",
        "http",
        "init",
        "kbit", "kb",
        "lang", "langs",
        "lclick", "rclick",
        "lensdist",
        "loc", "rot", "pos",
        "lorem",
        "luma",
        "mem",
        "multicam",
        "num",
        "ok",
        "orco",
        "ortho",
        "pano",
        "persp",
        "pref", "prefs",
        "prev",
        "param",
        "premul",
        "quad", "quads",
        "quat", "quats",
        "recalc", "recalcs",
        "refl",
        "sce",
        "sel",
        "spec",
        "struct", "structs",
        "sys",
        "tex",
        "tri", "tris",
        "uv", "uvs", "uvw", "uw", "uvmap",
        "ve",
        "vec",
        "vel",  # velocity!
        "vert", "verts",
        "vis",
        "xor",
        "xyz", "xzy", "yxz", "yzx", "zxy", "zyx",
        "xy", "xz", "yx", "yz", "zx", "zy",

        # General computer/science terms
        "bitangent",
        "boid", "boids",
        "equisolid",
        "euler", "eulers",
        "fribidi",
        "gettext",
        "hashable",
        "hotspot",
        "intrinsics",
        "isosurface",
        "jitter", "jittering", "jittered",
        "keymap", "keymaps",
        "lambertian",
        "laplacian",
        "metadata",
        "msgfmt",
        "nand", "xnor",
        "normals",
        "numpad",
        "octree",
        "omnidirectional",
        "opengl",
        "openmp",
        "photoreceptor",
        "poly",
        "polyline", "polylines",
        "pulldown", "pulldowns",
        "quantized",
        "samplerate",
        "scrollback",
        "scrollbar",
        "scroller",
        "searchable",
        "spacebar",
        "tooltip", "tooltips",
        "trackpad",
        "tuple",
        "unicode",
        "viewport", "viewports",
        "viscoelastic",
        "wildcard", "wildcards",

        # General computer graphics terms
        "anaglyph",
        "bezier", "beziers",
        "bicubic",
        "bilinear",
        "binormal",
        "blackpoint", "whitepoint",
        "blinn",
        "bokeh",
        "catadioptric",
        "centroid",
        "chrominance",
        "codec", "codecs",
        "collada",
        "compositing",
        "crossfade",
        "deinterlace",
        "dropoff",
        "dv",
        "eigenvectors",
        "equirectangular",
        "fisheye",
        "framerate",
        "gimbal",
        "grayscale",
        "icosphere",
        "inpaint",
        "lightmap",
        "linearlight",
        "lossless", "lossy",
        "matcap",
        "midtones",
        "mipmap", "mipmaps", "mip",
        "ngon", "ngons",
        "ntsc",
        "nurb", "nurbs",
        "perlin",
        "phong",
        "pinlight",
        "qi",
        "radiosity",
        "raycasting",
        "raytrace", "raytracing", "raytraced",
        "renderfarm",
        "scanfill",
        "shader", "shaders",
        "softlight",
        "specular", "specularity",
        "spillmap",
        "sobel",
        "texel",
        "tonemap",
        "toon",
        "timecode",
        "vividlight",
        "voronoi",
        "voxel", "voxels",
        "vsync",
        "wireframe",
        "zmask",
        "ztransp",

        # Blender terms
        "audaspace",
        "bbone",
        "breakdowner",
        "bspline",
        "bweight",
        "colorband",
        "datablock", "datablocks",
        "despeckle",
        "dopesheet",
        "dupliface", "duplifaces",
        "dupliframe", "dupliframes",
        "dupliobject", "dupliob",
        "dupligroup",
        "duplivert",
        "dyntopo",
        "editbone",
        "editmode",
        "fcurve", "fcurves",
        "fedge", "fedges",
        "fluidsim",
        "frameserver",
        "freestyle",
        "enum", "enums",
        "gpencil",
        "idcol",
        "keyframe", "keyframes", "keyframing", "keyframed",
        "metaball", "metaballs", "mball",
        "metaelement", "metaelements",
        "metastrip", "metastrips",
        "movieclip",
        "mpoly",
        "mtex",
        "nabla",
        "navmesh",
        "outliner",
        "paintmap", "paintmaps",
        "polygroup", "polygroups",
        "poselib",
        "pushpull",
        "pyconstraint", "pyconstraints",
        "qe",  # keys...
        "shapekey", "shapekeys",
        "shrinkfatten",
        "shrinkwrap",
        "softbody",
        "stucci",
        "sunsky",
        "subsurf",
        "tessface", "tessfaces",
        "texface",
        "timeline", "timelines",
        "tosphere",
        "uilist",
        "vcol", "vcols",
        "vgroup", "vgroups",
        "vinterlace",
        "vse",
        "wasd", "wasdqe",  # keys...
        "wetmap", "wetmaps",
        "wpaint",
        "uvwarp",

        # Algorithm names
        "ashikhmin",  # Ashikhmin-Shirley
        "beckmann",
        "catmull",
        "catrom",
        "chebychev",
        "courant",
        "hosek",
        "kutta",
        "lennard",
        "mikktspace",
        "minkowski",
        "minnaert",
        "musgrave",
        "nayar",
        "netravali",
        "oren",
        "preetham",
        "prewitt",
        "runge",
        "sobol",
        "verlet",
        "wilkie",
        "worley",

        # Acronyms
        "aa", "msaa",
        "ao",
        "api",
        "asc", "cdl",
        "ascii",
        "atrac",
        "bsdf",
        "bssrdf",
        "bw",
        "ccd",
        "cmd",
        "cpus",
        "ctrl",
        "cw", "ccw",
        "dev",
        "djv",
        "dpi",
        "dvar",
        "dx",
        "eo",
        "fh",
        "fov",
        "fft",
        "futura",
        "fx",
        "gfx",
        "gl",
        "glsl",
        "gpl",
        "gpu", "gpus",
        "hc",
        "hdc",
        "hdr",
        "hh", "mm", "ss", "ff",  # hh:mm:ss:ff timecode
        "hsv", "hsva", "hsl",
        "id",
        "ior",
        "itu",
        "lhs",
        "lmb", "mmb", "rmb",
        "kb",
        "mocap",
        "msgid", "msgids",
        "mux",
        "ndof",
        "ppc",
        "precisa",
        "px",
        "qmc",
        "rgb", "rgba",
        "rhs",
        "rv",
        "sdl",
        "sl",
        "smpte",
        "ssao",
        "svn",
        "ui",
        "unix",
        "vbo", "vbos",
        "ycc", "ycca",
        "yuv", "yuva",

        # Blender acronyms
        "bge",
        "bli",
        "bpy",
        "bvh",
        "dbvt",
        "dop",  # BLI K-Dop BVH
        "ik",
        "nla",
        "py",
        "qbvh",
        "rna",
        "rvo",
        "simd",
        "sph",
        "svbvh",

        # Files types/formats
        "avi",
        "attrac",
        "autocad",
        "autodesk",
        "bmp",
        "btx",
        "cineon",
        "dpx",
        "dwaa",
        "dwab",
        "dxf",
        "eps",
        "exr",
        "fbx",
        "ffmpeg",
        "flac",
        "gzip",
        "ico",
        "jpg", "jpeg",
        "json",
        "matroska",
        "mdd",
        "mkv",
        "mpeg", "mjpeg",
        "mtl",
        "ogg",
        "openjpeg",
        "osl",
        "oso",
        "piz",
        "png",
        "po",
        "quicktime",
        "rle",
        "sgi",
        "stl",
        "svg",
        "targa", "tga",
        "tiff",
        "theora",
        "vorbis",
        "wav",
        "xiph",
        "xml",
        "xna",
        "xvid",
    }

    _valid_before = "(?<=[\\s*'\"`])|(?<=[a-zA-Z][/-])|(?<=^)"
    _valid_after = "(?=[\\s'\"`.!?,;:])|(?=[/-]\\s*[a-zA-Z])|(?=$)"
    _valid_words = "(?:{})(?:(?:[A-Z]+[a-z]*)|[A-Z]*|[a-z]*)(?:{})".format(_valid_before, _valid_after)
    _split_words = re.compile(_valid_words).findall

    @classmethod
    def split_words(cls, text):
        return [w for w in cls._split_words(text) if w]

    def __init__(self, settings, lang="en_US"):
        self.settings = settings
        self.dict_spelling = enchant.Dict(lang)
        self.cache = set(self.uimsgs)

        cache = self.settings.SPELL_CACHE
        if cache and os.path.exists(cache):
            with open(cache, 'rb') as f:
                self.cache |= set(pickle.load(f))

    def __del__(self):
        cache = self.settings.SPELL_CACHE
        if cache and os.path.exists(cache):
            with open(cache, 'wb') as f:
                pickle.dump(self.cache, f)

    def check(self, txt):
        ret = []

        if txt in self.cache:
            return ret

        for w in self.split_words(txt):
            w_lower = w.lower()
            if w_lower in self.cache:
                continue
            if not self.dict_spelling.check(w):
                ret.append((w, self.dict_spelling.suggest(w)))
            else:
                self.cache.add(w_lower)

        if not ret:
            self.cache.add(txt)

        return ret
