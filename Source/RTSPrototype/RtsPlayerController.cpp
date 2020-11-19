// Fill out your copyright notice in the Description page of Project Settings.


#include "RtsPlayerController.h"
#include "RTSPrototypeCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "CameraPawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "RTSPrototype/GameHUD.h"
#include "Building.h"

ARtsPlayerController::ARtsPlayerController()
{
     bShowMouseCursor = true;
     DefaultMouseCursor = EMouseCursor::Default;
}

void ARtsPlayerController::PlayerTick(float DeltaTime) 
{
     Super::PlayerTick(DeltaTime);

     if(PlayerState != EPlayerState::Default)
     {
          // Update building location
          PositionPlacement();
     }
     if(IsPressLeft == true)
     {
          if(PlayerPawn->Units >= 1)
          {
               PlayerPawn->Units -= 1;
               ChangeState(EPlayerState::Default);
               CreateUnit();
          }
     }
}

void ARtsPlayerController::BeginPlay()
{
     HUD = Cast<AGameHUD>(GetHUD());

     BuildingSpawnParams.Owner = Cast<AActor>(GetPawn());

     PlayerState = EPlayerState::Default;

     PlayerPawn = Cast<ACameraPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
}

void ARtsPlayerController::SetupInputComponent()
{
     Super::SetupInputComponent();
     InputComponent->BindAction("LeftMouseClick", IE_Pressed, this, &ARtsPlayerController::LeftMousePress);
     InputComponent->BindAction("LeftMouseClick", IE_Released, this, &ARtsPlayerController::LeftMouseRelease);

     InputComponent->BindAction("RightMouseClick", IE_Released, this, &ARtsPlayerController::RightMousePress);

     InputComponent->BindAction("BuildGoldProduction", IE_Released, this, &ARtsPlayerController::CreateGoldBuilding);
     InputComponent->BindAction("BuildUnitProduction", IE_Released, this, &ARtsPlayerController::CreateUnitBuilding);

     InputComponent->BindAction("PlaceUnit", IE_Released, this, &ARtsPlayerController::CreateUnit);
}

void ARtsPlayerController::MoveTo()
{
     // Trace to see what is under the mouse cursor
		Hit;
		GetHitResultUnderCursor(ECC_Visibility, false, Hit);

		if (Hit.bBlockingHit)
		{
			// We hit something, cycle through selected units and move there
               for(ARTSPrototypeCharacter* Unit : SelectedUnits)
               {
                    UAIBlueprintHelperLibrary::SimpleMoveToLocation(Unit->GetController(), Hit.ImpactPoint);
               }
		}
}

void ARtsPlayerController::LeftMousePress() 
{
     if(PlayerState == EPlayerState::Default)
     {
          SelectionInitiate();
     }
     else
     {
          // Check PlacementBuffer isn't null
          if(Cast<ARTSPrototypeCharacter>(PlacementBuffer))
          {
               IsPressLeft = true;
          }
          else
          {
               // Check if building
               if(Cast<ABuilding>(PlacementBuffer))
               {
                    ABuilding* BuildingBuffer = Cast<ABuilding>(PlacementBuffer);
                    if(BuildingBuffer->GetBuildingType() == FName("Gold Building"))
                         PlayerPawn->Gold -= PlayerPawn->GoldPrice;

                    if(BuildingBuffer->GetBuildingType() == FName("Unit Building"))
                         PlayerPawn->Gold -= PlayerPawn->UnitPrice;
               }
               ChangeState(EPlayerState::Default);
          }
          
     }
     
}

void ARtsPlayerController::LeftMouseRelease() 
{
     SelectionTerminate();
     IsPressLeft = false;
}

void ARtsPlayerController::RightMousePress() 
{
     if(PlayerState == EPlayerState::Default)
     {
          MoveTo();
     }
     else
     {
          PlacementBuffer->Destroy();
          ChangeState(EPlayerState::Default);
          
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

void ARtsPlayerController::CreateGoldBuilding() 
{
     if(PlayerPawn->Gold >= PlayerPawn->GoldPrice)
     {
          PlacementBuffer = GetWorld()->SpawnActor<ABuilding>(GoldBuildingClass);
          if(PlacementBuffer)
          {
               PlayerPawn->AddGoldBuilding();
               // Makes Tick call PositionPlacement()
               ChangeState(EPlayerState::Placing);
          }
     }
     else
     {
          ChangeState(EPlayerState::Default);
     }
          
}

void ARtsPlayerController::CreateUnitBuilding() 
{
     if(PlayerPawn->Gold >= PlayerPawn->UnitPrice)
     {
          PlacementBuffer = GetWorld()->SpawnActor<ABuilding>(UnitBuildingClass);
          if(PlacementBuffer)
          {
               PlayerPawn->AddUnitBuilding();
               // Makes Tick call PositionPlacement()
               ChangeState(EPlayerState::Placing);
          }
     }
     else
     {
          ChangeState(EPlayerState::Default);
     }
}

void ARtsPlayerController::CreateUnit() 
{
          PlacementBuffer = GetWorld()->SpawnActor<ARTSPrototypeCharacter>(UnitClass);
          if(PlacementBuffer)
          {
               PlayerPawn->MyUnits.Add(PlacementBuffer);
               // Makes Tick call PositionPlacement()
               ChangeState(EPlayerState::Placing);
          }
}

void ARtsPlayerController::PositionPlacement() 
{
     GetHitResultUnderCursor(ECC_GameTraceChannel2, false, Hit);
     //Hit.Location.Z = 170.f;

	if (Hit.bBlockingHit)
	{
          AActor* HitActor = Hit.GetActor();
          FString HitActorName = HitActor->GetDebugName(HitActor);
          PlacementBuffer->SetActorLocation(Hit.Location);
     }

}

void ARtsPlayerController::Place() 
{
     
}

void ARtsPlayerController::ChangeState(EPlayerState NewState) 
{
     PlayerState = NewState;
}