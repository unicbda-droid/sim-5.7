import unreal

# Try loading the class directly
bp_class = unreal.load_object(name="/Game/MetaHumans/Hannah/MHC_Hannah/BP_MHC_Hannah.BP_MHC_Hannah_C", outer=None)
unreal.log(f"Loaded BP class: {bp_class} ({type(bp_class).__name__})")

# Check if it's a valid class
if bp_class and hasattr(bp_class, 'is_child_of'):
    pawn = unreal.find_class("Pawn", unreal.find_package("/Script/Engine"))
    char = unreal.find_class("Character", unreal.find_package("/Script/Engine"))
    unreal.log(f"Is Pawn: {bp_class.is_child_of(pawn)}")
    unreal.log(f"Is Character: {bp_class.is_child_of(char)}")
    
    if bp_class.is_child_of(char) or bp_class.is_child_of(pawn):
        unreal.log("BP_MHC_Hannah is a valid Pawn class!")
        
        # Set on game mode CDO
        bp_gm = unreal.load_asset("/Game/SimGameMode_BP.SimGameMode_BP")
        bp_gm_class = bp_gm.generated_class()
        cdo = unreal.get_default_object(bp_gm_class)
        
        try:
            cdo.set_editor_property("default_pawn_class", bp_class)
            unreal.log(f"Set default_pawn_class!")
            
            # Save
            unreal.EditorAssetLibrary.save_asset("/Game/SimGameMode_BP.SimGameMode_BP")
            unreal.log("Saved!")
        except Exception as e:
            unreal.log(f"set failed: {e}")
    else:
        unreal.log("BP_MHC_Hannah is NOT a Pawn class!")
else:
    unreal.log("Could not check class hierarchy")
    
    # Just try loading and setting anyway
    if bp_class:
        unreal.log(f"Class exists, trying to use it")
        
        bp_gm = unreal.load_asset("/Game/SimGameMode_BP.SimGameMode_BP")
        bp_gm_class = bp_gm.generated_class()
        cdo = unreal.get_default_object(bp_gm_class)
        
        try:
            cdo.set_editor_property("default_pawn_class", bp_class)
            unreal.log("Set!")
        except Exception as e:
            unreal.log(f"Set failed: {e}")

unreal.log("Script complete")
