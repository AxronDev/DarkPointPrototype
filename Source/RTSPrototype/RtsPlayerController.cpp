// Fill out your copyright notice in the Description page of Project Settings.


#include "RtsPlayerController.h"
#include "RTSPrototypeCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "CameraPawn.h"
#include "UnitAIController.h"
#include "Components/DecalComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Materials/MaterialInstance.h"
#include "Blueprint/AIBlueprintHelperLibrary.h" 
#include "NavigationSystem.h"
#include "RTSPrototype/GameHUD.h"
#include "AIController.h"
#include "Building.h"
#define NETMODE_WORLD (((GEngine == nullptr) || (GetWorld() == nullptr)) ? TEXT("") \
: (GEngine->GetNetMode(GetWorld()) == NM_Client) ? TEXT("[Client] ") \
: (GEngine->GetNetMode(GetWorld()) == NM_ListenServer) ? TEXT("[ListenServer] ") \
: (GEngine->GetNetMode(GetWorld()) == NM_DedicatedServer) ? TEXT("[DedicatedServer] ") \
: TEXT("[Standalone] "))

ARtsPlayerController::ARtsPlayerController()
{
     bReplicates = true;
     bShowMouseCursor = true;
     DefaultMouseCursor = EMouseCursor::Default;
     UGameplayStatics::SetPlayerControllerID(this, 1);
}

void ARtsPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const {
     Super::GetLifetimeReplicatedProps(OutLifetimeProps);
     DOREPLIFETIME(ARtsPlayerController, RTSPlayerState);
     DOREPLIFETIME(ARtsPlayerController, PlacementBuffer); // below this is new
     DOREPLIFETIME(ARtsPlayerController, UserName);
     DOREPLIFETIME(ARtsPlayerController, SelectedUnits);
     DOREPLIFETIME(ARtsPlayerController, SelectedBuildings);
     DOREPLIFETIME(ARtsPlayerController, PlayerPawn);
     DOREPLIFETIME(ARtsPlayerController, ControlledPawn);
     DOREPLIFETIME(ARtsPlayerController, bAggressive);
     DOREPLIFETIME(ARtsPlayerController, bUnitButtons); 
     DOREPLIFETIME(ARtsPlayerController, bCanPosition);
     DOREPLIFETIME(ARtsPlayerController, bPatrol);
     DOREPLIFETIME(ARtsPlayerController, QueuedMovements);
}

FName ARtsPlayerController::GetUserName() 
{
     return UserName;
}

void ARtsPlayerController::Server_SetUsername_Implementation(const FName& NewUserName)
{
     UserName = NewUserName;

     FString NameLog;
     UserName.ToString(NameLog);
     UE_LOG(LogTemp, Warning, TEXT("UserName set to %s on %s"), *NameLog, NETMODE_WORLD);
}

bool ARtsPlayerController::Server_SetUsername_Validate(const FName& NewUserName)
{
     return true;
}

void ARtsPlayerController::PlayerTick(float DeltaTime) 
{
     Super::PlayerTick(DeltaTime);

     // ForceNetUpdate();
     // UE_LOG(LogTemp, Warning, TEXT("Forced Net Update Tick"));

     if(PlayerPawn == nullptr && HasAuthority())
     {
          ACameraPawn* CamPawn = GetPawn<ACameraPawn>();
          Server_SetPlayerPawn(CamPawn);
     }

     // Return Enum Value
     const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPlayerState"), true);
     FString EnumName = EnumPtr->GetDisplayNameText((uint8)RTSPlayerState).ToString();
     // GEngine->AddOnScreenDebugMessage(0, 2, FColor::Red, *EnumPtr->GetDisplayNameText((uint8)PlayerState).ToString());

     if(RTSPlayerState == EPlayerState::Menu) return;

     if(RTSPlayerState != EPlayerState::Default)
     {
          FHitResult HitPlacement;
          GetHitResultUnderCursor(ECC_GameTraceChannel2, false, HitPlacement);

          // Update placement location
          Server_PositionPlacement(HitPlacement, PlacementBuffer);
     }
     if(IsPressLeft == true)
     {
          if(PlacementBuffer)
          {
               if(PlayerPawn->Units >= Cast<ARTSPrototypeCharacter>(PlacementBuffer)->UnitCost)
               {
                    if(Cast<ARTSPrototypeCharacter>(PlacementBuffer)->bHasBeenPositioned == true)
                    {
                         Server_SetUnitState(Cast<ARTSPrototypeCharacter>(PlacementBuffer), ECharacterState::Placed);
                         if(Cast<ARTSPrototypeCharacter>(PlacementBuffer)->bHasSpace == true && bCanPosition == true)
                         {
                              bCanPosition = false;
                              GetWorldTimerManager().SetTimer(Timer, this, &ARtsPlayerController::CanPosition, .13f);
                              Server_BuyUnit(Cast<ARTSPrototypeCharacter>(PlacementBuffer)->UnitCost);
                              Server_ChangePlayerState(EPlayerState::Default);
                              if(PlacementBuffer->GetClass() == MeleeClass)
                              {
                                   Server_CreateUnit(MeleeClass);
                              }
                              else if(PlacementBuffer->GetClass() == RangedClass)
                              {
                                   Server_CreateUnit(RangedClass);
                              }
                              else if(PlacementBuffer->GetClass() == TankClass)
                              {
                                   Server_CreateUnit(TankClass);
                              }
                              else if(PlacementBuffer->GetClass() == SpeedClass)
                              {
                                   Server_CreateUnit(SpeedClass);
                              }
                         }
                    }
               }
          }
     }
     ControlledPawn = GetPawn();
}

