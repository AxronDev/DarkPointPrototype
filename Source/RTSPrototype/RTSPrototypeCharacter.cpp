// Copyright Epic Games, Inc. All Rights Reserved.

#include "RTSPrototypeCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UnitAIController.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "Engine/World.h"

ARTSPrototypeCharacter::ARTSPrototypeCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Set Replication
	bReplicates = true;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create Box Collision to check if the character can spawn
	SpawnSpace = CreateDefaultSubobject<UBoxComponent>("SpawnSpace");
	SpawnSpace->SetupAttachment(RootComponent);

	// Create a decal in the world to show the cursor's location
	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/TopDownCPP/Blueprints/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}
	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Initialize attack slots array to false with size of 30
	AttackSlots.Init(false, 30);
}

void ARTSPrototypeCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const 
{
     Super::GetLifetimeReplicatedProps(OutLifetimeProps);
     DOREPLIFETIME(ARTSPrototypeCharacter, OwnerUserName);
	DOREPLIFETIME(ARTSPrototypeCharacter, UnitController);
     DOREPLIFETIME(ARTSPrototypeCharacter, CharacterState);
	DOREPLIFETIME(ARTSPrototypeCharacter, Health);
}

void ARTSPrototypeCharacter::BeginPlay()
{
	Super::BeginPlay();
	Server_GetUnitController();
	CursorToWorld->SetVisibility(false);
	if(UnitController)
	{
		UnitController->Server_GetAICharacter();
	}
	SpawnSpace->OnComponentBeginOverlap.AddDynamic(this, &ARTSPrototypeCharacter::UpdateToOverlap);
	SpawnSpace->OnComponentEndOverlap.AddDynamic(this, &ARTSPrototypeCharacter::UpdateNotOverlap);
}

float ARTSPrototypeCharacter::GetHealth() 
{
	return Health;
}

void ARTSPrototypeCharacter::Attack(AActor* Target) 
{
	UGameplayStatics::ApplyDamage(Target, AttackDamage, Cast<AController>(UnitController), this, DamageTypeClass);
}

void ARTSPrototypeCharacter::SetOwnerUserName(FName UserName) 
{
	OwnerUserName = UserName;
}

FName ARTSPrototypeCharacter::GetOwnerUserName() 
{
	return OwnerUserName;
}

void ARTSPrototypeCharacter::Server_ChangeCharacterState_Implementation(ECharacterState NewState) 
{
	CharacterState = NewState;
}

bool ARTSPrototypeCharacter::Server_ChangeCharacterState_Validate(ECharacterState NewState) 
{
	return true;
}

ECharacterState ARTSPrototypeCharacter::GetCharacterState() 
{
	return CharacterState;
}

void ARTSPrototypeCharacter::Server_GetUnitController_Implementation() 
{
	if(Cast<AUnitAIController>(GetController()))
	{
		UnitController = Cast<AUnitAIController>(GetController());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Get UnitAIController Failed in RTSPrototypeCharacter"));
	}
}

bool ARTSPrototypeCharacter::Server_GetUnitController_Validate()
{
	return true;
}

void ARTSPrototypeCharacter::UpdateToOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) 
{
	bHasSpace = false;
}

void ARTSPrototypeCharacter::UpdateNotOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) 
{
	bHasSpace = true;
}

float ARTSPrototypeCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) 
{
	float DamageDealt = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	Health -= DamageDealt;
	return DamageDealt;
}

void ARTSPrototypeCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	if (CursorToWorld != nullptr)
	{
		if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
		{
			if (UWorld* World = GetWorld())
			{
				FHitResult HitResult;
				FCollisionQueryParams Params(NAME_None, FCollisionQueryParams::GetUnknownStatId());
				FVector StartLocation = TopDownCameraComponent->GetComponentLocation();
				FVector EndLocation = TopDownCameraComponent->GetComponentRotation().Vector() * 2000.0f;
				Params.AddIgnoredActor(this);
				World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, Params);
				FQuat SurfaceRotation = HitResult.ImpactNormal.ToOrientationRotator().Quaternion();
				CursorToWorld->SetWorldLocationAndRotation(HitResult.Location, SurfaceRotation);
			}
		}
		else if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			
			FHitResult TraceHitResult;
			PC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();
			CursorToWorld->SetWorldLocation(TraceHitResult.Location);
			CursorToWorld->SetWorldRotation(CursorR);
		}
	}
}
