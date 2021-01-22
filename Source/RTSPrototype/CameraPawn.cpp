// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraPawn.h"
#include "RtsPlayerController.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ACameraPawn::ACameraPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACameraPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MouseMovement(DeltaTime);

	Gold += GoldBuildings * (DeltaTime * GoldRate);
	Units += UnitBuildings * (DeltaTime * UnitRate);

	//GetWorld()->GetFirstPlayerController()->SetName()

	//AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting"));
}

void ACameraPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const 
{
     Super::GetLifetimeReplicatedProps(OutLifetimeProps);
     DOREPLIFETIME(ACameraPawn, Gold);
	DOREPLIFETIME(ACameraPawn, Units);
	DOREPLIFETIME(ACameraPawn, GoldBuildings);
	DOREPLIFETIME(ACameraPawn, UnitBuildings);
	DOREPLIFETIME(ACameraPawn, MyUnits);
}

// Called to bind functionality to input
void ACameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
     InputComponent->BindAxis("Zoom", this, &ACameraPawn::Zoom);
}

void ACameraPawn::Server_AddGoldBuilding_Implementation()
{
	GoldBuildings++;
	// UE_LOG(LogTemp, Warning, TEXT("Gold Buildings: %i"), GoldBuildings);
}

bool ACameraPawn::Server_AddGoldBuilding_Validate()
{
    return true;
}

void ACameraPawn::Server_AddUnitBuilding_Implementation() 
{
	UnitBuildings++;
    FString DebugName = GetDebugName(GetController());
    UE_LOG(LogTemp, Warning, TEXT("Unit Buildings: %i Owning PlayerController: %s"), UnitBuildings, *DebugName);
}

bool ACameraPawn::Server_AddUnitBuilding_Validate()
{
    return true;
}

void ACameraPawn::MouseMovement(float DeltaTime) 
{
	FVector MousePos{0,0,0};
	GetWorld()->GetGameInstance()->GetFirstLocalPlayerController()->GetMousePosition(MousePos.X, MousePos.Y);

	int32 ViewSizeX;
	int32 ViewSizeY;
	GetWorld()->GetGameInstance()->GetFirstLocalPlayerController()->GetViewportSize(ViewSizeX, ViewSizeY);

	float LocPercentX = MousePos.X / ViewSizeX;
	float LocPercentY = MousePos.Y / ViewSizeY;

	MousePos.X -= ViewSizeX / 2;
	MousePos.Y -= ViewSizeY / 2;

	float DistanceToMove = PanRate * DeltaTime;

	// UE_LOG(LogTemp, Warning, TEXT("Mouse X: %f Y: %f"), MousePos.X, MousePos.Y);

	if(LocPercentX >= .95  || LocPercentX <= .05 || LocPercentY >= .95 || LocPercentY <= .05)
	{
		MoveDirection = MousePos.GetSafeNormal(.001);
		//AddActorLocalOffset(DistanceToMove * FVector(-MoveDirection.Y, MoveDirection.X, MoveDirection.Z), true);
	}
}

void ACameraPawn::Zoom(float value)
{
	value *= 2000;
	//UE_LOG(LogTemp, Warning, TEXT("Zoom %f"), value);
	MoveDirection.Z = value;
}