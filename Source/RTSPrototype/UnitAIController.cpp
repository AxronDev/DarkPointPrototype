// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitAIController.h"
#include "Net/UnrealNetwork.h"
#include "RTSPrototypeCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"


AUnitAIController::AUnitAIController() 
{
     bReplicates = true;

     AIPercep = CreateDefaultSubobject<UAIPerceptionComponent>("SightPerception");
     SightSenseConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("Sight Config");

     // Setup SightSenseConfig
     SightSenseConfig->SightRadius = 1800.f;
     SightSenseConfig->LoseSightRadius = 3000.f;
     SightSenseConfig->PeripheralVisionAngleDegrees = 360.f;
     SightSenseConfig->DetectionByAffiliation.bDetectEnemies = true;
     SightSenseConfig->DetectionByAffiliation.bDetectNeutrals = true;
     SightSenseConfig->DetectionByAffiliation.bDetectFriendlies = true;

     AIPercep->ConfigureSense(*SightSenseConfig);
     AIPercep->SetDominantSense(SightSenseConfig->GetSenseImplementation());
     // AIPercep->OnPercepetionUpdated.AddDynamic(this, AUnitAIController::)
}

void AUnitAIController::BeginPlay() 
{
     
     PrimaryActorTick.bCanEverTick = true;

     Server_GetAICharacter();

     AIPercep->SetSenseEnabled(SightSenseClass, false);
}

void AUnitAIController::Server_GetAICharacter_Implementation() 
{
     Char = GetPawn<ARTSPrototypeCharacter>();
}

bool AUnitAIController::Server_GetAICharacter_Validate() 
{
     return true;
}

void AUnitAIController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const 
{

     Super::GetLifetimeReplicatedProps(OutLifetimeProps);
     DOREPLIFETIME(AUnitAIController, Char);
}

void AUnitAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult & Result) 
{
     Super::OnMoveCompleted(RequestID, Result);
     /* const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("E"), true);
     FString EnumName = EnumPtr->GetDisplayNameText((uint8)RTSPlayerState).ToString(); */

     UE_LOG(LogTemp, Warning, TEXT("PathFollowingResult: %s"), *Result.ToString());
}

void AUnitAIController::Tick(float DeltaSeconds)
{
     Super::Tick(DeltaSeconds);

     if(Char->GetCharacterState() == ECharacterState::Aggressive)
     {
          AIPercep->SetSenseEnabled(SightSenseClass, true);
     }
}