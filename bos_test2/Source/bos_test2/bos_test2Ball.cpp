// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "bos_test2.h"
#include "bos_test2Ball.h"
#include "Engine.h"
#include "MyProjectile.h"


Abos_test2Ball::Abos_test2Ball()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BallMesh(TEXT("/Game/Rolling/Meshes/BallMesh.BallMesh"));

	SetActorTickEnabled(true);

	// Create mesh component for the ball
	Ball = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ball0"));
	Ball->SetStaticMesh(BallMesh.Object);
	Ball->BodyInstance.SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	Ball->SetSimulatePhysics(true);
	Ball->SetAngularDamping(0.1f);
	Ball->SetLinearDamping(0.1f);
	Ball->BodyInstance.MassScale = 3.5f;
	Ball->BodyInstance.MaxAngularVelocity = 800.0f;
	Ball->SetNotifyRigidBodyCollision(true);
	RootComponent = Ball;

	// Create a camera boom attached to the root (ball)
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->AttachTo(RootComponent);
	SpringArm->bDoCollisionTest = true;
	SpringArm->bAbsoluteRotation = true; // Rotation of the ball should not affect rotation of boom
	SpringArm->RelativeRotation = FRotator(-45.f, 0.f, 0.f);
	SpringArm->TargetArmLength = 1200.f;
	SpringArm->bEnableCameraLag = false;
	SpringArm->CameraLagSpeed = 3.f;

	// Create a camera and attach to boom
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->AttachTo(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // We don't want the controller rotating the camera

	// Set up forces
	RollTorque = 50000000.0f;
	JumpImpulse = 350000.0f;
	MaxDashImpulse = 1000000.0f;
	DashChargeRate = 500000.0f;
	DashImpulse = 0.0f;
	bCanJump = true; // Start being able to jump
}

void Abos_test2Ball::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (dashCharging)
		DashImpulse += DashChargeRate;
	DashImpulse = FMath::Clamp(DashImpulse, 0.0f, MaxDashImpulse);
}

void Abos_test2Ball::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	InputComponent->BindAxis("YawCamera", this, &Abos_test2Ball::YawCamera);
	InputComponent->BindAxis("PitchCamera", this, &Abos_test2Ball::PitchCamera);
	InputComponent->BindAxis("MoveForward", this, &Abos_test2Ball::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &Abos_test2Ball::MoveRight);

	InputComponent->BindAction("Dash", IE_Pressed, this, &Abos_test2Ball::DashCharge);
	InputComponent->BindAction("Dash", IE_Released, this, &Abos_test2Ball::DashRelease);
	InputComponent->BindAction("Jump", IE_Pressed, this, &Abos_test2Ball::Jump);
	InputComponent->BindAction("Fire", IE_Pressed, this, &Abos_test2Ball::Fire);
}

void Abos_test2Ball::YawCamera(float Val)
{
	FRotator newRotation = SpringArm->GetComponentRotation();
	newRotation.Yaw += Val;
	SpringArm->SetRelativeRotation(newRotation);
}

void Abos_test2Ball::PitchCamera(float Val)
{
	FRotator newRotation = SpringArm->GetComponentRotation();
	newRotation.Pitch = FMath::Clamp(newRotation.Pitch + Val, -80.0f, 80.0f);
	SpringArm->SetRelativeRotation(newRotation);
}

void Abos_test2Ball::MoveRight(float Val)
{
	FVector Direction = SpringArm->GetForwardVector();
	Direction.Z = 0;
	const FVector Torque = Direction * -FMath::Clamp(Val, -1.0f, 1.0f) * RollTorque;
	Ball->AddTorque(Torque);
}

void Abos_test2Ball::MoveForward(float Val)
{
	FVector Direction = SpringArm->GetRightVector();
	Direction.Z = 0;
	const FVector Torque = Direction * 	FMath::Clamp(Val, -1.0f, 1.0f) * RollTorque;
	Ball->AddTorque(Torque);	
}

void Abos_test2Ball::DashCharge()
{
	dashCharging = true;
}

void Abos_test2Ball::DashRelease()
{
	const FVector Impulse = SpringArm->GetForwardVector() * DashImpulse;
	Ball->AddImpulse(Impulse);
	DashImpulse = 0.0f;
	dashCharging = false;
}

void Abos_test2Ball::Jump()
{
	if(bCanJump)
	{
		const FVector Impulse = FVector(0.f, 0.f, JumpImpulse);
		Ball->AddImpulse(Impulse);
		bCanJump = false;
	}
}

void Abos_test2Ball::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	bCanJump = true;
}


void Abos_test2Ball::Fire()
{
	TSubclassOf<class AMyProjectile> ProjectileClass;
	//if (ProjectileClass != NULL)
//	{
		const FRotator SpawnRotation = SpringArm->GetComponentRotation();
		//const FRotator SpawnRotation = GetControlRotation();
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		const FVector SpawnLocation = GetActorLocation() + FVector(50.0, 50.0f, 50.0f);

		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			// spawn the projectile at the muzzle
			World->SpawnActor<AMyProjectile>(AMyProjectile::StaticClass(), SpawnLocation, SpawnRotation);
		}
//	}	
}

