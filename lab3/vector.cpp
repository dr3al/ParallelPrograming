#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <fstream>
#include <limits>
#include <cmath>
#include <chrono>
#include <mutex>

template<typename T>
class Vector {
private:
    size_t n;
    T* data;
    bool isInitialized; 
    mutable std::mutex mutex;

public:
    //конструктор
    Vector(size_t size): n(size), data(nullptr), isInitialized(false) {
        if (size > 0) {
            data = new T[size]; //Выделяем память
        } else {
            throw std::invalid_argument("Размер должен быть положительным");
        }
    }

    //деструктор
    ~Vector() {
        delete[] data;
    }

    void initializeConstant(T value) {
        // Захватываем мьютекс
        std::lock_guard<std::mutex> lock(mutex);
        // Заполняем всю data значением value 
        std::fill(data, data + n, value);
        // Указываем, что вектор инициализирован
        isInitialized = true;
    }

    void initializeRandom(T minValue, T maxValue) {
        // Захватываем мьютекс
        std::lock_guard<std::mutex> lock(mutex);
        std::random_device rd; //источник случайных чисел
        std::mt19937 gen(rd()); //генератор случайных чисел
        std::uniform_real_distribution<T> dist(minValue, maxValue); // равномерное распределение чисел с плавающей точкой
        //заполняем данные
        for (size_t i = 0; i < n; ++i) {
            data[i] = dist(gen);
        }
        isInitialized = true;
    }

    // Проверка на то инициализирован ли вектор
    void checkInitialization() const {
        if (!isInitialized) {
            throw std::logic_error("Вектор не инициализирован");
        }
    }

    void Export(const std::string& filename) const {
        checkInitialization();
        // Захватываем мьютекс
        std::lock_guard<std::mutex> lock(mutex);
        // Открываем файл
        std::ofstream file(filename, std::ios::out);
        // Проверяем открылся ли он
        if (!file) {
            throw std::ios_base::failure("Ошибка экспорта");
        }
        // Записываем в файл по одному элементу на строчку
        for (size_t i = 0; i < n; ++i) {
            file << data[i] << "\n";
        }
    }

    void Import(const std::string& filename) {
        std::lock_guard<std::mutex> lock(mutex);
        std::ifstream file(filename, std::ios::in);
        if (!file) {
            throw std::ios_base::failure("Ошибка импорта");
        }
        size_t i = 0;
        T value;
        while (file >> value && i < n) {
            data[i++] = value;
        }
        if (i < n) {
            throw std::runtime_error("Недостаточно данных");
        }
        isInitialized = true;
    }

