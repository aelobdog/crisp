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
#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PIXELSIZE 20
#define WIDTH 64
#define HEIGHT 32

typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;

struct chip {
  u8 mem[4096];
  u8 v[16];
  i8 sp;
  u16 i;
  u16 pc;
  u16 dtimer;
  u16 stimer;
  u16 stack[16];
  u8 display[HEIGHT * WIDTH];
};

u8 key_pressed[16];
i8 prev_key;

typedef struct chip Chip;
typedef void (*instrPointer)(Chip *, u16);

u16 getReg1(u16 opcode) {
  opcode &= 0x0f00;
  opcode >>= 8;
  return opcode;
}

u16 getReg2(u16 opcode) {
  opcode &= 0x00f0;
  opcode >>= 4;
  return opcode;
}

u16 getAddr(u16 opcode) { return opcode & 0x0fff; }

u16 get8BitVal(u16 opcode) { return opcode & 0x00ff; }

u16 get4BitVal(u16 opcode) { return opcode & 0x000f; }

void drawPixel(u8 x, u8 y) {
  u8 x2, y2;
  x2 = x + 1;
  y2 = y + 1;
  glBegin(GL_POLYGON);
  glVertex3f(x, y, 0);
  glVertex3f(x2, y, 0);
  glVertex3f(x2, y2, 0);
  glVertex3f(x, y2, 0);
  glEnd();
  glFlush();
}

void refreshScreen(Chip *c) {
  int j;
  for (j = 0; j < WIDTH * HEIGHT; j++) {
    if (c->display[j]) {
      drawPixel(j % WIDTH, j / WIDTH);
    }
  }
}

void OP_0NNN(Chip *c, u16 opcode) {}

void OP_00E0(Chip *c, u16 opcode) {
  int i;
  for (i = 0; i < WIDTH * HEIGHT; i++) {
    c->display[i] = (u8)0;
  }
}

void OP_00EE(Chip *c, u16 opcode) {
  if (c->sp < -1 || c->sp > 15) {
    printf("ERROR: stack bounds exceeded. Stopping machine.");
  }
  c->pc = c->stack[c->sp--];
  exit(1);
}

void OP_1NNN(Chip *c, u16 opcode) { c->pc = getAddr(opcode); }

void OP_2NNN(Chip *c, u16 opcode) {
  if (c->sp < -1 || c->sp > 15) {
    printf("ERROR: stack bounds exceeded. Stopping machine.");
    exit(1);
  }
  c->stack[++c->sp] = c->pc;
  c->pc = getAddr(opcode);
}

void OP_3XNN(Chip *c, u16 opcode) {
  if (c->v[getReg1(opcode)] == get8BitVal(opcode))
    c->pc += 2;
}

void OP_4XNN(Chip *c, u16 opcode) {
  if (c->v[getReg1(opcode)] != get8BitVal(opcode))
    c->pc += 2;
}

void OP_5XY0(Chip *c, u16 opcode) {
  if (c->v[getReg1(opcode)] == c->v[getReg2(opcode)])
    c->pc += 2;
}

void OP_6XNN(Chip *c, u16 opcode) {
  c->v[getReg1(opcode)] = (u8)get8BitVal(opcode);
}

void OP_7XNN(Chip *c, u16 opcode) {
  c->v[getReg1(opcode)] += (u8)get8BitVal(opcode);
}

void OP_8XY0(Chip *c, u16 opcode) {
  c->v[getReg1(opcode)] = c->v[getReg2(opcode)];
}

void OP_8XY1(Chip *c, u16 opcode) {
  c->v[getReg1(opcode)] |= c->v[getReg2(opcode)];
}

void OP_8XY2(Chip *c, u16 opcode) {
  c->v[getReg1(opcode)] &= c->v[getReg2(opcode)];
}

void OP_8XY3(Chip *c, u16 opcode) {
  c->v[getReg1(opcode)] ^= c->v[getReg2(opcode)];
}

void OP_8XY4(Chip *c, u16 opcode) {
  u16 sum;
  sum = c->v[getReg1(opcode)] + c->v[getReg2(opcode)];
  c->v[getReg1(opcode)] = (u8)sum;
  c->v[15] = (sum >= 0x100);
}

void OP_8XY5(Chip *c, u16 opcode) {
  i16 sum;
  sum = c->v[getReg1(opcode)] - c->v[getReg2(opcode)];
  c->v[getReg1(opcode)] = (u8)sum;
  c->v[15] = !(sum < 0);
}

void OP_8XY6(Chip *c, u16 opcode) {
  c->v[15] = c->v[getReg2(opcode)] & 0x1;
  c->v[getReg1(opcode)] = c->v[getReg2(opcode)] >> 1;
}

void OP_8XY7(Chip *c, u16 opcode) {
  i16 sum;
  sum = c->v[getReg2(opcode)] - c->v[getReg1(opcode)];
  c->v[getReg1(opcode)] = (u8)sum;
  c->v[15] = !(sum < 0);
}