void ARtsPlayerController::Server_MultiRightMouse_Implementation(const TArray<ARTSPrototypeCharacter*>& Units, FHitResult Hit) 
{
     if(Units.Num() == 0)
     {
          return;
     }
     UE_LOG(LogTemp, Warning, TEXT("Shift Right Click %i"), QueuedMovements.Num());
     if(QueuedMovements.Num() == 0)
     {
          UE_LOG(LogTemp, Warning, TEXT("First move at %f, %f, %f"), Hit.Location.X, Hit.Location.Y, Hit.Location.Z);
          if(bAggressive)
          {
               for(ARTSPrototypeCharacter* Unit : Units)
               {
                    if(bPatrol)
                    {
                         UE_LOG(LogTemp, Warning, TEXT("MultiRight Patrol"));
                         Cast<AUnitAIController>(Unit->GetController())->PreResetQueue();
                         Cast<AUnitAIController>(Unit->GetController())->SetPatroling(true);
                    }
                    else
                    {
                         UE_LOG(LogTemp, Warning, TEXT("MultiRight non-patrol"));
                         Cast<AUnitAIController>(Unit->GetController())->SetPatroling(false);
                    }
                    Cast<AUnitAIController>(Unit->GetController())->SetHit(Hit, true);
               }
          }
          else
          {
               for(ARTSPrototypeCharacter* Unit : Units)
               {
                    UE_LOG(LogTemp, Warning, TEXT("MultiRight non-patrol"));
                    Cast<AUnitAIController>(Unit->GetController())->SetPatroling(false);
                    Cast<AUnitAIController>(Unit->GetController())->SetHit(Hit, false);
                    UE_LOG(LogTemp, Warning, TEXT("First Move in sequence"));
               }
          }
     }
     for(ARTSPrototypeCharacter* Unit : Units)
     {
          Cast<AUnitAIController>(Unit->GetController())->QueuedMovements.Add(Hit);
     }
     QueuedMovements.Add(Hit);
}

bool ARtsPlayerController::Server_MultiRightMouse_Validate(const TArray<ARTSPrototypeCharacter*>& Units, FHitResult Hit) 
{
     return true;
}

void ARtsPlayerController::Server_SinglePatrol_Implementation(const TArray<ARTSPrototypeCharacter*>& Units, FHitResult Hit) 
{
     FHitResult Start = Hit;
     if(Hit.bBlockingHit)
     {
          for(ARTSPrototypeCharacter* Unit : Units)
          {
               Cast<AUnitAIController>(Unit->GetController())->PreResetQueue();
               Cast<AUnitAIController>(Unit->GetController())->SetPatroling(true);
               Cast<AUnitAIController>(Unit->GetController())->SetHit(Hit, true);
               Start.Location = Unit->GetActorLocation();
               UE_LOG(LogTemp, Warning, TEXT("Start loc of Patrol: %f, %f, %f"), Start.Location.X, Start.Location.Y, Start.Location.Z);
               Cast<AUnitAIController>(Unit->GetController())->QueuedMovements.Add(Start);
               Cast<AUnitAIController>(Unit->GetController())->QueuedMovements.Add(Hit);
          }
          QueuedMovements.Add(Start);
          QueuedMovements.Add(Hit);
     }
}

bool ARtsPlayerController::Server_SinglePatrol_Validate(const TArray<ARTSPrototypeCharacter*>& Units, FHitResult Hit) 
{
     return true;
}

void ARtsPlayerController::ShiftPress() 
{
     bIsShiftPress = true;
}

void ARtsPlayerController::ShiftReleased() 
{
     bIsShiftPress = false;
}

void ARtsPlayerController::Server_SetUnitState_Implementation(ARTSPrototypeCharacter* Unit, ECharacterState NewState) 
{
     Unit->Server_ChangeCharacterState(NewState);
}

bool ARtsPlayerController::Server_SetUnitState_Validate(ARTSPrototypeCharacter* Unit, ECharacterState NewState)
{
     return true;
}

