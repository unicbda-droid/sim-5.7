import unreal
import os

def check_for_commands():
    # Pfad zur Befehlsdatei
    path = "C:/Users/ionco/Documents/Unreal Projects/sim/Content/Scripts/ai_command.txt"
    
    if os.path.exists(path):
        with open(path, "r", encoding="utf-8") as f:
            code = f.read().strip()
        
        if code:
            unreal.log(f"Führe aus: {code}")
            try:
                # Führt den KI-Code direkt im Kontext der Engine aus
                exec(code, globals())
            except Exception as e:
                unreal.log_error(f"KI-Fehler: {e}")
            
            # Datei leeren, damit der Befehl nicht erneut ausgeführt wird
            with open(path, "w", encoding="utf-8") as f:
                f.write("")

# Timer starten: Prüft alle 1 Sekunde [cite: 8]
unreal.SystemLibrary.k2_set_timer_by_function_name(None, check_for_commands, 1.0, True)