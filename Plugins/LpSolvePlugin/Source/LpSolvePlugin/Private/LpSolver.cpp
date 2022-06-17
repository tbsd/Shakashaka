// Fill out your copyright notice in the Description page of Project Settings.

#include "LpSolver.h"

#include <vector>
#include <functional>
#include <chrono>

#include "Containers/UnrealString.h" 

LpSolver::LpSolver()
{
}

LpSolver::~LpSolver()
{
}
bool LpSolver::IsBlack(int x, int y) {
	CellType type = (*mInitBoard)[x][y];
	return type == CellType::Black1 || type == CellType::Black2 || type == CellType::Black3 || type == CellType::Black4 || type == CellType::Black;
}

bool LpSolver::IsBlack(int n) {
	int x = (n / 5) % mWidth;
	int y = (n / 5) / mWidth;
	return IsBlack(x, y);
}

void LpSolver::InitBoard(std::shared_ptr<LpSolver::GameBoard> board){
	mInitBoard = board;
	mSolution = nullptr;
	mTimeElapsed = 0;
	if (!mInitBoard || mInitBoard->empty())
		return;
	mWidth = mInitBoard->size();
	mHeight = (*mInitBoard)[0].size();
}

int LpSolver::GetVarNumber(int x_empty, LpSolver::CellType type) {
	switch (type)
	{
	case CellType::Empty:
		return x_empty;     // e
	case CellType::UpperLeft:
		return x_empty + 1; // ul
	case CellType::UpperRight:
		return x_empty + 2; // ur
	case CellType::BottomRight:
		return x_empty + 3; // br
	case CellType::BottomLeft:
		return x_empty + 4; // bl
	default:
		UE_LOG(LogTemp, Error, TEXT("LpSolver::Solve(): unexpected type in constrained!"));
		break;
	}
	return x_empty;
}
int LpSolver::GetVarNumber(int x, int y, LpSolver::CellType type) {
	int x_empty = 1 + (x + y * mWidth) * 5;
	return GetVarNumber(x_empty, type);
}

