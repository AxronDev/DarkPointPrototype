// Fill out your copyright notice in the Description page of Project Settings.


#include "Building.h"
#include "Components/DecalComponent.h"

// Sets default values
ABuilding::ABuilding()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/TopDownCPP/Blueprints/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}
	CursorToWorld->DecalSize = FVector(300.0f, 300.0f, 300.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());

	// Initialize attack slots array to false with size of 30
	AttackSlots.Init(false, 30);
}

// Called when the game starts or when spawned
void ABuilding::BeginPlay()
{
	Super::BeginPlay();
	CursorToWorld->SetVisibility(false);
}

// Called every frame
void ABuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FName ABuilding::GetBuildingType() 
{
	return BuildingType;
}

void ABuilding::SetOwnerUserName(FName UserName) 
{
	OwnerUserName = UserName;
}

FName ABuilding::GetOwnerUserName() 
{
	return OwnerUserName;
}

