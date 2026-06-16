#include "UETerminalBridgeModule.h"
#include "TerminalServer.h"

DEFINE_LOG_CATEGORY(LogUETerminal);

static TUniquePtr<FTerminalServer> GTerminalServer;

void FUETerminalBridgeModule::StartupModule()
{
	const int32 Port = 8090;
	GTerminalServer = MakeUnique<FTerminalServer>();
	if (GTerminalServer->Start(Port))
	{
		UE_LOG(LogUETerminal, Log, TEXT("Terminal Bridge started on http://127.0.0.1:%d"), Port);
	}
	else
	{
		UE_LOG(LogUETerminal, Error, TEXT("Failed to start Terminal Bridge on port %d"), Port);
	}
}

void FUETerminalBridgeModule::ShutdownModule()
{
	if (GTerminalServer)
	{
		GTerminalServer->Stop();
		GTerminalServer.Reset();
	}
}

IMPLEMENT_MODULE(FUETerminalBridgeModule, UETerminalBridge)
