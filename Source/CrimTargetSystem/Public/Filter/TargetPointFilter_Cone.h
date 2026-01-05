// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "TargetPointFilterBase.h"
#include "TargetPointFilter_Cone.generated.h"

/**
 * Filters out targets that are not in front of the actor's cone.
 */
UCLASS()
class TARGETINGSYSTEM_API UTargetPointFilter_Cone : public UTargetPointFilterBase
{
	GENERATED_BODY()

public:
	virtual void FilterTargetPoints(const AActor* SourceActor, TArray<UTargetPointComponent*>& TargetPoints) const override;

	// The half angle of the cone. Will be doubled for the full angle of the cone.
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0, ClampMax = 180))
	float ConeHalfAngle = 45.0f;
};
