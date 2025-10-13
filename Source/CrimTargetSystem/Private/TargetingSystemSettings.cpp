// Copyright Soccertitan


#include "TargetingSystemSettings.h"

#include "TargetingSystemLogChannels.h"
#include "Blueprint/UserWidget.h"
#include "Engine/AssetManager.h"

UTargetingSystemSettings::UTargetingSystemSettings()
{
	TargetWidgetClass  = TSoftClassPtr<UUserWidget>(FSoftClassPath(
		"/TargetingSystem/WBP_TargetIndicatorDefault.WBP_TargetIndicatorDefault"));
}

FName UTargetingSystemSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

TSubclassOf<UUserWidget> UTargetingSystemSettings::GetDefaultTargetWidgetClass()
{
	const UTargetingSystemSettings* Settings = GetDefault<UTargetingSystemSettings>();

	if (Settings->TargetWidgetClass.IsNull())
	{
		UE_LOG(LogTargetingSystem, Error, TEXT("UTargetingSystemSettings.TargetWidgetClass is not valid. "
			"Set a value in the project settings."));
	}

	if (!Settings->TargetWidgetClass.Get())
	{
		UAssetManager::Get().LoadAssetList({Settings->TargetWidgetClass.ToSoftObjectPath()})->WaitUntilComplete();
	}

	return Settings->TargetWidgetClass.Get();
}
