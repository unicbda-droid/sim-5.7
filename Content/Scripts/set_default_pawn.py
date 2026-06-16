import unreal

bp_mh = unreal.load_asset("/Game/MetaHumans/Hannah/MHC_Hannah/BP_MHC_Hannah.BP_MHC_Hannah")
bp_mh_class = bp_mh.generated_class()

bp_gm = unreal.load_asset("/Game/SimGameMode_BP.SimGameMode_BP")
bp_gm_class = bp_gm.generated_class()
cdo = unreal.get_default_object(bp_gm_class)

unreal.log(f"Current default_pawn_class: {cdo.get_editor_property('default_pawn_class')}")

# Set to MetaHuman Blueprint class
cdo.set_editor_property("default_pawn_class", bp_mh_class)
unreal.log(f"Set to: {cdo.get_editor_property('default_pawn_class')}")

# Save
unreal.EditorAssetLibrary.save_asset("/Game/SimGameMode_BP.SimGameMode_BP")
unreal.log("Saved SimGameMode_BP")

# Set as default game mode in World Settings
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
ws = world.get_world_settings()
ws.set_editor_property("default_game_mode", bp_gm_class)
unreal.log(f"Set DefaultGameMode to SimGameMode_BP")

# Save level
level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
level_subsystem.save_current_level()
unreal.log("Level saved")

unreal.log("Ready! MetaHuman is now the player character!")
