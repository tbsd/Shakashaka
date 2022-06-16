// Fill out your copyright notice in the Description page of Project Settings.
#include "TriangleCell.h"

#include "UObject/WeakObjectPtr.h"
#include "Components/BoxComponent.h" 


ATriangleCell::ATriangleCell() : ACell() {
}

void ATriangleCell::OnClick(UPrimitiveComponent* pComponent, FKey button) {
	//UE_LOG(LogTemp, Warning, TEXT("HEHMDA CLICK %s %s"), *(button.GetFName()), *(button.ToString()));
	UE_LOG(LogTemp, Warning, TEXT("HEHMDA CLICK %s , %s "), *(button.GetFName().ToString()), *(button.ToString()) );
	if (button.GetFName().ToString() == TEXT("LeftMouseButton"))
		ChangeToNext();
	else if (button.GetFName().ToString() == TEXT("RightMouseButton"))
		ChangeType(LpSolver::CellType::Empty);
}

LpSolver::CellType ATriangleCell::GetNextType() {
	switch (mType) {
	case LpSolver::CellType::Empty:
		return LpSolver::CellType::UpperLeft;
	case LpSolver::CellType::UpperLeft:
		return LpSolver::CellType::UpperRight;
	case LpSolver::CellType::UpperRight:
		return LpSolver::CellType::BottomRight;
	case LpSolver::CellType::BottomRight:
		return LpSolver::CellType::BottomLeft;
	case LpSolver::CellType::BottomLeft:
		return LpSolver::CellType::Empty;
	}
	UE_LOG(LogTemp, Warning, TEXT("GenNextType(): unexpected type"));
	return LpSolver::CellType::UpperLeft;
}
	
void ATriangleCell::ChangeType(LpSolver::CellType t) {
	mType = t;
	SetMeshComponent();
}

void ATriangleCell::ChangeToNext() {
	ChangeType(GetNextType());
}


void ATriangleCell::SetMeshComponent() {
	switch (mType) {
	case LpSolver::CellType::Empty:
		mCellMesh->SetStaticMesh(mEmpty);
		break;
	case LpSolver::CellType::UpperLeft:
		mCellMesh->SetStaticMesh(mUpperLeft);
		break;
	case LpSolver::CellType::UpperRight:
		mCellMesh->SetStaticMesh(mUpperRight);
		break;
	case LpSolver::CellType::BottomRight:
		mCellMesh->SetStaticMesh(mBottomRight);
		break;
	case LpSolver::CellType::BottomLeft:
		mCellMesh->SetStaticMesh(mBottomLeft);
		break;
	}
	mCellMesh->OnClicked.AddUniqueDynamic(this, &ATriangleCell::OnClick);
}

LpSolver::CellType ATriangleCell::GetType() {
	return mType;
}


