// Copyright Soccertitan


#include "TargetingSystemComponent.h"

#include "TargetPointComponent.h"
#include "TargetingSystemLogChannels.h"
#include "TargetingSystemSettings.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/OverlapResult.h"
#include "Filter/TargetPointFilterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"


UTargetingSystemComponent::UTargetingSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UTargetingSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!TargetWidgetClass)
	{
		TargetWidgetClass = UTargetingSystemSettings::GetDefaultTargetWidgetClass();	
	}

	OwnerPawn = Cast<APawn>(GetOwner());
	if (!ensure(OwnerPawn))
	{
		UE_LOG(LogTargetingSystem, Error, TEXT("[%s] TargetingSystemComponent: Component is meant to be added to Pawn only."), *GetName());
		Deactivate();
		return;
	}

	CameraComponent = OwnerPawn->FindComponentByClass<UCameraComponent>();
	if(!IsValid(CameraComponent))
	{
		UE_LOG(LogTargetingSystem, Error, TEXT("[%s] TargetingSystemComponent: Cannot get the Camera component. "), *GetName());
		Deactivate();
		return;
	}

	SetupLocalPlayerController();
}

void UTargetingSystemComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UTargetingSystemComponent, TargetedPoint, COND_None, REPNOTIFY_Always);
}

void UTargetingSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bCameraLocked && IsValid(TargetedPoint.Get()))
	{
		SetControlRotation(TargetedPoint.Get(), DeltaTime);
	}
}

void UTargetingSystemComponent::SetTarget(UTargetPointComponent* NewTargetPoint)
{
	if (TargetedPoint.Get() == NewTargetPoint)
	{
		return;
	}

	if (!IsValid(NewTargetPoint))
	{
		ClearTarget();
		return;
	}

	if (!IsTargetable(NewTargetPoint))
	{
		return;
	}

	if (!GetOwner()->HasAuthority())
	{
		Server_SetTarget(NewTargetPoint);
	}
	
	TargetedPoint = NewTargetPoint;
	
	CreateAndAttachTargetSelectedWidgetComponent(NewTargetPoint);
	NewTargetPoint->GetOwner()->OnDestroyed.AddUniqueDynamic(this, &UTargetingSystemComponent::OnTargetPointOwnerDestroyed);
	
	GetWorld()->GetTimerManager().SetTimer(
		CheckTargetPointTimerHandle,
		this,
		&UTargetingSystemComponent::CheckTargetPoint,
		CheckFrequency,
		true
	);

	if (GetOwner()->HasAuthority())
	{
		OnRep_TargetedPoint();	
	}
}

UTargetPointComponent* UTargetingSystemComponent::FindNearestTarget(const TArray<UTargetPointFilterBase*>& Filters) const
{
	TArray<UTargetPointComponent*> TargetablePoints;
	if (Filters.Num() > 0)
	{
		TargetablePoints = GetTargetablePoints(Filters);
	}

	if (TargetablePoints.IsEmpty())
	{
		return nullptr;
	}

	UTargetPointComponent* NearestTarget = nullptr;
	float ClosestDistance = TNumericLimits<float>::Max();
	FVector Origin = OwnerPawn->GetActorLocation();
	
	for (UTargetPointComponent* Target : TargetablePoints)
	{
		if (IsValid(Target))
		{
			const float Distance = (Origin - Target->GetComponentLocation()).Size();
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				NearestTarget = Target;
			}
		}
	}
	return NearestTarget;
}

