#include <stdint.h>
#include <graphite.h>
#include <cube.h>
#include <teapot.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>

#define BASE_IO 0xE0000000
#define BASE_VIDEO 0x1000000

#define TIMER (BASE_IO + 0)
#define LED (BASE_IO + 4)
#define GRAPHITE (BASE_IO + 32)

#define FB_WIDTH 640
#define FB_HEIGHT 480

#define TEXTURE_WIDTH 32
#define TEXTURE_HEIGHT 32

#define OP_SET_X0 0
#define OP_SET_Y0 1
#define OP_SET_Z0 2
#define OP_SET_X1 3
#define OP_SET_Y1 4
#define OP_SET_Z1 5
#define OP_SET_X2 6
#define OP_SET_Y2 7
#define OP_SET_Z2 8
#define OP_SET_R0 9
#define OP_SET_G0 10
#define OP_SET_B0 11
#define OP_SET_R1 12
#define OP_SET_G1 13
#define OP_SET_B1 14
#define OP_SET_R2 15
#define OP_SET_G2 16
#define OP_SET_B2 17
#define OP_SET_S0 18
#define OP_SET_T0 19
#define OP_SET_S1 20
#define OP_SET_T1 21
#define OP_SET_S2 22
#define OP_SET_T2 23
#define OP_CLEAR 24
#define OP_DRAW 25
#define OP_SWAP 26
#define OP_SET_TEX_ADDR 27
#define OP_WRITE_TEX 28

#define MEM_WRITE(_addr_, _value_) (*((volatile unsigned int *)(_addr_)) = _value_)
#define MEM_READ(_addr_) *((volatile unsigned int *)(_addr_))

#define PARAM(x) (x)

struct Command {
    uint32_t opcode : 8;
    uint32_t param : 24;
};

uint16_t *img = NULL;

bool init_img() {
    img = (uint16_t *)malloc(TEXTURE_WIDTH * TEXTURE_HEIGHT * sizeof(uint16_t));
    if (!img)
        return false;
    uint16_t *p = img;
    for (int v = 0; v < TEXTURE_HEIGHT; ++v) {
        for (int u = 0; u < TEXTURE_WIDTH; ++u) {
            if (u < TEXTURE_WIDTH / 2 && v < TEXTURE_HEIGHT / 2) {
                *p = 0xFFFF;
            } else if (u >= TEXTURE_WIDTH / 2 && v < TEXTURE_HEIGHT / 2) {
                *p = 0xFF00;
            } else if (u < TEXTURE_WIDTH / 2 && v >= TEXTURE_HEIGHT / 2) {
                *p = 0xF0F0;
            } else {
                *p = 0xF00F;
            }
            ++p;
        }
    }
    return true;
}

void dispose_img() {
    free(img);
}

void send_command(struct Command *cmd)
{
    while (!MEM_READ(GRAPHITE));
    MEM_WRITE(GRAPHITE, (cmd->opcode << 24) | cmd->param);
}

