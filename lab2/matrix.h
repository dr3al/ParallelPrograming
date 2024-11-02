#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <iostream>


class Matrix {
public:
    virtual ~Matrix() = default; // Деструктор

    virtual Matrix* add(const Matrix& other) const = 0; // Сложение
    virtual Matrix* subtract(const Matrix& other) const = 0; // Вычитание 
    virtual Matrix* elementwiseMultiply(const Matrix& other) const = 0; // Поэлементное умножение
    virtual Matrix* multiply(const Matrix& other) const = 0; // Матричное умножение
    virtual Matrix* transpose() const = 0; // Транспонирование матрицы

    virtual void Import(const std::string& filename) = 0; // Импорт матрицы из файла
    virtual void Export(const std::string& filename) const = 0; // Экспорт матрицы в файл
    virtual void Print() const = 0; // Вывод матрицы на экран
};

class MatrixDense : public Matrix {
private:
    std::vector<std::vector<double>> data;
    int rows, cols;

public:
    MatrixDense(int rows, int cols) : rows(rows), cols(cols), data(rows, std::vector<double>(cols)) {}

    // Проверка эквивалентности размеров матриц
    void checkSize(const MatrixDense* other) const {
        if (!other || rows != other->rows || cols != other->cols) {
            throw std::invalid_argument("Размеры матрицы не совпадают.");
        }
    }

    Matrix* add(const Matrix& other) const override {
        // Приводим матрицу к типу MatrixDense
        const MatrixDense* otherDense = dynamic_cast<const MatrixDense*>(&other);
        checkSize(otherDense);

        // Создаем результирующую матрицу
        MatrixDense* result = new MatrixDense(rows, cols);
        
        // Поэлементное сложение
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                result->data[i][j] = data[i][j] + otherDense->data[i][j];
            }
        }

        return result;
    }
    
    Matrix* subtract(const Matrix& other) const override {
        const MatrixDense* otherDense = dynamic_cast<const MatrixDense*>(&other);
        checkSize(otherDense);

        MatrixDense* result = new MatrixDense(rows, cols);
        
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                result->data[i][j] = data[i][j] - otherDense->data[i][j];
            }
        }

        return result;
    }

    Matrix* elementwiseMultiply(const Matrix& other) const override {
        const MatrixDense* otherDense = dynamic_cast<const MatrixDense*>(&other);
        checkSize(otherDense);

        MatrixDense* result = new MatrixDense(rows, cols);
        
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                result->data[i][j] = data[i][j] * otherDense->data[i][j];
            }
        }

        return result;
    }

    Matrix* multiply(const Matrix& other) const override {
        const MatrixDense* otherDense = dynamic_cast<const MatrixDense*>(&other);
        checkSize(otherDense);

        MatrixDense* result = new MatrixDense(rows, otherDense->cols);
        
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < otherDense->cols; ++j) {
                double sum = 0.0;
                for (int k = 0; k < cols; ++k) {
                    sum += data[i][k] * otherDense->data[k][j];
                }
                result->data[i][j] = sum;
            }
        }

        return result;
    }

    Matrix* transpose() const override {
        // Создаем новую матрицу с перевернутыми размерами.
        MatrixDense* result = new MatrixDense(cols, rows);
        
        // Транспонирование: меняем местами индексы строк и столбцов.
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                result->data[j][i] = data[i][j];
            }
        }

        // Возвращаем указатель на транспонированную матрицу.
        return result;
    }

    void set(int row, int col, double value) {
        if (row < 0 || row >= rows || col < 0 || col >= cols) {
            throw std::out_of_range("Индекс вне диапазона");
        }
        data[row][col] = value;
    }

    void Import(const std::string& filename) override {
        std::ifstream file(filename); 
        if (!file.is_open()) {
            throw std::runtime_error("Не удается открыть файл."); 
        }

        std::string className; 
        file >> className;
        if (className != "MatrixDense") {
            throw std::runtime_error("Недопустимый тип матрицы.");
        }

        file >> rows >> cols;
        data.resize(rows, std::vector<double>(cols)); 

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (!(file >> data[i][j])) { 
                    throw std::runtime_error("Ошибка при считывании данных матрицы.");
                }
            }
        }

        file.close();
    }

    void Export(const std::string& filename) const override {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Не удается открыть файл.."); 
        }

        file << "MatrixDense\n";
        file << rows << " " << cols << "\n"; 

        for (const auto& row : data) {
            for (double value : row) {
                file << value << " "; 
            }
            file << "\n";
        }

        file.close();
    }

    void Print() const override{
        for (const auto& row : data) {
            for (double value : row) {
                std::cout << value << " "; 
            }
            std::cout << "\n";
        }
        std::cout << std::endl;
    }

};

