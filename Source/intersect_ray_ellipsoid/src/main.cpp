#include <iostream>
#include <fstream>
#include <vector>
#include <random>

#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp> 

#include <geom_view.h>

void dump3(std::ofstream& file, float x, float y, float z) {
    file << "(" << x << "," << y << "," << z << ")";
}

struct App
{
    enum slider_types
    {
        SLIDER_N,
        SLIDER_M,
        
        SLIDER_MU_0,
        SLIDER_MU_1,
        SLIDER_MU_2,
        
        SLIDER_SIGMA_0,
        SLIDER_SIGMA_1,
        SLIDER_SIGMA_2,

        SLIDER_QUATERNION_0,
        SLIDER_QUATERNION_1,
        SLIDER_QUATERNION_2,
        SLIDER_QUATERNION_3,

        SLIDER_Q,
    };

    static void callback(void* raw)
    {
        using callback_typle = std::tuple<App*, std::size_t>;
        auto* typle = reinterpret_cast<callback_typle*>(raw);

        auto* app = std::get<0>(*typle);
        auto value = app->sliders[std::get<1>(*typle)]->value;

        switch (std::get<1>(*typle))
        {
        case slider_types::SLIDER_N:
            app->n = std::size_t(value);
            break;
        case slider_types::SLIDER_M:
            app->m = std::size_t(value);
            break;
        case slider_types::SLIDER_MU_0:
            app->mu[0] = float(value);
            break;
        case slider_types::SLIDER_MU_1:
            app->mu[1] = float(value);
            break;
        case slider_types::SLIDER_MU_2:
            app->mu[2] = float(value);
            break;
        case slider_types::SLIDER_SIGMA_0:
            app->sigma[0] = float(value);
            break;
        case slider_types::SLIDER_SIGMA_1:
            app->sigma[1] = float(value);
            break;
        case slider_types::SLIDER_SIGMA_2:
            app->sigma[2] = float(value);
            break;
        case slider_types::SLIDER_QUATERNION_0:
            app->quaternion[0] = float(value);
            break;
        case slider_types::SLIDER_QUATERNION_1:
            app->quaternion[1] = float(value);
            break;
        case slider_types::SLIDER_QUATERNION_2:
            app->quaternion[2] = float(value);
            break;
        case slider_types::SLIDER_QUATERNION_3:
            app->quaternion[3] = float(value);
            break;
        case slider_types::SLIDER_Q:
            app->r = float(value);
            break;
        default:
            break;
        }

        app->dump();
        app->gv.reload();
    }

    static void move_control_point(void *raw, std::vector<std::string> &sId, double x, double y, double z)
    {
        auto* app = reinterpret_cast<App*>(raw);

        if (std::stoi(sId.back()) == 0) // center
        {
            app->center = glm::vec3((float)x, (float)y, (float)z);
        }
        else if (std::stoi(sId.back()) == 1) // dir
        {
            app->dir = glm::vec3((float)x, (float)y, (float)z);
        }
        else
        {
            std::cout << "Unknown cp" << std::endl;
        }

        app->dump();
        app->gv.reload();
    }

