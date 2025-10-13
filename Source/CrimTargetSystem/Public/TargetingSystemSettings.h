// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TargetingSystemSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig)
class TARGETINGSYSTEM_API UTargetingSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UTargetingSystemSettings();
	virtual FName GetCategoryName() const override;

	/** The default widget to spawn when targeting. */
	UPROPERTY(Config, EditAnywhere)
	TSoftClassPtr<UUserWidget> TargetWidgetClass;

	static TSubclassOf<UUserWidget> GetDefaultTargetWidgetClass();
};
