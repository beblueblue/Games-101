#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace Eigen;

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_rotation(Vector3f axis, float angle)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();
    double theta = (angle / 180) * M_PI;
    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -axis[0],
        0, 1, 0, -axis[1],
        0, 0, 1, -axis[2],
        0, 0, 0, 1;

    Eigen::Matrix3f rotate;
    Eigen::Matrix3f axisMatrix;
    Eigen::Matrix3f identity = Eigen::Matrix3f::Identity();
    axisMatrix << 0, -axis[2], axis[1],
        axis[2], 0, -axis[0],
        -axis[1], axis[0], 0;

    rotate = cos(theta) * identity + (1 - cos(theta)) * axis * axis.transpose() + sin(theta) * axisMatrix;

    Eigen::Matrix4f rotateHomo;
    rotateHomo << rotate(0, 0), rotate(0, 1), rotate(0, 2), 0,
        rotate(1, 0), rotate(1, 1), rotate(1, 2), 0,
        rotate(2, 0), rotate(2, 1), rotate(2, 2), 0,
        0, 0, 0, 1;

    view = translate.inverse() * rotateHomo * translate;
    return view;
}

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.
    float theta = (rotation_angle / 180.0) * MY_PI;
    Eigen::Matrix4f rotate;
    rotate << std::cos(theta), -std::sin(theta), 0, 0,
        std::sin(theta), std::cos(theta), 0, 0,
        0, 0, 1.0, 0,
        0, 0, 0, 1.0;
    model = rotate * model;

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();
    float t, r, n, f;
    n = -zNear;
    f = -zFar;
    t = zNear * tan(eye_fov / 360.0f * MY_PI);
    r = t * aspect_ratio;

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.
    Eigen::Matrix4f scale;
    scale << 1 / r, 0, 0, 0,
        0, 1 / t, 0, 0,
        0, 0, 2 / (n - f), 0,
        0, 0, 0, 1;

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, -1 * ((n + f) / 2),
        0, 0, 0, 1;

    Eigen::Matrix4f ortho;
    ortho << n, 0, 0, 0,
        0, n, 0, 0,
        0, 0, n + f, -1 * n * f,
        0, 0, 1, 0;

    projection = scale * translate * ortho;

    return projection;
}

int main(int argc, const char **argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3)
    {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4)
        {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;
    Eigen::Vector3f demoAxis = {0, 0, 1};

    if (command_line)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_rotation(demoAxis, angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a')
        {
            angle += 10;
        }
        else if (key == 'd')
        {
            angle -= 10;
        }
    }

    return 0;
}
