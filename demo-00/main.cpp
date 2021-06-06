#include <cmath>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <iostream>

using namespace std;
using namespace Eigen;

int main()
{

    // Basic Example of cpp
    std::cout << "Example of cpp \n";
    float a = 1.0, b = 2.0;
    std::cout << a << std::endl;
    std::cout << a / b << std::endl;
    std::cout << std::sqrt(b) << std::endl;
    std::cout << std::acos(-1) << std::endl;
    std::cout << std::sin(30.0 / 180.0 * acos(-1)) << std::endl;

    // Example of vector
    std::cout << "Example of vector \n";
    // vector definition
    Eigen::Vector3f v(1.0f, 2.0f, 3.0f);
    Eigen::Vector3f w(1.0f, 0.0f, 0.0f);
    // vector output
    std::cout << "Example of output \n";
    std::cout << v << std::endl;
    // vector add
    std::cout << "Example of add \n";
    std::cout << v + w << std::endl;
    // vector scalar multiply
    std::cout << "Example of scalar multiply \n";
    std::cout << v * 3.0f << std::endl;
    std::cout << 2.0f * v << std::endl;

    // Example of matrix
    std::cout << "Example of matrix \n";
    // matrix definition
    Eigen::Matrix3f i, j;
    i << 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0;
    j << 2.0, 3.0, 1.0, 4.0, 6.0, 5.0, 9.0, 7.0, 8.0;
    // matrix output
    std::cout << "Example of output \n";
    std::cout << i << std::endl;
    // matrix add i + j
    std::cout << "Example of matrix add i + j \n";
    std::cout << i + j << std::endl;
    // matrix scalar multiply i * 2.0
    cout << "Example of matrix scalar multiply i * 2.0 \n";
    cout << i * 2.0 << std::endl;
    // matrix multiply i * j
    cout << "Example of matrix scalar multiply i * j \n";
    cout << i * j << std::endl;
    // matrix multiply vector i * v
    cout << "Example of matrix scalar multiply i * v \n";
    cout << i * v << std::endl;

    /*
    * 给定一个点 P=(2,1), 将该点绕原点先逆时针旋转 45◦，再平移 (1,2), 
    * 计算出变换后点的坐标（要求用齐次坐标进行计算）
    */
    Eigen::Vector3f p(1.0f, 2.0f, 1.0f);

    Matrix3f transform, rotate;
    double theta = (45 / 180) * M_PI;
    rotate << cos(theta), -1.0 * sin(theta), 0,
        sin(theta), cos(theta), 0,
        0, 0, 1;
    transform << 1, 0, 1,
        0, 1, 2,
        0, 0, 1;

    cout << "逆时针旋转 45◦:\n"
         << rotate * p << endl;
    cout << "再平移 (1,2):\n"
         << transform * rotate * p << endl;

    Matrix3f transformByEigen, rotateByEigen;
    rotateByEigen = AngleAxisf(theta, Vector3f::UnitZ());
    Affine2f t(Translation2f(Vector2f(1.0, 2.0)));
    transformByEigen = t.matrix();
    cout
        << "逆时针旋转 45◦:\n"
        << rotateByEigen * p << endl;
    cout << "再平移 (1,2):\n"
         << transformByEigen * rotateByEigen * p << endl;
    return 0;
}