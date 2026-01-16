// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Pilot.h"

// Sets default values
AC_Pilot::AC_Pilot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AC_Pilot::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AC_Pilot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AC_Pilot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

