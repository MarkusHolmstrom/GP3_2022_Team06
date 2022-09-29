// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "Pickup.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;
class UPickupComponent;

UCLASS()
class GP3_2022_TEAM06_API APickup : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MyMesh;

	UPROPERTY(VisibleAnywhere, Category = Holding)
	USceneComponent* HoldingComp;

	/*UPROPERTY(VisibleAnywhere, Category = Holding)
	UBoxComponent* BoxCollision;*/

	UPROPERTY(VisibleAnywhere, Category = Holding)
	bool bHolding;

	//UPROPERTY(EditAnywhere, meta = (ToolTip = 
	//	"Only gets called when this key is in correct generator"))
	//bool bFoundItsPurposeInLife = false;

	UPROPERTY(EditAnywhere, meta = (ToolTip = 
		"Control bool that will be set by Pickup component, default is false"))
	bool bPlacedInHand = false;

	UPROPERTY(VisibleAnywhere)
	bool bGravity;
	UPROPERTY(VisibleAnywhere)
	bool bDropped;
	UPROPERTY(EditAnywhere)
	UPickupComponent* PickupComp;

	UPROPERTY(EditAnywhere)
	USceneComponent* MeshComp;
	UPROPERTY(EditAnywhere)
	FString MeshCompName = "Mesh1P";

	UPROPERTY(EditAnywhere)
	FName SocketName = "hand_lSocket";

	FRotator ControlRotation;
	APawn* PlayerPawn;
	UCameraComponent* PlayerCamera;
	FVector ForwardVector;
	// End variables

	// Sets default values for this actor's properties
	APickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void RotateActor();

	UFUNCTION()
	void Interact(bool bPickingUp, int ThrowAwayForce);

	UFUNCTION()
	void AttachToPlayerMesh();

	UFUNCTION()
	void NullCheck();

};
