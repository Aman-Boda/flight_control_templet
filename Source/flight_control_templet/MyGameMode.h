#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameMode.generated.h"

UCLASS()
class FLIGHT_CONTROL_TEMPLET_API AMyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// Override the BeginPlay function (like the Event BeginPlay node)
	virtual void BeginPlay() override;

	// The custom function to handle spawning
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void SpawnPlayer();

protected:
	// Variable to store the list of Player Starts
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	TArray<AActor*> StartPoints;

	// Variable to let you select "BP_Pilot" in the Editor
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	TSubclassOf<APawn> PilotClass;
};