void ARtsPlayerController::Server_SetBuildingState_Implementation(ABuilding* NewBuilding, EBuildingState NewState) 
{
     UE_LOG(LogTemp, Warning, TEXT("PC Server Calling Building state change %s"), NETMODE_WORLD);
     NewBuilding->Server_SetBuildingState(NewState);
}

bool ARtsPlayerController::Server_SetBuildingState_Validate(ABuilding* NewBuilding, EBuildingState NewState) 
{
     return true;
}

void ARtsPlayerController::CanPosition() 
{
     bCanPosition = true;
}

// TODO Delete
void ARtsPlayerController::StartPatrol(FHitResult First) 
{
     return;
}

void ARtsPlayerController::Server_InversePatrol_Implementation() 
{
     bPatrol = !bPatrol;
     UE_LOG(LogTemp, Warning, TEXT("bPatrol changed to %s"), ( bPatrol ? TEXT("true") : TEXT("false") ));
     if(bPatrol == true && bAggressive == false)
     {
          Server_SetAggression();
     }
}

bool ARtsPlayerController::Server_InversePatrol_Validate()
{
     return true;
}

void ARtsPlayerController::Server_SetPlayerPawn_Implementation(ACameraPawn* Camera) 
{
     if(Camera)
     {
          UE_LOG(LogTemp, Warning, TEXT("Got Pawn %s"), NETMODE_WORLD);
          PlayerPawn = Camera;
     }
     else
     {
          UE_LOG(LogTemp, Warning, TEXT("Failed to get Pawn %s"), NETMODE_WORLD);
     }
}

bool ARtsPlayerController::Server_SetPlayerPawn_Validate(ACameraPawn* Camera) 
{
     return true;
}


void ARtsPlayerController::Server_SetAggression_Implementation() 
{
     if(bUnitButtons == false)
     {
          bAggressive = !bAggressive;
     }
}

bool ARtsPlayerController::Server_SetAggression_Validate() 
{
     return true;
}

void ARtsPlayerController::BeginPlay()
{
     ACameraPawn* CamPawn;
     HUD = Cast<AGameHUD>(GetHUD());

     BuildingSpawnParams.Owner = Cast<AActor>(GetPawn());

     RTSPlayerState = EPlayerState::Default;

     CamPawn = GetPawn<ACameraPawn>();
     Server_SetPlayerPawn(CamPawn);

     // PlayerPawn = Cast<ACameraPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

     /* UE_LOG(LogTemp, Warning, TEXT("PlayerController made %s"), NETMODE_WORLD);
     bool NameSet = false;
     bool NameTaken = false;
     for(int n = 0; n < 3; n++)
     {
          FName NameToSet;
          switch(n)
          {
               case 0:
                    NameToSet = FName("Host");
                    break;
               case 1:
                    NameToSet = FName("Aaron");
                    break;
               case 2:
                    NameToSet = FName("Josh");
                    break;
          }
          if(NameSet == false)
          {
               for(int i = 0; i < GetWorld()->GetNumPlayerControllers(); i++)
               {
                    if(NameTaken == false)
                    {
                         ARtsPlayerController *PC = Cast<ARtsPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), i));
                         if(PC)
                         {
                              if(NameToSet != PC->GetUserName())
                              {
                                   if(i == (GetWorld()->GetNumPlayerControllers()) - 1)
                                   {
                                        NameSet = true;
                                        Server_SetUsername(NameToSet);
                                   }
                              }
                              else
                              {
                                   NameTaken = true;
                              }
                         }
                    }
               }
          }
          else
          {
               CamPawn = GetPawn<ACameraPawn>();
               Server_SetPlayerPawn(CamPawn);
          }
     } */

     /* for(int i = 0; i < GetWorld()->GetNumPlayerControllers(); i++)
     {
          UE_LOG(LogTemp, Warning, TEXT("Player Controller num: %i"), GetWorld()->GetNumPlayerControllers());
          if(Cast<ARtsPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), i)) == this)
          {
               switch(i)
               {
                    case 0:
                         Server_SetUsername(FName("Host"));
                         if(GetPawn<ACameraPawn>())
                         {
                              UE_LOG(LogTemp, Warning, TEXT("Got Pawn with auth %s"), NETMODE_WORLD);
                              PlayerPawn = GetPawn<ACameraPawn>();
                         }
                         else
                         {
                              UE_LOG(LogTemp, Warning, TEXT("Failed to get Pawn with auth %s"), NETMODE_WORLD);
                         }
                         break;
                    case 1:
                         Server_SetUsername(FName("Aaron"));
                         CamPawn = GetPawn<ACameraPawn>();
                         Server_SetPlayerPawn(CamPawn);
                         break;
                    case 2:
                         Server_SetUsername(FName("Josh"));
                         CamPawn = GetPawn<ACameraPawn>();
                         Server_SetPlayerPawn(CamPawn);
                         break;
               }
          }
     } */
     /* if(HasAuthority())
     {
          Server_SetUsername(FName("Axron"));
          if(GetPawn<ACameraPawn>())
          {
               UE_LOG(LogTemp, Warning, TEXT("Got Pawn with auth %s"), NETMODE_WORLD);
               PlayerPawn = GetPawn<ACameraPawn>();
          }
          else
          {
               UE_LOG(LogTemp, Warning, TEXT("Failed to get Pawn with auth %s"), NETMODE_WORLD);
          }
     }
     else
     {
          Server_SetUsername(FName("Josh"));
          ACameraPawn* CamPawn = GetPawn<ACameraPawn>();
          Server_SetPlayerPawn(CamPawn);
     } */
}

