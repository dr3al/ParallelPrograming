#include "matrix.h"
#include <iostream>

int main() {
    try {
        MatrixDiagonal mat1(3);
        mat1.set(0, 0, 3.0);
        mat1.set(1, 1, 1.0);
        mat1.set(2, 2, 2.0);

        MatrixDiagonal mat2(3);
        mat2.Import("in_diagonal.txt");

        std::cout << "Ручное добавление: " << std::endl;
        mat1.Print();

        std::cout << "Импорт из файла: " << std::endl;
        mat2.Print();

        Matrix* sum = mat1.add(mat2);
        std::cout << "Сумма матриц: " << std::endl;
        sum->Print();
        sum->Export("diagonal/out_sum.txt");
        delete sum;

        Matrix* mult = mat1.multiply(mat2);
        std::cout << "Умножение матриц: " << std::endl;
        mult->Print(); 
        mult->Export("diagonal/out_multiply.txt");
        delete mult;

        Matrix* transpose = mat1.transpose();
        std::cout << "Транспонированная матрица 1: " << std::endl;
        transpose->Print();
        transpose->Export("diagonal/out_transpose.txt");

        delete transpose;

    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }

    return 0;
}
