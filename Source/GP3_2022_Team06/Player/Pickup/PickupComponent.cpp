// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "GP3_2022_Team06/GlitchComponent.h"
#include "GP3_2022_Team06/Generator.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UPickupComponent::UPickupComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	CurrentDistanceX = HoldDistanceX;
	CurrentItem = nullptr;
	bCanMove = true;
	bInspecting = false;

	bPickupInRange = false;
	bHoldOnToObject = false;
	bItemBehindWall = false;
}

// Called when the game starts
void UPickupComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Actor = GetOwner();

	if (!Actor)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.f, FColor::Red, FString::Printf(TEXT(
			"Error: Something is wrong, (actor not found, calling from pickup component here) can you hear me, Major Tom? x3")));

		return;
	}

	TArray<USceneComponent*> Components;
	Actor->GetComponents(Components);

	if (Components.Num() > 0)
	{
		for (auto& Comp : Components) {
			if (Comp->GetName().Compare("HoldComponent") == 0)
			{
				HoldComponent = Cast<USceneComponent>(Comp);
			}
			else if (Comp->GetName().Compare("CenterComponent") == 0)
			{
				CenterComponent = Cast<USceneComponent>(Comp);
			}
		}
	}

	if (!HoldComponent || !CenterComponent)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.f, FColor::Red, FString::Printf(TEXT(
			"Error: Something is wrong, (comp(s) not found) can you hear me, Major Tom? x3")));
	}
	HoldComponent->SetRelativeLocation(FVector(HoldDistanceX, HoldOffsetY, HoldOffsetY));

	PitchMax = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMax;
	PitchMin = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMin;

	CameraComponent = Actor->FindComponentByClass<UCameraComponent>();

	if (!CameraComponent)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.f, FColor::Red, FString::Printf(TEXT(
			"Error: Something is wrong, (camera not found) can you hear me, Major Tom? x3")));
	}

	InputComp = Actor->InputComponent;

	if (InputComp)
	{
		InputComp->BindAction(TEXT("Inspect"), IE_Pressed, this, &UPickupComponent::OnInspect);
		InputComp->BindAction(TEXT("Inspect"), IE_Released, this, &UPickupComponent::OnInspectReleased);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.f, FColor::Red, FString::Printf(TEXT(
			"Error: Something is wrong, (input from owner not found) can you hear me, Major Tom? x3")));
	}

}

// Called every frame
void UPickupComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// Getting values for the line tracings
	if (!bHoldingItem)
	{
		Start = GetLineTracePositionStart(true);
		ForwardVector = CameraComponent->GetForwardVector();
	}
	else
	{
		Start = GetLineTracePositionStart(false);
		ForwardVector = HoldComponent->GetForwardVector();
	}

	End = ((ForwardVector * PickupReach) + Start);

	FVector TowardsPlayerStart;
	FVector TowardsPlayerEnd;

	if (bHoldingItem)
	{
		TowardsPlayerStart = HoldComponent->GetComponentLocation() +
			(HoldComponent->GetForwardVector() * 2); // Adds a little offset
		TowardsPlayerEnd = ((ForwardVector * -HoldDistanceX) + Start);
	}

	if (bMoveWithCamera && !bAttachToHand)
	{
		CenterComponent->SetRelativeLocationAndRotation(CameraComponent->GetRelativeLocation(),
			MoveWithCameraRotation(CameraComponent->GetRelativeRotation(), DeltaTime));
	}

	FHitResult Hit;

	if (!bHoldingItem)
	{
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, DefaultComponentQueryParams,
			DefaultResponseParams))
		{
			// Is player aiming towards a pickup?
			if (Hit.GetActor()->GetClass()->IsChildOf(APickup::StaticClass()))
			{
				if (CurrentItem == nullptr) // needs to be nested
				{
					OnItemInSight.Broadcast();
					CurrentItem = Cast<APickup>(Hit.GetActor());
					CurrentItem->bPlacedInHand = bAttachToHand;
				}
			}
			else if (CurrentItem != nullptr)
			{
				OnItemOutOfSight.Broadcast();
				CurrentItem = nullptr;
			}
		}
		else if (CurrentItem != nullptr)
		{
			OnItemOutOfSight.Broadcast();
			CurrentItem = nullptr;
		}
	}
	else // if (bHoldingItem)
	{
		if (!bAttachToHand && GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, DefaultComponentQueryParams,
			DefaultResponseParams))
		{
			if (Hit.GetActor()->GetClass()->IsChildOf(AGenerator::StaticClass()))
			{
				return;
			}
			// Move item away from the wall/obstacle
			if (Hit.Distance < DistanceOffset)
			{
				if (CurrentDistanceX > MaxDistanceToSelf)
				{
					CurrentDistanceX -= (Hit.Distance * MoveHoldObjectSpeed * DeltaTime);// +DistanceOffset;
				}
			}
			// If player moves away from wall and a hit contact is still active
			else if (MaxDistanceToSelf >= CurrentDistanceX && CurrentDistanceX < HoldDistanceX)
			{
				CurrentDistanceX += (Hit.Distance * MoveHoldObjectSpeed * DeltaTime);// +DistanceOffset;
			}
		}
		// If player is away from wall and hit contact is not active
		else if (CurrentDistanceX < DistanceOffset)
		{
			CurrentDistanceX += PickupReach * MoveHoldObjectSpeed * DeltaTime;
		}

		// Is there something between the item and the player?
		if (GetWorld()->LineTraceSingleByChannel(Hit, TowardsPlayerStart, TowardsPlayerEnd,
			ECC_Visibility, DefaultComponentQueryParams, DefaultResponseParams))
		{
			CurrentDistanceX -= (Hit.Distance * MoveHoldObjectSpeed * 3 * DeltaTime);
			// Avoiding the possibility to drop the item when its behind/in a wall
			bItemBehindWall = true;
		}
		else
		{
			bItemBehindWall = false;
		}

		// Clamping, just to be safe
		CurrentDistanceX = FMath::Clamp(CurrentDistanceX, MaxDistanceToSelf, HoldDistanceX);
		
		// Move the component that holds the pickup
		if (!bAttachToHand)
		{
			HoldComponent->SetRelativeLocation(
				FVector(CurrentDistanceX, HoldOffsetY, CurrentAngleY));

		}
	}
	if (bInspecting)
	{
		// Rotate the item during inspection mode
		if (bHoldingItem)
		{
			CameraComponent->SetFieldOfView(FMath::Lerp(CameraComponent->FieldOfView, DefaultCameraFOV, 0.1f));
			
			HoldComponent->SetRelativeLocation(FVector(InspectDistance, HoldOffsetY, InspectingHeight));
			GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMax = 179.9000002f;
			GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMin = -179.9000002f;
			CurrentItem->RotateActor();
		}
		else
		{
			CameraComponent->SetFieldOfView(FMath::Lerp(CameraComponent->FieldOfView, ZoomInCameraFOV, 0.1f));
		}
	}
	else
	{
		CameraComponent->SetFieldOfView(FMath::Lerp(CameraComponent->FieldOfView, DefaultCameraFOV, 0.1f));

		if (bHoldingItem && CurrentItem && !bAttachToHand)
		{
			HoldComponent->SetRelativeLocation(
				FVector(CurrentDistanceX, HoldOffsetY, CurrentAngleY));
		}
	}

}

