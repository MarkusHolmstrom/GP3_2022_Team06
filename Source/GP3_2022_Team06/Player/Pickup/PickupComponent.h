// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Pawn.h"
#include "Pickup.h"
#include "Camera/CameraComponent.h"
#include "PickupComponent.generated.h"

class USceneComponent;
class UBoxComponent;
class APickup;
class UGlitchComponent;
class AGenerator;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FItemSightDelegate);

UCLASS( ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GP3_2022_TEAM06_API UPickupComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPickupComponent();

	UFUNCTION(BlueprintCallable)
	void ToggleItemPickup();

	UFUNCTION(BlueprintCallable)
	void OnAction();
	// End public functions

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FItemSightDelegate OnItemInSight;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FItemSightDelegate OnItemOutOfSight;

	// Properties
	UPROPERTY(VisibleAnywhere)
	bool bPickupInRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHoldOnToObject;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = Holding,
		meta = (ToolTip = "Centered component, for rotation only"))
	USceneComponent* CenterComponent;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = Holding,
		meta = (ToolTip = "Should be found in instance, else generates an error"))
	USceneComponent* HoldComponent;

	UPROPERTY(EditAnywhere, Category = Debugging, meta = (ToolTip = 
		"True = item moves with camera, false = item doesnt!"))
	bool bMoveWithCamera = false;

	UPROPERTY(EditAnywhere, Category = Debugging, meta = (ToolTip = 
		"True = item stays in player's hand, false = item floats in air!"))
	bool bAttachToHand = false;

	UPROPERTY(EditAnywhere, Category = Holding, meta = (ToolTip = 
		"Default holding distance from player, X"))
	float HoldDistanceX = 110.0f;

	UPROPERTY(EditAnywhere, Category = Holding, meta = (ToolTip = 
		"Default holding offset from player, neg Y = left of player, pos Y = right of player"))
	float HoldOffsetY = 40.0f;

	UPROPERTY(EditAnywhere, Category = Holding, meta = (ToolTip = 
		"Default holding height from the player's waist position, Z"))
	float HeightAngleOffset = 35.0f;

	UPROPERTY(EditAnywhere, Category = Inspecting, meta = (ToolTip = 
		"Default holding distance for inspection, X, should be shorter than HoldDistance"))
	float InspectDistance = 90.0f;

	UPROPERTY(EditAnywhere, Category = Inspecting, meta = (ToolTip = 
		"Default holding distance for inspection, X, should be shorter than HoldDistance"))
	float InspectingHeight = 30.0f;

	UPROPERTY(EditAnywhere, Category = Holding, meta = (ToolTip = 
		"Speed hold item travels when close to obstacles"))
	float MoveHoldObjectSpeed = 1.0f;

	UPROPERTY(EditAnywhere, Category = Holding, meta = (ToolTip = 
		"Default closest distance item can have from player, X"))
	float MaxDistanceToSelf = 75.0f;

	UPROPERTY(VisibleAnywhere, Category = Holding)
	float CurrentDistanceX;

	UPROPERTY(EditAnywhere, Category = Holding, meta = (ToolTip = 
		"Offset from walls, so its doesnt goes trough them, X, should be the same value as holddistance"))
	float DistanceOffset = 110.0f;

	UPROPERTY(EditAnywhere, Category = Holding)
	int ThrowAwayForce = 100;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;


	void OnInspect();
	void OnInspectReleased();

	void ToggleMovement();

	FVector GetLineTracePositionStart(bool IsCameraStart);

	FRotator MoveWithCameraRotation(FRotator CameraRotator, float DeltaTime);

	// Internal Events
	virtual void OnItemInSightInternal();

	virtual void OnItemOutOfSightInternal();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	APickup* CurrentItem;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Should be found in instance, else generates an error"))
	UInputComponent* InputComp;

	bool bCanMove;
	UPROPERTY(BlueprintReadOnly)
	bool bHoldingItem;
	bool bInspecting;
	UPROPERTY(VisibleAnywhere)
	bool bItemBehindWall;

	float PitchMax;
	float PitchMin;

	FVector HoldingComp;
	FRotator LastRotation;

	FVector Start;
	FVector ForwardVector;
	FVector End;

	FComponentQueryParams DefaultComponentQueryParams;
	FCollisionResponseParams DefaultResponseParams;

	UPROPERTY(EditAnywhere)
	UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, Category = Inspecting)
	float DefaultCameraFOV = 90.0f;
	UPROPERTY(EditAnywhere, Category = Inspecting)
	float ZoomInCameraFOV = 45.0f;

	UPROPERTY(EditAnywhere, Category = Holding)
	float SearchOffset = 5.0f;

	UPROPERTY(EditAnywhere, Category = Holding)
	float PickupReach = 400.0f;

	UPROPERTY(EditAnywhere, Category = CameraReading)
	float MAX_ANGLE_Y = 65;

	UPROPERTY(EditAnywhere, Category = CameraReading)
	float MIN_ANGLE_Y = -50;

	UPROPERTY(EditAnywhere, Category = CameraReading)
	float CurrentAngleY = 35;
};