void xd_draw_triangle(fx32 x0, fx32 y0, fx32 z0, fx32 u0, fx32 v0, fx32 r0, fx32 g0, fx32 b0, fx32 a0, fx32 x1, fx32 y1,
                      fx32 z1, fx32 u1, fx32 v1, fx32 r1, fx32 g1, fx32 b1, fx32 a1, fx32 x2, fx32 y2, fx32 z2, fx32 u2,
                      fx32 v2, fx32 r2, fx32 g2, fx32 b2, fx32 a2, texture_t* tex, bool clamp_s, bool clamp_t,
                      bool depth_test)
{
    struct Command c;

    c.opcode = OP_SET_X0;
    c.param = PARAM(x0) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(x0) >> 16);
    send_command(&c);

    c.opcode = OP_SET_Y0;
    c.param = PARAM(y0) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(y0) >> 16);
    send_command(&c);

    c.opcode = OP_SET_Z0;
    c.param = PARAM(z0) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(z0) >> 16);
    send_command(&c);

    c.opcode = OP_SET_X1;
    c.param = PARAM(x1) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(x1) >> 16);
    send_command(&c);

    c.opcode = OP_SET_Y1;
    c.param = PARAM(y1) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(y1) >> 16);
    send_command(&c);

    c.opcode = OP_SET_Z1;
    c.param = PARAM(z1) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(z1) >> 16);
    send_command(&c);

    c.opcode = OP_SET_X2;
    c.param = PARAM(x2) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(x2) >> 16);
    send_command(&c);

    c.opcode = OP_SET_Y2;
    c.param = PARAM(y2) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(y2) >> 16);
    send_command(&c);

    c.opcode = OP_SET_Z2;
    c.param = PARAM(z2) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(z2) >> 16);
    send_command(&c);

    c.opcode = OP_SET_S0;
    c.param = PARAM(u0) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(u0) >> 16);
    send_command(&c);

    c.opcode = OP_SET_T0;
    c.param = PARAM(v0) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(v0) >> 16);
    send_command(&c);

    c.opcode = OP_SET_S1;
    c.param = PARAM(u1) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(u1) >> 16);
    send_command(&c);

    c.opcode = OP_SET_T1;
    c.param = PARAM(v1) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(v1) >> 16);
    send_command(&c);

    c.opcode = OP_SET_S2;
    c.param = PARAM(u2) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(u2) >> 16);
    send_command(&c);

    c.opcode = OP_SET_T2;
    c.param = PARAM(v2) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(v2) >> 16);
    send_command(&c);

    c.opcode = OP_SET_R0;
    c.param = PARAM(r0) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(r0) >> 16);
    send_command(&c);

    c.opcode = OP_SET_G0;
    c.param = PARAM(g0) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(g0) >> 16);
    send_command(&c);

    c.opcode = OP_SET_B0;
    c.param = PARAM(b0) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(b0) >> 16);
    send_command(&c);

    c.opcode = OP_SET_R1;
    c.param = PARAM(r1) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(r1) >> 16);
    send_command(&c);

    c.opcode = OP_SET_G1;
    c.param = PARAM(g1) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(g1) >> 16);
    send_command(&c);

    c.opcode = OP_SET_B1;
    c.param = PARAM(b1) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(b1) >> 16);
    send_command(&c);

    c.opcode = OP_SET_R2;
    c.param = PARAM(r2) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(r2) >> 16);
    send_command(&c);

    c.opcode = OP_SET_G2;
    c.param = PARAM(g2) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(g2) >> 16);
    send_command(&c);

    c.opcode = OP_SET_B2;
    c.param = PARAM(b2) & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (PARAM(b2) >> 16);
    send_command(&c);

    c.opcode = OP_DRAW;
    c.param = (depth_test ? 0b1000 : 0b0000) | (clamp_s ? 0b0100 : 0b0000) | (clamp_t ? 0b0010 : 0b0000) |
              ((tex != NULL) ? 0b0001 : 0b0000);
    send_command(&c);

}

void clear(unsigned int color)
{
    struct Command cmd;

    // Clear framebuffer
    cmd.opcode = OP_CLEAR;
    cmd.param = color;
    send_command(&cmd);
    // Clear depth buffer
    cmd.opcode = OP_CLEAR;
    cmd.param = 0x010000;
    send_command(&cmd);
}

void write_texture() {
    uint32_t tex_addr = 3 * FB_WIDTH * FB_HEIGHT;

    struct Command c;
    c.opcode = OP_SET_TEX_ADDR;
    c.param = tex_addr & 0xFFFF;
    send_command(&c);
    c.param = 0x10000 | (tex_addr >> 16);
    send_command(&c);

    c.opcode = OP_WRITE_TEX;
    const uint16_t* p = img;
    for (int t = 0; t < 32; ++t)
        for (int s = 0; s < 32; ++s) {
            c.param = *p;
             send_command(&c);
            p++;
        }
}