void UPickupComponent::OnAction()
{
	if (CurrentItem && !bInspecting && !bHoldOnToObject && !bItemBehindWall)
	{
		ToggleItemPickup();
	}
}

void UPickupComponent::OnInspect()
{
	if (bHoldingItem)
	{
		LastRotation = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetControlRotation();
		ToggleMovement();
	}
	else
	{
		bInspecting = true;
	}
}

void UPickupComponent::OnInspectReleased()
{
	if (bInspecting && bHoldingItem)
	{
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetControlRotation(LastRotation);
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMax = PitchMax;
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMin = PitchMin;
		ToggleMovement();
	}
	else
	{
		bInspecting = false;
	}
}

void UPickupComponent::ToggleMovement()
{
	bCanMove = !bCanMove;
	bInspecting = !bInspecting;
	CameraComponent->bUsePawnControlRotation = ~CameraComponent->bUsePawnControlRotation;
	
	if (GetWorld()->GetFirstPlayerController()->GetPawn())
	{
		GetWorld()->GetFirstPlayerController()->GetPawn()->bUseControllerRotationYaw
			= ~GetWorld()->GetFirstPlayerController()->GetPawn()->bUseControllerRotationYaw;
	}
}

FVector UPickupComponent::GetLineTracePositionStart(bool IsCameraStart)
{
	if (IsCameraStart)
	{
		return CameraComponent->GetComponentLocation() + 
			(CameraComponent->GetForwardVector() * SearchOffset);
	}
	else
	{
		return HoldComponent->GetComponentLocation() + 
			(HoldComponent->GetForwardVector() * -SearchOffset);
	}
}

FRotator UPickupComponent::MoveWithCameraRotation(FRotator CameraRotator, float DeltaTime)
{
	CurrentAngleY = CameraRotator.Pitch + HeightAngleOffset;
	CurrentAngleY = FMath::Clamp(CurrentAngleY, MIN_ANGLE_Y, MAX_ANGLE_Y);

	// X(Roll) is here Y, Y(Pitch) is X, but Z(Yaw) is still Z!
	return FRotator(CurrentAngleY, CameraRotator.Roll, CameraRotator.Yaw);
}

void UPickupComponent::OnItemInSightInternal()
{
	// gets read in blueprints for icon in crosshair to show up
}

void UPickupComponent::OnItemOutOfSightInternal()
{
	// gets read in blueprints for icon in crosshair to disappear
}

void UPickupComponent::ToggleItemPickup()
{
	if (CurrentItem)
	{
		bHoldingItem = !bHoldingItem;
		CurrentItem->Interact(bHoldingItem, ThrowAwayForce);

		if (!bHoldingItem)
		{
			CurrentItem = nullptr;
			OnItemOutOfSight.Broadcast();
		}
		else
		{
			OnItemOutOfSight.Broadcast();
		}
	}
}