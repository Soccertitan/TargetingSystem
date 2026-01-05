// Copyright Soccertitan 2025


#include "Filter/TargetPointFilter_Cone.h"

#include "CrimMathStatics.h"
#include "TargetPointComponent.h"

void UTargetPointFilter_Cone::FilterTargetPoints(const AActor* SourceActor, TArray<UTargetPointComponent*>& TargetPoints) const
{
	Super::FilterTargetPoints(SourceActor, TargetPoints);

	for (int i = TargetPoints.Num() - 1; i >= 0; i--)
	{
		if (UCrimMathStatics::IsInCone(
				SourceActor->GetActorLocation(),
				SourceActor->GetActorForwardVector(),
				ConeHalfAngle,
				TargetPoints[i]->GetComponentLocation()))
		{
			continue;
		}

		TargetPoints.RemoveAt(i);
	}
}
