// Fill out your copyright notice in the Description page of Project Settings.


#include "Building.h"
#include "Components/DecalComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ABuilding::ABuilding()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);

	// Setup stimuli for sight
	Stimuli = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("Stimuli");

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
	CursorToWorld->SetVisibility(true);
}

// Called every frame
void ABuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABuilding::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const 
{
     Super::GetLifetimeReplicatedProps(OutLifetimeProps);
     DOREPLIFETIME(ABuilding, OwnerUserName);
	DOREPLIFETIME(ABuilding, bHasBeenPositioned);
}

float ABuilding::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) 
{
	float DamageDealt = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	Health -= DamageDealt;
	if(Health <= 0)
	{
		DamageDealt = DamageDealt - Health;
		Server_Death();
	}
	return DamageDealt;
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

float ABuilding::GetHealth() 
{
	return Health;
}

void ABuilding::Server_Death_Implementation() 
{
	Destroy();
}

bool ABuilding::Server_Death_Validate()
{
	return true;
}

void ABuilding::CanPlace_Implementation(bool val) 
{
	
}

