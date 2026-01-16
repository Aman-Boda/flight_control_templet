#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Rocket.generated.h"

UCLASS()
class FLIGHT_CONTROL_TEMPLET_API ARocket : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARocket();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// ====================================================================
	// COMPONENTS
	// ====================================================================
protected:
	// The collision component (Root)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* CollisionSphere;

	// The visual mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* RocketMesh;

	// ====================================================================
	// VARIABLES
	// ====================================================================
protected:
	// --- Movement Variables (From Part 2 Graph) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	double currentSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	double maxSpeed = 20000.0f; // High speed for a rocket

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	double SpeedInterp = 5000.0f; // Acceleration rate

	// --- Effects Variables (From Part 1 Graph) ---
	UPROPERTY(EditAnywhere, Category = "Combat")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditAnywhere, Category = "Combat")
	USoundBase* ExplosionSound;

	// Safety flag to prevent double explosions
	bool bHasExploded = false;

	// ====================================================================
	// FUNCTIONS
	// ====================================================================
protected:
	// Handles the acceleration and forward movement (Your "SetSpeed" logic)
	void SetSpeed(float DeltaTime);

	// The function bound to the Sphere's overlap event
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Custom event for handling the visual explosion
	void ExplosionSequence();

	// Final cleanup
	void DestroyRocket();
};