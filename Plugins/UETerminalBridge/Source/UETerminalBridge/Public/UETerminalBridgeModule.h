#pragma once
#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUETerminal, Log, All);

class FUETerminalBridgeModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
