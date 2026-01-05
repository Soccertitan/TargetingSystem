// Copyright Soccertitan 2025


#include "TargetPointComponent.h"


UTargetPointComponent::UTargetPointComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bHiddenInGame = true;
	SetIsReplicatedByDefault(false);
	SetCollisionProfileName(FName("Trigger"));
	SetCanEverAffectNavigation(false);

	SphereRadius = 0.f;
}

void UTargetPointComponent::SetIsTargetable(const bool bEnabled)
{
	bTargetable = bEnabled;
}



