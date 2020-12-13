// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSGameInstance.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "MenuSystem/MainMenu.h"
#include "MenuSystem/PauseMenu.h"
#include "Kismet/KismetSystemLibrary.h"

URTSGameInstance::URTSGameInstance(const FObjectInitializer& ObjectInitializer) 
{
     ConstructorHelpers::FClassFinder<UUserWidget> MenuBPClass(TEXT("/Game/MenuSystem/WBP_MainMenu"));
     if(MenuBPClass.Class != nullptr)
     {
          MenuClass = MenuBPClass.Class;
          UE_LOG(LogTemp, Warning, TEXT("Menu class set"));
     }

     ConstructorHelpers::FClassFinder<UUserWidget> PauseMenuBPClass(TEXT("/Game/MenuSystem/WBP_PauseMenu"));
     if(PauseMenuBPClass.Class != nullptr)
     {
          PauseMenuClass = PauseMenuBPClass.Class;
          UE_LOG(LogTemp, Warning, TEXT("PauseMenu class set"));
     }
}

void URTSGameInstance::Init() 
{
     UE_LOG(LogTemp, Warning, TEXT("%s"), *MenuClass->GetName());
}

void URTSGameInstance::LoadMenu() 
{
     UE_LOG(LogTemp, Warning, TEXT("LoadMenu Called in Game Instance"));
     Menu = CreateWidget<UMainMenu>(this, MenuClass);
     if (!ensure(Menu))
     {
          UE_LOG(LogTemp, Warning, TEXT("Menu not created in load menu"));
          return;
     }
     
     Menu->Setup();

     Menu->SetMenuInterface(this);
}

void URTSGameInstance::LoadPauseMenu() 
{
     PauseMenu = CreateWidget<UPauseMenu>(this, PauseMenuClass);
     if (!ensure(PauseMenu)) return;
     
     PauseMenu->Setup();

     PauseMenu->SetMenuInterface(this);
}

void URTSGameInstance::Host() 
{
     if (Menu) 
     {
          Menu->Teardown();
     }
     UEngine* Engine = GetEngine();
     if (!ensure(Engine)) return;

     Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting"));

     UWorld* World = GetWorld(); 
     if(!ensure(World)) return;
     World->ServerTravel("Game/TopDownCPP/Maps/TopDownExampleMap?listen");
}

void URTSGameInstance::Join(const FString& Address) 
{
     if (Menu)
     {
          Menu->Teardown();
     }
     UEngine* Engine = GetEngine();
     if (!ensure(Engine)) return;

     Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));
     APlayerController* PlayerController = GetFirstLocalPlayerController();
     if(!ensure(PlayerController)) 
     {
          Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, FString::Printf(TEXT("Player controller is bad")));
          return;
     }
     Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, FString::Printf(TEXT("Player controller is good")));

     PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

void URTSGameInstance::QuitSession() 
{
     PauseMenu->Teardown();
     ReturnToMainMenu();
}
