#include "Rocket.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h" // Needed for FInterp
#include "TimerManager.h"

// Sets default values
ARocket::ARocket()
{
	// Set this actor to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	// 1. Setup Collision Sphere
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	RootComponent = CollisionSphere;

	// Configure Physics/Collision
	CollisionSphere->SetSphereRadius(20.0f);
	CollisionSphere->SetCollisionProfileName(TEXT("BlockAllDynamic")); // or "OverlapAllDynamic" depending on your needs
	CollisionSphere->SetGenerateOverlapEvents(true);

	// 2. Setup Rocket Mesh
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	RocketMesh->SetupAttachment(RootComponent);
	// Mesh should not handle collision, the sphere does
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ARocket::BeginPlay()
{
	Super::BeginPlay();

	// [Part 1 Logic] Set Life Span (Auto-destroy after 30s if it hits nothing)
	SetLifeSpan(30.0f);

	// Bind the Overlap Event
	if (CollisionSphere)
	{
		CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ARocket::OnOverlap);
	}
}

// Called every frame
void ARocket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// [Part 2 Logic] Only move if we haven't exploded
	if (!bHasExploded)
	{
		SetSpeed(DeltaTime);
	}
}

// [Part 2 Logic] Acceleration and Movement
void ARocket::SetSpeed(float DeltaTime)
{
	// 1. Interpolate Speed (Acceleration)
	// Logic: FInterpTo_Constant (Current -> Max)
	if (currentSpeed < maxSpeed)
	{
		currentSpeed = FMath::FInterpConstantTo(currentSpeed, maxSpeed, DeltaTime, SpeedInterp);
	}

	// 2. Apply Movement
	// Logic: DeltaLocation = Forward * Speed * Time
	// Since we use AddActorLocalOffset, X is always Forward.
	double MoveDistance = currentSpeed * DeltaTime;
	FVector DeltaLocation(MoveDistance, 0.0f, 0.0f);

	// Sweep = true so we stop if we hit a wall
	AddActorLocalOffset(DeltaLocation, true);
}

// [Part 1 Logic] Collision Event
void ARocket::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Ensure we hit something valid (not ourselves, not null)
	if (OtherActor && OtherActor != this && !bHasExploded)
	{
		ExplosionSequence();
	}
}

// [Part 1 Logic] Explosion Sequence
void ARocket::ExplosionSequence()
{
	bHasExploded = true;

	// 1. Hide the Rocket Mesh (it "exploded")
	if (RocketMesh)
	{
		RocketMesh->SetVisibility(false);
	}

	// 2. Spawn Particle Effect
	if (ExplosionEffect)
	{
		// Scale is 10.0f based on your blueprint
		UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionEffect, GetActorLocation(), GetActorRotation(), FVector(10.0f));
	}

	// 3. Play Sound
	if (ExplosionSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ExplosionSound, GetActorLocation());
	}

	// 4. Disable Collision (so the invisible rocket doesn't block player)
	if (CollisionSphere)
	{
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 5. Delay 5 seconds (to let sound finish), then Destroy
	FTimerHandle DestroyTimer;
	GetWorld()->GetTimerManager().SetTimer(DestroyTimer, this, &ARocket::DestroyRocket, 5.0f, false);
}

void ARocket::DestroyRocket()
{
	Destroy();
}