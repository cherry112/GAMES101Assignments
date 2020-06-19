// clang-format off
#include <iostream>
#include <opencv2/opencv.hpp>
#include "rasterizer.hpp"
#include "global.hpp"
#include "Triangle.hpp"

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1,0,0,-eye_pos[0],
                 0,1,0,-eye_pos[1],
                 0,0,1,-eye_pos[2],
                 0,0,0,1;

    view = translate*view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    float radian = rotation_angle * MY_PI / 180.0;
    float sinr = sin(radian);
    float cosr = cos(radian);
    model << cosr, -sinr, 0.0, 0.0, \
            sinr, cosr, 0.0, 0.0, \
            0.0, 0.0, 1.0, 0.0, \
            0.0, 0.0, 0.0, 1.0;
    return model;
}

Eigen::Matrix4f get_rotation(Vector3f axis, float angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    float x = axis[0];
    float y = axis[1];
    float z = axis[2];
    float norm = sqrt(x * x + y * y + z * z);
    float n_x = x / norm;
    float n_y = y / norm;
    float n_z = z / norm;
    Vector3f n_axis(n_x, n_y, n_z);
    
    float radian = angle * MY_PI / 180.0;
    float sinr = sin(radian);
    float cosr = cos(radian);

    Eigen::Matrix3f I = Eigen::Matrix3f::Identity();
    
    Eigen::Matrix3f N;
    N << 0.0, -n_z, n_y, \
        n_z, 0.0, -n_x, \
        -n_y, n_x, 0.0;

    model.block(0, 0, 3, 3) = cosr * I + (1 - cosr) * n_axis * n_axis.transpose() + sinr * N;
    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar)
{
    // TODO: Copy-paste your implementation from the previous assignment.
    Eigen::Matrix4f projection;
    
    Eigen::Matrix4f m_persp2ortho;
    m_persp2ortho << zNear, 0.0, 0.0, 0.0, \
        0.0, zNear, 0.0, 0.0, \
        0.0, 0.0, zNear + zFar, -1.0 * zNear * zFar, \
        0.0, 0.0, 1.0, 0.0;

    float radian = eye_fov * MY_PI / 180.0;
    float t = zNear * tan(radian / 2);
    float r = t * aspect_ratio;
    float b = -t;
    float l = -r;

    Eigen::Matrix4f m_ortho_trans;
    m_ortho_trans << 1.0, 0.0, 0.0, -(r + l) / 2, \
		0.0, 1.0, 0.0, -(t + b) / 2, \
		0.0, 0.0, 1.0, -(zNear + zFar) / 2, \
		0.0, 0.0, 0.0, 1.0;

    Eigen::Matrix4f m_ortho_scale;
    m_ortho_scale << 2.0 / (r - l), 0.0, 0.0, 0.0, \
		0.0, 2.0 / (t - b), 0.0, 0.0, \
		0.0, 0.0, 2.0 / (zNear - zFar), 0.0, \
		0.0, 0.0, 0.0, 1.0;

    projection = m_ortho_scale * m_ortho_trans * m_persp2ortho;
    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc == 2)
    {
        command_line = true;
        filename = std::string(argv[1]);
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0,0,5};


    std::vector<Eigen::Vector3f> pos
            {
                    {2, 0, -2},
                    {0, 2, -2},
                    {-2, 0, -2},
                    {3.5, -1, -5},
                    {2.5, 1.5, -5},
                    {-1, 0.5, -5}
            };

    std::vector<Eigen::Vector3i> ind
            {
                    {0, 1, 2},
                    {3, 4, 5}
            };

    std::vector<Eigen::Vector3f> cols
            {
                    {217.0, 238.0, 185.0},
                    {217.0, 238.0, 185.0},
                    {217.0, 238.0, 185.0},
                    {185.0, 217.0, 238.0},
                    {185.0, 217.0, 238.0},
                    {185.0, 217.0, 238.0}
            };

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);
    auto col_id = r.load_colors(cols);

    int key = 0;
    int frame_count = 0;

    if (command_line)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

        cv::imwrite(filename, image);

        return 0;
    }

    while(key != 27)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';
    }

    return 0;
}
// clang-format on