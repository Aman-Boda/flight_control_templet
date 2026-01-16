#include "MyPilot.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h" 
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"

AMyPilot::AMyPilot()
{
	PrimaryActorTick.bCanEverTick = true;

	// RENAMED COMPONENTS
	C_SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("C_SpringArm"));
	C_SpringArm->SetupAttachment(RootComponent);

	C_Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("C_Camera"));
	C_Camera->SetupAttachment(C_SpringArm);

	M_EngineAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("M_EngineAudio"));
	M_EngineAudio->SetupAttachment(RootComponent);

	M_MachineGunAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("M_MachineGunAudio"));
	M_MachineGunAudio->SetupAttachment(RootComponent);
	M_MachineGunAudio->bAutoActivate = false;
}

void AMyPilot::BeginPlay()
{
	Super::BeginPlay();

	targetSpeed = MinSpeed;
	currentSpeed = MinSpeed;

	if (!JetMesh) JetMesh = GetMesh();

	if (JetMesh)
	{
		JetMesh->OnComponentBeginOverlap.AddDynamic(this, &AMyPilot::OnJetOverlap);
	}

	TArray<USceneComponent*> AllComps;
	GetComponents<USceneComponent>(AllComps);

	for (USceneComponent* Comp : AllComps)
	{
		FString CompName = Comp->GetName();
		if (CompName.Contains("Arrow") || CompName.Contains("Bullet") || CompName.Contains("Spawn"))
		{
			BulletSpawnPoint = Comp;
		}
		if (Comp->IsA(UStaticMeshComponent::StaticClass()))
		{
			UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Comp);
			if (CompName.Contains("Left") || CompName.Contains("L_")) LeftRocketMesh = MeshComp;
			else if (CompName.Contains("Right") || CompName.Contains("R_")) RightRocketMesh = MeshComp;
		}
	}
}

void AMyPilot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (isDead) return;

	CalculateRotationRates(DeltaTime);
	UpdateSpeedAndEffects(DeltaTime);
	ForwardMovement(DeltaTime);
	RotationControl(DeltaTime);
}

void AMyPilot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("pitch", this, &AMyPilot::InputPitch);
	PlayerInputComponent->BindAxis("roll", this, &AMyPilot::InputRoll);
	PlayerInputComponent->BindAxis("yaw", this, &AMyPilot::InputYaw);
	PlayerInputComponent->BindAxis("speed", this, &AMyPilot::InputSpeed);

	PlayerInputComponent->BindAction("shoot_bullet", IE_Pressed, this, &AMyPilot::StartFiring);
	PlayerInputComponent->BindAction("shoot_bullet", IE_Released, this, &AMyPilot::StopFiring);
	PlayerInputComponent->BindAction("shoot_rocket", IE_Pressed, this, &AMyPilot::FireRocket);
	PlayerInputComponent->BindAction("open_wheels", IE_Pressed, this, &AMyPilot::ToggleWheels);
}

void AMyPilot::InputPitch(float Val) { pitch = Val; }
void AMyPilot::InputRoll(float Val) { roll = Val; }
void AMyPilot::InputYaw(float Val) { yaw = Val; }

void AMyPilot::InputSpeed(float Val)
{
	if (Val >= 0.1f) targetSpeed = maxSpeed;
	else if (Val <= -0.1f) targetSpeed = MinSpeed;
	else targetSpeed = currentSpeed;
}

void AMyPilot::CalculateRotationRates(float DeltaTime)
{
	double TargetPitch = pitch * pitch_max;
	double PitchInterpSpeed = FMath::Abs(pitch_adder) + interpPitch;
	pitch_adder = FMath::FInterpConstantTo(pitch_adder, TargetPitch, DeltaTime, PitchInterpSpeed);
	if (FMath::Abs(pitch_adder) > pitch_max) pitch_adder = pitch_max * FMath::Sign(pitch_adder);

	double TargetRoll = roll * roll_max;
	double RollInterpSpeed = FMath::Abs(roll_adder) + interpRoll;
	roll_adder = FMath::FInterpConstantTo(roll_adder, TargetRoll, DeltaTime, RollInterpSpeed);
	if (FMath::Abs(roll_adder) > roll_max) roll_adder = roll_max * FMath::Sign(roll_adder);

	double TargetYaw = yaw * yaw_max;
	double YawInterpSpeed = FMath::Abs(yaw_adder) + interpYaw;
	yaw_adder = FMath::FInterpConstantTo(yaw_adder, TargetYaw, DeltaTime, YawInterpSpeed);
	if (FMath::Abs(yaw_adder) > yaw_max) yaw_adder = yaw_max * FMath::Sign(yaw_adder);
}

void AMyPilot::ForwardMovement(float DeltaTime)
{
	double MoveAmount = currentSpeed * DeltaTime;
	FVector DeltaLocation(MoveAmount, 0.0f, 0.0f);
	AddActorLocalOffset(DeltaLocation, true);
}

