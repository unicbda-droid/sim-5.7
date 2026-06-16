#!/usr/bin/env python3
"""
UE Terminal Bridge CLI
Control Unreal Engine from terminal/PowerShell/CMD.
Usage:
  python uecmd.py status
  python uecmd.py actors
  python uecmd.py spawn StaticMeshActor --location 0,0,200 --label MyCube
  python uecmd.py exec "print('hello')"
  python uecmd.py get Floor bHiddenEd
  python uecmd.py set Floor bHiddenEd true
  python uecmd.py mcp              # MCP server mode for AI/LLM
"""

import sys
import json
import argparse
import urllib.request
import urllib.error
import os

DEFAULT_PORT = 8090
BASE_URL = f"http://127.0.0.1:{DEFAULT_PORT}"

def api_get(path):
    url = f"{BASE_URL}{path}"
    try:
        resp = urllib.request.urlopen(url, timeout=10)
        return json.loads(resp.read().decode())
    except urllib.error.URLError as e:
        return {"error": str(e)}
    except json.JSONDecodeError:
        return {"error": "Invalid JSON response"}

def api_post(path, data):
    url = f"{BASE_URL}{path}"
    try:
        req = urllib.request.Request(
            url,
            data=json.dumps(data).encode(),
            headers={"Content-Type": "application/json"},
            method="POST"
        )
        resp = urllib.request.urlopen(req, timeout=10)
        return json.loads(resp.read().decode())
    except urllib.error.URLError as e:
        return {"error": str(e)}
    except json.JSONDecodeError:
        return {"error": "Invalid JSON response"}

def cmd_status(args):
    result = api_get("/api/status")
    print(json.dumps(result, indent=2))

def cmd_actors(args):
    result = api_get("/api/actors")
    if "error" in result:
        print(f"Error: {result['error']}")
        return
    print(f"Actors in level: {result['count']}")
    for a in result.get("actors", []):
        loc = a.get("location", [0, 0, 0])
        print(f"  {a['name']:40s} [{a['class']:25s}] at ({loc[0]:.1f}, {loc[1]:.1f}, {loc[2]:.1f})")

def cmd_spawn(args):
    loc = [float(x) for x in args.location.split(",")] if args.location else [0, 0, 0]
    data = {
        "class": args.actor_class,
        "location": loc,
    }
    if args.label:
        data["label"] = args.label
    result = api_post("/api/spawn", data)
    if "error" in result:
        print(f"Error: {result['error']}")
    else:
        print(f"Spawned: {result.get('name', '?')} ({result.get('label', '?')})")

def cmd_exec(args):
    data = {"code": args.code}
    result = api_post("/api/exec", data)
    print(json.dumps(result, indent=2))

def cmd_get(args):
    result = api_get(f"/api/property?actor={args.actor}&property={args.property}")
    if "error" in result:
        print(f"Error: {result['error']}")
    else:
        print(f"{result['actor']}.{result['property']} = {result['value']} ({result['type']})")

def cmd_set(args):
    data = {"actor": args.actor, "property": args.property, "value": args.value}
    result = api_post("/api/property", data)
    if "error" in result:
        print(f"Error: {result['error']}")
    else:
        print(f"Set {result['property']} = {result['value']}")

