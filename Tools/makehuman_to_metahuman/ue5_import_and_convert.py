"""UE5 Python: Import MPFB FBX → Create conformed MetaHuman → Build Blueprint.

Run inside UE5 via UETerminalBridge:
  POST /api/exec {"code":"C:/path/to/ue5_import_and_convert.py"}

Config section at bottom controls character parameters.
"""

import os
import sys
import json
from pathlib import Path

CONFIG_PATH = Path(__file__).parent / "pipeline_config.json"


def load_config():
    if CONFIG_PATH.exists():
        with open(CONFIG_PATH) as f:
            return json.load(f)
    return {}


import unreal


def fbx_path_for(name):
    return str(Path(CONFIG_PATH.parent if CONFIG_PATH.exists() else Path(__file__).parent) / "exports" / f"{name}.fbx")


def import_fbx_as_skeletal(file_path, dest_path="/Game/Imported"):
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"FBX not found: {file_path}")

    task = unreal.AssetImportTask()
    task.filename = file_path
    task.destination_path = dest_path
    task.automated = True
    task.save = True
    task.replace_existing = True

    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_as_skeletal = True
    options.import_materials = True
    options.import_textures = True
    options.import_animations = False
    options.skeleton = None

    task.options = options

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset_tools.import_asset_tasks([task])

    imported = task.get_objects()
    if not imported:
        raise RuntimeError(f"FBX import returned no objects: {file_path}")
    return imported[0]


def create_metahuman_character(asset_name, package_path):
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    character = asset_tools.create_asset(
        asset_name=asset_name,
        package_path=package_path,
        asset_class=unreal.MetaHumanCharacter,
        factory=unreal.new_object(type=unreal.MetaHumanCharacterFactoryNew),
    )
    if character is None:
        raise RuntimeError(f"Failed to create MetaHumanCharacter '{asset_name}'")
    return character


def conform_from_skeletal(character, skeletal_mesh):
    subsystem = unreal.get_editor_subsystem(unreal.MetaHumanCharacterEditorSubsystem)

    if not subsystem.try_add_object_to_edit(character=character):
        raise RuntimeError("Failed to open character for editing")

    try:
        import_params = unreal.ImportFromTemplateParams()
        import_params.use_eye_meshes = False
        import_params.use_teeth_mesh = False
        import_params.alignment_options = unreal.MetaHumanAlignmentOptions.SCALING_ROTATION_TRANSLATION

        result = subsystem.import_from_template(
            character, skeletal_mesh, None, None, None, import_params
        )

        if result != unreal.ImportErrorCode.SUCCESS:
            unreal.log_warning(f"import_from_template returned {result} (may still have partial result)")

        subsystem.commit_face_state(character)
    finally:
        if subsystem.is_object_added_for_editing(character):
            subsystem.remove_object_to_edit(character)


def auto_rig_and_textures(character):
    subsystem = unreal.get_editor_subsystem(unreal.MetaHumanCharacterEditorSubsystem)

    if not subsystem.try_add_object_to_edit(character=character):
        raise RuntimeError("Failed to open character for auto-rig")

    try:
        unreal.log("Requesting auto-rigging (blocking)...")
        rig_params = unreal.MetaHumanCharacterAutoRiggingRequestParams()
        rig_params.blocking = True
        rig_params.report_progress = False
        rig_params.rig_type = unreal.MetaHumanRigType.JOINTS_ONLY
        subsystem.request_auto_rigging(character=character, params=rig_params)

        unreal.log("Requesting texture sources (blocking)...")
        tex_params = unreal.MetaHumanCharacterTextureRequestParams()
        tex_params.blocking = True
        tex_params.report_progress = False
        subsystem.request_texture_sources(character=character, params=tex_params)

        if not character.has_high_resolution_textures:
            unreal.log_warning("Character does not have high-res textures after request")
    finally:
        if subsystem.is_object_added_for_editing(character):
            subsystem.remove_object_to_edit(character)


def build_metahuman(character, build_path):
    subsystem = unreal.get_editor_subsystem(unreal.MetaHumanCharacterEditorSubsystem)

    if not subsystem.try_add_object_to_edit(character=character):
        raise RuntimeError("Failed to open character for build")

    try:
        if not subsystem.can_build_meta_human(character=character):
            unreal.log_warning("can_build_meta_human returned False, attempting build anyway")

        build_params = unreal.MetaHumanCharacterEditorBuildParameters()
        build_params.pipeline_type = unreal.MetaHumanDefaultPipelineType.OPTIMIZED
        build_params.pipeline_quality = unreal.MetaHumanQualityLevel.HIGH
        build_params.absolute_build_path = build_path
        build_params.common_folder_path = f"{build_path}/Common"

        unreal.log(f"Building MetaHuman at {build_path}...")
        subsystem.build_meta_human(character=character, params=build_params)
        unreal.log("Build complete!")
    finally:
        if subsystem.is_object_added_for_editing(character):
            subsystem.remove_object_to_edit(character)


def main():
    config = load_config()
    name = config.get("name", "MPFB_Character")
    fbx_file = config.get("fbx_path", "")
    base_path = config.get("build_path", "/Game/MetaHumans/MPFB")

    if not fbx_file or not os.path.exists(fbx_file):
        fbx_file = fbx_path_for(name)

    unreal.log(f"=== MPFB→MetaHuman Pipeline ===")
    unreal.log(f"Character: {name}")
    unreal.log(f"FBX: {fbx_file}")

    if not os.path.exists(fbx_file):
        unreal.log_error(f"FBX file not found: {fbx_file}")
        return

    unreal.log("[1/5] Importing FBX as skeletal mesh...")
    skeletal_mesh = import_fbx_as_skeletal(fbx_file, f"{base_path}/Source")
    unreal.log(f"  Imported: {skeletal_mesh.get_path_name()}")

    unreal.log("[2/5] Creating MetaHumanCharacter asset...")
    character = create_metahuman_character(name, f"{base_path}/Source")
    unreal.log(f"  Created: {character.get_path_name()}")

    unreal.log("[3/5] Conforming face from imported mesh...")
    conform_from_skeletal(character, skeletal_mesh)
    unreal.log("  Conformed")

    unreal.log("[4/5] Auto-rigging and texture download...")
    auto_rig_and_textures(character)

    unreal.log("[5/5] Building MetaHuman Blueprint...")
    build_metahuman(character, f"{base_path}/Built")

    unreal.log(f"=== Done: {name} ===")


if __name__ == "__main__":
    main()
