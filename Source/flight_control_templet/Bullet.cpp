#include "Bullet.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
ABullet::ABullet()
{
	// Bullets don't need to tick every frame if we use ProjectileMovement
	PrimaryActorTick.bCanEverTick = false;

	// 1. Collision Sphere
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	RootComponent = CollisionSphere;

	// Setup Collision (Matches your Rocket logic)
	CollisionSphere->SetSphereRadius(10.0f);
	CollisionSphere->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionSphere->SetGenerateOverlapEvents(true);

	// 2. Visual Mesh
	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
	BulletMesh->SetupAttachment(RootComponent);
	BulletMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Make the bullet look like a bullet (Scale it down)
	BulletMesh->SetWorldScale3D(FVector(0.5f));

	// 3. Movement Component (Auto-Flight)
	BulletMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("BulletMovement")); // Renamed
	BulletMovement->UpdatedComponent = CollisionSphere;
	BulletMovement->InitialSpeed = 8000.0f;
	BulletMovement->MaxSpeed = 8000.0f;
	BulletMovement->bRotationFollowsVelocity = true;
	BulletMovement->bShouldBounce = false;
	BulletMovement->ProjectileGravityScale = 0.0f;
}

// Called when the game starts or when spawned
void ABullet::BeginPlay()
{
	Super::BeginPlay();

	// [Logic from Graph] Set Life Span (5.0 seconds)
	SetLifeSpan(5.0f);

	// [Logic from Graph] Bind Overlap
	if (CollisionSphere)
	{
		CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ABullet::OnOverlap);
	}
}

void ABullet::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// [Logic from Graph] Destroy Actor on impact
	// Ignore self to prevent instant deletion on spawn
	if (OtherActor && OtherActor != this)
	{
		Destroy();
	}
}