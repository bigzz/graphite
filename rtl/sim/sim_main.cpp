#include <SDL.h>
#include <Vtop.h>
#include <cube.h>
#include <graphite.h>
#include <teapot.h>
#include <verilated.h>

#include <deque>
#include <iostream>
#include <limits>

#define FB_WIDTH 128
#define FB_HEIGHT 128

#define OP_SET_X0 0
#define OP_SET_Y0 1
#define OP_SET_X1 2
#define OP_SET_Y1 3
#define OP_SET_X2 4
#define OP_SET_Y2 5
#define OP_SET_U0 6
#define OP_SET_V0 7
#define OP_SET_U1 8
#define OP_SET_V1 9
#define OP_SET_U2 10
#define OP_SET_V2 11
#define OP_SET_COLOR 12
#define OP_CLEAR 13
#define OP_DRAW_LINE 14
#define OP_DRAW_TRIANGLE 15

struct Command {
    uint16_t opcode : 4;
    uint16_t param : 12;
};

std::deque<Command> g_commands;

void pulse_clk(Vtop* top) {
    top->contextp()->timeInc(1);
    top->clk = 1;
    top->eval();

    top->contextp()->timeInc(1);
    top->clk = 0;
    top->eval();
}

static void swapi(int* a, int* b) {
    int c = *a;
    *a = *b;
    *b = c;
}

void xd_draw_line(int x0, int y0, int x1, int y1, int color) {
    if (y0 > y1) {
        swapi(&x0, &x1);
        swapi(&y0, &y1);
    }

    Command c;
    c.opcode = OP_SET_COLOR;
    c.param = color | color << 4 | color << 8;
    g_commands.push_back(c);
    c.opcode = OP_SET_X0;
    c.param = x0;
    g_commands.push_back(c);
    c.opcode = OP_SET_Y0;
    c.param = y0;
    g_commands.push_back(c);
    c.opcode = OP_SET_X1;
    c.param = x1;
    g_commands.push_back(c);
    c.opcode = OP_SET_Y1;
    c.param = y1;
    g_commands.push_back(c);
    c.opcode = OP_DRAW_LINE;
    c.param = 0;
    g_commands.push_back(c);
}

void xd_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, int color) {
    xd_draw_line(x0, y0, x1, y1, color);
    xd_draw_line(x1, y1, x2, y2, color);
    xd_draw_line(x2, y2, x0, y0, color);
}

void xd_draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, int color) {
    Command c;
    c.opcode = OP_SET_COLOR;
    c.param = color | color << 4 | color << 8;
    g_commands.push_back(c);
    c.opcode = OP_SET_X0;
    c.param = x0;
    g_commands.push_back(c);
    c.opcode = OP_SET_Y0;
    c.param = y0;
    g_commands.push_back(c);
    c.opcode = OP_SET_X1;
    c.param = x1;
    g_commands.push_back(c);
    c.opcode = OP_SET_Y1;
    c.param = y1;
    g_commands.push_back(c);
    c.opcode = OP_SET_X2;
    c.param = x2;
    g_commands.push_back(c);
    c.opcode = OP_SET_Y2;
    c.param = y2;
    g_commands.push_back(c);
    c.opcode = OP_DRAW_TRIANGLE;
    c.param = 0;
    g_commands.push_back(c);
}

void xd_draw_filled_rectangle(int x0, int y0, int x1, int y1, int color) {}

void xd_draw_textured_triangle(int x0, int y0, fx32 u0, fx32 v0, int x1, int y1, fx32 u1, fx32 v1, int x2, int y2,
                               fx32 u2, fx32 v2, texture_t* tex) {
    Command c;
    c.opcode = OP_SET_COLOR;
    c.param = 0x555;
    g_commands.push_back(c);
    c.opcode = OP_SET_X0;
    c.param = x0;
    g_commands.push_back(c);
    c.opcode = OP_SET_Y0;
    c.param = y0;
    g_commands.push_back(c);
    c.opcode = OP_SET_X1;
    c.param = x1;
    g_commands.push_back(c);
    c.opcode = OP_SET_Y1;
    c.param = y1;
    g_commands.push_back(c);
    c.opcode = OP_SET_X2;
    c.param = x2;
    g_commands.push_back(c);
    c.opcode = OP_SET_Y2;
    c.param = y2;
    g_commands.push_back(c);
    c.opcode = OP_SET_U0;
    c.param = u0 >> 5;
    g_commands.push_back(c);
    c.opcode = OP_SET_V0;
    c.param = v0 >> 5;
    g_commands.push_back(c);
    c.opcode = OP_SET_U1;
    c.param = u1 >> 5;
    g_commands.push_back(c);
    c.opcode = OP_SET_V1;
    c.param = v1 >> 5;
    g_commands.push_back(c);
    c.opcode = OP_SET_U2;
    c.param = u2 >> 5;
    g_commands.push_back(c);
    c.opcode = OP_SET_V2;
    c.param = v2 >> 5;
    g_commands.push_back(c);
    c.opcode = OP_DRAW_TRIANGLE;
    c.param = 0;
    g_commands.push_back(c);
}

