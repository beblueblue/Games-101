// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}


// static bool insideTriangle(int x, int y, const Vector3f* _v)
static bool insideTriangle(float x, float y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    auto a = _v[0];
    auto b = _v[1];
    auto c = _v[2];
    auto ab = Vector3f(b[0] - a[0], b[1] - a[1], 1.0f);
    auto bc = Vector3f(c[0] - b[0], c[1] - b[1], 1.0f);
    auto ca = Vector3f(a[0] - c[0], a[1] - c[1], 1.0f);
    auto ap = Vector3f(x - a[0], y - a[1], 1.0f);
    auto bp = Vector3f(x - b[0], y - b[1], 1.0f);
    auto cp = Vector3f(x - c[0], y - c[1], 1.0f);
    auto abc_normal = ab.cross(ca);
    bool isInside1 = ab.cross(ap).dot(abc_normal) > 0.0;
    bool isInside2 = bc.cross(bp).dot(abc_normal) > 0.0;
    bool isInside3 = ca.cross(cp).dot(abc_normal) > 0.0;

    return isInside1 == isInside2 && isInside2 == isInside3;
            
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
    
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle
    float x_min, y_min, x_max, y_max;
    auto a = v[0];
    auto b = v[1];
    auto c = v[2];
    x_min = a[0];
    y_min = a[1];
    x_max = a[0];
    y_max = a[1];
    for (int i = 1; i < 3; ++i) {
        auto x = v[i][0];
        auto y = v[i][1];
        x_min = x_min < x ? x_min : x;
        y_min = y_min < y ? y_min : y;

        x_max = x_max > x ? x_max : x;
        y_max = y_max > y ? y_max : y;
    }

    for (int i = x_min; i < x_max; ++i){
        for (int j = y_min; j < y_max; ++j){
            // if(insideTriangle(i, j, t.v)){
            //     // If so, use the following code to get the interpolated z value.
            //     auto[alpha, beta, gamma] = computeBarycentric2D(i, j, t.v);
            //     float w_reciprocal = 1.0/(alpha / a.w() + beta / b.w() + gamma / c.w());
            //     float z_interpolated = alpha * a.z() / a.w() + beta * b.z() / b.w() + gamma * c.z() / c.w();
            //     z_interpolated *= w_reciprocal;
            //     Eigen::Vector3f point = Eigen::Vector3f(i, j, 1.0f);
            //     auto index = get_index(i, j);
            //     if(depth_buf[index] > -z_interpolated) {
            //         // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
            //         set_pixel(point, t.getColor());
            //         depth_buf[index] = -z_interpolated;
            //     }
            // }
            int insideCount = 0;
            for (int x_sup: {0, 1}) {
                for (int y_sup: {0, 1}) {
                    float x_pos = (float)i + 0.25 + x_sup*0.5;
                    float y_pos = (float)j + 0.25 + y_sup*0.5;

                    // If so, use the following code to get the interpolated z value.
                    auto[alpha, beta, gamma] = computeBarycentric2D(i, j, t.v);
                    float w_reciprocal = 1.0/(alpha / a.w() + beta / b.w() + gamma / c.w());
                    float z_interpolated = alpha * a.z() / a.w() + beta * b.z() / b.w() + gamma * c.z() / c.w();
                    z_interpolated *= w_reciprocal;

                    int buff_ind = (i*2 + x_sup) + (j*2 + y_sup)*width*2;
                    if (insideTriangle(x_pos, y_pos, t.v))
                    {
                        // depth_buf_msaa2x2, ??????????????????????????????
                        if ((-z_interpolated<depth_buf_msaa2x2[buff_ind]))
                        {
                            depth_buf_msaa2x2[buff_ind] = -z_interpolated;
                            ++insideCount;
                        }
                    }
                }
            }
            if((insideCount > 0)){
                
                Eigen::Vector3f point = Eigen::Vector3f(i, j, 1.0f);
                auto index = get_index(i, j);
                mix_pixel(point, t.getColor() * insideCount / 4.0f);
            }
        }
    }
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
        std::fill(depth_buf_msaa2x2.begin(), depth_buf_msaa2x2.end(), std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
    depth_buf_msaa2x2.resize(w * h * 2 * 2);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

void rst::rasterizer::mix_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] += color;
}

// clang-format on