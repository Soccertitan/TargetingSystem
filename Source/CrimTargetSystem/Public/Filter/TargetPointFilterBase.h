// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"

#include "TargetPointFilterBase.generated.h"

class UTargetPointComponent;

/**
 * An abstract class for defining which Target Points to filter out.
 */
UCLASS(Abstract, Blueprintable, DefaultToInstanced, EditInlineNew)
class TARGETINGSYSTEM_API UTargetPointFilterBase : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Filters out the passed in TargetPoints given a SourceActor.
	 * 
	 * @param SourceActor The actor used to filter the TargetPoints against.
	 * @param TargetPoints Modifies this array of passed in TargetPoints with the filtered list.
	 */
	UFUNCTION(BlueprintCallable, Category = "Targeting System|Filter")
	virtual void FilterTargetPoints(const AActor* SourceActor, TArray<UTargetPointComponent*>& TargetPoints) const;

protected:
	/**
	 * Filters out the passed in TargetPoints given a SourceActor.
	 * 
	 * @param SourceActor The actor used to filter the TargetPoints against.
	 * @param TargetPoints Modifies this array of passed in TargetPoints with the filtered list.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Targeting System|Filter", meta = (DisplayName = "FilterTargetPoints"))
	void K2_FilterTargetPoints(const AActor* SourceActor, UPARAM(ref) TArray<UTargetPointComponent*>& TargetPoints) const;
	
};
