// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetingSystemComponent.generated.h"

class UTargetPointFilterBase;
class UWidgetComponent;
class UCameraComponent;
class UTargetPointComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTargetingSystemCompTargetPointDelegate, UTargetPointComponent*, NewTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTargetingSystemCompGenericBoolDelegate, bool, bEnabled);

/**
 * Finds a TargetPointComponent within range to target and attach a widget to it. Can also control the camera and
 * pawn's rotation to face the target.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TARGETINGSYSTEM_API UTargetingSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTargetingSystemComponent();
	
	/**
	 * Updates the TargetPoint with the passed in value. If Pawn doesn't have authority calls the server version.
	 * @param NewTargetPoint Updates the currently selected TargetPoint with the NewTargetPoint.
	 */
	UFUNCTION(BlueprintCallable, Category="Targeting System")
	void SetTarget(UTargetPointComponent* NewTargetPoint);

	/**
	 * Finds the target that is closest to the of this pawn.
	 * @param Filters TargetPoints to filter out.
	 */
	UFUNCTION(BlueprintCallable, Category = "Targeting System")
	UTargetPointComponent* FindNearestTarget(const TArray<UTargetPointFilterBase*>& Filters) const;
	
	/** Searches for the next targetable point right of the current target point. If there is no current target,
	 * will call FindNearestTarget.
	 * @param Filters TargetPoints to filter out.
	 * @param bReverseDirection If true, will search left of target. False, right of target.
	 */
	UFUNCTION(BlueprintCallable, Category = "Targeting System")
	UTargetPointComponent* FindNextTarget(const TArray<UTargetPointFilterBase*>& Filters, bool bReverseDirection = false) const;
	
	/** Clears the currently selected target and unlocks the camera. */
	UFUNCTION(BlueprintCallable, Category = "Targeting System")
	void ClearTarget();
	
	/** Returns the reference to currently targeted TargetPoint. */
	UFUNCTION(BlueprintPure, Category = "Targeting System")
	UTargetPointComponent* GetTarget() const;

	/** Returns true if the TargetPoint is targetable */
	UFUNCTION(BlueprintPure, Category = "Targeting System")
	bool IsTargetable(UTargetPointComponent* InTargetPoint);
	
	/** Toggles between locking and unlocking the camera and rotation. */
	UFUNCTION(BlueprintCallable, Category = "Targeting System")
	void ToggleCameraLock();

	/**
	 * Enables or disables the camera lock.
	 * @param bLock If true will lock the camera.
	*/
	UFUNCTION(BlueprintCallable, Category = "Targeting System")
	void SetCameraLock(bool bLock);

	/** Gets if the target is currently locked onto. */
	UFUNCTION(BlueprintPure, Category = "Targeting System")
	bool IsCameraLocked() const;
	
	/**
	 * Called when a new target is selected. Guaranteed to be a valid TargetPoint.
	 */
	UPROPERTY(BlueprintAssignable)
	FTargetingSystemCompTargetPointDelegate OnTargetPointSelected;

	/**
	 *	Called when the TargetedPoint is out of reach (based on MinimumDistanceToEnable) or behind an Object.
	 */
	UPROPERTY(BlueprintAssignable)
	FTargetingSystemCompTargetPointDelegate OnTargetPointCleared;

	/**
	 * Called when the target is locked onto or removed. Returns true if the camera is locked.
	 */
	UPROPERTY(BlueprintAssignable)
	FTargetingSystemCompGenericBoolDelegate OnCameraLockToggled;
	
	/**
	 * Finds all the TargetablePoints within range.
	 * @param Filters The filter to use to find targets. If null, will return all TargetPoints.
	 */
	UFUNCTION(BlueprintPure, Category = "Targeting System")
	TArray<UTargetPointComponent*> GetTargetablePoints(const TArray<UTargetPointFilterBase*>& Filters) const;
	
	/** Gets the distance between OwnerPawn and InTargetPoint */
	float GetDistanceToPoint(const UTargetPointComponent* InTargetPoint) const;

	//----------------------------------------------------------------------------------------------------------------
	// Component Overrides.
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//----------------------------------------------------------------------------------------------------------------