void AMyPilot::RotationControl(float DeltaTime)
{
	FRotator DeltaRot;
	DeltaRot.Roll = roll_adder * DeltaTime;
	DeltaRot.Pitch = pitch_adder * DeltaTime;
	DeltaRot.Yaw = yaw_adder * DeltaTime;
	AddActorLocalRotation(DeltaRot);

	if (C_SpringArm) // Updated variable
	{
		double SpeedDivisor = (currentSpeed * 2.0 <= 1.0f) ? 1.0f : (currentSpeed * 2.0);
		double Intensity = maxSpeed / SpeedDivisor;

		FRotator TargetCamRot;
		TargetCamRot.Roll = roll_adder * cameraLag * Intensity;
		TargetCamRot.Pitch = pitch_adder * cameraLag * Intensity;
		TargetCamRot.Yaw = yaw_adder * cameraLag * Intensity;

		FRotator NewCamRot = FMath::RInterpTo(C_SpringArm->GetRelativeRotation(), TargetCamRot, DeltaTime, springArmInterp);
		C_SpringArm->SetRelativeRotation(NewCamRot);
	}
}

void AMyPilot::UpdateSpeedAndEffects(float DeltaTime)
{
	currentSpeed = FMath::FInterpConstantTo(currentSpeed, targetSpeed, DeltaTime, SpeedInterp);

	if (C_SpringArm) // Updated variable
	{
		double SpeedRatio = currentSpeed / maxSpeed;
		double ZoomFactor = (SpeedRatio * 5.0f) + 1.0f;
		C_SpringArm->TargetArmLength = ZoomFactor * springarmLengh;
	}

	if (M_EngineAudio)
	{
		double SpeedRatio = currentSpeed / maxSpeed;
		float NewPitch = 1.0f + (float)SpeedRatio;
		float NewVolume = (float)SpeedRatio * 0.5f;
		if (NewVolume < 0.2f) NewVolume = 0.2f;

		M_EngineAudio->SetPitchMultiplier(NewPitch);
		M_EngineAudio->SetVolumeMultiplier(NewVolume);
	}
}

void AMyPilot::StartFiring()
{
	isFiringBullet = true;
	FireMachineGun(); // Updated Name
	if (M_MachineGunAudio) M_MachineGunAudio->Play();
}

void AMyPilot::StopFiring()
{
	isFiringBullet = false;
	if (M_MachineGunAudio) M_MachineGunAudio->Stop();
}

void AMyPilot::FireMachineGun() // Updated Name
{
	if (!isFiringBullet)
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
		return;
	}

	if (BulletSpawnPoint && BulletClass)
	{
		FVector SpawnLoc = BulletSpawnPoint->GetComponentLocation();
		FRotator SpawnRot = BulletSpawnPoint->GetComponentRotation();

		SpawnRot.Pitch += UKismetMathLibrary::RandomFloatInRange(-1.0f, 1.0f);
		SpawnRot.Yaw += UKismetMathLibrary::RandomFloatInRange(-1.0f, 1.0f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		AActor* SpawnedBullet = GetWorld()->SpawnActor<AActor>(BulletClass, SpawnLoc, SpawnRot, SpawnParams);

		if (SpawnedBullet)
		{
			TArray<UPrimitiveComponent*> Comps;
			SpawnedBullet->GetComponents(Comps);
			for (auto Comp : Comps) Comp->IgnoreActorWhenMoving(this, true);
		}
	}

	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &AMyPilot::FireMachineGun, shootingSpeed, false);
}

void AMyPilot::FireRocket()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(RocketReloadTimer)) return;

	UStaticMeshComponent* SelectedMesh = (RocketSpawnedID == 0) ? RightRocketMesh : LeftRocketMesh;
	RocketSpawnedID = (RocketSpawnedID == 0) ? 1 : 0;

	if (SelectedMesh && RocketClass)
	{
		FVector SpawnLoc = SelectedMesh->GetComponentLocation();
		FRotator SpawnRot = SelectedMesh->GetComponentRotation();

		SelectedMesh->SetVisibility(false);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		AActor* SpawnedRocket = GetWorld()->SpawnActor<AActor>(RocketClass, SpawnLoc, SpawnRot, SpawnParams);

		if (RocketLaunchSound)
		{
			UGameplayStatics::SpawnSoundAtLocation(this, RocketLaunchSound, SpawnLoc);
		}

		if (SpawnedRocket)
		{
			TArray<UPrimitiveComponent*> Comps;
			SpawnedRocket->GetComponents(Comps);
			for (auto Comp : Comps) Comp->IgnoreActorWhenMoving(this, true);
		}
	}

	GetWorld()->GetTimerManager().SetTimer(RocketReloadTimer, this, &AMyPilot::ReloadRockets, 3.0f, false);
}

void AMyPilot::ReloadRockets()
{
	if (LeftRocketMesh) LeftRocketMesh->SetVisibility(true);
	if (RightRocketMesh) RightRocketMesh->SetVisibility(true);
}

void AMyPilot::ToggleWheels()
{
	bWheelOpen = !bWheelOpen;
}

void AMyPilot::OnJetOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (isDead) return;
	isDead = true;

	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionEffect, GetActorLocation(), GetActorRotation(), FVector(10.0f));
	}

	if (ExplosionSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ExplosionSound, GetActorLocation());
	}

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	if (GameOverWidgetClass)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			UUserWidget* Menu = CreateWidget<UUserWidget>(PC, GameOverWidgetClass);
			if (Menu)
			{
				Menu->AddToViewport();
				PC->bShowMouseCursor = true;
			}
		}
	}

	FTimerHandle DestroyTimer;
	GetWorld()->GetTimerManager().SetTimer(DestroyTimer, this, &AMyPilot::DestroyPilot, 0.2f, false);
}

void AMyPilot::DestroyPilot()
{
	Destroy();
}