UTargetPointComponent* UTargetingSystemComponent::FindNextTarget(const TArray<UTargetPointFilterBase*>& Filters, bool bReverseDirection) const
{
	TArray<UTargetPointComponent*> TargetablePoints = GetTargetablePoints(Filters);
	UTargetPointComponent* NewTarget = TargetedPoint.Get();

	if (IsValid(NewTarget))
	{
		const FVector ReferenceLocation = CameraComponent->GetComponentLocation();
		const FVector ReferenceActor = NewTarget->GetComponentLocation();
		FVector2D ReferenceVector = {ReferenceActor.X - ReferenceLocation.X , ReferenceActor.Y - ReferenceLocation.Y};
		ReferenceVector.Normalize();

		float RightComparison;
		float LeftComparison;
		UTargetPointComponent* RightTarget = nullptr;
		UTargetPointComponent* LeftTarget = nullptr;

		if(bReverseDirection)
		{
			// Cycle to the left
			RightComparison = 0.f;
			LeftComparison = -180.f;
		}
		else
		{
			// Cycle to the right
			RightComparison = 180.f;
			LeftComparison = 0.f;
		}

		for (UTargetPointComponent* Target : TargetablePoints)
		{
			if (!IsValid(Target))
			{
				continue;
			}
			
			// Gets the rotation from the source actor to the next potential targetable actor.
			const FVector TargetLocation = Target->GetComponentLocation();
			FVector2D ComparisonActorVector = {TargetLocation.X - ReferenceLocation.X , TargetLocation.Y - ReferenceLocation.Y};
			ComparisonActorVector.Normalize();

			const float ZRotation = FMath::RadiansToDegrees(FGenericPlatformMath::Acos(
				FVector2D::DotProduct(ReferenceVector, ComparisonActorVector))) *
					FMath::Sign(FVector2D::CrossProduct(ReferenceVector, ComparisonActorVector));

			//Get the target closest to the right and furthest from the left of the original target if we choose targets to the right
			if (!bReverseDirection)
			{
				if (ZRotation > 0 && ZRotation < RightComparison)
				{
					RightComparison = ZRotation;
					RightTarget = Target;
				}
				else if (ZRotation < 0 && ZRotation < LeftComparison)
				{
					LeftComparison = ZRotation;
					LeftTarget = Target;
				}
			}
			else
			{
				if (ZRotation > 0 && ZRotation > RightComparison)
				{
					RightComparison = ZRotation;
					RightTarget = Target;
				}
				else if (ZRotation < 0 && ZRotation > LeftComparison)
				{
					LeftComparison = ZRotation;
					LeftTarget = Target;
				}
			}
		}

		if (RightTarget || LeftTarget)
		{
			// Selects target to the right
			if (!bReverseDirection)
			{
				if (RightTarget)
				{
					NewTarget = RightTarget;
				}
				else if (LeftTarget)
				{
					NewTarget = LeftTarget;
				}
			}
			else
			{
				if (LeftTarget)
				{
					NewTarget = LeftTarget;
				}
				else if (RightTarget)
				{
					NewTarget = RightTarget;
				}
			}
		}
	}
	else
	{
		NewTarget = FindNearestTarget(Filters);
	}
	
	return NewTarget;
}

void UTargetingSystemComponent::ClearTarget()
{
	if (!GetOwner()->HasAuthority())
	{
		Server_ClearTarget();
	}
	
	TargetedPoint = nullptr;
	if (IsValid(TargetWidgetComponent))
	{
		TargetWidgetComponent->DestroyComponent();
	}
	SetCameraLock(false);
	OnRep_TargetedPoint();
}

UTargetPointComponent* UTargetingSystemComponent::GetTarget() const
{
	return TargetedPoint.Get();
}

bool UTargetingSystemComponent::IsTargetable(UTargetPointComponent* InTargetPoint)
{
	if (IsValid(InTargetPoint))
	{
		return InTargetPoint->GetIsTargetable();
	}
	return false;
}

void UTargetingSystemComponent::ToggleCameraLock()
{
	if (bCameraLocked)
	{
		SetCameraLock(false);
	}
	else
	{
		SetCameraLock(true);
	}
}

void UTargetingSystemComponent::SetCameraLock(bool bLock)
{
	if (bLock == bCameraLocked)
	{
		return;
	}

	if (!GetOwner()->HasAuthority())
	{
		Server_SetCameraLock(bLock);
	}
	
	// Recast PlayerController in case it wasn't already setup on Begin Play (local split screen)
	SetupLocalPlayerController();

	if (bLock)
	{
		// Won't lock the camera if we are targeting a Point on ourselves.
		if (IsValid(TargetedPoint.Get()) &&
			TargetedPoint.Get()->GetOwner() != GetOwner())
		{
			SetOrientRotationToMovement(!bForceOrientRotationToLockOnTarget);

			if ((bAdjustPitchBasedOnDistanceToTarget || bIgnoreLookInput) &&
			IsValid(OwnerPlayerController))
			{
				OwnerPlayerController->SetIgnoreLookInput(true);
			}
			bCameraLocked = bLock;
			OnCameraLockToggled.Broadcast(bCameraLocked);
		}
	}
	else
	{
		SetOrientRotationToMovement(true);
		
		if (IsValid(OwnerPlayerController))
		{
			OwnerPlayerController->ResetIgnoreLookInput();
		}
		bCameraLocked = bLock;
		OnCameraLockToggled.Broadcast(bCameraLocked);
	}
}