void OP_8XYE(Chip *c, u16 opcode) {
  c->v[15] = c->v[getReg2(opcode)] >> 7;
  c->v[getReg1(opcode)] = c->v[getReg2(opcode)] << 1;
}

void OP_9XY0(Chip *c, u16 opcode) {
  if (c->v[getReg1(opcode)] != c->v[getReg2(opcode)])
    c->pc += 2;
}

void OP_ANNN(Chip *c, u16 opcode) { c->i = getAddr(opcode); }

void OP_BNNN(Chip *c, u16 opcode) {
  c->pc = (c->v[0] + getAddr(opcode)) & 0x0fff;
}

void OP_CXNN(Chip *c, u16 opcode) {
  time_t t;
  srand((unsigned)time(&t));
  c->v[getReg1(opcode)] = get8BitVal(opcode) + (rand() % 256);
}

void OP_DXYN(Chip *c, u16 opcode) {
  u8 x, y, n, rowData, k, l;
  x = c->v[getReg1(opcode)] % WIDTH;
  y = c->v[getReg2(opcode)] % HEIGHT;
  n = (u8)get4BitVal(opcode);

  for (k = 0; k < n && y < HEIGHT; k++, y++) {
    rowData = c->mem[c->i + k];

    for (l = 0; l < 8 && (x + k) < WIDTH; l++) {
      c->display[x + l + y * WIDTH] ^= (u8)((rowData >> (7 - l)) & 0x1);
      c->v[15] = ((rowData >> (7 - l)) & 0x1) & (c->display[x + k + y * 64]);
    }
  }
}

void OP_EX9E(Chip *c, u16 opcode) {
  u8 x;
  x = c->v[getReg1(opcode)];
  if (x < 16 && key_pressed[x])
    c->pc += 2;
}

void OP_EXA1(Chip *c, u16 opcode) {
  u8 x;
  x = c->v[getReg1(opcode)];
  if (x < 16 && !key_pressed[x])
    c->pc += 2;
}

void OP_FX07(Chip *c, u16 opcode) { c->v[getReg1(opcode)] = c->dtimer; }

void OP_FX0A(Chip *c, u16 opcode) {
  u8 k;
  while (prev_key == -1)
    continue;
  c->v[getReg1(opcode)] = prev_key;
}

void OP_FX15(Chip *c, u16 opcode) { c->dtimer = c->v[getReg1(opcode)]; }

void OP_FX18(Chip *c, u16 opcode) { c->stimer = c->v[getReg1(opcode)]; }

void OP_FX1E(Chip *c, u16 opcode) {
  c->i = (c->i + c->v[getReg1(opcode)]) & 0x0fff;
}

void OP_FX29(Chip *c, u16 opcode) {}

void OP_FX33(Chip *c, u16 opcode) {
  u16 temp, loc;
  temp = c->v[getReg1(opcode)];
  loc = c->i % 4096;
  c->mem[loc] = temp / 100;
  loc = (loc + 1) % 4096;
  c->mem[loc] = (temp / 10) % 10;
  loc = (loc + 1) % 4096;
  c->mem[c->i + 2] = temp % 10;
}

void OP_FX55(Chip *c, u16 opcode) {
  u16 tillReg, loc, j;
  tillReg = getReg1(opcode);
  loc = c->i;
  for (j = 0; j <= tillReg; j++) {
    if (loc == 4096)
      loc = 0;
    c->mem[loc++] = c->v[j];
  }
  c->i += tillReg + 1;
}

void OP_FX65(Chip *c, u16 opcode) {
  u16 tillReg, loc, j;
  tillReg = getReg1(opcode);
  loc = c->i;
  for (j = 0; j <= tillReg; j++) {
    if (loc == 4096)
      loc = 0;
    c->v[j] = c->mem[loc++];
  }
  c->i += tillReg + 1;
}

instrPointer Eight[8] = {
    &OP_8XY0, &OP_8XY1, &OP_8XY2, &OP_8XY3,
    &OP_8XY4, &OP_8XY5, &OP_8XY6, &OP_8XY7,
};

void OP_Zero(Chip *c, u16 opcode) {
  switch (opcode & 0x00ff) {
  case 0xe0:
    OP_00E0(c, opcode);
    break;
  case 0xee:
    OP_00EE(c, opcode);
    break;
  default:
    OP_0NNN(c, opcode);
  }
}

void OP_Eight(Chip *c, u16 opcode) {
  if ((opcode & 0xf) == 0xe)
    OP_8XYE(c, opcode);
  else
    (*Eight[opcode & 0xf])(c, opcode);
}

void OP_E(Chip *c, u16 opcode) {
  switch (opcode & 0x00ff) {
  case 0xa1:
    OP_EXA1(c, opcode);
    break;
  case 0x9e:
    OP_EX9E(c, opcode);
    break;
  }
}

