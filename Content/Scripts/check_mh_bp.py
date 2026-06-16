import unreal

bp_mh = unreal.load_asset("/Game/MetaHumans/Hannah/MHC_Hannah/BP_MHC_Hannah.BP_MHC_Hannah")
bp_mh_class = bp_mh.generated_class()

# Get parent class
parent = bp_mh_class.get_editor_property("parent_class")
unreal.log(f"BP_MHC_Hannah parent class: {parent}")

# Check if it's a Pawn type
unreal.log(f"Is Character? {parent.is_child_of(unreal.load_class('Character'))}")
unreal.log(f"Is Pawn? {parent.is_child_of(unreal.load_class('Pawn'))}")
unreal.log(f"Is Actor? {parent.is_child_of(unreal.load_class('Actor'))}")

# Check the CDO for properties
cdo = unreal.get_default_object(bp_mh_class)
unreal.log(f"CDO: {cdo}")
unreal.log(f"CDO class: {cdo.get_class()}")

# If it extends Character, it should be usable as DefaultPawnClass
# But the error said "allowed Class type: 'Pawn'" - this means BP_MHC_Hannah_C 
# might be a non-Pawn type

# Alternate approach: Create a new Pawn Blueprint that spawns the MetaHuman
# Or: Just use SimCharacter as-is and place BP_MHC_Hannah in the level

unreal.log("Script complete")