void ARtsPlayerController::SetupInputComponent()
{
     Super::SetupInputComponent();
     InputComponent->BindAction("LeftMouseClick", IE_Pressed, this, &ARtsPlayerController::LeftMousePress);
     InputComponent->BindAction("LeftMouseClick", IE_Released, this, &ARtsPlayerController::LeftMouseRelease);

     InputComponent->BindAction("RightMouseClick", IE_Released, this, &ARtsPlayerController::RightMousePress);

     InputComponent->BindAction("BuildGoldProduction", IE_Released, this, &ARtsPlayerController::Server_CreateGoldBuilding);
     InputComponent->BindAction("BuildUnitProduction", IE_Released, this, &ARtsPlayerController::Server_CreateUnitBuilding);
     InputComponent->BindAction("BuildHealthBuilding", IE_Released, this, &ARtsPlayerController::Server_CreateHealthBuilding);

     InputComponent->BindAction("UnitModifier", IE_Released, this, &ARtsPlayerController::UnitModifierButtons);
     InputComponent->BindAction("A", IE_Released, this, &ARtsPlayerController::APress);
     InputComponent->BindAction("S", IE_Released, this, &ARtsPlayerController::SPress);
     InputComponent->BindAction("D", IE_Released, this, &ARtsPlayerController::DPress);
     InputComponent->BindAction("F", IE_Released, this, &ARtsPlayerController::FPress);

     InputComponent->BindAction("P", IE_Released, this, &ARtsPlayerController::Server_InversePatrol);
     InputComponent->BindAction("AttackMovement", IE_Released, this, &ARtsPlayerController::Server_SetAggression);
     InputComponent->BindAction("Shift", IE_Pressed, this, &ARtsPlayerController::ShiftPress);
     InputComponent->BindAction("Shift", IE_Released, this, &ARtsPlayerController::ShiftReleased);
}

void ARtsPlayerController::MoveTo()
{
     if(bAggressive == true)
     {
          UE_LOG(LogTemp, Warning, TEXT("Attack"));
          PlayerPawn->CursorToWorld->SetDecalMaterial(PlayerPawn->RedX);
          PlayerPawn->CursorToWorld->SetWorldLocation(MoveToHit.ImpactPoint);
          PlayerPawn->SetXLocation(MoveToHit.ImpactPoint);
          PlayerPawn->CursorToWorld->SetVisibility(true);
     }

     else 
     {
          UE_LOG(LogTemp, Warning, TEXT("Passive"));
          PlayerPawn->CursorToWorld->SetDecalMaterial(PlayerPawn->WhiteX);
          PlayerPawn->CursorToWorld->SetWorldLocation(MoveToHit.ImpactPoint);
          PlayerPawn->SetXLocation(MoveToHit.ImpactPoint);
          PlayerPawn->CursorToWorld->SetVisibility(true);
     }

     Server_MoveTo(MoveToHit, SelectedUnits);
}

