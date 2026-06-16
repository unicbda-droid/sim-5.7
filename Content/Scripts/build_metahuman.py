import unreal

bp_path = "/Game/MetaHumans/Hannah/MHC_Hannah/BP_MHC_Hannah.BP_MHC_Hannah"
bp = unreal.load_asset(bp_path)
bp_class = bp.generated_class()
unreal.log(f"MetaHuman BP class: {bp_class}")

# Load existing SimGameMode_BP or create it
bp_gm = unreal.EditorAssetLibrary.load_asset("/Game/SimGameMode_BP.SimGameMode_BP")
if not bp_gm:
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.BlueprintFactory()
    sim_gm_class = unreal.load_object(name="/Script/sim.SimGameMode", outer=None)
    factory.set_editor_property("parent_class", sim_gm_class)
    bp_gm = tools.create_asset("SimGameMode_BP", "/Game", None, factory)
    unreal.log(f"Created BP: {bp_gm}")

# Set DefaultPawnClass on the GENERATED CLASS, not the Blueprint asset
bp_gm_class = bp_gm.generated_class()
unreal.log(f"BP GM class: {bp_gm_class}")

if bp_gm_class:
    try:
        bp_gm_class.set_editor_property("default_pawn_class", bp_class)
        unreal.log(f"Set default_pawn_class on BP generated class")
    except Exception as e:
        unreal.log(f"Set on class failed: {e}")
    
    # Verify
    val = bp_gm_class.get_editor_property("default_pawn_class")
    unreal.log(f"Verified: default_pawn_class = {val}")
    
    # Save
    unreal.EditorAssetLibrary.save_asset("/Game/SimGameMode_BP.SimGameMode_BP")
    unreal.log("Saved SimGameMode_BP")

# Now set this as default game mode in World Settings
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if world:
    ws = world.get_world_settings()
    try:
        ws.set_editor_property("default_game_mode", bp_gm.get_path_name())
        unreal.log("Set DefaultGameMode on WorldSettings")
    except Exception as e:
        unreal.log(f"Set GM on WS failed: {e}")
    
    # Also try setting as direct class reference
    try:
        ws.set_editor_property("DefaultGameMode", bp_gm_class)
        unreal.log("Set DefaultGameMode as class")
    except Exception as e:
        unreal.log(f"Set GM as class failed: {e}")

# Save level
unreal.EditorLevelLibrary.save_current_level()
unreal.log("Saved level")

unreal.log("MetaHuman player character configured!")
