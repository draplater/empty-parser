//
// Created by chenyufei on 17-9-25.
//

#ifndef NN_PARSER_DYNET20_MATRIX_VIEW_H
#define NN_PARSER_DYNET20_MATRIX_VIEW_H

#include <cstdlib>
#include <stdexcept>

namespace MatrixView {
    template <typename T>
    class Array2D {
    protected:
        T* base = nullptr;
    public:
        size_t dim_1 = 0, dim_2 = 0;
        Array2D() = default;
        Array2D(T* base, size_t dim_1, size_t dim_2): base(base), dim_1(dim_1), dim_2(dim_2) {}
        inline T& operator()(int x1, int x2) {
            return base[x1 * dim_2 + x2];
        }
    };

    template <typename T>
    class Array2DRotated: public Array2D<T> {
    public:
        Array2DRotated() = default;
        Array2DRotated(T* base, size_t dim_1, size_t dim_2): Array2D<T>(base, dim_1, dim_2) {}
        inline T& operator()(int x1, int x2) {
            auto x1_ = (x1 + 1) % this->dim_1;
            auto x2_ = (x2 + 1) % this->dim_2;
            return this->base[x1_ * this->dim_2 + x2_];
        }
    };

    template <typename T>
    class Array3D {
    protected:
        T* base = nullptr;
    public:
        size_t dim_1 = 0, dim_2 = 0, dim_3 = 0;
        Array3D() = default;
        Array3D(T* base, size_t dim_1, size_t dim_2, size_t dim_3): base(base), dim_1(dim_1), dim_2(dim_2), dim_3(dim_3) {}
        inline T& operator()(int x1, int x2, int x3) {
            return base[x1 * dim_2 * dim_3 + x2 * dim_3 + x3];
        }
    };

    template <typename T>
    class Array3DRotated: public Array3D<T> {
    public:
        Array3DRotated() = default;
        Array3DRotated(T* base, size_t dim_1, size_t dim_2, size_t dim_3): Array3D<T>(base, dim_1, dim_2, dim_3) {}
        inline T& operator()(int x1, int x2, int x3) {
            auto x1_ = (x1 + 1) % this->dim_1;
            auto x2_ = (x2 + 1) % this->dim_2;
            auto x3_ = (x3 + 1) % this->dim_3;
            return this->base[x1_ * this->dim_2 * this->dim_3 + x2_ * this->dim_3 + x3_];
        }
    };
}

#endif //NN_PARSER_DYNET20_MATRIX_VIEW_H