void ARtsPlayerController::Server_MoveTo_Implementation(FHitResult Hit, const TArray<ARTSPrototypeCharacter*>& Units)
{// Trace to see what is under the mouse cursor

     UE_LOG(LogTemp, Warning, TEXT("MoveTo called"));

     bool IsUnit = false;
     UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *GetDebugName(Hit.GetActor()));
     if(Cast<IPlaceable>(Hit.GetActor()))
     {
          IsUnit = true;
     }

	if (Hit.bBlockingHit)
	{
          UE_LOG(LogTemp, Warning, TEXT("Hit Something at X: %f Y: %f Z: %f"), Hit.ImpactPoint.X, Hit.ImpactPoint.Y, Hit.ImpactPoint.Z);
		// We hit something, cycle through selected units and move there
          for(ARTSPrototypeCharacter* Unit : Units)
          {
               if(bAggressive == true)
               {
                    Server_SetUnitState(Unit , ECharacterState::Aggressive);
                    UE_LOG(LogTemp, Warning, TEXT("Attack"));
               }
               else 
               {
                    Server_SetUnitState(Unit ,ECharacterState::Passive);
                    UE_LOG(LogTemp, Warning, TEXT("Passive"));
               }
               
               AAIController* UnitController = Cast<AAIController>(Unit->GetController());
               if(UnitController != nullptr)
               {
                    FString UnitName = UnitController->GetDebugName(UnitController);
                    UE_LOG(LogTemp, Warning, TEXT("Attained unit controller %s"), *UnitName);
                    if(HasAuthority())
                    {
                         if(IsUnit == true)
                         {
                              UAIBlueprintHelperLibrary::SimpleMoveToActor(Unit->GetController(), Hit.GetActor());
                              UE_LOG(LogTemp, Warning, TEXT("SimpleMoveToACTOR called in Server MoveTo"));
                              if(bAggressive == true)
                              {
                                   for(auto TempUnit : SelectedUnits)
                                   {
                                        Cast<AUnitAIController>(TempUnit->GetController())->SetTarget(Cast<IPlaceable>(Hit.GetActor()));
                                   }
                              }
                         }
                         else
                         {
                              UAIBlueprintHelperLibrary::SimpleMoveToLocation(Unit->GetController(), Hit.ImpactPoint);
                              UE_LOG(LogTemp, Warning, TEXT("SimpleMoveToLOCATION called in Server MoveTo"));
                         }
                    }
               }
               else
               {
                    UE_LOG(LogTemp, Warning, TEXT("Failed to get unit controller"));
               }            
          }
	}
     // reset to passive
     bAggressive = false;
}

bool ARtsPlayerController::Server_MoveTo_Validate(FHitResult Hit, const TArray<ARTSPrototypeCharacter*>& Units)
{
     return true;
}

void ARtsPlayerController::LeftMousePress() 
{
     UE_LOG(LogTemp, Warning, TEXT("Left Mouse Pressed"));
     if(RTSPlayerState == EPlayerState::Menu) return;
     if(RTSPlayerState == EPlayerState::Default)
     {
          if(bPatrol == true)
          {
               Server_InversePatrol();
          }
          SelectionInitiate();
     }
     else
     {
          // Check PlacementBuffer isn't null
          if(Cast<ARTSPrototypeCharacter>(PlacementBuffer))
          {
               IsPressLeft = true;
          }
          // Check if building
          else if(Cast<ABuilding>(PlacementBuffer))
          {
               UE_LOG(LogTemp, Warning, TEXT("Building in placement buffer"));
               ABuilding* BuildingBuffer = Cast<ABuilding>(PlacementBuffer);

               if(BuildingBuffer->bCanPlace)
               {
                    UE_LOG(LogTemp, Warning, TEXT("Can Place Building"));
                    if(BuildingBuffer->GetBuildingType() == FName("Gold Building"))
                    {
                         if(PlayerPawn->Gold >= PlayerPawn->GoldPrice)
                         {
                              PlayerPawn->Gold -= PlayerPawn->GoldPrice;
                              PlayerPawn->Server_AddGoldBuilding();
                              Server_ChangePlayerState(EPlayerState::Default);
                              Server_SetBuildingState(BuildingBuffer, EBuildingState::Built);
                              BuildingBuffer->CursorToWorld->SetVisibility(false);
                              return;
                         }
                    }

                    if(BuildingBuffer->GetBuildingType() == FName("Unit Building"))
                    {
                         if(PlayerPawn->Gold >= PlayerPawn->UnitPrice)
                         {
                              PlayerPawn->Gold -= PlayerPawn->UnitPrice;
                              PlayerPawn->Server_AddUnitBuilding();
                              Server_ChangePlayerState(EPlayerState::Default);
                              Server_SetBuildingState(BuildingBuffer, EBuildingState::Built);
                              BuildingBuffer->CursorToWorld->SetVisibility(false);
                              return;
                         }
                    }

                    if(BuildingBuffer->GetBuildingType() == FName("Health Building"))
                    {
                         if(PlayerPawn->Gold >= PlayerPawn->HealthPrice)
                         {
                              PlayerPawn->Gold -= PlayerPawn->HealthPrice;
                              PlayerPawn->Server_AddHealthBuilding(Cast<ABuilding>(PlacementBuffer));
                              Server_ChangePlayerState(EPlayerState::Default);
                              Server_SetBuildingState(BuildingBuffer, EBuildingState::Built);
                              BuildingBuffer->CursorToWorld->SetVisibility(false);
                              return;
                         }
                    }                  
               }
          }
          else
          {
               Server_ChangePlayerState(EPlayerState::Default);
          }
     }     
}

void ARtsPlayerController::LeftMouseRelease() 
{
     if(RTSPlayerState == EPlayerState::Menu) return;
     SelectionTerminate();
     IsPressLeft = false;
     FString LogName;
     UserName.ToString(LogName);
     UE_LOG(LogTemp, Warning, TEXT("Username: %s Left Mouse Released %s"), *LogName, NETMODE_WORLD)
}

