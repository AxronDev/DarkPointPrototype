// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Building.generated.h"

class UDecalComponent;
class USceneComponent;
class UMaterialInstanceDynamic;
class UAIPerceptionStimuliSourceComponent;
class UAISense;

UENUM()
enum class EBuildingState : uint8
{
	Preview,
	Built,
	Destroyed,
};

UCLASS()
class RTSPROTOTYPE_API ABuilding : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ABuilding();

	virtual void OnConstruction(const FTransform& Transform);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite)
	FName BuildingType;

	UMaterial* TempGold;
	UMaterial* TempUnit;

	virtual void OnRep_Owner() override;

	/* UMaterialInstanceDynamic* GoldMat;
	UMaterialInstanceDynamic* UnitMat; */

	UPROPERTY(BlueprintReadOnly, replicated)
	UMaterialInstanceDynamic* GoldBuildingMat;

	UPROPERTY(BlueprintReadOnly, replicated)
	UMaterialInstanceDynamic* UnitBuildingMat;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxHealth = 500;

	UPROPERTY(EditDefaultsOnly, replicated)
	float Health = MaxHealth;

	UPROPERTY(EditDefaultsOnly)
	UAIPerceptionStimuliSourceComponent* Stimuli;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAISense> SightSense;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) override;

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Server_SetBuildingMaterials(UMaterialInstanceDynamic* Gold, UMaterialInstanceDynamic* Unit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDecalComponent* CursorToWorld;

	UPROPERTY(EditDefaultsOnly)
	USceneComponent* Root;

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Server_SetBuildingState(EBuildingState NewState);

	EBuildingState GetBuildingState();

	FName GetBuildingType();

	void SetOwnerUserName(FName UserName);

	UFUNCTION(BlueprintCallable)
	FName GetOwnerUserName();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<bool> AttackSlots;

	UPROPERTY(replicated)
	bool bHasBeenPositioned = false;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Decal")
	void CanPlace(bool val);

	UPROPERTY(BlueprintReadWrite)
	bool bCanPlace = false;

	UFUNCTION(BlueprintCallable)
	float GetHealth();

private:
	UPROPERTY(VisibleAnywhere, replicated)
	EBuildingState BuildingState = EBuildingState::Built;

	UPROPERTY(replicated)
	FName OwnerUserName = "";

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Death();

};
