# Pseudocode für ai_term.py
import requests
import ollama

def send_to_engine(command):
    # Logik: KI interpretiert "Spawne einen Würfel" -> HTTP POST Request an Unreal
    response = requests.post("http://localhost:7000/remote/object/call", json=...)
    print("Befehl an Engine gesendet.")

user_input = input("KI-Terminal: ")
ai_response = ollama.chat(model='qwen2.5', messages=[{'role': 'user', 'content': user_input}])
send_to_engine(ai_response['message']['content'])