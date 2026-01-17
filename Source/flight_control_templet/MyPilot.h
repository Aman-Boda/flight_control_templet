#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyPilot.generated.h"

// Forward declarations
class USpringArmComponent;
class UCameraComponent;
class UAudioComponent;
class USoundBase;
class UParticleSystem;

UCLASS()
class FLIGHT_CONTROL_TEMPLET_API AMyPilot : public ACharacter
{
	GENERATED_BODY()

public:
	AMyPilot();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ====================================================================
	// COMPONENTS
	// ====================================================================
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* C_SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* C_Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* JetMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pilot Setup")
	USceneComponent* BulletSpawnPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pilot Setup")
	UStaticMeshComponent* LeftRocketMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pilot Setup")
	UStaticMeshComponent* RightRocketMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAudioComponent* M_EngineAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAudioComponent* M_MachineGunAudio;

	// ====================================================================
	// VARIABLES
	// ====================================================================
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	double currentSpeed = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	double targetSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	double maxSpeed = 6000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	double MinSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	double SpeedInterp = 2000.0f;

	// Rotation Internal
	double pitch = 0.0f;
	double roll = 0.0f;
	double yaw = 0.0f;

	// Rotation Current State
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	double pitch_adder = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	double roll_adder = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	double yaw_adder = 0.0f;

	// Rotation Limits
	UPROPERTY(EditAnywhere, Category = "Controls")
	double pitch_max = 60.0f;
	UPROPERTY(EditAnywhere, Category = "Controls")
	double roll_max = 120.0f;
	UPROPERTY(EditAnywhere, Category = "Controls")
	double yaw_max = 40.0f;

	// Rotation Interp Speeds
	UPROPERTY(EditAnywhere, Category = "Controls")
	double interpPitch = 150.0f;
	UPROPERTY(EditAnywhere, Category = "Controls")
	double interpRoll = 100.0f;
	UPROPERTY(EditAnywhere, Category = "Controls")
	double interpYaw = 100.0f;

	// --- YOUR SPECIFIC VALUE (100) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	double SpringArmLength = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	double cameraLag = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	double springArmInterp = 5.0f;

	// Combat
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool isFiringBullet = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	double shootingSpeed = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<AActor> BulletClass;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<AActor> RocketClass;

	UPROPERTY(EditAnywhere, Category = "Combat")
	USoundBase* RocketLaunchSound;

	int32 RocketSpawnedID = 0;
	FTimerHandle RocketReloadTimer;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditAnywhere, Category = "Combat")
	USoundBase* ExplosionSound;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool isDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bWheelOpen = false;

	FTimerHandle FireTimerHandle;

	// ====================================================================
	// FUNCTIONS
	// ====================================================================
protected:
	void ForwardMovement(float DeltaTime);
	void RotationControl(float DeltaTime);
	void CalculateRotationRates(float DeltaTime);
	void UpdateSpeedAndEffects(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void FireMachineGun();

	void FireRocket();
	void ReloadRockets();
	void ToggleWheels();
	void DestroyPilot();

	// Input Handlers
	void InputPitch(float Val);
	void InputRoll(float Val);
	void InputYaw(float Val);
	void InputSpeed(float Val);
	void StartFiring();
	void StopFiring();

	UFUNCTION()
	void OnJetOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};