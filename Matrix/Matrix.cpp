// Matrix.cpp
// FS, 21-04-2007

#include "../../Inc/Matrix.h"


int main(int argc, char *argv[])
{
	//Matrix<double> m(3, 3);

	//MatrixRow<double> r2 = m[2];
	//MatrixColumn<double> c3 = m.column(3);

	//for(int idx = 0; idx < 16; ++idx)
	//{
	//	m[idx / 4][idx % 4] = idx;
	//}
	//m[0][0] = 1;
	//m[0][1] = 2;
	//m[0][2] = 3;
	//m[1][0] = 2;
	//m[1][1] = -1;
	//m[1][2] = 1;
	//m[2][0] = 3;
	//m[2][1] = 0;
	//m[2][2] = -1;

	//m[0][0] = 2;
	//m[0][1] = 1;
	//m[0][2] = -1;
	//m[1][0] = -3;
	//m[1][1] = -1;
	//m[1][2] = 2;
	//m[2][0] = -2;
	//m[2][1] = 1;
	//m[2][2] = 2;
	double arr[] = { 2, 1, -1, -3, -1, 2, -2, 1, 2 };
	Matrix<double> m(3, 3, arr);

	std::cout << "Matrix: " << m << std::endl;

	MatrixRow<double> r(3);
	//r[0] = 9;
	//r[1] = 8;
	//r[2] = 3;
	r[0] = 2;
	r[1] = 3;
	r[2] = -1;

	try
	{
		std::cout << "Row to solve matrix with: " << r << std::endl;
		MatrixRow<double> solved = m.solve(r);
		std::cout << "Solved result: " << solved << std::endl;
	}
	catch(char *c)
	{
		std::cout << "ERROR: " << c << std::endl;
	}

	int x;
	std::cin >> x;

	return 0;
}
