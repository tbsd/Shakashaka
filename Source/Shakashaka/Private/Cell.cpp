// Fill out your copyright notice in the Description page of Project Settings.


#include "Cell.h"

// Sets default values
ACell::ACell()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	mCellMesh = CreateDefaultSubobject<UStaticMeshComponent>("CellMesh");
	SetRootComponent(mCellMesh);
}

// Called when the game starts or when spawned
void ACell::BeginPlay()
{
	Super::BeginPlay();

    SetMeshComponent();
	
}

// Called every frame
void ACell::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACell::SetMeshComponent() {
	// HEHMDA add change mesh
    /*
   static ConstructorHelpers::FObjectFinder<UStaticMesh> cube(TEXT("'/Engine/BasicShapes/Cylinder.Cylinder'"));
   
    
   UE_LOG(LogTemp, Warning, TEXT("HEHMDA ACell SetMeshComponent"));
    // check if path is valid
    if (cube.Succeeded())
    {
		UE_LOG(LogTemp, Warning, TEXT("HEHMDA ACell SetMeshComponent succeeded"));
    	// mesh = valid path
    	mCellMesh->SetStaticMesh(cube.Object);
    	// set relative location of mesh
    	mCellMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    }
    */
}

