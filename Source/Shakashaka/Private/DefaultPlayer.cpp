// Fill out your copyright notice in the Description page of Project Settings.

#include "DefaultPlayer.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
ADefaultPlayer::ADefaultPlayer()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//Create our components
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	StaticMeshComp = CreateDefaultSubobject <UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));

	//Attach our components
	StaticMeshComp->SetupAttachment(RootComponent);
	SpringArmComp->SetupAttachment(StaticMeshComp);
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);

	//Assign SpringArm class variables.
	SpringArmComp->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 50.0f), FRotator(-88.0f, 0.0f, 0.0f));
	SpringArmComp->TargetArmLength = 1400.f;
	SpringArmComp->bEnableCameraLag = false;
	ZoomFactor = 0.5f;

}

// Called when the game starts or when spawned
void ADefaultPlayer::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ADefaultPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ZoomFactor = FMath::Clamp<float>(ZoomFactor, 0.0f, 1.0f);

	// Zoom
	CameraComp->FieldOfView = FMath::Lerp<float>(90.0f, 60.0f, ZoomFactor);
	SpringArmComp->TargetArmLength = FMath::Lerp<float>(1400.0f, 200.0f, ZoomFactor);

	// Rotation
	if (mRotateBtnPressed) {
		FTransform player_transform = GetActorTransform();
		auto rot = player_transform.GetRotation();
		FVector2D delta_rot;
		delta_rot.X = DeltaTime * CameraInput.X * mRotSpeed;
		delta_rot.Y = DeltaTime * CameraInput.Y * mRotSpeed;
		FRotator forwad_rot(0.0f, 0.0f, delta_rot.X);
		FVector actor_up = GetActorUpVector();
		FVector rotated_up = forwad_rot.RotateVector(actor_up);
		if (rot.X >= 0.0f && delta_rot.X < 0.0f || FVector::DotProduct(actor_up, { 0.0f, 0.0f, 1.0f }) < 0.2f && delta_rot.X > 0.0f)
			delta_rot.X = 0.0f;
		{

			FRotator NewRotation = GetActorRotation();
			NewRotation.Roll += delta_rot.X;
			SetActorRotation(NewRotation);
		}
		{
			FRotator NewRotation = GetActorRotation();
			NewRotation.Yaw += delta_rot.Y;
			SetActorRotation(NewRotation);
		}
	}
}

// Called to bind functionality to input
void ADefaultPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent->BindAction("RotateBtn", IE_Pressed, this, &ADefaultPlayer::RotateBtnPressed);
	InputComponent->BindAction("RotateBtn", IE_Released, this, &ADefaultPlayer::RotateBtnReleased);

	InputComponent->BindAxis("Zoom", this, &ADefaultPlayer::Zoom);
	InputComponent->BindAxis("RotateForward", this, &ADefaultPlayer::RotateForward);
	InputComponent->BindAxis("RotateRight", this, &ADefaultPlayer::RotateRight);
}
//Input functions
void ADefaultPlayer::RotateForward(float AxisValue)
{
	CameraInput.X = AxisValue;
}

void ADefaultPlayer::RotateRight(float AxisValue)
{
	CameraInput.Y = AxisValue;
}

void ADefaultPlayer::RotateBtnPressed() {
	mRotateBtnPressed = true;
}

void ADefaultPlayer::RotateBtnReleased() {
	mRotateBtnPressed = false;
}

void ADefaultPlayer::Zoom(float AxisValue)
{
	if (AxisValue > 0.1f) {
		ZoomFactor += 0.1f;
	}
	else if (AxisValue < -0.1f) {
		ZoomFactor -= 0.1f;
	}
}

