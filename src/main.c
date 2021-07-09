/*------------------------------------------------------------------
 *
 *  Copyright (C) 2021 Ashwin Godbole
 *  
 *  Crisp is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public 
 *  License as published by the Free Software Foundation, 
 *  either version 3 of the License, or (at your option) 
 *  any later version.
 *  
 *  Crisp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied 
 *  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE. See the GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with Crisp. If not, see <https://www.gnu.org/licenses/>. 
 *
 *-----------------------------------------------------------------*/

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define PIXELSIZE 20
#define WIDTH     64
#define HEIGHT    32

typedef uint8_t  u8;
typedef uint16_t u16;
typedef int16_t  i16;
typedef uint32_t u32;

struct chip {
    u8  mem[4096];
    u8  v[16];
    u8  sp;
    u16 i;
    u16 pc;
    u16 stack[16];
    u32 display[HEIGHT*WIDTH];
};

typedef struct chip Chip;
typedef void (*instrPointer)(Chip*, u16);

u16 getReg1 (u16 opcode) {
    opcode &= 0x0f00;
    opcode >>= 8;
    return opcode;
}

u16 getReg2 (u16 opcode) {
    opcode &= 0x00f0;
    opcode >>= 4;
    return opcode;
}

u16 getAddr(u16 opcode) {
    return opcode & 0x0fff;
}

u16 get8BitVal(u16 opcode) {
    return opcode & 0x00ff;
}

u16 get4BitVal(u16 opcode) {
    return opcode & 0x000f;
}

void OP_0NNN (Chip *c, u16 opcode) {

}

void OP_00E0 (Chip *c, u16 opcode) {
    /* clear screen */
    memset(c, 0, WIDTH*HEIGHT * sizeof(u32));
}

void OP_00EE (Chip *c, u16 opcode) {
}

void OP_1NNN (Chip *c, u16 opcode) {
    c->pc = getAddr(opcode);
}

void OP_2NNN (Chip *c, u16 opcode) {
}

void OP_3XNN (Chip *c, u16 opcode) {
    if (c->v[getReg1(opcode)] == get8BitVal(opcode)) c->pc+=2;
}

void OP_4XNN (Chip *c, u16 opcode) {
    if (c->v[getReg1(opcode)] != get8BitVal(opcode)) c->pc+=2;
}

void OP_5XY0 (Chip *c, u16 opcode) {
    if (c->v[getReg1(opcode)] == c->v[getReg2(opcode)]) c->pc+=2;
}

void OP_6XNN (Chip *c, u16 opcode) {
    c->v[getReg1(opcode)] = get8BitVal(opcode);
}

void OP_7XNN (Chip *c, u16 opcode) {
    c->v[getReg1(opcode)] += get8BitVal(opcode);
}

void OP_8XY0 (Chip *c, u16 opcode) {
    c->v[getReg1(opcode)] = c->v[getReg2(opcode)];
}

void OP_8XY1 (Chip *c, u16 opcode) {
    c->v[getReg1(opcode)] |= c->v[getReg2(opcode)];
}

void OP_8XY2 (Chip *c, u16 opcode) {
    c->v[getReg1(opcode)] &= c->v[getReg2(opcode)];
}

void OP_8XY3 (Chip *c, u16 opcode) {
    c->v[getReg1(opcode)] ^= c->v[getReg2(opcode)];
}

void OP_8XY4 (Chip *c, u16 opcode) {
    u16 sum = c->v[getReg1(opcode)] + c->v[getReg2(opcode)];
    c->v[getReg1(opcode)] = (u8)sum;
    c->v[15] = (sum >= 0x100);
}

void OP_8XY5 (Chip *c, u16 opcode) {
    i16 sum = c->v[getReg1(opcode)] - c->v[getReg2(opcode)];
    c->v[getReg1(opcode)] = (u8)sum;
    c->v[15] = !(sum < 0);
}

void OP_8XY6 (Chip *c, u16 opcode) {
    c->v[15] = c->v[getReg1(opcode)] & 0x1;
    c->v[getReg1(opcode)] >>= 1;
}

void OP_8XY7 (Chip *c, u16 opcode) {
    i16 sum = c->v[getReg2(opcode)] - c->v[getReg1(opcode)];
    c->v[getReg1(opcode)] = (u8)sum;
    c->v[15] = !(sum < 0);
}

void OP_8XYE (Chip *c, u16 opcode) {
    c->v[15] = c->v[getReg1(opcode)] >> 7;
    c->v[getReg1(opcode)] <<= 1;
}

void OP_9XY0 (Chip *c, u16 opcode) {
    if (c->v[getReg1(opcode)] != c->v[getReg2(opcode)]) c->pc+=2;
}

void OP_ANNN (Chip *c, u16 opcode) {
    c->i = getAddr(opcode);
}

void OP_BNNN (Chip *c, u16 opcode) {
    c->pc = (c->v[0] + getAddr(opcode)) & 0x0fff;
}

void OP_CXNN (Chip *c, u16 opcode) {
    time_t t;
    srand((unsigned) time(&t));
    c->v[getReg1(opcode)] = get8BitVal(opcode) + (rand() % 256);
}

