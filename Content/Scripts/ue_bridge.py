import requests
import json
import ollama # Stellen Sie sicher, dass 'pip install ollama' installiert ist

# Ersetzen Sie den ollama.chat Aufruf durch diesen System-Prompt:
response = ollama.chat(model='qwen2.5', messages=[
    {'role': 'system', 'content': 'Du bist eine Unreal Python API Schnittstelle. Antworte NUR mit dem exakten Python-Befehl für die Unreal API. Keine Erklärungen.'},
    {'role': 'user', 'content': f"Schreibe den Python-Einzeiler, um einen StaticMeshActor an Position (0,0,0) zu spawnen: {user_input}"}
])

# Statt requests an einen Web-Server zu schicken, 
# führen Sie den Code direkt in der laufenden Engine aus:
def execute_ai_command(ai_code):
    try:
        # EXEC bewirkt, dass die Engine den String als Python-Code interpretiert
        unreal.SystemLibrary.execute_console_command(None, ai_code) 
        # Oder besser direkt über die API:
        exec(ai_code) 
        print("Befehl in Engine ausgeführt.")
    except Exception as e:
        print(f"Fehler bei Ausführung: {e}"))

print("--- KI-Terminal für Unreal bereit ---")
while True:
    user_input = input("Was soll die Engine tun? > ")
    if user_input.lower() == "exit": break
    
    # 1. KI fragen, was sie tun soll (z.B. Blueprint-Funktionsname)
    response = ollama.chat(model='qwen2.5', messages=[{'role': 'user', 'content': f"Gib mir nur den Namen der Unreal-Python-Funktion für: {user_input}"}])
    func_name = response['message']['content'].strip()
    
    print(f"Sende Befehl: {func_name} an Unreal...")
    send_to_unreal(func_name)