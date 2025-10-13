// Copyright Soccertitan


#include "TargetPointManagerComponent.h"

#include "TargetPointComponent.h"
#include "Net/UnrealNetwork.h"


UTargetPointManagerComponent::UTargetPointManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UTargetPointManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTargetPointManagerComponent, TargetPointList);
}

void UTargetPointManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeTargetPoints();
}

void UTargetPointManagerComponent::SetTargetPointEnabled(UTargetPointComponent* TargetPoint, bool bEnabled)
{
	if (!IsValid(TargetPoint))
	{
		return;
	}
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	for (auto& Item : TargetPointList.Items)
	{
		if (Item.TargetPointComponent == TargetPoint)
		{
			Item.bCanBeTargeted = bEnabled;
			Item.TargetPointComponent->SetIsTargetable(bEnabled);
			TargetPointList.MarkItemDirty(Item);
			return;
		}
	}
}

void UTargetPointManagerComponent::SetTargetPointEnabledByTag(const FGameplayTag& Type, bool bEnabled)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	for (auto& Item : TargetPointList.Items)
	{
		if (Item.TargetPointTag.MatchesTag(Type))
		{
			Item.bCanBeTargeted = bEnabled;
			Item.TargetPointComponent->SetIsTargetable(bEnabled);
			TargetPointList.MarkItemDirty(Item);
		}
	}
}

void UTargetPointManagerComponent::InitializeTargetPoints()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	
	TArray<UTargetPointComponent*> TargetPointComponents;
	GetOwner()->GetComponents(UTargetPointComponent::StaticClass(), TargetPointComponents);
	for (const auto& Item : TargetPointComponents)
	{
		FTargetPointItem& NewItem = TargetPointList.Items.AddDefaulted_GetRef();
		NewItem.bCanBeTargeted = Item->GetIsTargetable();
		NewItem.TargetPointTag = Item->GetTargetPointTag();
		NewItem.TargetPointComponent = Item;
		TargetPointList.MarkItemDirty(NewItem);
	}
}
