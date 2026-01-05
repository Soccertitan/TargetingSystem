// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "TargetingSystemTypes.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "TargetPointManagerComponent.generated.h"


class UTargetPointComponent;

/**
 *	An actor with this component can manage their TargetPointComponents. On BeginPlay it will search the
 *	OwningActor for existing TargetPointComponents and add them to the manager.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TARGETINGSYSTEM_API UTargetPointManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTargetPointManagerComponent();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Enables/Disables the specified TargetPointComponent for targeting. */
	UFUNCTION(BlueprintCallable, Category = "Targeting System|Target Point")
	void SetTargetPointEnabled(UTargetPointComponent* TargetPoint, bool bEnabled);

	/** Enables/Disables all TargetPoints that have the specified tag. */
	UFUNCTION(BlueprintCallable, Category = "Targeting System|Target Point")
	void SetTargetPointEnabledByTag(const FGameplayTag& Type, bool bEnabled);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(Replicated)
	FTargetPointContainer TargetPointList;
	
	/** Called on BeginPlay to add TargetPoints to the manager. */
	void InitializeTargetPoints();
};
