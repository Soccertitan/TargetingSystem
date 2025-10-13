// Copyright Soccertitan


#include "Filter/TargetPointFilterBase.h"


void UTargetPointFilterBase::FilterTargetPoints(const AActor* SourceActor, TArray<UTargetPointComponent*>& TargetPoints) const
{
	K2_FilterTargetPoints(SourceActor, TargetPoints);
}