class MatrixDiagonal : public Matrix {
private:
    std::vector<double> data;
    int size;

public:
    MatrixDiagonal(int size) : size(size), data(size) {}

    // Проверка эквивалентности размеров матриц
    void checkSize(const MatrixDiagonal* other) const {
        if (!other || size != other->size) {
            throw std::invalid_argument("Размеры матрицы не совпадают.");
        }
    }

    Matrix* add(const Matrix& other) const override {
        // Приводим матрицу к типу MatrixDense
        const MatrixDiagonal* otherDiag = dynamic_cast<const MatrixDiagonal*>(&other);
        checkSize(otherDiag);
        
        //Создание новой диагональной матрицы для хранения результата.
        MatrixDiagonal* result = new MatrixDiagonal(size);

        //Сложение соответствующих элементов диагоналей двух матриц.
        for (int i = 0; i < size; ++i) {
            result->data[i] = data[i] + otherDiag->data[i];
        }
    
        return result;
    }

    Matrix* subtract(const Matrix& other) const override {
        const MatrixDiagonal* otherDiag = dynamic_cast<const MatrixDiagonal*>(&other);
        checkSize(otherDiag);
        
        MatrixDiagonal* result = new MatrixDiagonal(size);

        for (int i = 0; i < size; ++i) {
            result->data[i] = data[i] - otherDiag->data[i];
        }
    
        return result; 
    }

    Matrix* elementwiseMultiply(const Matrix& other) const override {
        const MatrixDiagonal* otherDiag = dynamic_cast<const MatrixDiagonal*>(&other);
        checkSize(otherDiag);
        
        MatrixDiagonal* result = new MatrixDiagonal(size);

        for (int i = 0; i < size; ++i) {
            result->data[i] = data[i] * otherDiag->data[i];
        }
    
        return result; 
    }

    Matrix* multiply(const Matrix& other) const override {
        const MatrixDiagonal* otherDiag = dynamic_cast<const MatrixDiagonal*>(&other);
        checkSize(otherDiag);
        
        MatrixDiagonal* result = new MatrixDiagonal(size);

        for (int i = 0; i < size; ++i) {
            result->data[i] = data[i] * otherDiag->data[i];
        }
    
        return result; 
    }

    Matrix* transpose() const override {
        return new MatrixDiagonal(*this);
    }

    void set(int row, int col, double value) {
        if (row == col) {
            data[row] = value;
        } else {
            throw std::invalid_argument("Допустимо изменить лишь диагональные значения");
        }
    }

    void Import(const std::string& filename) override {
        std::ifstream file(filename);
        if (!file.is_open()) throw std::runtime_error("Не удается открыть файл.");

        std::string className;
        file >> className;
        if (className != "MatrixDiagonal") throw std::runtime_error("Недопустимый тип матрицы.");

        file >> size;
        data.resize(size);

        for (int i = 0; i < size; ++i) {
            if (!(file >> data[i])) {
                    throw std::runtime_error("Ошибка при считывании матричных данных.");
            }
        }

        file.close();
    }

    void Export(const std::string& filename) const override {
        std::ofstream file(filename);
        if (!file.is_open()) throw std::runtime_error("Не удается открыть файл.");

        file << "MatrixDiagonal\n";
        file << size << "\n";
        for (double value : data) {
            file << value << " ";
        }
        file << "\n";

        file.close();
    }

    void Print() const override {
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                if (i == j) {
                    std::cout << data[i] << " ";
                } else {
                    std::cout << "0 ";
                }
            }
            std::cout << "\n";
        }
        std::cout << std::endl;
    }

};
