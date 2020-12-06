// Fill out your copyright notice in the Description page of Project Settings.


#include "GameHUD.h"
#include "RTSPrototype/RtsPlayerController.h"
#include "RTSPrototypeCharacter.h"
#include "Components/DecalComponent.h"
#include "Building.h"

void AGameHUD::DrawHUD()
{
     ARtsPlayerController* PlayerController = Cast<ARtsPlayerController>(GetOwningPlayerController());
     if (SelectPressed == true)
     {
          // Reset Green highlight each time a new selection is made
          for(ARTSPrototypeCharacter* Unit : PlayerController->SelectedUnits)
          {
               Unit->CursorToWorld->SetVisibility(false);
          }
          for(ABuilding* Building : PlayerController->SelectedBuildings)
          {
               Building->CursorToWorld->SetVisibility(false);
          }
          SelectionBuffer.Empty();
          PlayerController->SelectedUnits.Empty();
          PlayerController->SelectedBuildings.Empty();
          // Start Getting selected units
          GetOwningPlayerController()->GetMousePosition(CurrentMousePos.X, CurrentMousePos.Y);
          DrawRect(FLinearColor(0,0,1, .15f), InitMousePos.X, InitMousePos.Y, CurrentMousePos.X - InitMousePos.X, CurrentMousePos.Y - InitMousePos.Y);
          GetActorsInSelectionRectangle(InitMousePos, CurrentMousePos, SelectionBuffer, false);/* 
          GetActorsInSelectionRectangle(InitMousePos, CurrentMousePos, SelectionBuffer, false); */
          for(AActor* Unit : SelectionBuffer)
          {
               if(Cast<ARTSPrototypeCharacter>(Unit))
               {
                    ARTSPrototypeCharacter* Character = Cast<ARTSPrototypeCharacter>(Unit);
                    if(Character->GetOwnerUserName() == PlayerController->GetUserName())
                    {
                         PlayerController->SelectedUnits.Add(Character);
                         Character->CursorToWorld->SetVisibility(true);
                    }
               }
               if(Cast<ABuilding>(Unit))
               {
                    ABuilding* Building = Cast<ABuilding>(Unit);
                    if(Building->GetOwnerUserName() == PlayerController->GetUserName())
                    {
                         PlayerController->SelectedBuildings.Add(Building);
                         Building->CursorToWorld->SetVisibility(true);
                    }
               }
          }
     }
}