bool LpSolver::Solve() {
	auto current_timestamp = []() {return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); };
	long long model_creation_start = current_timestamp();
	UE_LOG(LogTemp, Log, TEXT("LpSolver::Solver() construction start!"));
	if (!mInitBoard || mInitBoard->empty())
		return false;

	int n_col = mWidth * mHeight * 5; // var_num
	UE_LOG(LogTemp, Log, TEXT("LpSolver::Solver(): number of variables: %d"), n_col);
	if (n_col == 0)
		return false;

	lp = std::shared_ptr<lprec>(make_lp(0, n_col), [](lprec* p) {
		if (p) {
			delete_lp(p);
		}});

	if (lp == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("LpSolver::Solver(): can't create lp"));
		return false;
	}
	set_timeout(lp.get(), mTimeoutSec);

	int constraint_sz = -1;
	std::shared_ptr<int> colno;
	std::shared_ptr<REAL> row;
	auto set_colno_row = [&constraint_sz, &colno, &row](int sz) {
		//HEHMDA?
		//if (sz != constraint_sz) {// optimization to avoid extra memory allocation/free
		constraint_sz = sz;
		colno = std::shared_ptr<int>(new int[constraint_sz], std::default_delete<int[]>());
		row = std::shared_ptr<REAL>(new REAL[constraint_sz], std::default_delete<REAL[]>());
		//}
	};
	auto make_lhs = [&](int var_begin, const std::vector<CellType>& types, const std::vector<int>& coefs) {
		if (types.size() != coefs.size()) {
			UE_LOG(LogTemp, Error, TEXT("LpSolver::Solver() types.size() != coefs.size()"));
		}
		set_colno_row(types.size());
		for (int i = 0; i < types.size(); ++i) {
			colno.get()[i] = GetVarNumber(var_begin, types[i]);
			row.get()[i] = coefs[i];
		}

	};

	bool is_ok = true;

	auto iterate = [&](const std::function<void(int, int, CellType, int, int, int, int, int, int, int, int, int)>& fun) {
		for (int i = 0; i < mWidth; ++i) {
			for (int j = 0; j < mHeight; ++j) {
				auto type = (*mInitBoard)[i][j];
				int center = GetVarNumber(i, j);
				int left = GetVarNumber(i - 1, j);
				int right = GetVarNumber(i + 1, j);
				int upper = GetVarNumber(i, j - 1);
				int bottom = GetVarNumber(i, j + 1);
				int upper_left = GetVarNumber(i - 1, j - 1);
				int bottom_left = GetVarNumber(i - 1, j + 1);
				int upper_right = GetVarNumber(i + 1, j - 1);
				int bottom_right = GetVarNumber(i + 1, j + 1);
				fun(i, j, type, center, left, right, upper, bottom, upper_left, bottom_left, upper_right, bottom_right);
			}
		}

	};
	
	iterate([&](int i, int j, CellType type, int center, int left, int right, int upper, int bottom, int upper_left, int bottom_left, int upper_right, int bottom_right) {
		char name[20];
		char type_str[] = "12";
		for (int t = 0; t < 5; ++t) {
			set_binary(lp.get(), center + t, TRUE);
			switch (t % 5) {
			case 0:
				std::strcpy(type_str, "e");
				break;
			case 1:
				std::strcpy(type_str, "ul");
				break;
			case 2:
				std::strcpy(type_str, "ur");
				break;
			case 3:
				std::strcpy(type_str, "br");
				break;
			case 4:
				std::strcpy(type_str, "bl");
				break;
			}
			sprintf(name, "(%d,%d,%s)", i, j, type_str);
			set_col_name(lp.get(), center + t, name);
		}
	});

	set_colno_row(1);
	colno.get()[0] = 0;
	row.get()[0] = 1;
	if (!set_obj_fnex(lp.get(), 1, row.get(), colno.get())) {
		UE_LOG(LogTemp, Error, TEXT("LpSolver::Solver() can't add constraint on phase 10."));
		return false;
	}
	set_maxim(lp.get());
	set_break_at_first(lp.get(), TRUE);

	set_add_rowmode(lp.get(), TRUE);

	set_colno_row(5);
	// x_e + x_ul + x_ur + x_br + x_bl = 1
	iterate([&](int i, int j, CellType type, int center, int left, int right, int upper, int bottom, int upper_left, int bottom_left, int upper_right, int bottom_right) {
		make_lhs(center, { CellType::Empty, CellType::UpperLeft, CellType::UpperRight, CellType::BottomRight, CellType::BottomLeft }, { 1, 1, 1, 1, 1 });
		// if cell is black then it must not be triangle, so setting zero to eq
		if (!add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, static_cast<int>(type) > 5 ? 1 : 0)) {
			UE_LOG(LogTemp, Error, TEXT("LpSolver::Solver() can't add constraint on phase 0"));
			return;
		}
		});
	//write_lp(lp.get(), "C:\\Users\\tbsd\\Documents\\Unreal Projects\\ue\\Shakashaka\\model2.log");//HEHMDA

	// Fix triangle cells if any. Needed this to solve from board in the middle of the game
	iterate([&](int i, int j, CellType type, int center, int left, int right, int upper, int bottom, int upper_left, int bottom_left, int upper_right, int bottom_right) {
		if (static_cast<int>(type) >= 7 && static_cast<int>(type) <= 10) {
				make_lhs(center, { CellType::Empty }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 0));
				make_lhs(center, { CellType::UpperLeft }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, type == CellType::UpperLeft ? 1 : 0));
				make_lhs(center, { CellType::UpperRight }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, type == CellType::UpperRight ? 1 : 0));
				make_lhs(center, { CellType::BottomRight }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, type == CellType::BottomRight ? 1 : 0));
				make_lhs(center, { CellType::BottomLeft }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, type == CellType::BottomLeft ? 1 : 0));
		}
		});
	if (!is_ok)
		UE_LOG(LogTemp, Error, TEXT("LpSolver::Solver() can't add constraint on phase -1"));

	// x_i-1,j,e + x_i-1,j,ur + x_i-i,j,br = 1
	// x_i-1,j,ul = x_i-1,j,bl = 0
	// and 3 other sides. Black cells
	iterate([&](int i, int j, CellType type, int center, int left, int right, int upper, int bottom, int upper_left, int bottom_left, int upper_right, int bottom_right) {
		if (type == CellType::Black) {
			if (i > 0 && !IsBlack(left)) {
				make_lhs(left, { CellType::Empty, CellType::UpperRight, CellType::BottomRight }, { 1, 1, 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 1));
				make_lhs(left, { CellType::UpperLeft }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 0));
				make_lhs(left, { CellType::BottomLeft }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 0));
			}
			if (i < mWidth - 1 && !IsBlack(right)) {
				make_lhs(right, { CellType::Empty, CellType::UpperLeft, CellType::BottomLeft }, { 1, 1, 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 1));
				make_lhs(right, { CellType::UpperRight }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 0));
				make_lhs(right, { CellType::BottomRight }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 0));
			}
			if (j > 0 && !IsBlack(upper)) {
				make_lhs(upper, { CellType::Empty, CellType::BottomLeft, CellType::BottomRight }, { 1, 1, 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 1));
				make_lhs(upper, { CellType::UpperLeft }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 0));
				make_lhs(upper, { CellType::UpperRight }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 0));
			}
			if (j < mHeight - 1 && !IsBlack(bottom)) {
				make_lhs(bottom, { CellType::Empty, CellType::UpperLeft, CellType::UpperRight }, { 1, 1, 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 1));
				make_lhs(bottom, { CellType::BottomLeft }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 0));
				make_lhs(bottom, { CellType::BottomRight }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 0));
			}
			if (!is_ok) {
				UE_LOG(LogTemp, Error, TEXT("LpSolver::Solver() can't add constraint on phase 1. i: %d j: %d type: %d"), i, j, static_cast<int>(type));
				return;
			}
		}
		});
	//write_lp(lp.get(), "C:\\Users\\tbsd\\Documents\\Unreal Projects\\ue\\Shakashaka\\model3.log");//HEHMDA

	// numbered cells
	iterate([&](int i, int j, CellType type, int center, int left, int right, int upper, int bottom, int upper_left, int bottom_left, int upper_right, int bottom_right) {
		if (type == CellType::Black1 || type == CellType::Black2 || type == CellType::Black3 || type == CellType::Black4) {
			int vars_count = 0;
			std::shared_ptr<int> left_colno, right_colno, upper_colno, bottom_colno;
			std::shared_ptr<REAL> left_row, right_row, upper_row, bottom_row;
			if (i > 0 && !IsBlack(left)) {
				make_lhs(left, { CellType::UpperRight, CellType::BottomRight }, { 1, 1 });
				left_colno = colno;
				left_row = row;
				vars_count += 2;
				make_lhs(left, { CellType::UpperLeft }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 0));
				make_lhs(left, { CellType::BottomLeft }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 0));
			}
			if (i < mWidth - 1 && !IsBlack(right)) {
				make_lhs(right, { CellType::UpperLeft, CellType::BottomLeft }, { 1, 1 });
				right_colno = colno;
				right_row = row;
				vars_count += 2;
				make_lhs(right, { CellType::UpperRight }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 0));
				make_lhs(right, { CellType::BottomRight }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 0));
			}
			if (j > 0 && !IsBlack(upper)) {
				make_lhs(upper, { CellType::BottomLeft, CellType::BottomRight }, { 1, 1 });
				upper_colno = colno;
				upper_row = row;
				vars_count += 2;
				make_lhs(upper, { CellType::UpperLeft }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 0));
				make_lhs(upper, { CellType::UpperRight }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 0));
			}
			if (j < mHeight - 1 && !IsBlack(bottom)) {
				make_lhs(bottom, { CellType::UpperLeft, CellType::UpperRight }, { 1, 1 });
				bottom_colno = colno;
				bottom_row = row;
				vars_count += 2;
				make_lhs(bottom, { CellType::BottomLeft }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 0));
				make_lhs(bottom, { CellType::BottomRight }, { 1 });
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), EQ, 0));
			}
			auto colno_full = std::shared_ptr<int>(new int[vars_count], std::default_delete<int[]>());
			auto row_full = std::shared_ptr<REAL>(new REAL[vars_count], std::default_delete<REAL[]>());
			int k = 0;
			auto copy_vals = [&](std::shared_ptr<int> src_colno, std::shared_ptr<REAL> src_row) {
				if (src_colno) {
					colno_full.get()[k] = src_colno.get()[0];
					row_full.get()[k++] = src_row.get()[0];
					colno_full.get()[k] = src_colno.get()[1];
					row_full.get()[k++] = src_row.get()[1];
				}
			};
			copy_vals(left_colno, left_row);
			copy_vals(right_colno, right_row);
			copy_vals(upper_colno, upper_row);
			copy_vals(bottom_colno, bottom_row);
			is_ok &= static_cast<bool>(add_constraintex(lp.get(), vars_count, row_full.get(), colno_full.get(), EQ, static_cast<int>(type)));
			if (!is_ok) {
				UE_LOG(LogTemp, Error, TEXT("LpSolver::Solver() can't add constraint on phase 2. i: %d j: %d type: %d"), i, j, static_cast<int>(type));
				return;
			}
		}		});
	//write_lp(lp.get(), "C:\\Users\\tbsd\\Documents\\Unreal Projects\\ue\\Shakashaka\\model4.log");//HEHMDA

	// x_i,j,bl <= x_i+1,j,br + x_i+1,j+1,bl ; etc.
	iterate([&](int i, int j, CellType type, int center, int left, int right, int upper, int bottom, int upper_left, int bottom_left, int upper_right, int bottom_right) {
		if (!IsBlack(center)) {
			if (i < mWidth - 1 && j < mHeight - 1) { // bl
				set_colno_row(3);
				colno.get()[0] = GetVarNumber(center, CellType::BottomLeft);
				colno.get()[1] = GetVarNumber(right, CellType::BottomRight);
				colno.get()[2] = GetVarNumber(bottom_right, CellType::BottomLeft);
				row.get()[0] = 1;
				row.get()[1] = row.get()[2] = -1;
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), LE, 0));
			}
			else {
				{
					UE_LOG(LogTemp, Error, TEXT("LpSolver::Solver() can't add constraint on phase 4. i: %d j: %d type: %d"), i, j, static_cast<int>(type));
					return;
				}
			}
			if (i > 0 && j > 0) { // ur
				set_colno_row(3);
				colno.get()[0] = GetVarNumber(center, CellType::UpperRight);
				colno.get()[1] = GetVarNumber(left, CellType::UpperLeft);
				colno.get()[2] = GetVarNumber(upper_left, CellType::UpperRight);
				row.get()[0] = 1;
				row.get()[1] = row.get()[2] = -1;
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), LE, 0));
			}
			else {
				{
					UE_LOG(LogTemp, Error, TEXT("LpSolver::Solver() can't add constraint on phase 5. i: %d j: %d type: %d"), i, j, static_cast<int>(type));
					return;
				}
			}
			if (i > 0 && j < mHeight - 1) { // ul
				set_colno_row(3);
				colno.get()[0] = GetVarNumber(center, CellType::UpperLeft);
				colno.get()[1] = GetVarNumber(bottom, CellType::BottomLeft);
				colno.get()[2] = GetVarNumber(bottom_left, CellType::UpperLeft);
				row.get()[0] = 1;
				row.get()[1] = row.get()[2] = -1;
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), LE, 0));
			}
			else {
				{
					UE_LOG(LogTemp, Error, TEXT("LpSolver::Solver() can't add constraint on phase 6. i: %d j: %d type: %d"), i, j, static_cast<int>(type));
					return;
				}
			}
			if (i < mWidth - 1 && j > 0) { // br
				set_colno_row(3);
				colno.get()[0] = GetVarNumber(center, CellType::BottomRight);
				colno.get()[1] = GetVarNumber(upper, CellType::UpperRight);
				colno.get()[2] = GetVarNumber(upper_right, CellType::BottomRight);
				row.get()[0] = 1;
				row.get()[1] = row.get()[2] = -1;
				is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), LE, 0));
			}
			else {
				{
					UE_LOG(LogTemp, Error, TEXT("LpSolver::Solver() can't add constraint on phase 7. i: %d j: %d type: %d"), i, j, static_cast<int>(type));
					return;
				}
			}
			if (!is_ok) {
				UE_LOG(LogTemp, Error, TEXT("LpSolver::Solver() can't add constraint on phase 1. i: %d j: %d type: %d"), i, j, static_cast<int>(type));
				return;
			}

		}
		});
	//write_lp(lp.get(), "C:\\Users\\tbsd\\Documents\\Unreal Projects\\ue\\Shakashaka\\model5.log");//HEHMDA

	// x_i,j,bl + x_i+1,j+1,bl <= x_i+1,j,e + 1 ; etc
	iterate([&](int i, int j, CellType type, int center, int left, int right, int upper, int bottom, int upper_left, int bottom_left, int upper_right, int bottom_right) {
		if (!IsBlack(center)) {
			set_colno_row(3);
			row.get()[0] = row.get()[1] = 1;
			row.get()[2] = -1;
			colno.get()[0] = GetVarNumber(center, CellType::BottomLeft);
			colno.get()[1] = GetVarNumber(bottom_right, CellType::BottomLeft);
			colno.get()[2] = GetVarNumber(right, CellType::Empty);
			is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), LE, 1));
			row.get()[0] = row.get()[1] = 1;
			row.get()[2] = -1;
			colno.get()[0] = GetVarNumber(center, CellType::UpperLeft);
			colno.get()[1] = GetVarNumber(bottom_left, CellType::UpperLeft);
			colno.get()[2] = GetVarNumber(bottom, CellType::Empty);
			is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), LE, 1));
			row.get()[0] = row.get()[1] = 1;
			row.get()[2] = -1;
			colno.get()[0] = GetVarNumber(center, CellType::UpperRight);
			colno.get()[1] = GetVarNumber(upper_left, CellType::UpperRight);
			colno.get()[2] = GetVarNumber(left, CellType::Empty);
			is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), LE, 1));
			row.get()[0] = row.get()[1] = 1;
			row.get()[2] = -1;
			colno.get()[0] = GetVarNumber(center, CellType::BottomRight);
			colno.get()[1] = GetVarNumber(upper_right, CellType::BottomRight);
			colno.get()[2] = GetVarNumber(upper, CellType::Empty);
			is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), LE, 1));
			if (!is_ok) {
				UE_LOG(LogTemp, Error, TEXT("LpSolver::Solver() can't add constraint on phase 8. i: %d j: %d type: %d"), i, j, static_cast<int>(type));
				return;
			}
		}
		});
	//write_lp(lp.get(), "C:\\Users\\tbsd\\Documents\\Unreal Projects\\ue\\Shakashaka\\model6.log");//HEHMDA

			// x_i,j,ul + x_i+k,j+k,ul <= sum_0<k'<k (x_i+k',j+k',br) + 1 ; n
	// x_i,j,e + x_i+1,j,e + x_i,j+1,e <= x_i+1,j+1,e + x_i+1,j+1,br + 2 ; etc
	iterate([&](int i, int j, CellType type, int center, int left, int right, int upper, int bottom, int upper_left, int bottom_left, int upper_right, int bottom_right) {
		if (!IsBlack(center)) {
			set_colno_row(5);
			row.get()[0] = row.get()[1] = row.get()[2] = 1;
			row.get()[3] = row.get()[4] = -1;
			colno.get()[0] = GetVarNumber(center, CellType::Empty);
			colno.get()[1] = GetVarNumber(right, CellType::Empty);
			colno.get()[2] = GetVarNumber(bottom, CellType::Empty);
			colno.get()[3] = GetVarNumber(bottom_right, CellType::Empty);
			colno.get()[4] = GetVarNumber(bottom_right, CellType::BottomRight);
			is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), LE, 2));
			row.get()[0] = row.get()[1] = row.get()[2] = 1;
			row.get()[3] = row.get()[4] = -1;
			colno.get()[0] = GetVarNumber(center, CellType::Empty);
			colno.get()[1] = GetVarNumber(left, CellType::Empty);
			colno.get()[2] = GetVarNumber(bottom, CellType::Empty);
			colno.get()[3] = GetVarNumber(bottom_left, CellType::Empty);
			colno.get()[4] = GetVarNumber(bottom_left, CellType::BottomLeft);
			is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), LE, 2));
			row.get()[0] = row.get()[1] = row.get()[2] = 1;
			row.get()[3] = row.get()[4] = -1;
			colno.get()[0] = GetVarNumber(center, CellType::Empty);
			colno.get()[1] = GetVarNumber(left, CellType::Empty);
			colno.get()[2] = GetVarNumber(upper, CellType::Empty);
			colno.get()[3] = GetVarNumber(upper_left, CellType::Empty);
			colno.get()[4] = GetVarNumber(upper_left, CellType::UpperLeft);
			is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), LE, 2));
			row.get()[0] = row.get()[1] = row.get()[2] = 1;
			row.get()[3] = row.get()[4] = -1;
			colno.get()[0] = GetVarNumber(center, CellType::Empty);
			colno.get()[1] = GetVarNumber(upper, CellType::Empty);
			colno.get()[2] = GetVarNumber(right, CellType::Empty);
			colno.get()[3] = GetVarNumber(upper_right, CellType::Empty);
			colno.get()[4] = GetVarNumber(upper_right, CellType::UpperRight);
			is_ok &= static_cast<bool>(add_constraintex(lp.get(), constraint_sz, row.get(), colno.get(), LE, 2));
			if (!is_ok) {
				UE_LOG(LogTemp, Error, TEXT("LpSolver::Solver() can't add constraint on phase 9. i: %d j: %d type: %d"), i, j, static_cast<int>(type));
				return;
			}
		}
		});
	//write_lp(lp.get(), "C:\\Users\\tbsd\\Documents\\Unreal Projects\\ue\\Shakashaka\\model7.log");//HEHMDA

	set_add_rowmode(lp.get(), FALSE);
	// fake objective function
	mModelCreationTime = current_timestamp() - model_creation_start;
	UE_LOG(LogTemp, Log, TEXT("LpSolver::Solver(): construction finished"));
	UE_LOG(LogTemp, Log, TEXT("LpSolver::Solver(): number of constraints: %d"), get_Norig_rows(lp.get()));
	UE_LOG(LogTemp, Log, TEXT("LpSolver::Solver(): solveing"));
	auto result = solve(lp.get());
	mTimeElapsed = time_elapsed(lp.get());
	if (result == OPTIMAL || result == SUBOPTIMAL) {
		UE_LOG(LogTemp, Log, TEXT("LpSolver::Solver(): solution found"));
		UE_LOG(LogTemp, Log, TEXT("LpSolver::Solver():4l time elapsed: %f"), time_elapsed(lp.get()));
		row = std::shared_ptr<REAL>(new REAL[n_col], std::default_delete<REAL[]>());
		get_variables(lp.get(), row.get());
		mSolution = std::make_shared<GameBoard>(*mInitBoard);
		for (int i = 0; i < n_col; i += 5) {
			int x = (i / 5) % mWidth;
			int y = (i / 5) / mWidth;
			if (row.get()[i] > 0) // e
				(*mSolution)[x][y] = CellType::Empty;
			else if (row.get()[i + 1] > 0) // ul
				(*mSolution)[x][y] = CellType::UpperLeft;
			else if (row.get()[i + 2] > 0) // ur
				(*mSolution)[x][y] = CellType::UpperRight;
			else if (row.get()[i + 3] > 0) // br
				(*mSolution)[x][y] = CellType::BottomRight;
			else if (row.get()[i + 4] > 0) // bl
				(*mSolution)[x][y] = CellType::BottomLeft;
		}
		return true;
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("LpSolver::Solver(): no solution"));
	}
	return false;
}

