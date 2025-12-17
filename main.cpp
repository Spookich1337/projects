#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <future>
#include <fstream>
#include <algorithm>
#include <string>

using Matrix = std::vector<std::vector<int>>;

Matrix generateMatrix(int n) {
    Matrix res(n, std::vector<int>(n));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 10);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            res[i][j] = dist(gen);
    return res;
}

std::vector<int> generateArray(int n) {
    std::vector<int> res(n);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 10000);
    for (int i = 0; i < n; ++i) res[i] = dist(gen);
    return res;
}

Matrix add(const Matrix& A, const Matrix& B) {
    int n = A.size();
    Matrix C(n, std::vector<int>(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            C[i][j] = A[i][j] + B[i][j];
    return C;
}

Matrix sub(const Matrix& A, const Matrix& B) {
    int n = A.size();
    Matrix C(n, std::vector<int>(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            C[i][j] = A[i][j] - B[i][j];
    return C;
}

Matrix multiplyNaive(const Matrix& A, const Matrix& B) {
    int n = A.size();
    Matrix C(n, std::vector<int>(n, 0));
    for (int i = 0; i < n; i++)
        for (int k = 0; k < n; k++)
            for (int j = 0; j < n; j++)
                C[i][j] += A[i][k] * B[k][j];
    return C;
}

Matrix strassenRecursive(const Matrix& A, const Matrix& B, int depth) {
    int n = A.size();

    if (n <= 64) {
        return multiplyNaive(A, B);
    }

    int k = n / 2;
    Matrix A11(k, std::vector<int>(k)), A12(k, std::vector<int>(k)),
           A21(k, std::vector<int>(k)), A22(k, std::vector<int>(k));
    Matrix B11(k, std::vector<int>(k)), B12(k, std::vector<int>(k)),
           B21(k, std::vector<int>(k)), B22(k, std::vector<int>(k));

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            A11[i][j] = A[i][j];         A12[i][j] = A[i][j + k];
            A21[i][j] = A[i + k][j];     A22[i][j] = A[i + k][j + k];
            B11[i][j] = B[i][j];         B12[i][j] = B[i][j + k];
            B21[i][j] = B[i + k][j];     B22[i][j] = B[i + k][j + k];
        }
    }

    auto run_task = [&](const Matrix& m1, const Matrix& m2) {
        return strassenRecursive(m1, m2, depth + 1);
    };

    std::future<Matrix> fP1, fP2, fP3, fP4, fP5, fP6, fP7;
    bool parallel = depth < 2; 

    if (parallel) {
        fP1 = std::async(std::launch::async, run_task, A11, sub(B12, B22));
        fP2 = std::async(std::launch::async, run_task, add(A11, A12), B22);
        fP3 = std::async(std::launch::async, run_task, add(A21, A22), B11);
        fP4 = std::async(std::launch::async, run_task, A22, sub(B21, B11));
        fP5 = std::async(std::launch::async, run_task, add(A11, A22), add(B11, B22));
        fP6 = std::async(std::launch::async, run_task, sub(A12, A22), add(B21, B22));
        fP7 = std::async(std::launch::async, run_task, sub(A11, A21), add(B11, B12));
    }

    Matrix P1 = parallel ? fP1.get() : run_task(A11, sub(B12, B22));
    Matrix P2 = parallel ? fP2.get() : run_task(add(A11, A12), B22);
    Matrix P3 = parallel ? fP3.get() : run_task(add(A21, A22), B11);
    Matrix P4 = parallel ? fP4.get() : run_task(A22, sub(B21, B11));
    Matrix P5 = parallel ? fP5.get() : run_task(add(A11, A22), add(B11, B22));
    Matrix P6 = parallel ? fP6.get() : run_task(sub(A12, A22), add(B21, B22));
    Matrix P7 = parallel ? fP7.get() : run_task(sub(A11, A21), add(B11, B12));

    Matrix C11 = add(sub(add(P5, P4), P2), P6);
    Matrix C12 = add(P1, P2);
    Matrix C21 = add(P3, P4);
    Matrix C22 = sub(sub(add(P5, P1), P3), P7);

    Matrix C(n, std::vector<int>(n));
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            C[i][j] = C11[i][j];
            C[i][j + k] = C12[i][j];
            C[i + k][j] = C21[i][j];
            C[i + k][j + k] = C22[i][j];
        }
    }
    return C;
}