    // Поиск минимального элемента
    std::pair<T, size_t> findMin() const {
        checkInitialization();
        auto startTime = std::chrono::high_resolution_clock::now();

        T minValue = std::numeric_limits<T>::max();
        size_t minIndex = 0;
        for (size_t i = 0; i < n; ++i) {
            if (data[i] < minValue) {
                minValue = data[i];
                minIndex = i;
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = endTime - startTime;
        std::cout << "Время поиска минимума: " << elapsed.count() << " секунд" << std::endl;

        return std::make_pair(minValue, minIndex);
    }

    // Поиск минимального элемента std::thread
    std::tuple<T, size_t> findMinParallel(size_t numThreads) {
        checkInitialization();

        auto start = std::chrono::high_resolution_clock::now();
        T minValue = data[0];
        size_t minIndex = 0;

        std::vector<std::thread> threads;
        std::vector<T> minValues(numThreads, std::numeric_limits<T>::max());
        std::vector<size_t> minIndexes(numThreads, 0);

        // Функция для поиска минимального элемента в каждой части вектора
        auto findMinInRange = [&](size_t threadId, size_t start, size_t end) {
            T minValueLocal = data[start];
            size_t minIndexLocal = start;

            for (size_t i = start + 1; i < end; ++i) {
                if (data[i] < minValueLocal) {
                    minValueLocal = data[i];
                    minIndexLocal = i;
                }
            }

            minValues[threadId] = minValueLocal;
            minIndexes[threadId] = minIndexLocal;
        };

        // Разбиение работы между потоками
        size_t chunkSize = n / numThreads;
        for (size_t i = 0; i < numThreads; ++i) {
            size_t start = i * chunkSize;
            size_t end = (i == numThreads - 1) ? n : (i + 1) * chunkSize;
            threads.push_back(std::thread(findMinInRange, i, start, end));
        }

        // Ожидание завершения всех потоков
        for (auto& th : threads) {
            th.join();
        }

        // Обработка результатов
        for (size_t i = 0; i < numThreads; ++i) {
            if (minValues[i] < minValue) {
                minValue = minValues[i];
                minIndex = minIndexes[i];
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Время поиска минимума в параллельном: " << elapsed.count() << " секунд" << std::endl;

        return {minValue, minIndex};
    }

    std::pair<T, size_t> findMax() const {
        checkInitialization();
        auto startTime = std::chrono::high_resolution_clock::now();

        T maxValue = std::numeric_limits<T>::lowest();
        size_t maxIndex = 0;
        for (size_t i = 0; i < n; ++i) {
            if (data[i] > maxValue) {
                maxValue = data[i];
                maxIndex = i;
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = endTime - startTime;
        std::cout << "Время поиска максимума: " << elapsed.count() << " секунд" << std::endl;

        return std::make_pair(maxValue, maxIndex);
    }

    std::tuple<T, size_t> findMaxParallel(size_t numThreads) {
        checkInitialization();

        auto startTime = std::chrono::high_resolution_clock::now();

        T maxValue = data[0];
        size_t maxIndex = 0;

        std::vector<std::thread> threads;
        std::vector<T> maxValues(numThreads, std::numeric_limits<T>::min());
        std::vector<size_t> maxIndexes(numThreads, 0);

        // Функция для поиска максимального элемента в каждой части вектора
        auto findMaxInRange = [&](size_t threadId, size_t start, size_t end) {
            T maxValueLocal = data[start];
            size_t maxIndexLocal = start;

            for (size_t i = start + 1; i < end; ++i) {
                if (data[i] > maxValueLocal) {
                    maxValueLocal = data[i];
                    maxIndexLocal = i;
                }
            }

            maxValues[threadId] = maxValueLocal;
            maxIndexes[threadId] = maxIndexLocal;
        };

        // Разбиение работы между потоками
        size_t chunkSize = n / numThreads;
        for (size_t i = 0; i < numThreads; ++i) {
            size_t start = i * chunkSize;
            size_t end = (i == numThreads - 1) ? n : (i + 1) * chunkSize;
            threads.push_back(std::thread(findMaxInRange, i, start, end));
        }

        // Ожидание завершения всех потоков
        for (auto& th : threads) {
            th.join();
        }

        // Обработка результатов
        for (size_t i = 0; i < numThreads; ++i) {
            if (maxValues[i] > maxValue) {
                maxValue = maxValues[i];
                maxIndex = maxIndexes[i];
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = endTime - startTime;
        std::cout << "Время поиска максимума в параллельном: " << elapsed.count() << " секунд" << std::endl;

        return {maxValue, maxIndex};
    }

    T calculateMean() const {
        checkInitialization();
        auto startTime = std::chrono::high_resolution_clock::now();

        T sum = 0;
        for (size_t i = 0; i < n; ++i) {
            sum += data[i];
        }

        T mean = sum / static_cast<T>(n);

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = endTime - startTime;
        std::cout << "Время вычисления среднего: " << elapsed.count() << " секунд" << std::endl;

        return mean;
    }

    T calculateMeanParallel(size_t numThreads) const {
        checkInitialization();
        auto startTime = std::chrono::high_resolution_clock::now();

        T sum = 0;

        std::vector<std::thread> threads;
        std::vector<T> allSum(numThreads);

        auto calculateMeanInRange = [&](size_t threadId, size_t start, size_t end) {
            for (size_t i = start; i < end; ++i) {
                allSum[threadId] += data[i];
            }
        };

        size_t chunkSize = n / numThreads;
        for (size_t i = 0; i < numThreads; ++i) {
            size_t start = i * chunkSize;
            size_t end = (i == numThreads - 1) ? n : (i + 1) * chunkSize;
            threads.push_back(std::thread(calculateMeanInRange, i, start, end));
        }

        for (auto& th : threads) {
            th.join();
        }

        for (size_t i = 0; i < numThreads; ++i) {
            sum += allSum[i];
        }
        T mean = sum / static_cast<T>(n);

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = endTime - startTime;
        std::cout << "Время поиска среднего в параллельном: " << elapsed.count() << " секунд" << std::endl;

        return mean;
    }

    T calculateSum() const {
        checkInitialization();
        auto startTime = std::chrono::high_resolution_clock::now();

        T sum = 0;
        for (size_t i = 0; i < n; ++i) {
            sum += data[i];
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = endTime - startTime;
        std::cout << "Время вычисления суммы: " << elapsed.count() << " секунд" << std::endl;

        return sum;
    }

    T calculateSumParallel(size_t numThreads) const {
        checkInitialization();
        auto startTime = std::chrono::high_resolution_clock::now();

       T sum = 0;

        std::vector<std::thread> threads;
        std::vector<T> allSum(numThreads);

        auto calculateMeanInRange = [&](size_t threadId, size_t start, size_t end) {
            for (size_t i = start; i < end; ++i) {
                allSum[threadId] += data[i];
            }
        };

        size_t chunkSize = n / numThreads;
        for (size_t i = 0; i < numThreads; ++i) {
            size_t start = i * chunkSize;
            size_t end = (i == numThreads - 1) ? n : (i + 1) * chunkSize;
            threads.push_back(std::thread(calculateMeanInRange, i, start, end));
        }

        for (auto& th : threads) {
            th.join();
        }

        for (size_t i = 0; i < numThreads; ++i) {
            sum += allSum[i];
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = endTime - startTime;
        std::cout << "Время вычисления суммы в параллельном: " << elapsed.count() << " секунд" << std::endl;

        return sum;
    }
};

int main() {
    try {
        Vector<double> vec(10000000);
        vec.initializeRandom(0.0, 5.0);

        auto [minValue, minIndex] = vec.findMin();
        std::cout << "Минимум: " << minValue << ", индекс: " << minIndex << std::endl;

        auto [minValueParallel, minIndexParallel] = vec.findMinParallel(10);
        std::cout << "Минимум: " << minValueParallel << ", индекс: " << minIndexParallel << std::endl;

        auto [maxValue, maxIndex] = vec.findMax();
        std::cout << "Максимум: " << maxValue << ", индекс: " << maxIndex << std::endl;

        auto [maxValueParallel, maxIndexParallel] = vec.findMaxParallel(10);
        std::cout << "Максимум: " << maxValueParallel << ", индекс: " << maxIndexParallel << std::endl;

        double mean = vec.calculateMean();
        std::cout << "Среднее: " << mean << std::endl;

        double meanParallel = vec.calculateMeanParallel(10);
        std::cout << "Среднее: " << meanParallel << std::endl;

        double sum = vec.calculateSum();
        std::cout << "Сумма: " << sum << std::endl;

        double sumParallel = vec.calculateSumParallel(10);
        std::cout << "Сумма: " << sumParallel << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }

    return 0;
}