    void init()
    {
        gv.init("output.txt");

        panel = geom_view_control_panel::makeCustomPanel("my_panel");
        gv.addCustomControl(std::static_pointer_cast<geom_view_control>(panel));

        sliders[0] = geom_view_control_slidevalue::makeCustomSlidevalue("n_slider", 1, 100);
        sliders[1] = geom_view_control_slidevalue::makeCustomSlidevalue("m_slider", 1, 100);

        sliders[2] = geom_view_control_slidevalue::makeCustomSlidevalue("mu[0]_slider", -100.0f, 100.0f);
        sliders[3] = geom_view_control_slidevalue::makeCustomSlidevalue("mu[1]_slider", -100.0f, 100.0f);
        sliders[4] = geom_view_control_slidevalue::makeCustomSlidevalue("mu[2]_slider", -100.0f, 100.0f);

        sliders[5] = geom_view_control_slidevalue::makeCustomSlidevalue("sigma[0]_slider", 0.0001f, 100.0f);
        sliders[6] = geom_view_control_slidevalue::makeCustomSlidevalue("sigma[1]_slider", 0.0001f, 100.0f);
        sliders[7] = geom_view_control_slidevalue::makeCustomSlidevalue("sigma[2]_slider", 0.0001f, 100.0f);

        sliders[8] = geom_view_control_slidevalue::makeCustomSlidevalue("quaterion[0]_slider", -100.0f, 100.0f);
        sliders[9] = geom_view_control_slidevalue::makeCustomSlidevalue("quaterion[1]_slider", -100.0f, 100.0f);
        sliders[10] = geom_view_control_slidevalue::makeCustomSlidevalue("quaterion[2]_slider", -100.0f, 100.0f);
        sliders[11] = geom_view_control_slidevalue::makeCustomSlidevalue("quaterion[3]_slider", -100.0f, 100.0f);

        sliders[12] = geom_view_control_slidevalue::makeCustomSlidevalue("Q", 0.0001f, 100.0f);
        
        for (std::size_t idx = 0; idx < sliders.size(); ++idx)
        {
            panel->add(std::static_pointer_cast<geom_view_control>(sliders[idx]));
            
            auto* data = new std::tuple<App*, std::size_t>(this, idx);
            
            sliders[idx]->callback = callback;
            sliders[idx]->callback_data = data;
            
            if (idx < sliders.size() - 1)
            {
                auto empty_label = geom_view_control_textLabel::makeCustomTextLabel("");
                panel->add(std::static_pointer_cast<geom_view_control>(empty_label));
                empty_label->newline();
            }
        }

        sliders[0]->value = n;
        sliders[1]->value = m;
        sliders[2]->value = mu.x;
        sliders[3]->value = mu.y;
        sliders[4]->value = mu.z;
        sliders[5]->value = sigma.x;
        sliders[6]->value = sigma.y;
        sliders[7]->value = sigma.z;
        sliders[8]->value = quaternion[0];
        sliders[9]->value = quaternion[1];
        sliders[10]->value = quaternion[2];
        sliders[11]->value = quaternion[3];
        sliders[12]->value = r;

        gv.setCallBack(this, &move_control_point);
    }

    void close()
    {
        gv.close();
    }

    void run()
    {
        dump();
        gv.reload();

        std::string cmd;

        while (true)
        {
            std::cin >> cmd;
            
            if (cmd == "exit")
            {
                break;
            }
        }
    }