Matrix strassenParallel(Matrix A, Matrix B) {
    int n = A.size();
    int m = 1;
    while (m < n) m *= 2;

    Matrix A_prep(m, std::vector<int>(m, 0));
    Matrix B_prep(m, std::vector<int>(m, 0));
    for(int i=0; i<n; ++i)
        for(int j=0; j<n; ++j) {
            A_prep[i][j] = A[i][j];
            B_prep[i][j] = B[i][j];
        }

    Matrix C_prep = strassenRecursive(A_prep, B_prep, 0);

    Matrix C(n, std::vector<int>(n));
    for(int i=0; i<n; ++i)
        for(int j=0; j<n; ++j)
            C[i][j] = C_prep[i][j];
            
    return C;
}

void merge(std::vector<int>& arr, int l, int m, int r) {
    std::vector<int> left(arr.begin() + l, arr.begin() + m + 1);
    std::vector<int> right(arr.begin() + m + 1, arr.begin() + r + 1);

    int i = 0, j = 0, k = l;
    while (i < left.size() && j < right.size()) {
        if (left[i] <= right[j]) arr[k++] = left[i++];
        else arr[k++] = right[j++];
    }
    while (i < left.size()) arr[k++] = left[i++];
    while (j < right.size()) arr[k++] = right[j++];
}

void parallelMergeSort(std::vector<int>& arr, int l, int r, int depth = 0) {
    if (l >= r) return;

    if (r - l < 10000 || depth > 4) {
        std::sort(arr.begin() + l, arr.begin() + r + 1);
        return;
    }

    int m = l + (r - l) / 2;

    auto futureLeft = std::async(std::launch::async, [&]() {
        parallelMergeSort(arr, l, m, depth + 1);
    });

    parallelMergeSort(arr, m + 1, r, depth + 1);

    futureLeft.get();
    merge(arr, l, m, r);
}

int main() {
    std::ofstream file("benchmark_results.csv");
    file << "Task,Algorithm,Size,Time_ms\n";

    std::cout << "--- Lab 3: Parallel Algorithms ---\n";
    
    std::cout << "Testing Sorting...\n";
    std::vector<int> sortSizes = {10000, 50000, 100000, 500000, 1000000, 2000000};
    
    for (int n : sortSizes) {
        auto data = generateArray(n);
        auto dataCopy = data;

        auto start = std::chrono::high_resolution_clock::now();
        std::sort(data.begin(), data.end());
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> seqTime = end - start;
        file << "Sort,Sequential," << n << "," << seqTime.count() << "\n";

        start = std::chrono::high_resolution_clock::now();
        parallelMergeSort(dataCopy, 0, n - 1);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> parTime = end - start;
        file << "Sort,Parallel," << n << "," << parTime.count() << "\n";

        if (data != dataCopy) std::cerr << "Error: Sorting incorrect for size " << n << "\n";
        
        std::cout << "Sort Size: " << n << " | Seq: " << seqTime.count() << "ms | Par: " << parTime.count() << "ms\n";
    }

    std::cout << "\nTesting Matrix Multiplication...\n";
    std::vector<int> matrixSizes = {64, 128, 256, 512, 1024};

    for (int n : matrixSizes) {
        auto A = generateMatrix(n);
        auto B = generateMatrix(n);

        auto start = std::chrono::high_resolution_clock::now();
        Matrix C_seq = multiplyNaive(A, B);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> seqTime = end - start;
        file << "Matrix,Naive," << n << "," << seqTime.count() << "\n";

        start = std::chrono::high_resolution_clock::now();
        Matrix C_par = strassenParallel(A, B);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> parTime = end - start;
        file << "Matrix,Strassen_Parallel," << n << "," << parTime.count() << "\n";

        if (C_seq != C_par) std::cerr << "Error: Matrix mult incorrect for size " << n << "\n";

        std::cout << "Matrix Size: " << n << "x" << n << " | Naive: " << seqTime.count() << "ms | Strassen: " << parTime.count() << "ms\n";
    }

    file.close();
    std::cout << "\nDone. Results saved to benchmark_results.csv. Run python script now.\n";
    return 0;
}