void swap()
{
    struct Command cmd;

    cmd.opcode = OP_SWAP;
    cmd.param = 0x1;
    send_command(&cmd);
}

void main(void)
{
    printf("[q]: quit, [s]: stats, [SPACE]: rotation, [t]: texture, [l]: lighting, [w]: wireframe, [m]: teapot/cube, [u]: clamp s, [v] clamp t\r\n");

    float theta = 0.5f;

    mat4x4 mat_proj = matrix_make_projection(FB_WIDTH, FB_HEIGHT, 60.0f);

    // camera
    vec3d  vec_camera = {FX(0.0f), FX(0.0f), FX(0.0f), FX(1.0f)};
    mat4x4 mat_view   = matrix_make_identity();

    model_t *cube_model = load_cube();
    model_t *teapot_model = load_teapot();

    model_t *model = cube_model;

    if (!init_img()) {
        printf("Out of memory\r\n");
        return;
    }

    write_texture();

    bool quit = false;
    bool print_stats = false;
    bool is_rotating = false;
    bool is_textured = false;
    bool is_lighting_ena = false;
    bool is_wireframe = false;
    bool clamp_s = false;
    bool clamp_t = false;

    clear(0xF333);

    uint32_t counter = 0;
    while(!quit) {
        MEM_WRITE(LED, counter >> 2);
        counter++;

        if (chr_avail()) {
            char c = get_chr();
            if (c == 'q') {
                quit = true;
            } else if (c == 's') {
                print_stats = !print_stats;
            } else if (c == ' ') {
                is_rotating = !is_rotating;
            } else if (c == 't') {
                is_textured = !is_textured;
            } else if (c == 'l') {
                is_lighting_ena = !is_lighting_ena;
            } else if (c == 'w') {
                is_wireframe = !is_wireframe;
            } else if (c == 'm') {
                if (model == cube_model) {
                    model = teapot_model;
                } else {
                    model = cube_model;
                }
            } else if (c == 'u') {
                clamp_s = !clamp_s;
            } else if (c == 'v') {
                clamp_t = !clamp_t;
            } 
        }        

        uint32_t t1 = MEM_READ(TIMER);

        uint32_t t1_clear = MEM_READ(TIMER);
        clear(0x00F333);
        uint32_t t2_clear = MEM_READ(TIMER);

        uint32_t t1_xform = MEM_READ(TIMER);
        // world
        mat4x4 mat_rot_z = matrix_make_rotation_z(theta);
        mat4x4 mat_rot_x = matrix_make_rotation_x(theta);

        mat4x4 mat_trans = matrix_make_translation(FX(0.0f), FX(0.0f), FX(5.0f));
        mat4x4 mat_world;
        mat_world = matrix_make_identity();
        mat_world = matrix_multiply_matrix(&mat_rot_z, &mat_rot_x);
        mat_world = matrix_multiply_matrix(&mat_world, &mat_trans);
        uint32_t t2_xform = MEM_READ(TIMER);

        uint32_t t1_draw = MEM_READ(TIMER);
        texture_t dummy_texture;
        draw_model(FB_WIDTH, FB_HEIGHT, &vec_camera, model, &mat_world, &mat_proj, &mat_view, is_lighting_ena, is_wireframe, is_textured ? &dummy_texture : NULL, clamp_s, clamp_t);
        uint32_t t2_draw = MEM_READ(TIMER);

        swap();

        if (is_rotating) {
            theta += 0.1f;
            if (theta > 6.28f)
                theta = 0.0f;
        }

        uint32_t t2 = MEM_READ(TIMER);

        if (print_stats)
            printf("xform: %d ms, clear: %d ms, draw: %d ms, total: %d ms\r\n", t2_xform - t1_xform, t2_clear - t1_clear, t2_draw - t1_draw, t2 - t1);
    }

    dispose_img();
}
