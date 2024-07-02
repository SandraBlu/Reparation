// Copyright 2023 Klian. All Rights Reserved.

#include "ReplicatedToss.h"

#define LOCTEXT_NAMESPACE "FReplicatedTossModule"

void FReplicatedTossModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FReplicatedTossModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FReplicatedTossModule, ReplicatedToss)