void ARtsPlayerController::RightMousePress() 
{
     if(RTSPlayerState == EPlayerState::Menu) return;
     if(bIsShiftPress)
     {
          FHitResult TempHit;
          GetHitResultUnderCursor(ECC_GameTraceChannel2, false, TempHit);
          // TODO LIne trace here not in server functions and pass in hit result
          Server_MultiRightMouse(SelectedUnits, TempHit);
          return;
     }
     for(ARTSPrototypeCharacter* Unit : SelectedUnits)
     {
          Cast<AUnitAIController>(Unit->GetController())->PreResetQueue();
     }
     if(bPatrol == true)
     {
          FHitResult TempHit;
          GetHitResultUnderCursor(ECC_GameTraceChannel2, false, TempHit);
          Server_SinglePatrol(SelectedUnits, TempHit);
          return;
     }
     for(ARTSPrototypeCharacter* Unit : SelectedUnits)
     {
          Cast<AUnitAIController>(Unit->GetController())->PreResetQueue();
     }
     UE_LOG(LogTemp, Warning, TEXT("RightClick"));
     if(RTSPlayerState == EPlayerState::Default)
     {
          GetHitResultUnderCursor(ECC_GameTraceChannel4, false, MoveToHit);
          MoveTo();
     }
     else
     {
          Server_DestroyUnit();
          Server_ChangePlayerState(EPlayerState::Default);
          
     }
}

void ARtsPlayerController::RightMouseRelease() 
{
     
}

void ARtsPlayerController::SelectionInitiate()
{
     GetMousePosition(HUD->InitMousePos.X, HUD->InitMousePos.Y);
     HUD->SelectPressed = true;
}

void ARtsPlayerController::SelectionTerminate()
{
     HUD->SelectPressed = false;
}

void ARtsPlayerController::Server_CreateHealthBuilding_Implementation() 
{
     if(RTSPlayerState == EPlayerState::Menu) return;

     if(PlayerPawn->Gold >= PlayerPawn->HealthPrice)
     {
          // Removes previous placement if never placed
          if(PlacementBuffer)
          {
               if(Cast<IPlaceable>(PlacementBuffer)->GetPlaceableState() == EPlaceableState::Preview)
               {
                    PlacementBuffer->Destroy();
               }
          }
          // UE_LOG(LogTemp, Warning, TEXT("Starting Spawning Building")) SpawnActorDeferred , FTransform(FRotator(.0f)), this
          PlacementBuffer = GetWorld()->SpawnActor<ABuilding>(HealthBuildingClass);
          if(PlacementBuffer)
          {
               // Cast<ABuilding>(PlacementBuffer)->SetOwner(this);
               // UE_LOG(LogTemp, Warning, TEXT("Set Building Owner"))
               // UE_LOG(LogTemp, Warning, TEXT("CreatGoldBuilding Owner: %s on %s"), *GetDebugName(PlacementBuffer->GetOwner()), NETMODE_WORLD);
               // UGameplayStatics::FinishSpawningActor(PlacementBuffer, FTransform(FRotator(.0f)));
               // UE_LOG(LogTemp, Warning, TEXT("Finished Spawning Building"))
               Cast<ABuilding>(PlacementBuffer)->SetOwnerUserName(UserName);
               // Makes Tick call Server_PositionPlacement()
               Server_ChangePlayerState(EPlayerState::Placing);
               UE_LOG(LogTemp, Warning, TEXT("Calling Change building state in create health %s"), NETMODE_WORLD);
               Cast<ABuilding>(PlacementBuffer)->Server_SetBuildingState(EBuildingState::Preview);
          }
     }
     else
     {
          Server_ChangePlayerState(EPlayerState::Default);
     }
}

bool ARtsPlayerController::Server_CreateHealthBuilding_Validate()
{
     return true;
}

void ARtsPlayerController::Server_DestroyUnit_Implementation() 
{
     PlacementBuffer->Destroy();
}

bool ARtsPlayerController::Server_DestroyUnit_Validate()
{
     return true;
}

void ARtsPlayerController::Server_BuyUnit_Implementation(uint8 UnitCost) 
{
     PlayerPawn->Units -= UnitCost;
}

bool ARtsPlayerController::Server_BuyUnit_Validate(uint8 UnitCost)
{
     return true;
}

void ARtsPlayerController::UnitModifierButtons() 
{
     bUnitButtons = true;
}

void ARtsPlayerController::APress() 
{
     if(bUnitButtons == true)
     {
          if(RTSPlayerState == EPlayerState::Placing)
          {
               Server_DestroyUnit();
          }
          bUnitButtons = false;
          Server_CreateUnit(MeleeClass);
     }
}

