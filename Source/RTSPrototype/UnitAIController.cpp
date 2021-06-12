// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitAIController.h"
#include "Net/UnrealNetwork.h"
#include "RTSPrototypeCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "RTSPrototypeCharacter.h"
#include "RtsPlayerController.h"
#include "Building.h"
#include "TimerManager.h"
#include "Blueprint/AIBlueprintHelperLibrary.h" 
#include "Engine/EngineTypes.h"
#include "Perception/AISenseConfig_Sight.h"

AUnitAIController::AUnitAIController() 
{
     bReplicates = true;

     AIPercep = CreateDefaultSubobject<UAIPerceptionComponent>("SightPerception");
     SightSenseConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("Sight Config");

     // Setup SightSenseConfig
     SightSenseConfig->SightRadius = 800.f;
     SightSenseConfig->LoseSightRadius = 1000.f;
     SightSenseConfig->PeripheralVisionAngleDegrees = 360.f;
     SightSenseConfig->DetectionByAffiliation.bDetectEnemies = true;
     SightSenseConfig->DetectionByAffiliation.bDetectNeutrals = true;
     SightSenseConfig->DetectionByAffiliation.bDetectFriendlies = true;
     SightSenseConfig->SetMaxAge(MaxAge);

     AIPercep->ConfigureSense(*SightSenseConfig);
     AIPercep->SetDominantSense(SightSenseConfig->GetSenseImplementation());
     AIPercep->OnPerceptionUpdated.AddDynamic(this, &AUnitAIController::SortEnemyObjects);
}

void AUnitAIController::BeginPlay() 
{
     Super::BeginPlay();
     PrimaryActorTick.bCanEverTick = true;

     AIPercep->SetSenseEnabled(SightSenseClass, true);
}

void AUnitAIController::NextMoveQueue(bool Aggro) 
{
     UE_LOG(LogTemp, Warning, TEXT("NextMoveQueue() Called"));
     if(QueuedMovements.Num() <= 0)
     {
          UE_LOG(LogTemp, Warning, TEXT("Queue Movements Empty"));
          return;
     }
     if(QueueNum == (QueuedMovements.Num() - 1))
     {
          if(bPatroling)
          {
               UE_LOG(LogTemp, Warning, TEXT("Queue Finished Cycling as Patrol"));
               // restart patrol loop
               QueueNum = 0;
          }
          else
          {
               UE_LOG(LogTemp, Warning, TEXT("Queue Finished and will reset as not patrol"));
               Server_ResetMoveQueue();
               return;
          }
          
     }
     else
     {
          QueueNum++;
     }

     if(Aggro == true)
     {
          FHitResult MoveToHit = QueuedMovements[QueueNum];
          SetHit(MoveToHit, true);
     }
     else
     {
          FHitResult MoveToHit = QueuedMovements[QueueNum];
          SetHit(MoveToHit, false);
     }
}

void AUnitAIController::Server_MoveTo_Implementation(FHitResult Hit, bool Aggressive) 
{
     if(Aggressive == true)
     {
          Char->Server_ChangeCharacterState(ECharacterState::Aggressive);
          UE_LOG(LogTemp, Warning, TEXT("Attack"));
     }
     else 
     {
          Char->Server_ChangeCharacterState(ECharacterState::Passive);
          UE_LOG(LogTemp, Warning, TEXT("Passive"));
     }
     UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Hit.Location);
     UE_LOG(LogTemp, Warning, TEXT("SimpleMoveToLocation called in UnitAIController Server MoveTo"));
}

bool AUnitAIController::Server_MoveTo_Validate(FHitResult Hit, bool Aggressive) 
{
     return true;
}

bool AUnitAIController::WithinRange() 
{
     float Range = Char->AttackDist + Char->GetRadius() + Target->GetRadius();
     float DistBetween = Cast<AActor>(Target)->GetDistanceTo(Char);
     if(DistBetween <= Range && DistBetween != -2.f)
     {
          UE_LOG(LogTemp, Warning, TEXT("Within Range DistBetween %f, Range %f"), DistBetween, Range);
          return true;
     }
     else
     {
          UE_LOG(LogTemp, Warning, TEXT("NOT Within DistBetween %f, Range %f"), DistBetween, Range);
          return false;
     }
}

void AUnitAIController::CanAttack() 
{
    Char->bCanAttack = true;
}