void OP_F(Chip *c, u16 opcode) {
  switch (opcode & 0x00ff) {
  case 0x07:
    OP_FX07(c, opcode);
    break;
  case 0x0a:
    OP_FX0A(c, opcode);
    break;
  case 0x15:
    OP_FX15(c, opcode);
    break;
  case 0x18:
    OP_FX18(c, opcode);
    break;
  case 0x1e:
    OP_FX1E(c, opcode);
    break;
  case 0x29:
    OP_FX29(c, opcode);
    break;
  case 0x33:
    OP_FX33(c, opcode);
    break;
  case 0x55:
    OP_FX55(c, opcode);
    break;
  case 0x65:
    OP_FX65(c, opcode);
    break;
  }
}

instrPointer opTable[16] = {
    OP_Zero,  OP_1NNN, OP_2NNN, OP_3XNN, OP_4XNN, OP_5XY0, OP_6XNN, OP_7XNN,
    OP_Eight, OP_9XY0, OP_ANNN, OP_BNNN, OP_CXNN, OP_DXYN, OP_E,    OP_F,
};

void resetChip(Chip *c) {
  memset(c->mem, 0, 4096 * sizeof(u8));
  memset(c->v, 0, 16 * sizeof(u8));
  memset(c->stack, 0, 16 * sizeof(u16));
  memset(c->display, 0, WIDTH * HEIGHT * sizeof(u8));
  c->i = 0;
  c->sp = -1;
  c->pc = 0x200;
  c->dtimer = 0;
  c->stimer = 0;
  prev_key = -1;
  key_pressed[0] = 0;
  key_pressed[1] = 0;
  key_pressed[2] = 0;
  key_pressed[3] = 0;
  key_pressed[4] = 0;
  key_pressed[5] = 0;
  key_pressed[6] = 0;
  key_pressed[7] = 0;
  key_pressed[8] = 0;
  key_pressed[9] = 0;
  key_pressed[10] = 0;
  key_pressed[11] = 0;
  key_pressed[12] = 0;
  key_pressed[13] = 0;
  key_pressed[14] = 0;
  key_pressed[15] = 0;
}

void loadFonts(Chip *c) {}

int loadProgram(Chip *c, char *filename) {
  FILE *f;
  int bRead;
  f = fopen(filename, "rb");
  if (!f)
    return 1;
  bRead = fread(c->mem + 512, 1, 4096 - 512, f);
  if (!bRead)
    return 1;
  return fclose(f);
}
void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}

u8 keyToIndex(u8 keycode) {
  switch (keycode) {
  case GLFW_KEY_1:
    return 1;
  case GLFW_KEY_2:
    return 2;
  case GLFW_KEY_3:
    return 3;
  case GLFW_KEY_4:
    return 0xc;
  case GLFW_KEY_Q:
    return 4;
  case GLFW_KEY_W:
    return 5;
  case GLFW_KEY_E:
    return 6;
  case GLFW_KEY_R:
    return 0xd;
  case GLFW_KEY_A:
    return 7;
  case GLFW_KEY_S:
    return 8;
  case GLFW_KEY_D:
    return 9;
  case GLFW_KEY_F:
    return 0xe;
  case GLFW_KEY_Z:
    return 0xa;
  case GLFW_KEY_X:
    return 0;
  case GLFW_KEY_C:
    return 0xb;
  case GLFW_KEY_V:
    return 0xf;
  }
  return 100;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  u8 k;
  if (action == GLFW_PRESS) {
    k = keyToIndex(key);
    if (k == 100)
      return;
    prev_key = k;
    key_pressed[k] = 1;
  } else if (action == GLFW_RELEASE) {
    int l, active;
    active = 0;
    for (l = 0; l < 16; l++)
      if (key_pressed[l]) {
        active = 1;
        break;
      }
    if (!active)
      prev_key = -1;
  }
}

int main(int argc, char **argv) {
  Chip c;
  int err;
  double last, current;
  GLFWwindow *window;

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
  window = glfwCreateWindow(WIDTH * PIXELSIZE, HEIGHT * PIXELSIZE, "Crisp",
                            NULL, NULL);
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
  glfwSetTime(0.0);
  last = glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    u16 instruction;
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    current = glfwGetTime();
    if (current - last > 16.666) {
      /*    _________ ______
           |         |******|
            --------- ------
          last     16.66   now

      'last' is set to using the formula to ensure time
      marked with '*' is not 'wasted'                */
      last = current - ((current - last) - 16.666);
      c.dtimer = c.dtimer == 0 ? 0 : c.dtimer - 1;
      c.stimer = c.stimer == 0 ? 0 : c.stimer - 1;
    }

    if (c.pc > 4095) {
      printf("ran out of memory");
      return 1;
    }

    instruction = (c.mem[c.pc] << 8) + c.mem[c.pc + 1];
    c.pc += 2;

    (*opTable[(instruction & 0xf000) >> 12])(&c, instruction);

    refreshScreen(&c);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
