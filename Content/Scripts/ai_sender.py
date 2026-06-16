import ollama

path = "C:/Users/ionco/Documents/Unreal Projects/sim/Content/Scripts/ai_command.txt"

print("--- KI-Terminal bereit. Schreibe 'exit' zum Beenden. ---")
while True:
    user_input = input("Was soll die Engine tun? > ")
    if user_input.lower() == "exit": break
    
    # KI generiert den korrekten API-Code
    response = ollama.chat(model='qwen2.5', messages=[
        {'role': 'system', 'content': 'Du bist ein Experte für die Unreal Python API. Antworte NUR mit dem reinen Python-Code. Keine Erklärungen.'},
        {'role': 'user', 'content': f"Schreibe den Python-Code für: {user_input}. Nutze das 'unreal' Modul."}
    ])
    
    # Text bereinigen
    code = response['message']['content'].replace('```python', '').replace('```', '').strip()
    
    # In Datei schreiben mit utf-8, um Fehler zu vermeiden
    with open(path, "w", encoding="utf-8") as f:
        f.write(code)
    print(f"Befehl in Queue geschrieben: {user_input}")