bool UTargetingSystemComponent::IsCameraLocked() const
{
	return bCameraLocked;
}

TArray<UTargetPointComponent*> UTargetingSystemComponent::GetTargetablePoints(const TArray<UTargetPointFilterBase*>& Filters) const
{
	TArray<UTargetPointComponent*> TargetablePoints;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(SphereOverlapComponenets), false);
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		OwnerPawn->GetActorLocation(),
		FQuat::Identity,
		ObjectParams,
		FCollisionShape::MakeSphere(MaxTargetingRange),
		Params
	);

	for (FOverlapResult& Overlap : Overlaps)
	{
		if (Overlap.Component.IsValid())
		{
			if (UTargetPointComponent* C = Cast<UTargetPointComponent>(Overlap.Component.Get()))
			{
				TargetablePoints.Add(C);
			}
		}
	}

	for (const UTargetPointFilterBase* Filter : Filters)
	{
		if (IsValid(Filter))
		{
			Filter->FilterTargetPoints(OwnerPawn, TargetablePoints);
		}
	}
	
	return TargetablePoints;
}

float UTargetingSystemComponent::GetDistanceToPoint(const UTargetPointComponent* InTargetPoint) const
{
	if (IsValid(InTargetPoint))
	{
		return (OwnerPawn->GetActorLocation() - InTargetPoint->GetComponentLocation()).Size();	
	}
	return 0.f;
}

void UTargetingSystemComponent::CheckTargetPoint()
{
	if (ShouldBreakTargeting() && !bIsBreakingLineOfSight)
	{
		bIsBreakingLineOfSight = true;
		GetWorld()->GetTimerManager().SetTimer(
			BreakTargetPointTimerHandle,
			this,
			&UTargetingSystemComponent::BreakTargeting,
			BreakTargetingDelay
		);
	}
}

bool UTargetingSystemComponent::ShouldBreakTargeting() const
{
	if (!TargetedPoint.Get())
	{
		return true;
	}

	if (!TargetedPoint.Get()->GetIsTargetable())
	{
		return true;
	}

	FHitResult HitResult;
	FCollisionQueryParams Params = FCollisionQueryParams(FName("LineTraceSingle"));
	Params.AddIgnoredActor(OwnerPawn);

	bool BlockedHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		OwnerPawn->GetActorLocation(),
		TargetedPoint.Get()->GetComponentLocation(),
		ECC_Visibility,
		Params
	);

	if (BlockedHit)
	{
		return true;
	}

	if (GetDistanceToPoint(TargetedPoint.Get()) > MaxTargetingRange)
	{
		return true;
	}

	return false;
}

void UTargetingSystemComponent::BreakTargeting()
{
	bIsBreakingLineOfSight = false;
	if (ShouldBreakTargeting())
	{
		ClearTarget();
	}
}

FRotator UTargetingSystemComponent::GetControlRotation(const UTargetPointComponent* InTargetPoint, float DeltaTime) const
{
	if (!IsValid(OwnerPlayerController))
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetSystemComponent::GetControlRotation - OwnerPlayerController is not valid."))
		return FRotator::ZeroRotator;
	}

	const FRotator ControlRotation = OwnerPlayerController->GetControlRotation();
	const FVector CharacterLocation = OwnerPawn->GetActorLocation();
	const FVector TargetPointLocation = InTargetPoint->GetComponentLocation();

	// Find look at rotation
	const FRotator LookRotation = FRotationMatrix::MakeFromX(TargetPointLocation - CharacterLocation).Rotator();
	float Pitch = LookRotation.Pitch;
	FRotator TargetRotation;
	if (bAdjustPitchBasedOnDistanceToTarget)
	{
		const float DistanceToTarget = (CharacterLocation - TargetPointLocation).Size();
		const float PitchInRange = (DistanceToTarget * PitchDistanceCoefficient + PitchDistanceOffset) * -1.0f;
		const float PitchOffset = FMath::Clamp(PitchInRange, PitchMin, PitchMax);

		Pitch = Pitch + PitchOffset;
		TargetRotation = FRotator(Pitch, LookRotation.Yaw, ControlRotation.Roll);
	}
	else
	{
		if (bIgnoreLookInput)
		{
			TargetRotation = FRotator(Pitch, LookRotation.Yaw, ControlRotation.Roll);
		}
		else
		{
			TargetRotation = FRotator(ControlRotation.Pitch, LookRotation.Yaw, ControlRotation.Roll);
		}
	}

	return FMath::RInterpTo(ControlRotation, TargetRotation, DeltaTime, PawnInterpSpeed);
}

