import unreal
import sys

def execute_ai_command(command_string):
    # Hier könnte eine Logik stehen, die den String des LLM interpretiert
    # Beispiel: Spawnen eines Actors
    if "spawn" in command_string:
        location = unreal.Vector(0, 0, 0)
        unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, location)
        print("KI-Befehl ausgeführt: Actor gespawnt.")

# Empfang der Argumente aus dem Terminal
if __name__ == "__main__":
    cmd = " ".join(sys.argv[1:])
    execute_ai_command(cmd)