void draw_model(model_t* model) {
    vec3d vec_up = {FX(0.0f), FX(1.0f), FX(0.0f), FX(1.0f)};
    vec3d vec_camera = {FX(0.0f), FX(0.0f), FX(0.0f), FX(1.0f)};

    // Projection matrix
    mat4x4 mat_proj = matrix_make_projection(FB_WIDTH, FB_HEIGHT, 60.0f);

    float theta = 1.0f;
    float yaw = 0.0f;

    vec3d vec_target = {FX(0.0f), FX(0.0f), FX(1.0f), FX(1.0f)};
    mat4x4 mat_camera_rot = matrix_make_rotation_y(yaw);
    vec3d vec_look_dir = matrix_multiply_vector(&mat_camera_rot, &vec_target);
    vec_target = vector_add(&vec_camera, &vec_look_dir);

    mat4x4 mat_camera = matrix_point_at(&vec_camera, &vec_target, &vec_up);

    // make view matrix from camera
    mat4x4 mat_view = matrix_quick_inverse(&mat_camera);

    //
    // world
    //

    mat4x4 mat_rot_z = matrix_make_rotation_z(theta);
    mat4x4 mat_rot_x = matrix_make_rotation_x(theta);

    mat4x4 mat_trans = matrix_make_translation(FX(0.0f), FX(0.0f), FX(3.0f));
    mat4x4 mat_world;
    mat_world = matrix_make_identity();
    mat_world = matrix_multiply_matrix(&mat_rot_z, &mat_rot_x);
    mat_world = matrix_multiply_matrix(&mat_world, &mat_trans);

    // Draw model
    draw_model(FB_WIDTH, FB_HEIGHT, &vec_camera, model, &mat_world, &mat_proj, &mat_view, true, true, NULL);
}

int main(int argc, char** argv, char** env) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window =
        SDL_CreateWindow("Graphite", SDL_WINDOWPOS_UNDEFINED_DISPLAY(1), SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    const size_t vram_size = FB_WIDTH * FB_HEIGHT;
    uint16_t* vram_data = new uint16_t[vram_size];
    for (size_t i = 0; i < vram_size; ++i) vram_data[i] = 0xFF00;

    SDL_Texture* texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STREAMING, FB_WIDTH, FB_HEIGHT);

    const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};

    Vtop* top = new Vtop{contextp.get(), "TOP"};

    top->reset_i = 1;
    pulse_clk(top);

    top->reset_i = 0;

    Command c;

    model_t* teapot_model = load_teapot();
    model_t* cube_model = load_cube();

    bool quit = false;
    while (!contextp->gotFinish() && !quit) {
        SDL_Event e;

        if (top->cmd_axis_tready_o && g_commands.size() == 0) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                    continue;
                } else if (e.type == SDL_KEYUP) {
                    switch (e.key.keysym.sym) {
                        case SDLK_1:
                            c.opcode = OP_SET_COLOR;
                            c.param = 0x00F;
                            g_commands.push_back(c);
                            c.opcode = OP_CLEAR;
                            c.param = 0x000;
                            g_commands.push_back(c);
                            break;
                        case SDLK_2:
                            xd_draw_line(10, 10, 100, 50, 0xFFF);
                            break;
                        case SDLK_3:
                            draw_model(cube_model);
                            break;
                        case SDLK_4:
                            draw_model(teapot_model);
                            break;
                        case SDLK_5:
                            xd_draw_textured_triangle(50, 100, FX(0.0f), FX(0.0f), 100, 100, FX(1.0f), FX(0.0f), 80, 10,
                                                      FX(0.0f), FX(1.0f), NULL);
                            xd_draw_triangle(50, 100, 100, 100, 80, 10, 0xF00);
                            break;
                    }
                }
            }
        }

        if (top->cmd_axis_tready_o) {
            if (g_commands.size() > 0) {
                auto c = g_commands.front();
                g_commands.pop_front();
                top->cmd_axis_tdata_i = (c.opcode << 12) | c.param;
                top->cmd_axis_tvalid_i = 1;
            }
        }

        if (top->vram_sel_o && top->vram_wr_o) {
            assert(top->vram_addr_o < FB_WIDTH * FB_HEIGHT);
            vram_data[top->vram_addr_o] = top->vram_data_out_o;
        }

        if (top->cmd_axis_tready_o && g_commands.size() == 0) {
            void* p;
            int pitch;
            SDL_LockTexture(texture, NULL, &p, &pitch);
            assert(pitch == FB_WIDTH * 2);
            memcpy(p, vram_data, FB_WIDTH * FB_HEIGHT * 2);
            SDL_UnlockTexture(texture);

            int draw_w, draw_h;
            SDL_GL_GetDrawableSize(window, &draw_w, &draw_h);

            int scale_x, scale_y;
            scale_x = draw_w / 640;
            scale_y = draw_h / 480;

            SDL_Rect vga_r = {0, 0, scale_x * 640, scale_y * 480};
            SDL_RenderCopy(renderer, texture, NULL, &vga_r);

            SDL_RenderPresent(renderer);
        }

        pulse_clk(top);
        top->cmd_axis_tvalid_i = 0;
    };

    top->final();

    delete top;

    SDL_DestroyTexture(texture);
    SDL_Quit();

    return 0;
}