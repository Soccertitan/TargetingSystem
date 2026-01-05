// Copyright Soccertitan 2025


#include "TargetingSystemBlueprintFunctionLibrary.h"

#include "TargetingSystemComponent.h"
#include "TargetingSystemInterface.h"

UTargetingSystemComponent* UTargetingSystemBlueprintFunctionLibrary::GetTargetingSystemComponent(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	if (Actor->Implements<UTargetingSystemInterface>())
	{
		return ITargetingSystemInterface::Execute_GetTargetingSystemComponent(Actor);
	}

	return Actor->FindComponentByClass<UTargetingSystemComponent>();
}
