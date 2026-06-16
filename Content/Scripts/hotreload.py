import unreal

# Try to recompile the game module using the editor
# This works in UE5 through the module manager
try:
    recompiled = unreal.SystemLibrary.execute_console_command(None, "RecompileGameProject")
    unreal.log(f"RecompileGameProject: {recompiled}")
except Exception as e:
    unreal.log(f"RecompileGameProject error: {e}")

# Alternative: Use hot reload
try:
    recompiled = unreal.SystemLibrary.execute_console_command(None, "HotReload")
    unreal.log(f"HotReload: {recompiled}")
except Exception as e:
    unreal.log(f"HotReload error: {e}")

# Another approach: force reload of SimGameMode module
try:
    recompiled = unreal.SystemLibrary.execute_console_command(None, "Reload 'SimGameMode'")
    unreal.log(f"Reload: {recompiled}")
except Exception as e:
    unreal.log(f"Reload error: {e}")

unreal.log("Script complete")