void ARtsPlayerController::SPress() 
{
     if(bUnitButtons == true)
     {
          if(RTSPlayerState == EPlayerState::Placing)
          {
               Server_DestroyUnit();
          }
          bUnitButtons = false;
          Server_CreateUnit(RangedClass);
     }
}

void ARtsPlayerController::DPress() 
{
     if(bUnitButtons == true)
     {
          if(RTSPlayerState == EPlayerState::Placing)
          {
               Server_DestroyUnit();
          }
          bUnitButtons = false;
          Server_CreateUnit(TankClass);
     }
}

void ARtsPlayerController::FPress() 
{
     if(bUnitButtons == true)
     {
          if(RTSPlayerState == EPlayerState::Placing)
          {
               Server_DestroyUnit();
          }
          bUnitButtons = false;
          Server_CreateUnit(SpeedClass);
     }
}

void ARtsPlayerController::Server_CreateGoldBuilding_Implementation() 
{
     if(RTSPlayerState == EPlayerState::Menu) return;

     if(PlayerPawn->Gold >= PlayerPawn->GoldPrice)
     {
          // Removes previous placement if never placed
          if(PlacementBuffer)
          {
               if(Cast<IPlaceable>(PlacementBuffer)->GetPlaceableState() == EPlaceableState::Preview)
               {
                    PlacementBuffer->Destroy();
               }
          }
          // UE_LOG(LogTemp, Warning, TEXT("Starting Spawning Building")) SpawnActorDeferred , FTransform(FRotator(.0f)), this
          PlacementBuffer = GetWorld()->SpawnActor<ABuilding>(GoldBuildingClass);
          if(PlacementBuffer)
          {
               // Cast<ABuilding>(PlacementBuffer)->SetOwner(this);
               // UE_LOG(LogTemp, Warning, TEXT("Set Building Owner"))
               // UE_LOG(LogTemp, Warning, TEXT("CreatGoldBuilding Owner: %s on %s"), *GetDebugName(PlacementBuffer->GetOwner()), NETMODE_WORLD);
               // UGameplayStatics::FinishSpawningActor(PlacementBuffer, FTransform(FRotator(.0f)));
               // UE_LOG(LogTemp, Warning, TEXT("Finished Spawning Building"))
               Cast<ABuilding>(PlacementBuffer)->SetOwnerUserName(UserName);
               // Makes Tick call Server_PositionPlacement()
               Server_ChangePlayerState(EPlayerState::Placing);
               UE_LOG(LogTemp, Warning, TEXT("Calling Change building state in create gold %s"), NETMODE_WORLD);
               Cast<ABuilding>(PlacementBuffer)->Server_SetBuildingState(EBuildingState::Preview);
          }
     }
     else
     {
          Server_ChangePlayerState(EPlayerState::Default);
     }
          
}

bool ARtsPlayerController::Server_CreateGoldBuilding_Validate()
{
     return true;
}

void ARtsPlayerController::Server_CreateUnitBuilding_Implementation() 
{
     if(RTSPlayerState == EPlayerState::Menu) return;
     // Removes previous placement if never placed
     if(PlacementBuffer)
     {
          if(Cast<IPlaceable>(PlacementBuffer)->GetPlaceableState() == EPlaceableState::Preview)
          {
               PlacementBuffer->Destroy();
          }
     }
/* FActorSpawnParameters SpawnParams;
     SpawnParams.Owner = this; , SpawnParams */
     PlacementBuffer = GetWorld()->SpawnActor<ABuilding>(UnitBuildingClass);
     if(PlacementBuffer)
     {
          Cast<ABuilding>(PlacementBuffer)->SetOwner(this);
          //UE_LOG(LogTemp, Warning, TEXT("Set Building Owner"))
          Cast<ABuilding>(PlacementBuffer)->SetOwnerUserName(UserName);
          FString LogName;
          UserName.ToString(LogName);
          UE_LOG(LogTemp, Warning, TEXT("Username: %s create unit building %s"), *LogName, NETMODE_WORLD)
          // Makes Tick call Server_PositionPlacement()
          Server_ChangePlayerState(EPlayerState::Placing);
          UE_LOG(LogTemp, Warning, TEXT("Calling Change building state in create unit %s"), NETMODE_WORLD);
          Cast<ABuilding>(PlacementBuffer)->Server_SetBuildingState(EBuildingState::Preview);
     }
     else
     {
          Server_ChangePlayerState(EPlayerState::Default);
     }
}

bool ARtsPlayerController::Server_CreateUnitBuilding_Validate() 
{
     return true;
}