protected:
	
	/** The maximum distance from a TargetPoint that allows targeting. */
	UPROPERTY(EditDefaultsOnly, Category = "Targeting System")
	float MaxTargetingRange = 2000.0f;;

	/** Frequency to check if the target is in line of sight, within range, and is generally targetable. */
	UPROPERTY(EditDefaultsOnly, Category = "Targeting System")
	float CheckFrequency = 0.1f;
	
	/** The amount of time to break targeting when the Actor is too far away or obstructed behind an Object. */
	UPROPERTY(EditDefaultsOnly, Category = "Targeting System")
	float BreakTargetingDelay = 2.0f;

	/** Whether to accept pitch input when bAdjustPitchBasedOnDistanceToTarget is disabled */
	UPROPERTY(EditDefaultsOnly, Category = "Targeting System")
	bool bIgnoreLookInput = true;

	/** When true, the characters rotation will rotate towards the LockedOnTarget. */
	UPROPERTY(EditDefaultsOnly, Category = "Targeting System|Rotation")
	bool bForceOrientRotationToLockOnTarget = false;

	/** The rate of rotation to face the target. */
	UPROPERTY(EditDefaultsOnly, Category = "Targeting System|Rotation", meta = (EditCondition="bForceOrientRotationToLockOnTarget"))
	float PawnInterpSpeed = 25.0f;

	/**
	 * The Widget Class to use when spawning a targeting widget. If empty, fallback to using the default in settings.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Targeting System|Widget")
	TSubclassOf<UUserWidget> TargetWidgetClass;

	/**
	 * Setting this to true will tell the Targeting System to adjust the Pitch Offset (the Y axis) when locked on,
	 * depending on the distance to the target actor.
	 * It will ensure that the Camera will be moved up vertically the closer this Actor gets to its target.
	 * Formula:
	 * (DistanceToTarget * PitchDistanceCoefficient + PitchDistanceOffset) * -1.0f
	 * Then Clamped by PitchMin / PitchMax
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Targeting System|Pitch Offset")
	bool bAdjustPitchBasedOnDistanceToTarget = true;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting System|Pitch Offset")
	float PitchDistanceCoefficient = -0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting System|Pitch Offset")
	float PitchDistanceOffset = 60.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting System|Pitch Offset")
	float PitchMin = -50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting System|Pitch Offset")
	float PitchMax = -20.0f;

private:	
	/**
	 * Functionality to clear the target if line of sight is broken during target selection
	 */
	UPROPERTY()
	FTimerHandle CheckTargetPointTimerHandle;
	UPROPERTY()
	FTimerHandle BreakTargetPointTimerHandle;
	UPROPERTY()
	bool bIsBreakingLineOfSight;
	
	void CheckTargetPoint();
	bool ShouldBreakTargeting() const;
	void BreakTargeting();
	
	//~ Actor rotation

	/** Gets the rotation to face the TargetPoint */
	FRotator GetControlRotation(const UTargetPointComponent* InTargetPoint, float DeltaTime) const;

	/** Sets the control rotation on the owning player controller and smoothly rotates the pawn
	 * to face the selected TargetPoint.
	 */
	void SetControlRotation(UTargetPointComponent* InTargetPoint, float DeltaTime) const;

	/** Sets the owning player's character movement component OrientRotationToMovement. */
	void SetOrientRotationToMovement(bool bOrientRotationToMovement) const;
	
	/** Attaches a widget to the component. */
	void CreateAndAttachTargetSelectedWidgetComponent(UTargetPointComponent* InTargetPoint);
	
	/**
	 *  Sets up cached Owner PlayerController from Owner Pawn.
	 *  For local split screen, Pawn's Controller may not have been set up already when this component begins play.
	 */
	 void SetupLocalPlayerController();
	
	/** Cached reference of the owner of this component. */
	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn;
	UPROPERTY()
	TObjectPtr<APlayerController> OwnerPlayerController;
	UPROPERTY()
	TObjectPtr<UCameraComponent> CameraComponent;
	UPROPERTY()
	TObjectPtr<UWidgetComponent> TargetWidgetComponent;
	UPROPERTY(ReplicatedUsing = OnRep_TargetedPoint)
	TWeakObjectPtr<UTargetPointComponent> TargetedPoint;
	UFUNCTION()
	void OnRep_TargetedPoint();
	UPROPERTY()
	bool bCameraLocked = false;

	UFUNCTION()
	void OnTargetPointOwnerDestroyed(AActor* DestroyedActor);

	UFUNCTION(Server, Reliable)
	void Server_SetCameraLock(bool bLocked);
	UFUNCTION(Server, Reliable)
	void Server_ClearTarget();
	UFUNCTION(Server, Reliable)
	void Server_SetTarget(UTargetPointComponent* NewTargetPoint);
};