void AUnitAIController::AssignTarget() 
{
     bool bFindNewTarget = false;
     if(Target == nullptr)
     {
          bFindNewTarget = true;
     }

     else if(EnemyUnits.Find(Target) == INDEX_NONE || Target->GetPlaceableState() == EPlaceableState::Destroyed)
     {
          bFindNewTarget = true;
     }

     if(bFindNewTarget == true && Char)
     {
          Target = nullptr;
          for(uint8 Slot = 0; Slot < (Char->GetAttackSlots()).Num(); Slot++)
          {
               for(uint8 UnitIndex = 0; UnitIndex < EnemyUnits.Num(); UnitIndex++)
               {
                    UE_LOG(LogTemp, Warning, TEXT("Enemy units array size %i"), EnemyUnits.Num());
                    if(EnemyUnits[UnitIndex] != nullptr)
                    {
                         UE_LOG(LogTemp, Warning, TEXT("Bruh"));
                         // Check if attack slot is empty
                         if((EnemyUnits[UnitIndex]->GetAttackSlots())[Slot] == false)
                         {
                              if(EnemyUnits[UnitIndex])
                              {
                                   // Set slot to taken and assign Target
                                   (EnemyUnits[UnitIndex]->GetAttackSlots())[Slot] = true;
                                   Target = EnemyUnits[UnitIndex];
                                   UE_LOG(LogTemp, Warning, TEXT("Cast Succeded AssignTarget"));
                                   // End looking for target
                                   Slot = (Char->GetAttackSlots()).Num();
                                   UnitIndex = EnemyUnits.Num();
                              }
                              else
                              {
                                   UE_LOG(LogTemp, Warning, TEXT("Cast Failed AssignTarget"));
                              }
                         }
                         /* FString UnitName = GetDebugName(units);
                         FString Auth;    
                         UE_LOG(LogTemp, Warning, TEXT("Can see unit controller %s"), *UnitName); */
                    }
               }
          }
     }
}

void AUnitAIController::SortEnemyObjects(const TArray<AActor*>& Actors) 
{
     if(!Char)
     {
          UE_LOG(LogTemp, Warning, TEXT("Char null in UnitAIController"));
          return;
     }
     if(Char->GetCharacterState() == ECharacterState::Dead)
     {
          return;
     }
     // UE_LOG(LogTemp, Warning, TEXT("Perception Updated"));
     for(AActor* Actor : Actors)
     {
          // Check if Placeable type
          if(Cast<IPlaceable>(Actor) && Char)
          {
               IPlaceable* Unit = Cast<IPlaceable>(Actor);
               // Check if enemy unit
               if(Unit->GetOwnerUserName() != Char->GetOwnerUserName())
               {
                    FActorPerceptionBlueprintInfo Info;
                    AIPercep->GetActorsPerception(Actor, Info);
                    if(Info.LastSensedStimuli.Num() == 0)
                    break;
                    FAIStimulus Stim = Info.LastSensedStimuli[0];
                    // Check if should be targeted
                    if(Stim.WasSuccessfullySensed() == false || Stim.GetAge() >= MaxAge || Unit->GetPlaceableState() == EPlaceableState::Destroyed)
                    {
                         // Check if in Enemy units and remove if it is
                         if(EnemyUnits.Find(Unit) != INDEX_NONE)
                         {
                              EnemyUnits.Remove(Unit);
                         }
                    }
                    else
                    {
                         FString UnitName = GetDebugName(Actor);   
                         UE_LOG(LogTemp, Warning, TEXT("Can see unit controller %s"), *UnitName);
                         // Check if in Enemy units and add if it isn't
                         if(EnemyUnits.Find(Unit) == INDEX_NONE)
                         {
                              EnemyUnits.Add(Unit);
                         }
                    }
               }
               /* // Check if enemy building
               if(Cast<ABuilding>(Unit))
               {
                    return;
               } */
          }
     }

     if(EnemyUnits.Num() != 0)
     {
          AssignTarget();
     }
}

void AUnitAIController::Server_GetAICharacter_Implementation() 
{
     Char = GetPawn<ARTSPrototypeCharacter>();
     if(Char)
     {
          // UE_LOG(LogTemp, Warning, TEXT("Got Char in Server_GetAICharacter"));
     }
     else
     {
          UE_LOG(LogTemp, Warning, TEXT("Failed to get Char in Server_GetAICharacter"));
     }
     
}

bool AUnitAIController::Server_GetAICharacter_Validate() 
{
     return true;
}

void AUnitAIController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const 
{
     Super::GetLifetimeReplicatedProps(OutLifetimeProps);
     DOREPLIFETIME(AUnitAIController, Char);
     DOREPLIFETIME(AUnitAIController, QueuedMovements);
     DOREPLIFETIME(AUnitAIController, bPatroling);
}