void ARtsPlayerController::PrepareUnit_Implementation(AActor* NewUnit) 
{
     // Removes previous placement if never placed
     if(PlacementBuffer)
     {
          if(Cast<IPlaceable>(PlacementBuffer)->GetPlaceableState() == EPlaceableState::Preview)
          {
               PlacementBuffer->Destroy();
          }
     }

     PlacementBuffer = NewUnit;
     if(NewUnit != nullptr && Cast<ARTSPrototypeCharacter>(NewUnit) && PlacementBuffer == NewUnit)
     {
          UE_LOG(LogTemp, Warning, TEXT("New Unit is good %s"), NETMODE_WORLD);
          
          // Makes Tick call PositionPlacement()
          Server_ChangePlayerState(EPlayerState::Placing);
          const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPlayerState"), true);
          GEngine->AddOnScreenDebugMessage(0, 2, FColor::Red, *EnumPtr->GetDisplayNameText((uint8)RTSPlayerState).ToString());
     }
     else
     {
          UE_LOG(LogTemp, Warning, TEXT("New Unit is bad"));
     }
     GEngine->AddOnScreenDebugMessage(0, 2, FColor::Red, TEXT("PrepareUnit Called "));
     
}

// Called when U is pressed
void ARtsPlayerController::Server_CreateUnit_Implementation(TSubclassOf<ARTSPrototypeCharacter> UnitClass) 
{
     if(RTSPlayerState == EPlayerState::Menu) return;

     AActor* NewUnit = GetWorld()->SpawnActor<ARTSPrototypeCharacter>(UnitClass, FVector(-3028.0f, -276.0f, 277.56f), FRotator(0.0f));
     if(NewUnit)
     {
          Cast<APawn>(NewUnit)->GetController()->SetOwner(this);
          Cast<ARTSPrototypeCharacter>(NewUnit)->SetOwnerUserName(UserName);
          Cast<ARTSPrototypeCharacter>(NewUnit)->SetOwningPlayer(this);
          NewUnit->SetReplicates(true);
          PlayerPawn->MyUnits.Add(NewUnit);
          FString UnitName = GetDebugName(NewUnit);
          UE_LOG(LogTemp, Warning, TEXT("Unit Buffer in CreateUnit: %s   %s"), *UnitName, NETMODE_WORLD);
          PrepareUnit_Implementation(NewUnit);
     }
}

bool ARtsPlayerController::Server_CreateUnit_Validate(TSubclassOf<ARTSPrototypeCharacter> UnitClass)
{
     return true;
}

// TODO Make this call only on client from tick then once on server after placement is confirmed
void ARtsPlayerController::Server_PositionPlacement_Implementation(FHitResult HitRes, AActor* UnitToPlace) 
{
     // Placement Trace Channel
     if(Cast<ARTSPrototypeCharacter>(UnitToPlace) != nullptr)
     {
          // FString UnitName = GetDebugName(UnitToPlace);
          // UE_LOG(LogTemp, Warning, TEXT("Unit Buffer in PositionPlacement: %s  %s"), *UnitName, NETMODE_WORLD);
          HitRes.Location.Z += 100.f;
          Cast<ARTSPrototypeCharacter>(UnitToPlace)->bHasBeenPositioned = true;
     }
     else
     {
          if(UnitToPlace != nullptr)
          {
               // UE_LOG(LogTemp, Warning, TEXT("Cast Failed in Position Placement"));
               Cast<ABuilding>(UnitToPlace)->bHasBeenPositioned = true;
          }
     }
     

	if (HitRes.bBlockingHit)
	{
          if(Cast<ABuilding>(UnitToPlace))
          {
               HitRes.Location.X = 100*(round(HitRes.Location.X * .01f));
               HitRes.Location.Y = 100*(round(HitRes.Location.Y * .01f));
          }
          if(UnitToPlace != nullptr)
          {
               UnitToPlace->SetActorLocation(HitRes.Location, false, nullptr, ETeleportType::TeleportPhysics);
          }
     }
     else
     {
          UE_LOG(LogTemp, Warning, TEXT("Nothing Blocking Hit"));
     }
     

}

bool ARtsPlayerController::Server_PositionPlacement_Validate(FHitResult HitRes, AActor* UnitToPlace)
{
     return true;
}

void ARtsPlayerController::Server_ChangePlayerState_Implementation(EPlayerState NewState) 
{
     RTSPlayerState = NewState;
     const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPlayerState"), true);
     FString Message = FString("Changed State to: ").Append(*EnumPtr->GetDisplayNameText((uint8)RTSPlayerState).ToString());
     GEngine->AddOnScreenDebugMessage(2, 2, FColor::Red, Message);
     FString EnumName = EnumPtr->GetDisplayNameText((uint8)RTSPlayerState).ToString();
     UE_LOG(LogTemp, Warning, TEXT("Changed State to: %s    %s"), *EnumName, NETMODE_WORLD);
}

bool ARtsPlayerController::Server_ChangePlayerState_Validate(EPlayerState NewState)
{
     return true;
}