std::shared_ptr<LpSolver::GameBoard> LpSolver::GetInitBoard() {
	return mInitBoard;
}

std::shared_ptr<LpSolver::GameBoard> LpSolver::GetSolution() {
	return mSolution;
}

double LpSolver::GetTimeElapsed() const {
	return mTimeElapsed;
}

FString LpSolver::ToString() const {
	FString ss;
		UE_LOG(LogTemp, Warning, TEXT("HEHMDA LpSolver::Solver(): toString 1; mWidth: %d, mHeight: %d"), mWidth, mHeight);
	if (!mInitBoard)
		return ss;
	UE_LOG(LogTemp, Warning, TEXT("HEHMDA LpSolver::Solver(): toString 2 initBoard not null"));
	auto to_string = [&ss, this](std::shared_ptr<GameBoard> board) {
		if (!board) {
			UE_LOG(LogTemp, Warning, TEXT("HEHMDA LpSolver::Solver(): toString 2.1 board null"));
			return;
		}
		for (size_t j = 0; j < mHeight; ++j) {
			UE_LOG(LogTemp, Warning, TEXT("HEHMD LpSolver::Solver(): toString 3 board->size() %d, j: %d"), board->size(), j);
			for (size_t i = 0; i < mWidth; ++i) {
				UE_LOG(LogTemp, Warning, TEXT("HEHMD LpSolver::Solver(): toString 4 board[j]->size() %d, i: %d"), (*board)[j].size(), i);
				switch (auto cell = (*board)[i][j]) {
				case CellType::Black:
					ss += FString(TEXT("■"));
					break;
				case CellType::Black1: [[fallthrough]];
				case CellType::Black2: [[fallthrough]];
				case CellType::Black3: [[fallthrough]];
				case CellType::Black4:
					ss.AppendInt(static_cast<int>(cell));
					break;
				case CellType::Empty:
					ss += FString(TEXT("□"));
					break;
				case CellType::BottomLeft:
					ss += FString(TEXT("◣"));
					break;
				case CellType::BottomRight:
					ss += FString(TEXT("◢"));
					break;
				case CellType::UpperLeft:
					ss += FString(TEXT("◤"));
					break;
				case CellType::UpperRight:
					ss += FString(TEXT("◥"));
					break;
				}
			}
			ss += FString(TEXT("\n"));
		}
	};
	ss += FString(TEXT("\nInitial:\n"));
	to_string(mInitBoard);
	if (!mSolution)
		return ss;
	UE_LOG(LogTemp, Warning, TEXT("HEHMDA LpSolver::Solver(): toString 5 initBoard not null"));
	ss += FString(TEXT("\nSolution:\n"));
	to_string(mSolution);
	return ss;
}

long long LpSolver::GetModelCreationTimeMs() const {
	return mModelCreationTime;
}