void AUnitAIController::SetPatroling(bool NewPatrol) 
{
    bPatroling = NewPatrol;
}

void AUnitAIController::PreResetQueue() 
{
    Server_ResetMoveQueue();
}

void AUnitAIController::Server_ResetMoveQueue_Implementation() 
{
     UE_LOG(LogTemp, Warning, TEXT("ResetMoveQueue()"));
     QueueNum = 0;
     QueuedMovements.Empty();
     Char->OwningPlayer->QueuedMovements.Empty();
}

bool AUnitAIController::Server_ResetMoveQueue_Validate()
{
     return true;
}

void AUnitAIController::SetHit(FHitResult& InHit, bool Aggressive) 
{
     UE_LOG(LogTemp, Warning, TEXT("Set hit to %f, %f, %f"), InHit.Location.X, InHit.Location.Y, InHit.Location.Z);
     HitLoc = InHit;
     Server_MoveTo(InHit, Aggressive);
}

void AUnitAIController::SetTarget(IPlaceable* NewTarget) 
{
    Target = NewTarget;
    EnemyUnits.Add(NewTarget);
}

void AUnitAIController::Tick(float DeltaSeconds)
{
     Super::Tick(DeltaSeconds);
     if(Char->GetCharacterState() == ECharacterState::Dead) return;
     if(Char)
     {
          if(Char->GetCharacterState() == ECharacterState::Aggressive)
          {
               if(!Target)
               {
                    TArray<AActor*> Actors{};
                    for(uint8 i = 0; i < Actors.Num(); i++)
                    {
                         Actors[i] = Cast<AActor>(EnemyUnits[i]);
                    }
                    SortEnemyObjects(Actors);
               }
               else if(EnemyUnits.Find(Target) == INDEX_NONE || Target->GetPlaceableState() == EPlaceableState::Destroyed)
               {
                    TArray<AActor*> Actors{};
                    for(uint8 i = 0; i < Actors.Num(); i++)
                    {
                         Actors[i] = Cast<AActor>(EnemyUnits[i]);
                    }
                    SortEnemyObjects(Actors);
               }
               else
               {
                    float AcceptanceRadius = /* Char->GetRadius() + */ Target->GetRadius() + Char->DistToTarget + Char->AttackDist;
                    UE_LOG(LogTemp, Warning, TEXT("MoveTo Acceptance Radius %f"), AcceptanceRadius + Char->GetRadius());
                    switch(Target->GetPlaceableState())
                    {
                         case EPlaceableState::Destroyed:
                              UE_LOG(LogTemp, Warning, TEXT("Target Destroyed"));
                              break;
                         case EPlaceableState::Preview:
                              UE_LOG(LogTemp, Warning, TEXT("Target Preview"));
                              break;
                         case EPlaceableState::Placed:
                              UE_LOG(LogTemp, Warning, TEXT("Target Placed"));
                              break;
                    }
                    MoveToActor(Cast<AActor>(Target), AcceptanceRadius, true, true, true);
                    
                    if(WithinRange())
                    {
                         UE_LOG(LogTemp, Display, TEXT("Within range"));
                         if(Char->bCanAttack == true)
                         {
                              UE_LOG(LogTemp, Display, TEXT("Can attack"));
                              Char->Attack(Cast<AActor>(Target));
                              Char->bCanAttack = false;
                              GetWorldTimerManager().SetTimer(Timer, this, &AUnitAIController::CanAttack, Char->AttackSpeed);
                         }
                    }
               }
          }
          else if(Char->GetCharacterState() == ECharacterState::Passive)
          {
               // AIPercep->SetSenseEnabled(SightSenseClass, false);
          }
          
     }
     else
     {
          // UE_LOG(LogTemp, Warning, TEXT("Char is null in AIController Tick"));
     }

     // Queued movement arrival check
     if(GetPawn() != nullptr)
     {
          // Has some leniance
          if((HitLoc.Location.X >= (GetPawn()->GetActorLocation().X - 100) && HitLoc.Location.X <= (GetPawn()->GetActorLocation().X + 100)) && (HitLoc.Location.Y >= (GetPawn()->GetActorLocation().Y - 100) && HitLoc.Location.Y <= (GetPawn()->GetActorLocation().Y + 100)))
          {
               UE_LOG(LogTemp, Warning, TEXT("Arrived at destination Unit AI"));
               if(Char->GetCharacterState() == ECharacterState::Aggressive)
               {
                    NextMoveQueue(true);
               }
               else
               {
                    NextMoveQueue(false);
               }
          }
     }

}