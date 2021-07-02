#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata)
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 8)
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
                  << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window)
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001)
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                     3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t)
{
    // TODO: Implement de Casteljau's algorithm
    int control_points_size = control_points.size();

    if (control_points_size == 1)
    {
        return control_points[0];
    }
    else
    {
        cv::Point2f left_bezier_point, right_bezier_point;
        std::vector<cv::Point2f> sub_left_points(&(control_points[0]), &(control_points[control_points_size - 1]));
        std::vector<cv::Point2f> sub_right_points(&(control_points[1]), &(control_points[control_points_size]));
        left_bezier_point = recursive_bezier(sub_left_points, t);
        right_bezier_point = recursive_bezier(sub_right_points, t);
        return left_bezier_point * (1 - t) + right_bezier_point * t;
    }
}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window)
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's
    // recursive Bezier algorithm.
    static float t_step = 0.001;
    static cv::Point2f point;
    for (float t = 0.0f; t <= 1.0f; t += t_step)
    {
        point = recursive_bezier(control_points, t);
        window.at<cv::Vec3b>(point.y, point.x)[3] = 255;
    }
}

int main()
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27)
    {
        for (auto &point : control_points)
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4)
        {
            std::vector<cv::Point2f> sub_left_points(&(control_points[0]), &(control_points[4]));
            // red line
            naive_bezier(sub_left_points, window);
        }
        if (control_points.size() == 8)
        {
            std::vector<cv::Point2f> sub_right_points(&(control_points[4]), &(control_points[8]));
            // blue line
            bezier(sub_right_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

    return 0;
}
