// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/SphereComponent.h"
#include "TargetPointComponent.generated.h"

class UTargetPointManagerComponent;

/**
 * Can be managed by a TargetPointManagerComponent. Allows a pawn with the TargetSystemComponent to lock
 * on to and target this point.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TARGETINGSYSTEM_API UTargetPointComponent : public USphereComponent
{
	GENERATED_BODY()

	friend UTargetPointManagerComponent;
	friend struct FTargetPointContainer;

public:
	UTargetPointComponent();

	UFUNCTION(BlueprintPure, Category = "Targeting System|Target Point")
	FGameplayTag GetTargetPointTag () const {return TargetPointTag;}

	UFUNCTION(BlueprintPure, Category = "Targeting System|Target Point")
	bool GetIsTargetable () const {return bTargetable;}

private:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag TargetPointTag;
	
	UPROPERTY(EditDefaultsOnly)
	bool bTargetable = true;

	void SetIsTargetable(const bool bEnabled);
};
