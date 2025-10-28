// @MaoX Coding 

#pragma once

#include "Modules/ModuleManager.h"

class FLocomotionSystemModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
