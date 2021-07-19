// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSGameInstance.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "MenuSystem/MainMenu.h"
#include "MenuSystem/PregameMenu.h"
#include "Input/Reply.h"
#include "MenuSystem/PauseMenu.h"
#include "RtsPlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#define NETMODE_WORLD (((GEngine == nullptr) || (GetWorld() == nullptr)) ? TEXT("") \
: (GEngine->GetNetMode(GetWorld()) == NM_Client) ? TEXT("[Client] ") \
: (GEngine->GetNetMode(GetWorld()) == NM_ListenServer) ? TEXT("[ListenServer] ") \
: (GEngine->GetNetMode(GetWorld()) == NM_DedicatedServer) ? TEXT("[DedicatedServer] ") \
: TEXT("[Standalone] "))

URTSGameInstance::URTSGameInstance(const FObjectInitializer& ObjectInitializer) 
{
     ConstructorHelpers::FClassFinder<UUserWidget> MenuBPClass(TEXT("/Game/MenuSystem/WBP_MainMenu"));
     if(MenuBPClass.Class != nullptr)
     {
          MenuClass = MenuBPClass.Class;
          UE_LOG(LogTemp, Warning, TEXT("Menu class set"));
     }

     ConstructorHelpers::FClassFinder<UUserWidget> PregameMenuBPClass(TEXT("/Game/MenuSystem/WBP_PregameMenu"));
     if(PregameMenuBPClass.Class != nullptr)
     {
          PregameMenuClass = PregameMenuBPClass.Class;
          UE_LOG(LogTemp, Warning, TEXT("Pregame Menu class set"));
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
     APlayerController* PlayerController = GetFirstLocalPlayerController();
     Cast<ARtsPlayerController>(PlayerController)->Server_ChangePlayerState(EPlayerState::Menu);
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

void URTSGameInstance::LoadPregameMenu() 
{
     APlayerController* PlayerController = GetFirstLocalPlayerController();
     Cast<ARtsPlayerController>(PlayerController)->Server_ChangePlayerState(EPlayerState::Menu);
     PregameMenu = CreateWidget<UPregameMenu>(this, PregameMenuClass);
     if (!ensure(PregameMenu))
     {
          UE_LOG(LogTemp, Warning, TEXT("PregameMenu not created in load PregameMenu"));
          return;
     }
     
     PregameMenu->Setup();

     PregameMenu->SetMenuInterface(this);
}

void URTSGameInstance::LoadPauseMenu() 
{
     APlayerController* PlayerController = GetFirstLocalPlayerController();
     Cast<ARtsPlayerController>(PlayerController)->Server_ChangePlayerState(EPlayerState::Menu);
     PauseMenu = CreateWidget<UPauseMenu>(this, PauseMenuClass);
     if (!ensure(PauseMenu)) return;
     
     PauseMenu->Setup();

     PauseMenu->SetMenuInterface(this);
}

void URTSGameInstance::SetPlayerReady(ARtsPlayerController* ReadyPlayer) 
{
     APlayerController* PlayerController = GetFirstLocalPlayerController();
     PregameMenu->Teardown();
     Cast<ARtsPlayerController>(PlayerController)->Server_ChangePlayerState(EPlayerState::Default);
     // Fix input mode
     FInputModeGameOnly InputModeData;
     InputModeData.SetConsumeCaptureMouseDown(false);
     PlayerController->SetInputMode(InputModeData);
}

void URTSGameInstance::Host() 
{
     if (Menu) 
     {
          Menu->Teardown();
     }
     // Fix input mode
     FInputModeGameOnly InputModeData;
     InputModeData.SetConsumeCaptureMouseDown(false);
     APlayerController* PlayerController = GetFirstLocalPlayerController();
     PlayerController->SetInputMode(InputModeData);

     UEngine* Engine = GetEngine();
     if (!ensure(Engine)) return;

     Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting"));

     UWorld* World = GetWorld(); 
     if(!ensure(World)) return;
     Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Starting server travel"));
     World->ServerTravel("/Game/TopDownCPP/Maps/GenericMap?listen");
     Cast<ARtsPlayerController>(PlayerController)->Server_ChangePlayerState(EPlayerState::Default);
}

void URTSGameInstance::Join(const FString& Address) 
{
     if (Menu)
     {
          Menu->Teardown();
     }

     APlayerController* PlayerController = GetFirstLocalPlayerController();

     UEngine* Engine = GetEngine();
     if (!ensure(Engine)) return;

     Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));
     if(!ensure(PlayerController)) 
     {
          Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, FString::Printf(TEXT("Player controller is bad")));
          return;
     }
     Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, FString::Printf(TEXT("Player controller is good")));

     PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute, true);
     Cast<ARtsPlayerController>(PlayerController)->Server_ChangePlayerState(EPlayerState::Default);
     UE_LOG(LogTemp, Warning, TEXT("Join called in Game Instance on: %s "), NETMODE_WORLD);
}

void URTSGameInstance::QuitSession() 
{
     PauseMenu->Teardown();
     ReturnToMainMenu();
}

void URTSGameInstance::SetUsername(const FName& NewUsername) 
{
     ARtsPlayerController* PlayerController = Cast<ARtsPlayerController>(GetFirstLocalPlayerController());
     if(!ensure(PlayerController)) return;
     PlayerController->Server_SetUsername(NewUsername);
}