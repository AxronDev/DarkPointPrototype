// Fill out your copyright notice in the Description page of Project Settings.


#include "Building.h"
#include "Components/DecalComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"

#define NETMODE_WORLD (((GEngine == nullptr) || (GetWorld() == nullptr)) ? TEXT("") \
: (GEngine->GetNetMode(GetWorld()) == NM_Client) ? TEXT("[Client] ") \
: (GEngine->GetNetMode(GetWorld()) == NM_ListenServer) ? TEXT("[ListenServer] ") \
: (GEngine->GetNetMode(GetWorld()) == NM_DedicatedServer) ? TEXT("[DedicatedServer] ") \
: TEXT("[Standalone] "))

// Sets default values
ABuilding::ABuilding()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);
	SetReplicates(true);

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

	// Get materials
	static ConstructorHelpers::FObjectFinder<UMaterial> GoldBuildingMaterial(TEXT("Material'/Game/StarterContent/Materials/M_Concrete_Tiles.M_Concrete_Tiles'"));
	static ConstructorHelpers::FObjectFinder<UMaterial> UnitBuildingMaterial(TEXT("Material'/Game/StarterContent/Materials/M_Brick_Clay_Beveled.M_Brick_Clay_Beveled'"));
	
	if (GoldBuildingMaterial.Succeeded() && UnitBuildingMaterial.Succeeded())
	{
		TempGold = GoldBuildingMaterial.Object;
		TempUnit = UnitBuildingMaterial.Object;
		UE_LOG(LogTemp, Warning, TEXT("Materials found in constructor 1"));
	}

	// Initialize attack slots array to false with size of 30
	AttackSlots.Init(false, 30);
}

void ABuilding::OnConstruction(const FTransform& Transform) 
{
	Super::OnConstruction(Transform);

	// Create Dynamic Instances
	if (TempGold && TempUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Materials good in OnConstruction 2"));
		UE_LOG(LogTemp, Warning, TEXT("OnConstruction Owner: %s on %s"), *GetDebugName(GetOwner()), NETMODE_WORLD);
		UMaterialInstanceDynamic* GoldMat = UMaterialInstanceDynamic::Create(TempGold, this);
		UMaterialInstanceDynamic* UnitMat = UMaterialInstanceDynamic::Create(TempUnit, this);
		GoldMat->SetScalarParameterValue(FName("Transparency"), .5f);
		UnitMat->SetScalarParameterValue(FName("Transparency"), .5f);

		GoldBuildingMat = GoldMat;
		UnitBuildingMat = UnitMat;
		Server_SetBuildingMaterials(GoldMat, UnitMat);
	}
}

// Called when the game starts or when spawned
void ABuilding::BeginPlay()
{
	Super::BeginPlay();
	CursorToWorld->SetVisibility(true);
}

void ABuilding::OnRep_Owner() 
{
	UE_LOG(LogTemp, Warning, TEXT("OnRep Owner %s on %s"), *GetDebugName(GetOwner()), NETMODE_WORLD);
	
	/* if (TempGold && TempUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Materials good in OnRep"));
		UMaterialInstanceDynamic* GoldMat = UMaterialInstanceDynamic::Create(TempGold, this);
		UMaterialInstanceDynamic* UnitMat = UMaterialInstanceDynamic::Create(TempUnit, this);
		GoldMat->SetScalarParameterValue(FName("Transparency"), .5f);
		UnitMat->SetScalarParameterValue(FName("Transparency"), .5f);

		GoldBuildingMat = GoldMat;
		UnitBuildingMat = UnitMat;
		Server_SetBuildingMaterials(GoldMat, UnitMat);
	} */
}

// Called every frame
void ABuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(GetOwner())
	{
		// UE_LOG(LogTemp, Warning, TEXT("Tick %s on %s"), *GetDebugName(GetOwner()), NETMODE_WORLD);
	}

}

void ABuilding::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
     DOREPLIFETIME(ABuilding, Health);
	DOREPLIFETIME(ABuilding, bHasBeenPositioned);
	DOREPLIFETIME(ABuilding, OwnerUserName);
	DOREPLIFETIME(ABuilding, BuildingState);
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

void ABuilding::Server_SetBuildingMaterials_Implementation(UMaterialInstanceDynamic* Gold, UMaterialInstanceDynamic* Unit) 
{
	// Create Dynamic Instances
	if (Gold && Unit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Materials good in SetBuildingMaterials 3 %s"), NETMODE_WORLD);
		GoldBuildingMat = Gold;
		UnitBuildingMat = Unit;
	}
}

bool ABuilding::Server_SetBuildingMaterials_Validate(UMaterialInstanceDynamic* Gold, UMaterialInstanceDynamic* Unit) 
{
	return true;
}

void ABuilding::Server_SetBuildingState_Implementation(EBuildingState NewState) 
{	
	if(!GoldBuildingMat || !UnitBuildingMat)
	{
		if(!UnitBuildingMat)
		{
			UE_LOG(LogTemp, Warning, TEXT("Unit building mat is NULL %s"), NETMODE_WORLD);
		}
		if(!GoldBuildingMat)
		{
			UE_LOG(LogTemp, Warning, TEXT("Gold building mat is NULL %s"), NETMODE_WORLD);
		}
		return;
	}
	
	BuildingState = NewState;

	if(GetBuildingType() == FName("Gold Building"))
	{
		UE_LOG(LogTemp, Warning, TEXT("Change gold building state %s"), NETMODE_WORLD);
		switch(NewState)
		{
			case EBuildingState::Preview :
				// GoldBuildingMat->SetScalarParameterValue(FName("Transparency"), 0.5f);
				break;
			case EBuildingState::Built :
				GoldBuildingMat->SetScalarParameterValue(FName("Transparency"), 1.0f);
				break;
			case EBuildingState::Destroyed :
				break;
		}
	}

	else if(GetBuildingType() == FName("Unit Building"))
	{
		UE_LOG(LogTemp, Warning, TEXT("Change unit building state %s"), NETMODE_WORLD);
		switch(NewState)
		{
			case EBuildingState::Preview :
				// UnitBuildingMat->SetScalarParameterValue(FName("Transparency"), 0.5f);
				break;
			case EBuildingState::Built :
				UnitBuildingMat->SetScalarParameterValue(FName("Transparency"), 1.f);
				break;
			case EBuildingState::Destroyed :
				break;
		}
	}
}

bool ABuilding::Server_SetBuildingState_Validate(EBuildingState NewState) 
{
	return true;
}

EBuildingState ABuilding::GetBuildingState() 
{
	return BuildingState;
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

