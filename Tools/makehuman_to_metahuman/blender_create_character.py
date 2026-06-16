"""Blender script: Create MPFB character → export FBX with embedded textures.

Usage:
  blender --background --python blender_create_character.py -- \\
    --name "OldMan" --age 0.85 --gender 0.9 --muscle 0.3 \\
    --output "C:/path/to/output.fbx"

Parameters (0.0–1.0 range):
  --age     0.0=baby, 0.1875=child, 0.5=young adult, 1.0=old
  --gender  0.0=female, 1.0=male
  --muscle  0.0=frail, 1.0=bodybuilder
  --weight  0.0=thin, 1.0=fat
  --height  0.0=short, 1.0=tall
"""

import argparse
import os
import sys


_MPFB_INITIALIZED = False


def _ensure_mpfb():
    global _MPFB_INITIALIZED
    if _MPFB_INITIALIZED:
        return
    import bpy

    # Monkey-patch extension_path_user so MPFB (an extension) works in --background mode
    _orig_extension_path_user = bpy.utils.extension_path_user
    def _patched_extension_path_user(package, *args, **kwargs):
        try:
            return _orig_extension_path_user(package, *args, **kwargs)
        except ValueError:
            import os
            return os.path.join(
                bpy.utils.resource_path('USER'), "data", "extensions",
                package if isinstance(package, str) else str(package)
            )
    bpy.utils.extension_path_user = _patched_extension_path_user

    if "mpfb" not in bpy.context.preferences.addons:
        bpy.ops.preferences.addon_enable(module="mpfb")

    import mpfb as _mpfb
    if _mpfb.MPFB_CONTEXTUAL_INFORMATION is None:
        _mpfb.register()
    _MPFB_INITIALIZED = True


def create_character(name, age, gender, muscle, weight, height, output_path):
    import bpy
    _ensure_mpfb()

    from mpfb.services.humanservice import HumanService
    from mpfb.services.targetservice import TargetService
    from mpfb.entities.objectproperties import HumanObjectProperties

    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)

    basemesh = HumanService.create_human()
    if basemesh is None:
        raise RuntimeError("HumanService.create_human() returned None")

    basemesh.name = name
    bpy.context.view_layer.update()

    HumanObjectProperties.set_value("gender", max(0.0, min(1.0, gender)), entity_reference=basemesh)
    HumanObjectProperties.set_value("age", max(0.0, min(1.0, age)), entity_reference=basemesh)
    HumanObjectProperties.set_value("muscle", max(0.0, min(1.0, muscle)), entity_reference=basemesh)
    HumanObjectProperties.set_value("weight", max(0.0, min(1.0, weight)), entity_reference=basemesh)
    HumanObjectProperties.set_value("height", max(0.0, min(1.0, height)), entity_reference=basemesh)
    HumanObjectProperties.set_value("proportions", 0.5, entity_reference=basemesh)
    HumanObjectProperties.set_value("cupsize", 0.3 if gender < 0.7 else 0.0, entity_reference=basemesh)
    HumanObjectProperties.set_value("firmness", 0.5, entity_reference=basemesh)

    TargetService.reapply_macro_details(basemesh, remove_zero_weight_targets=True)

    bpy.context.view_layer.update()

    HumanService.add_builtin_rig(basemesh, "default_no_toes", import_weights=True)

    bpy.context.view_layer.update()

    # Select only the basemesh and its armature for FBX export
    bpy.ops.object.select_all(action="DESELECT")
    basemesh.select_set(True)
    armature = None
    for mod in basemesh.modifiers:
        if mod.type == 'ARMATURE' and mod.object:
            armature = mod.object
            armature.select_set(True)
            break

    bpy.context.view_layer.update()

    bpy.ops.export_scene.fbx(
        filepath=output_path,
        use_selection=True,
        mesh_smooth_type='FACE',
        add_leaf_bones=False,
        bake_anim=False,
        embed_textures=True,
        path_mode='COPY',
        use_armature_deform_only=True,
        use_tspace=False
    )

    return output_path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--name", default="Character")
    parser.add_argument("--age", type=float, default=0.5)
    parser.add_argument("--gender", type=float, default=0.5)
    parser.add_argument("--muscle", type=float, default=0.5)
    parser.add_argument("--weight", type=float, default=0.5)
    parser.add_argument("--height", type=float, default=0.5)
    parser.add_argument("--output", required=True)

    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    args = parser.parse_args(argv)

    out = os.path.abspath(args.output)
    os.makedirs(os.path.dirname(out), exist_ok=True)

    print(f"Creating character '{args.name}' (age={args.age}, gender={args.gender})")
    result = create_character(
        name=args.name,
        age=args.age,
        gender=args.gender,
        muscle=args.muscle,
        weight=args.weight,
        height=args.height,
        output_path=out,
    )
    print(f"FBX exported to: {result}")


if __name__ == "__main__":
    main()
