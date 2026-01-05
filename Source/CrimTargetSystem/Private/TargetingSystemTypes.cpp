// Copyright Soccertitan 2025


#include "TargetingSystemTypes.h"

#include "TargetPointComponent.h"

const TArray<FTargetPointItem>& FTargetPointContainer::GetAllItems() const
{
	return Items;
}

void FTargetPointContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Index : AddedIndices)
	{
		FTargetPointItem& Item = Items[Index];
		if (IsValid(Item.TargetPointComponent))
		{
			Item.TargetPointComponent->SetIsTargetable(Item.bCanBeTargeted);
		}
	}
}

void FTargetPointContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (const int32 Index : ChangedIndices)
	{
		FTargetPointItem& Item = Items[Index];
		if (IsValid(Item.TargetPointComponent))
		{
			Item.TargetPointComponent->SetIsTargetable(Item.bCanBeTargeted);	
		}
	}
}