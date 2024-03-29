#include "stdafx.h"
#include <omp.h>
#include <iostream>
#include <random>
#include <vector>
#include <ctime>
#include <stdio.h>
using namespace std;


//std::vector<double> rand_matrix(int cols, int rows)
//{
//    vector<double> result(cols*rows);
//    for (int i = 0; i < result.size(); i++)
//    {
//        result[i] = rand();
//    }
//    for (int i = 0; i < result.size(); i++)
//        cout << result[i] << ' ';
//    return result;
//}

class Matrix
{
public:
	Matrix()
	{
		_data = nullptr;
		_cols = 0;
		_rows = 0;
	}
	Matrix(double * data, int rows, int cols)
	{
		_data = data;
		_cols = cols;
		_rows = rows;
	}

	Matrix(int rows, int cols)
	{
		_cols = cols;
		_rows = rows;
		_data = new double[rows*cols];
	}

	Matrix(const Matrix& rhs)
	{
		_data = rhs._data;
		_cols = rhs._cols;
		_rows = rhs._rows;
	}

	Matrix& operator=(const Matrix& rhs)
	{
		_data = rhs._data;
		_cols = rhs._cols;
		_rows = rhs._rows;

		return *this;
	}

	~Matrix()
	{
		/*if (_data)
		delete[] _data;*/

		_data = nullptr;
	}

	static Matrix rand_matrix(int rows, int cols)
	{
		double * result = new double[rows*cols];
		for (int i = 0; i < cols*rows; i++)
		{
			result[i] = rand() % 2;
		}
		return Matrix(result, rows, cols);
	}

	double iloc(int row, int col) const
	{
		return _data[row*_cols + col];
	}

	inline int index(int row_idx, int cols, int col_idx)
	{
		return row_idx * cols + col_idx;
	}

	void set_value(int row, int col, double value)
	{
		_data[row*_cols + col] = value;
	}

	Matrix operator * (const Matrix& B)
	{
		time_t start, end;
		double dif;
		//Matrix C = Matrix(_rows, B.get_cols_num()); 
		auto result = new double[_rows * B.get_cols_num()];
		time(&start);
		for (int i = 0; i < _rows; i++)
		{
			for (int j = 0; j < B.get_cols_num(); j++)
			{
				result[i * _cols + j] = 0;
				for (int k = 0; k < _cols; k++)
				{
					//result[i * _cols + j] += iloc(i, k) * B.iloc(k, j);
					result[index(i,_cols,j)] += _data[index(i, _cols, k)] * B._data[index(k, _rows, j)];
				}
			}
		}
		time(&end);
		dif = difftime(end, start);
		printf("Serial multiplication took %.2lf seconds to run.\n", dif);
		return Matrix(result, _rows, B.get_cols_num());
	}

	Matrix SPMD_Mult(const Matrix& B, int thrd_number)
	{

		cout << "-----------" << endl;
		cout << "SPMD with " << thrd_number << endl;
		time_t start, end;
		double dif;
		//Matrix C = Matrix(_rows, B.get_cols_num()); 
		auto result = new double[_rows * B.get_cols_num()];
		time(&start);
		#pragma omp parallel num_threads(thrd_number)
		{
			int thrd_idx = omp_get_thread_num();
			int res_rows_start = thrd_idx * _rows / thrd_number;
			int res_rows_end = (thrd_idx + 1) * _rows / thrd_number;
			int res_cols_start = 0; 
			int res_cols_end = B.get_cols_num(); 
			for (int i = res_rows_start; i < res_rows_end; i++)
			{
				for (int j = res_cols_start; j < res_cols_end; j++)
				{
					result[i * _cols + j] = 0;
					for (int k = 0; k < _cols; k++)
					{
						result[i * _cols + j] += _data[i * _cols + k] * B._data[k * _cols + j];
					}
				}
			}
		}
		time(&end);
		dif = difftime(end, start);
		printf("SPMD %d multiplication took %.2lf seconds to run.\n", thrd_number, dif);
		return Matrix(result, _rows, B.get_cols_num());
	}

