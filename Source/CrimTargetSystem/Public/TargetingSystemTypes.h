// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "TargetingSystemTypes.generated.h"

class UTargetPointManagerComponent;
class UTargetPointComponent;

USTRUCT(BlueprintType)
struct TARGETINGSYSTEM_API FTargetPointItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	/** Tag that maps to a TargetPointComponent */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag TargetPointTag;

	/** Enables/Disables the ability to target this point. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bCanBeTargeted = true;

	/** The reference to this target point. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTargetPointComponent> TargetPointComponent;
};

USTRUCT(BlueprintType)
struct TARGETINGSYSTEM_API FTargetPointContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	FTargetPointContainer()
	{}

	const TArray<FTargetPointItem>& GetAllItems() const;
	
	//~FFastArraySerializer contract
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FTargetPointItem, FTargetPointContainer>(Items, DeltaParams, *this);
	}

private:
	friend UTargetPointManagerComponent;
	
	// Replicated list of TargetPoints.
	UPROPERTY()
	TArray<FTargetPointItem> Items;
};
template<>
struct TStructOpsTypeTraits<FTargetPointContainer> : TStructOpsTypeTraitsBase2<FTargetPointContainer>
{
	enum { WithNetDeltaSerializer = true };
};
