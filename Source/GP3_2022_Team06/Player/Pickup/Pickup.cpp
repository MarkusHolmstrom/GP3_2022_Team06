// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "PickupComponent.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APickup::APickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MyMesh"));
	MyMesh->SetSimulatePhysics(true);
	RootComponent = MyMesh;

	//BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	//BoxCollision->SetBoxExtent(FVector(20, 20, 10));
	//BoxCollision->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;

	bHolding = false;
	bGravity = false;
	bDropped = false;
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
	if (!PlayerPawn || !PlayerCamera)
	{
		NullCheck();
	}

	TArray<USceneComponent*> Components;
	PlayerPawn->GetComponents(Components);

	if (Components.Num() > 0)
	{
		for (auto& Comp : Components) {
			if (Comp->GetName().Compare("HoldComponent") == 0)
			{
				HoldingComp = Cast<USceneComponent>(Comp);
			}
			else if (Comp->GetName().Compare("Pickup") == 0)
			{
				PickupComp = Cast<UPickupComponent>(Comp);
			}
			else if (Comp->GetName().Compare(MeshCompName) == 0)
			{
				MeshComp = Cast<USceneComponent>(Comp);
			}
		}
	}
}

// Called every frame
void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bHolding && HoldingComp)
	{
		SetActorLocationAndRotation(HoldingComp->GetComponentLocation(), HoldingComp->GetComponentRotation());
	}
}

void APickup::RotateActor()
{
	ControlRotation = GetWorld()->GetFirstPlayerController()->GetControlRotation();
	SetActorRotation(FQuat(ControlRotation));
}

// Drop or pick up object
void APickup::Interact(bool bPickingUp, int DropForce)
{
	bHolding = bPickingUp;
	bGravity = !bPickingUp;
	if (MyMesh)
	{
		MyMesh->SetEnableGravity(bGravity);
		MyMesh->SetSimulatePhysics(bHolding ? false : true);
		MyMesh->SetCollisionEnabled(bHolding ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics); 
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.f, FColor::Red, FString::Printf(TEXT(
			"Error: Something is wrong, (MyMesh not found in this pick up, yell at Markus now!) can you hear me, Major Tom? x3")));

		return;
	}

	if (!bPlacedInHand && HoldingComp && bHolding)
	{
		MyMesh->AttachToComponent(HoldingComp, FAttachmentTransformRules::KeepRelativeTransform);

		SetActorRelativeLocation(FVector::ZeroVector);
	}

	if (bPlacedInHand && bHolding)
	{
		if (!PlayerPawn || !PlayerCamera)
		{
			NullCheck();
		}

		if (MeshComp)
		{
			AttachToPlayerMesh();
		}
	}

	if (!bHolding) 
	{
		if (MyMesh->IsAttachedTo(HoldingComp))
		{
			MyMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		}

		if (!PlayerPawn || !PlayerCamera)
		{
			NullCheck();
		}
		if (!PlayerCamera)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 50.f, FColor::Red, FString::Printf(TEXT(
				"Error: Something is wrong, (camera not found, not even after null check, what the fucking duces is going on here? forget about dropping this item, you need to find a camera, get your priorites straight!) can you hear me, Major Tom? x3")));
		}
		else
		{
			ForwardVector = PlayerCamera->GetForwardVector();
		}

		if (MyMesh->IsSimulatingPhysics())
		{
			MyMesh->SetPhysicsLinearVelocity(PlayerPawn->GetActorForwardVector() * DropForce);		// <- This doesn't crash (could be solved in other ways probably...)
		}
	}
}

void APickup::AttachToPlayerMesh()
{
	MyMesh->AttachToComponent(MeshComp, 
		FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
	
}

void APickup::NullCheck()
{
	PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	PlayerCamera = PlayerPawn->FindComponentByClass<UCameraComponent>();
}

// Safe plays so the player's and the pickup's physics dont mess with each other 
// when player stands on top on the pickup and then chooses to pick it up

/*void APickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bHolding && OtherActor == PlayerPawn)
	{
		MyMesh->SetSimulatePhysics(false);
		MyMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	}

}

void APickup::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!bHolding && OtherActor == PlayerPawn)
	{
		MyMesh->SetSimulatePhysics(true);
		MyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	}
}*/


