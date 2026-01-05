// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TargetingSystemBlueprintFunctionLibrary.generated.h"

class UTargetingSystemComponent;

/**
 * 
 */
UCLASS()
class TARGETINGSYSTEM_API UTargetingSystemBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Targeting System")
	static UTargetingSystemComponent* GetTargetingSystemComponent(AActor* Actor);
};
