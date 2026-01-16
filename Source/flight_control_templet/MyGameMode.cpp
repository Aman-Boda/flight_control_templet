#include "MyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h" // Needed for PrintString

void AMyGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 1. Get All Actors of Class (Player Start)
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), StartPoints);

	// 2. Call the SpawnPlayer function
	SpawnPlayer();

	// 3. Print Strings (Instructions)
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Yellow, TEXT("Right shift + Spacebar to control speed"));
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Orange, TEXT("Q + E keys to control yaw"));
	}
}

void AMyGameMode::SpawnPlayer()
{
	// Safety Check: Make sure we found at least one spawn point
	if (StartPoints.Num() == 0) return;

	// 1. Get the Transform of the first spawn point (Index 0)
	FTransform SpawnTransform = StartPoints[0]->GetActorTransform();

	// 2. Get the Player Controller
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

	if (PC)
	{
		// 3. Check if there is already a pawn, and destroy it (Logic from your screenshot)
		APawn* OldPawn = PC->GetPawn();
		if (OldPawn)
		{
			OldPawn->Destroy();
		}

		// 4. Spawn the new Pilot (BP_Pilot)
		if (PilotClass)
		{
			APawn* NewPawn = GetWorld()->SpawnActor<APawn>(PilotClass, SpawnTransform);

			// 5. Possess the new pawn
			if (NewPawn)
			{
				PC->Possess(NewPawn);
			}
		}
	}
}