void UTargetingSystemComponent::SetControlRotation(UTargetPointComponent* InTargetPoint, float DeltaTime) const
{
	if (!IsValid(OwnerPlayerController))
	{
		return;
	}
	
	const FRotator ControlRotation = GetControlRotation(InTargetPoint, DeltaTime);

	if(bForceOrientRotationToLockOnTarget && OwnerPawn->GetVelocity().Size() > 0)
	{
		FRotator TargetRotation = FRotator(0, ControlRotation.Yaw, 0);
		OwnerPawn->SetActorRotation(FMath::RInterpTo(OwnerPawn->GetActorRotation(), TargetRotation, DeltaTime, PawnInterpSpeed));
	}
 
	OwnerPlayerController->SetControlRotation(ControlRotation);
}

void UTargetingSystemComponent::SetOrientRotationToMovement(bool bOrientRotationToMovement) const
{
	if (!IsValid(OwnerPawn))
	{
		return;
	}
	
	if (UCharacterMovementComponent* CharacterMovementComponent = OwnerPawn->FindComponentByClass<UCharacterMovementComponent>())
	{
		CharacterMovementComponent->bOrientRotationToMovement = bOrientRotationToMovement;
	}
}

void UTargetingSystemComponent::CreateAndAttachTargetSelectedWidgetComponent(UTargetPointComponent* InTargetPoint)
{
	if (!TargetWidgetClass)
	{
		TargetWidgetClass = UTargetingSystemSettings::GetDefaultTargetWidgetClass();
		if (!TargetWidgetClass)
		{
			UE_LOG(LogTargetingSystem, Error, TEXT("TargetSystemComponent: Cannot find a TargetWidgetClass, please ensure it is a valid reference in the Component Properties."));
			return;
		}
	}

	if (IsValid(TargetWidgetComponent))
	{
		TargetWidgetComponent->DestroyComponent();
	}
	
	if (IsValid(OwnerPlayerController) && OwnerPlayerController->IsLocalPlayerController() && IsValid(InTargetPoint))
	{
		TargetWidgetComponent = NewObject<UWidgetComponent>(InTargetPoint->GetOwner(), MakeUniqueObjectName(InTargetPoint->GetOwner(), UWidgetComponent::StaticClass(), FName("TargetLockOn")));
		TargetWidgetComponent->SetWidgetClass(TargetWidgetClass);
		TargetWidgetComponent->SetOwnerPlayer(OwnerPlayerController->GetLocalPlayer());
		TargetWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		TargetWidgetComponent->SetupAttachment(InTargetPoint);
		TargetWidgetComponent->SetDrawAtDesiredSize(true);
		TargetWidgetComponent->SetVisibility(true);
		TargetWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		TargetWidgetComponent->RegisterComponent();
	}
}

void UTargetingSystemComponent::SetupLocalPlayerController()
{
	if (!IsValid(OwnerPawn))
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] TargetSystemComponent: Component is meant to be added to Pawn only."), *GetName());
		Deactivate();
		return;
	}

	OwnerPlayerController = Cast<APlayerController>(OwnerPawn->GetController());
}

void UTargetingSystemComponent::OnRep_TargetedPoint()
{
	if (TargetedPoint.Get())
	{
		OnTargetPointSelected.Broadcast(TargetedPoint.Get());
	}
	else
	{
		OnTargetPointCleared.Broadcast(nullptr);
	}
}

void UTargetingSystemComponent::OnTargetPointOwnerDestroyed(AActor* DestroyedActor)
{
	ClearTarget();
}

void UTargetingSystemComponent::Server_SetCameraLock_Implementation(bool bLocked)
{
	SetCameraLock(bLocked);
}

void UTargetingSystemComponent::Server_ClearTarget_Implementation()
{
	ClearTarget();
}

void UTargetingSystemComponent::Server_SetTarget_Implementation(UTargetPointComponent* NewTargetPoint)
{
	SetTarget(NewTargetPoint);
}