	/*Matrix& SPMD_Mult2(Matrix B)
	{
	Matrix C = Matrix(_rows, B.get_cols_num());
	#pragma omp parallel num_threads(THRD_NUMBER)
	{
	int thrd_idx = omp_get_thread_num();
	int res_rows_start = (thrd_idx)* _rows * 2 / THRD_NUMBER;
	int res_rows_end = (thrd_idx + 1) * _rows * 2 / THRD_NUMBER;
	int res_cols_start = (thrd_idx) * B.get_cols_num() * 4 / THRD_NUMBER;
	int res_cols_end = (thrd_idx + 1) * B.get_cols_num() * 4 / THRD_NUMBER;
	for (int i = res_rows_start; i < res_rows_end; i++)
	{
	for (int j = res_cols_start; j < res_cols_end; j++)
	{
	double res = 0;
	for (int k = 0; k < _cols; k++)
	{
	res += iloc(i, k) * B.iloc(k, j);
	}
	C.set_value(i, j, res);
	}
	}
	}
	return C;
	}*/

	Matrix Schedule_Mult(const Matrix& B, int thrd_number)
	{
		time_t start, end;
		double dif;
		//Matrix C = Matrix(_rows, B.get_cols_num()); 
		auto result = new double[_rows * B.get_cols_num()];
		time(&start);
		#pragma omp for schedule(dynamic) 
		for (int i = 0; i < _rows; i++)
		{
			for (int j = 0; j < B.get_cols_num(); j++)
			{
				result[i * _cols + j] = 0;
				for (int k = 0; k < _cols; k++)
				{
					result[i * _cols + j] += _data[i * _cols + k] * B._data[k * _cols + j];
				}
			}
		}
		time(&end);
		dif = difftime(end, start);
		printf("Schedule dynamic parallel %d multiplication took %.2lf seconds to run.\n", thrd_number, dif);
		return Matrix(result, _rows, B.get_cols_num());
	}

	Matrix Schedule_Mult2(const Matrix& B)
	{
		time_t start, end;
		double dif;
		//Matrix C = Matrix(_rows, B.get_cols_num()); 
		auto result = new double[_rows * B.get_cols_num()];
		time(&start);
#pragma omp for schedule(static) 
		for (int i = 0; i < _rows; i++)
		{
			for (int j = 0; j < B.get_cols_num(); j++)
			{
				result[i * _cols + j] = 0;
				for (int k = 0; k < _cols; k++)
				{
					result[i * _cols + j] += _data[i * _cols + k] * B._data[k * _cols + j];
				}
			}
		}
		time(&end);
		dif = difftime(end, start);
		printf("Schedule static parallel multiplication took %.2lf seconds to run.\n", dif);
		return Matrix(result, _rows, B.get_cols_num());
	}

	int get_rows_num() const
	{
		return _rows;
	}
	int get_cols_num() const
	{
		return _cols;
	}

	void print()
	{
		for (int i = 0; i < _rows * _cols; i++)
		{
			cout << _data[i] << endl;
		}
	}
private:
	double * _data;
	int _cols;
	int _rows;
};

int main()
{
	time_t start, end;
	auto A = Matrix::rand_matrix(1000, 1000);
	auto B = Matrix::rand_matrix(1000, 1000);
	double dif;

	//time(&start);
	auto C = A * B;
	//time(&end);
	//dif = difftime(end, start);
	//printf("Your calculations took %.2lf seconds to run.\n", dif);
	//time(&start);
	C = A.SPMD_Mult(B, 2);

	C = A.SPMD_Mult(B, 4);

	C = A.SPMD_Mult(B, 8);
	//time(&end);
	//dif = difftime(end, start);
	//printf("Your calculations took %.2lf seconds to run.\n", dif);
	////C.print();

	//time(&start);
	C = A.Schedule_Mult(B, 2);

	C = A.Schedule_Mult2(B);
	//time(&end);
	//dif = difftime(end, start);
	//printf("Your calculations took %.2lf seconds to run.\n", dif);

	system("pause");
	return 0;
}