def cmd_mcp(args):
    """MCP server mode - runs a JSON-RPC server for AI/LLM integration."""
    try:
        from http.server import HTTPServer, BaseHTTPRequestHandler
    except ImportError:
        print("Error: http.server module not available")
        return

    class MCPHandler(BaseHTTPRequestHandler):
        def do_POST(self):
            length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(length).decode() if length else "{}"
            try:
                req = json.loads(body)
            except json.JSONDecodeError:
                req = {}

            method = req.get("method", "")
            req_id = req.get("id", 1)
            params = req.get("params", {})

            if method == "mcp.tools.list":
                response = {
                    "jsonrpc": "2.0",
                    "id": req_id,
                    "result": {
                        "tools": [
                            {"name": "status", "description": "Get bridge status"},
                            {"name": "actors", "description": "List all actors in level"},
                            {"name": "spawn", "description": "Spawn an actor",
                             "inputSchema": {"type": "object", "properties": {
                                 "class": {"type": "string"},
                                 "location": {"type": "string"},
                                 "label": {"type": "string"}
                             }}},
                            {"name": "exec", "description": "Execute Python code in editor",
                             "inputSchema": {"type": "object", "properties": {
                                 "code": {"type": "string"}
                             }}},
                            {"name": "get_property", "description": "Get actor property",
                             "inputSchema": {"type": "object", "properties": {
                                 "actor": {"type": "string"},
                                 "property": {"type": "string"}
                             }}},
                            {"name": "set_property", "description": "Set actor property",
                             "inputSchema": {"type": "object", "properties": {
                                 "actor": {"type": "string"},
                                 "property": {"type": "string"},
                                 "value": {"type": "string"}
                             }}},
                        ]
                    }
                }
            elif method == "mcp.tools.call":
                tool_name = params.get("name", "")
                tool_args = params.get("arguments", {})
                if tool_name == "status":
                    result_data = api_get("/api/status")
                elif tool_name == "actors":
                    result_data = api_get("/api/actors")
                elif tool_name == "spawn":
                    loc = [float(x) for x in tool_args.get("location", "0,0,0").split(",")]
                    data = {"class": tool_args["class"], "location": loc}
                    if "label" in tool_args:
                        data["label"] = tool_args["label"]
                    result_data = api_post("/api/spawn", data)
                elif tool_name == "exec":
                    result_data = api_post("/api/exec", {"code": tool_args["code"]})
                elif tool_name == "get_property":
                    result_data = api_get(f"/api/property?actor={tool_args['actor']}&property={tool_args['property']}")
                elif tool_name == "set_property":
                    result_data = api_post("/api/property", tool_args)
                else:
                    result_data = {"error": f"Unknown tool: {tool_name}"}

                response = {
                    "jsonrpc": "2.0",
                    "id": req_id,
                    "result": {"content": [{"type": "text", "text": json.dumps(result_data, indent=2)}]}
                }
            else:
                response = {
                    "jsonrpc": "2.0",
                    "id": req_id,
                    "error": {"code": -32601, "message": f"Method not found: {method}"}
                }

            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(json.dumps(response).encode())

        def do_OPTIONS(self):
            self.send_response(200)
            self.send_header("Access-Control-Allow-Origin", "*")
            self.send_header("Access-Control-Allow-Methods", "POST, OPTIONS")
            self.send_header("Access-Control-Allow-Headers", "Content-Type")
            self.end_headers()

        def log_message(self, format, *args):
            print(f"[MCP] {args[0]} {args[1]} {args[2]}")

    port = 8091
    server = HTTPServer(("127.0.0.1", port), MCPHandler)
    print(f"MCP server running on http://127.0.0.1:{port}")
    print("Configure your AI tool to use this MCP server.")
    print("Press Ctrl+C to stop.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down.")
        server.server_close()

def main():
    parser = argparse.ArgumentParser(description="UE Terminal Bridge - Control Unreal Engine from terminal")
    parser.add_argument("--port", type=int, default=DEFAULT_PORT, help=f"Bridge port (default: {DEFAULT_PORT})")

    sub = parser.add_subparsers(dest="command", help="Command to execute")

    sub.add_parser("status", help="Check bridge status")
    sub.add_parser("actors", help="List all actors in level")

    p_spawn = sub.add_parser("spawn", help="Spawn an actor")
    p_spawn.add_argument("actor_class", help="Class name (e.g. StaticMeshActor)")
    p_spawn.add_argument("--location", default="0,0,0", help="X,Y,Z location")
    p_spawn.add_argument("--label", help="Actor label")

    p_exec = sub.add_parser("exec", help="Execute Python code")
    p_exec.add_argument("code", help="Python code to execute")

    p_get = sub.add_parser("get", help="Get actor property")
    p_get.add_argument("actor", help="Actor name or label")
    p_get.add_argument("property", help="Property name")

    p_set = sub.add_parser("set", help="Set actor property")
    p_set.add_argument("actor", help="Actor name or label")
    p_set.add_argument("property", help="Property name")
    p_set.add_argument("value", help="Property value")

    sub.add_parser("mcp", help="Start MCP server for AI/LLM integration")

    args = parser.parse_args()

    global BASE_URL
    BASE_URL = f"http://127.0.0.1:{args.port}"

    if args.command == "status":
        cmd_status(args)
    elif args.command == "actors":
        cmd_actors(args)
    elif args.command == "spawn":
        cmd_spawn(args)
    elif args.command == "exec":
        cmd_exec(args)
    elif args.command == "get":
        cmd_get(args)
    elif args.command == "set":
        cmd_set(args)
    elif args.command == "mcp":
        cmd_mcp(args)
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