void OP_DXYN (Chip *c, u16 opcode) {
}
void OP_EXA1 (Chip *c, u16 opcode) {
}
void OP_EX9E (Chip *c, u16 opcode) {
}
void OP_FX07 (Chip *c, u16 opcode) {
}
void OP_FX0A (Chip *c, u16 opcode) {
}
void OP_FX15 (Chip *c, u16 opcode) {
}
void OP_FX18 (Chip *c, u16 opcode) {
}

void OP_FX1E (Chip *c, u16 opcode) {
    /* don't really get why i'm truncating here */
    c->i = (c->i + c->v[getReg1(opcode)]) & 0x0fff;
}

void OP_FX29 (Chip *c, u16 opcode) {
}

void OP_FX33 (Chip *c, u16 opcode) {
    u16 temp, loc;

    temp = c->v[getReg1(opcode)];
    loc = c->i;

    if (loc == 4096) loc = 0;
    c->mem[loc] = temp / 100;
    loc++;
    
    if (loc == 4096) loc = 0;
    c->mem[loc] = (temp / 10) % 10;
    loc++;

    if (loc == 4096) loc = 0;
    c->mem[c->i + 2] = temp % 10;
}

void OP_FX55 (Chip *c, u16 opcode) {
    u16 tillReg, loc, j;
    tillReg = getReg1(opcode);
    loc = c->i;
    for (j = 0; j <= tillReg; j++) {
        if (loc == 4096) loc = 0;
        c->mem[loc++] = c->v[j];
    }
}

void OP_FX65 (Chip *c, u16 opcode) {
    u16 tillReg, loc, j;
    tillReg = getReg1(opcode);
    loc = c->i;
    for (j = 0; j <= tillReg; j++) {
        if (loc == 4096) loc = 0;
        c->v[j] = c->mem[loc++];
    }
}

instrPointer Zero[3] = {
    OP_0NNN,
    OP_00E0,
    OP_00EE,
};

instrPointer Eight[9] = {
    OP_8XY0,
    OP_8XY1,
    OP_8XY2,
    OP_8XY3,
    OP_8XY4,
    OP_8XY5,
    OP_8XY6,
    OP_8XY7,
    OP_8XYE,
};

instrPointer E[2] = {
    OP_EXA1,
    OP_EX9E,
};

instrPointer F[9] = {
    OP_FX07,
    OP_FX0A,
    OP_FX15,
    OP_FX18,
    OP_FX1E,
    OP_FX29,
    OP_FX33,
    OP_FX55,
    OP_FX65,
};

void OP_Zero(Chip *c, u16 opcode) {

}

void OP_Eight(Chip *c, u16 opcode) {

}

void OP_E(Chip *c, u16 opcode) {

}

void OP_F(Chip *c, u16 opcode) {

}

instrPointer opTable[36] = {
    OP_Zero,
    OP_1NNN,
    OP_2NNN,
    OP_3XNN,
    OP_4XNN,
    OP_5XY0,
    OP_6XNN,
    OP_7XNN,
    OP_Eight,
    OP_9XY0,
    OP_ANNN,
    OP_BNNN,
    OP_CXNN,
    OP_DXYN,
    OP_E,
    OP_F,
};

void
resetChip(Chip *c) {
    memset(c, 0,         4096 * sizeof(u8));
    memset(c, 0,           16 * sizeof(u8));
    memset(c, 0,           16 * sizeof(u16));
    memset(c, 0, WIDTH*HEIGHT * sizeof(u32));
    c->i  = 0;
    c->sp = 0;
    c->pc = 0x200;
}

int
loadProgram(Chip *c, char* filename) {
    FILE *f;
    int bRead;
    f = fopen(filename, "rb");
    if (!f) return 1;
    bRead = fread(c->mem + 512, 1, 4096-512, f);
    if (!bRead) return 1;
    return fclose(f);
}

void
error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void 
key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void
drawPixel(u8 x, u8 y, u8 r, u8 g, u8 b) {
    u8 x2, y2;
    x2 = x+1;
    y2 = y+1;
    glColor3f (r/255.0f, g/255.0f, b/255.0f);
    glBegin(GL_POLYGON);
    glVertex3f(x, y, 0);
    glVertex3f(x2, y, 0);
    glVertex3f(x2, y2, 0);
    glVertex3f(x, y2, 0);
    printf("%d, %d, %d, %d\n", x, y, x2, y2);
    glEnd();
    glFlush();
}

int
main(int argc, char** argv) {
    Chip c;
    int err;
    GLFWwindow* window;
    u8 x, y;
    x = 0; y = 0;
    
    if (argc != 2) {
        printf("usage : crisp <program>\n");
        return 1;
    }
    
    resetChip(&c);
    err = loadProgram(&c, argv[1]);
    if (err) {
        printf("ERROR: did not read or close file [ %s ]\n", argv[1]);
        return 1;
    }
    
    if (!glfwInit()) {
        printf("ERROR: glfw initialization failed");
        return 1;
    }
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    window = glfwCreateWindow(WIDTH*PIXELSIZE, HEIGHT*PIXELSIZE, "Crisp", NULL, NULL);
    if (!window) {
        printf("ERROR: window creation failed.");
        glfwTerminate();
        return 1;
    }
    
    glfwSetErrorCallback(error_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, HEIGHT, 0);
    
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
        
        drawPixel(x, y, 255, 255, 255);
        drawPixel(x+1, y, 255, 0, 0);
        drawPixel(x+2, y, 255, 255, 255);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
