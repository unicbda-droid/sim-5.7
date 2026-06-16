#pragma once
#include "CoreMinimal.h"
#include "IHttpRouter.h"

class FJsonObject;

class FTerminalServer
{
public:
	bool Start(int32 Port = 8090);
	void Stop();
	bool IsRunning() const { return bRunning; }

private:
	bool bRunning = false;
	int32 ServerPort = 8090;
	TSharedPtr<IHttpRouter> HttpRouter;
	TArray<FHttpRouteHandle> RouteHandles;
};
