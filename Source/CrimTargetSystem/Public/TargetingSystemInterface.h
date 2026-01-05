// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TargetingSystemInterface.generated.h"

class UTargetingSystemComponent;
// This class does not need to be modified.
UINTERFACE()
class UTargetingSystemInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TARGETINGSYSTEM_API ITargetingSystemInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent)
	UTargetingSystemComponent* GetTargetingSystemComponent() const;
};