    void dump()
    {

        float min_phi = 0.0f;
        float max_phi = (float)M_PI * 2.0f;

        float min_theta = 0.0f;
        float max_theta = (float)M_PI * 1.0f;

        glm::mat3 scale = glm::mat3(
            glm::vec3(sigma[0], 0.0f, 0.0f),
            glm::vec3(0.0f, sigma[1], 0.0f),
            glm::vec3(0.0f, 0.0f, sigma[2])
        );

        glm::mat3 inv_scale = glm::mat3(
            glm::vec3(1.0f / sigma[0], 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f / sigma[1], 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f / sigma[2])
        );

        glm::mat3 rotation = glm::mat3(glm::mat4_cast(glm::quat(glm::normalize(quaternion))));

        phis.clear();
        phis.resize(n);

        for (std::size_t idx = 0; idx < n; ++idx)
        {
            float phi = min_phi + (float)idx * (max_phi - min_phi) / float(n - 1);
            phis[idx] = phi;
        }

        thetas.clear();
        thetas.resize(m);

        for (std::size_t idx = 0; idx < m; ++idx)
        {
            float theta = min_theta + (float)idx * (max_theta - min_theta) / float(m - 1);
            thetas[idx] = theta;
        }

        std::ofstream file("output.txt");

        if (file.is_open() == false)
        {
            std::cout << "ERROR: Can't open file" << std::endl;
            return;
        }

        // dump ellipsoid
        {
            file << "lines: ellipsoid_lines" << std::endl;
    
            for (std::size_t idx_i = 0; idx_i < n - 1; ++idx_i)
            {
                for (std::size_t idx_j = 0; idx_j < m - 1; ++idx_j)
                {
                    glm::vec3 ld = glm::vec3(
                        r * std::cos(phis[idx_i]) * std::sin(thetas[idx_j]),
                        r * std::sin(phis[idx_i]) * std::sin(thetas[idx_j]),
                        r * std::cos(thetas[idx_j])
                    );
                    
                    glm::vec3 rd = glm::vec3(
                        r * std::cos(phis[idx_i + 1]) * std::sin(thetas[idx_j]),
                        r * std::sin(phis[idx_i + 1]) * std::sin(thetas[idx_j]),
                        r * std::cos(thetas[idx_j])
                    );
    
                    glm::vec3 lu = glm::vec3(
                        r * std::cos(phis[idx_i]) * std::sin(thetas[idx_j + 1]),
                        r * std::sin(phis[idx_i]) * std::sin(thetas[idx_j + 1]),
                        r * std::cos(thetas[idx_j + 1])
                    );
    
                    glm::vec3 ru = glm::vec3(
                        r * std::cos(phis[idx_i + 1]) * std::sin(thetas[idx_j + 1]),
                        r * std::sin(phis[idx_i + 1]) * std::sin(thetas[idx_j + 1]),
                        r * std::cos(thetas[idx_j + 1])
                    );
    
                    ld = rotation * scale * ld + mu;
                    rd = rotation * scale * rd + mu;
                    lu = rotation * scale * lu + mu;
                    ru = rotation * scale * ru + mu;
    
                    dump3(file, ld[0], ld[1], ld[2]);
                    dump3(file, rd[0], rd[1], rd[2]);
                    dump3(file, 1.0f, 1.0f, 1.0f);
                    file << "\n";
                    
                    dump3(file, rd[0], rd[1], rd[2]);
                    dump3(file, ru[0], ru[1], ru[2]);
                    dump3(file, 1.0f, 1.0f, 1.0f);
                    file << "\n";
                    
                    dump3(file, ru[0], ru[1], ru[2]);
                    dump3(file, lu[0], lu[1], lu[2]);
                    dump3(file, 1.0f, 1.0f, 1.0f);
                    file << "\n";
                    
                    dump3(file, ld[0], ld[1], ld[2]);
                    dump3(file, lu[0], lu[1], lu[2]);
                    dump3(file, 1.0f, 1.0f, 1.0f);
                    file << "\n";
                }
            }
        }


        // calculate AABB
        auto r_1 = (glm::transpose(rotation))[0];
        auto r_2 = (glm::transpose(rotation))[1];
        auto r_3 = (glm::transpose(rotation))[2];

        glm::vec3 min_box = glm::vec3(
            mu[0] - float(r * std::sqrt((glm::dot(r_1 * r_1, sigma * sigma)))),
            mu[1] - float(r * std::sqrt((glm::dot(r_2 * r_2, sigma * sigma)))),
            mu[2] - float(r * std::sqrt((glm::dot(r_3 * r_3, sigma * sigma))))
        );

        glm::vec3 max_box = glm::vec3(
            mu[0] + float(r * std::sqrt((glm::dot(r_1 * r_1, sigma * sigma)))),
            mu[1] + float(r * std::sqrt((glm::dot(r_2 * r_2, sigma * sigma)))),
            mu[2] + float(r * std::sqrt((glm::dot(r_3 * r_3, sigma * sigma))))
        );

        // dump AABB
        {
            file << "lines: bb" << std::endl;

            dump3(file, min_box.x, min_box.y, min_box.z);
            dump3(file, max_box.x, min_box.y, min_box.z);
            dump3(file, 1.0f, 1.0f, 1.0f);
            file << "\n";

            dump3(file, max_box.x, min_box.y, min_box.z);
            dump3(file, max_box.x, max_box.y, min_box.z);
            dump3(file, 1.0f, 1.0f, 1.0f);
            file << "\n";

            dump3(file, max_box.x, max_box.y, min_box.z);
            dump3(file, min_box.x, max_box.y, min_box.z);
            dump3(file, 1.0f, 1.0f, 1.0f);
            file << "\n";

            dump3(file, min_box.x, max_box.y, min_box.z);
            dump3(file, min_box.x, min_box.y, min_box.z);
            dump3(file, 1.0f, 1.0f, 1.0f);
            file << "\n";

            dump3(file, min_box.x, min_box.y, max_box.z);
            dump3(file, max_box.x, min_box.y, max_box.z);
            dump3(file, 1.0f, 1.0f, 1.0f);
            file << "\n";

            dump3(file, max_box.x, min_box.y, max_box.z);
            dump3(file, max_box.x, max_box.y, max_box.z);
            dump3(file, 1.0f, 1.0f, 1.0f);
            file << "\n";

            dump3(file, max_box.x, max_box.y, max_box.z);
            dump3(file, min_box.x, max_box.y, max_box.z);
            dump3(file, 1.0f, 1.0f, 1.0f);
            file << "\n";

            dump3(file, min_box.x, max_box.y, max_box.z);
            dump3(file, min_box.x, min_box.y, max_box.z);
            dump3(file, 1.0f, 1.0f, 1.0f);
            file << "\n";

            dump3(file, min_box.x, min_box.y, min_box.z);
            dump3(file, min_box.x, min_box.y, max_box.z);
            dump3(file, 1.0f, 1.0f, 1.0f);
            file << "\n";

            dump3(file, max_box.x, min_box.y, min_box.z);
            dump3(file, max_box.x, min_box.y, max_box.z);
            dump3(file, 1.0f, 1.0f, 1.0f);
            file << "\n";

            dump3(file, max_box.x, max_box.y, min_box.z);
            dump3(file, max_box.x, max_box.y, max_box.z);
            dump3(file, 1.0f, 1.0f, 1.0f);
            file << "\n";

            dump3(file, min_box.x, max_box.y, min_box.z);
            dump3(file, min_box.x, max_box.y, max_box.z);
            dump3(file, 1.0f, 1.0f, 1.0f);
            file << "\n";
        }

        file << "control_points: cps" << std::endl;
        
        dump3(file, center.x, center.y, center.z);
        file << 5;
        dump3(file, 1.0f, 0.0f, 0.0f);
        file << "\n";

        dump3(file, dir.x, dir.y, dir.z);
        file << 5;
        dump3(file, 0.0f, 1.0f, 0.0f);
        file << "\n";

        file << "vectors: camera_ray" << std::endl;
        dump3(file, center.x, center.y, center.z);
        dump3(file, dir.x, dir.y, dir.z);
        file << "\n";

        glm::vec3 direction = glm::normalize(dir - center);

        // intersection points in sphere space

        glm::vec3 e_sphere = inv_scale * glm::transpose(rotation) * (center - mu);
        glm::vec3 d_sphere = inv_scale * glm::transpose(rotation) * direction;

        float d_dot_d = glm::dot(d_sphere, d_sphere);
        float o_dot_d = glm::dot(e_sphere, d_sphere);

        float t_ca = -o_dot_d / d_dot_d;

        glm::vec3 closest_point = e_sphere + t_ca * d_sphere;
        float d2 = glm::dot(closest_point, closest_point);

        float Q2 = r * r;

        if (d2 <= Q2) {

            float t_hc = glm::sqrt(Q2 - d2) / glm::sqrt(d_dot_d);
    
            float t1 = t_ca - t_hc;
            float t2 = t_ca + t_hc;

            glm::vec3 sphere_inter_1 = e_sphere + t1 * d_sphere;
            glm::vec3 sphere_inter_2 = e_sphere + t2 * d_sphere;
                    
            file << "points: intersection_points_with_sphere" << std::endl;

            dump3(file, sphere_inter_1.x + 10.0f, sphere_inter_1.y + 10.0f, sphere_inter_1.z + 10.0f);
            file << 10;
            dump3(file, 1.0f, 0.0f, 1.0f);
            file << "\n";

            dump3(file, sphere_inter_2.x + 10.0f, sphere_inter_2.y + 10.0f, sphere_inter_2.z + 10.0f);
            file << 10;
            dump3(file, 1.0f, 0.0f, 1.0f);
            file << "\n";
            
            glm::vec3 ellipsoid_inter_1 = center + t1 * direction;
            glm::vec3 ellipsoid_inter_2 = center + t2 * direction;
                        
            file << "points: intersection_points_with_ellipsoid" << std::endl;
    
            dump3(file, ellipsoid_inter_1.x, ellipsoid_inter_1.y, ellipsoid_inter_1.z);
            file << 10;
            dump3(file, 1.0f, 0.0f, 1.0f);
            file << "\n";
            
            dump3(file, ellipsoid_inter_2.x, ellipsoid_inter_2.y, ellipsoid_inter_2.z);
            file << 10;
            dump3(file, 1.0f, 0.0f, 1.0f);
            file << "\n";
        }



        float tx1 = (min_box.x - center.x) / direction.x;
        float tx2 = (max_box.x - center.x) / direction.x;
        
        float ty1 = (min_box.y - center.y) / direction.y;
        float ty2 = (max_box.y - center.y) / direction.y;
        
        float tz1 = (min_box.z - center.z) / direction.z;
        float tz2 = (max_box.z - center.z) / direction.z;

        float t_min_x = min(tx1, tx2);
        float t_max_x = max(tx1, tx2);
        
        float t_min_y = min(ty1, ty2);
        float t_max_y = max(ty1, ty2);
        
        float t_min_z = min(tz1, tz2);
        float t_max_z = max(tz1, tz2);

        float t_enter = max(t_min_x, t_min_y, t_min_z);
        float t_exit  = min(t_max_x, t_max_y, t_max_z);
        
        glm::vec3 intersection_aabb_1 = center + t_enter * direction;
        glm::vec3 intersection_aabb_2 = center + t_exit * direction;
        
        file << "points: intersection_points_with_aabb" << std::endl;
        
        dump3(file, intersection_aabb_1.x, intersection_aabb_1.y, intersection_aabb_1.z);
        file << 10;
        dump3(file, 0.0f, 1.0f, 1.0f);
        file << "\n";
        
        dump3(file, intersection_aabb_2.x, intersection_aabb_2.y, intersection_aabb_2.z);
        file << 10;
        dump3(file, 0.0f, 1.0f, 1.0f);
        file << "\n";


        // dump sphere
        file << "lines: sphere_lines" << std::endl;
    
        for (std::size_t idx_i = 0; idx_i < n - 1; ++idx_i)
        {
            for (std::size_t idx_j = 0; idx_j < m - 1; ++idx_j)
            {
                glm::vec3 ld = glm::vec3(
                    r * std::cos(phis[idx_i]) * std::sin(thetas[idx_j]),
                    r * std::sin(phis[idx_i]) * std::sin(thetas[idx_j]),
                    r * std::cos(thetas[idx_j])
                );
                
                glm::vec3 rd = glm::vec3(
                    r * std::cos(phis[idx_i + 1]) * std::sin(thetas[idx_j]),
                    r * std::sin(phis[idx_i + 1]) * std::sin(thetas[idx_j]),
                    r * std::cos(thetas[idx_j])
                );

                glm::vec3 lu = glm::vec3(
                    r * std::cos(phis[idx_i]) * std::sin(thetas[idx_j + 1]),
                    r * std::sin(phis[idx_i]) * std::sin(thetas[idx_j + 1]),
                    r * std::cos(thetas[idx_j + 1])
                );

                glm::vec3 ru = glm::vec3(
                    r * std::cos(phis[idx_i + 1]) * std::sin(thetas[idx_j + 1]),
                    r * std::sin(phis[idx_i + 1]) * std::sin(thetas[idx_j + 1]),
                    r * std::cos(thetas[idx_j + 1])
                );

                ld = rotation * scale * ld + mu;
                rd = rotation * scale * rd + mu;
                lu = rotation * scale * lu + mu;
                ru = rotation * scale * ru + mu;

                ld = inv_scale * glm::transpose(rotation) * (ld - mu);
                rd = inv_scale * glm::transpose(rotation) * (rd - mu);
                lu = inv_scale * glm::transpose(rotation) * (lu - mu);
                ru = inv_scale * glm::transpose(rotation) * (ru - mu);

                ld += glm::vec3(10.0f, 10.0f, 10.0f);
                rd += glm::vec3(10.0f, 10.0f, 10.0f);
                lu += glm::vec3(10.0f, 10.0f, 10.0f);
                ru += glm::vec3(10.0f, 10.0f, 10.0f);

                dump3(file, ld[0], ld[1], ld[2]);
                dump3(file, rd[0], rd[1], rd[2]);
                dump3(file, 1.0f, 1.0f, 1.0f);
                file << "\n";
                
                dump3(file, rd[0], rd[1], rd[2]);
                dump3(file, ru[0], ru[1], ru[2]);
                dump3(file, 1.0f, 1.0f, 1.0f);
                file << "\n";
                
                dump3(file, ru[0], ru[1], ru[2]);
                dump3(file, lu[0], lu[1], lu[2]);
                dump3(file, 1.0f, 1.0f, 1.0f);
                file << "\n";
                
                dump3(file, ld[0], ld[1], ld[2]);
                dump3(file, lu[0], lu[1], lu[2]);
                dump3(file, 1.0f, 1.0f, 1.0f);
                file << "\n";
            }
        }

        file << "points: sphere_key_points" << std::endl;
        
        glm::vec3 sphere_center = inv_scale * glm::transpose(rotation) * (center - mu);
        glm::vec3 sphere_dir_point = inv_scale * glm::transpose(rotation) * (dir - mu);

        dump3(file, sphere_center.x + 10.0f, sphere_center.y + 10.0f, sphere_center.z + 10.0f);
        file << 5;
        dump3(file, 1.0f, 0.0f, 0.0f);
        file << "\n";

        dump3(file, sphere_dir_point.x + 10.0f, sphere_dir_point.y + 10.0f, sphere_dir_point.z + 10.0f);
        file << 5;
        dump3(file, 0.0f, 1.0f, 0.0f);
        file << "\n";

        file << "vectors: camera_ray_sphere" << std::endl;
        dump3(file, sphere_center.x + 10.0f, sphere_center.y + 10.0f, sphere_center.z + 10.0f);
        dump3(file, sphere_dir_point.x + 10.0f, sphere_dir_point.y + 10.0f, sphere_dir_point.z + 10.0f);
        file << "\n";

        file.close();
    }

private:

    // geom_view object
    geom_view gv;

    std::shared_ptr<geom_view_control_panel> panel;
    std::array<std::shared_ptr<geom_view_control_slidevalue>, 13> sliders;

    // uv points
    std::size_t n = 100;
    std::size_t m = 100;

    std::vector<float> phis;
    std::vector<float> thetas;

    glm::vec3 mu = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 sigma = glm::vec3(0.00001f, 1.0f, 1.0f);
    glm::vec4 quaternion = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);

    float r = 1.0f;

    glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 dir = glm::vec3(10.0f, 0.0f, 0.0f);
};

int main()
{

    App app;
    app.init();
    app.run();
    app.close();

    return 0;
}