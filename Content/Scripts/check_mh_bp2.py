import unreal

# Check the super class of BP_MHC_Hannah
bp_mh = unreal.load_asset("/Game/MetaHumans/Hannah/MHC_Hannah/BP_MHC_Hannah.BP_MHC_Hannah")
bp_class = bp_mh.generated_class()

# Try different methods to get super class
super_cls = bp_class.get_super_class()
unreal.log(f"Super class: {super_cls}")

# Check if it's a pawn
pawn_cls = unreal.find_class("Pawn", unreal.find_package("/Script/Engine"))
unreal.log(f"Is based on Pawn: {bp_class.is_child_of(pawn_cls)}")

# Try to convert to SoftClassPath or string
bp_path = "/Game/MetaHumans/Hannah/MHC_Hannah/BP_MHC_Hannah.BP_MHC_Hannah_C"
unreal.log(f"BP class path: {bp_path}")

# Method: Try using the CDO with a SoftObjectPath
bp_gm = unreal.load_asset("/Game/SimGameMode_BP.SimGameMode_BP")
bp_gm_class = bp_gm.generated_class()
cdo = unreal.get_default_object(bp_gm_class)

# Try setting as string path
try:
    cdo.set_editor_property("default_pawn_class", bp_path)
    unreal.log("Set via string path")
except Exception as e:
    unreal.log(f"String path failed: {e}")

# Try setting as SoftClassPath
try:
    scp = unreal.SoftClassPath()
    scp.set_editor_property("asset_path", bp_path)
    cdo.set_editor_property("default_pawn_class", scp)
    unreal.log("Set via SoftClassPath")
except Exception as e:
    unreal.log(f"SoftClassPath failed: {e}")

# Try getting the actual Class object from Blueprint
# Some APIs use blueprint_class vs generated_class
cls_from_bp = bp_mh.get_editor_property("generated_class")
unreal.log(f"generated_class prop: {cls_from_bp}")

# Try through the Blueprint's parent class
parent = bp_mh.get_editor_property("parent_class")
unreal.log(f"parent_class: {parent}")

# Get the blueprint skeleton class
skel = bp_mh.get_editor_property("skeleton_class")  
unreal.log(f"skeleton_class: {skel}")

unreal.log("Script complete")
