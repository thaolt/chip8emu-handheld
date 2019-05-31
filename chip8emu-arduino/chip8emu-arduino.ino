#include <SPI.h>
#include <SD.h>
#include "Adafruit_GFX.h"
#include <MCUFRIEND_kbv.h>
#include <Keypad.h>
#include "chip8emu.h"

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define SD_CS     10


const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads

char hexaKeys[ROWS][COLS] = {
  {'1','4','7','f'},
  {'2','5','8','0'},
  {'3','6','9','e'},
  {'a','b','c','d'}
};

byte kidx[0x10][2] = {
  {1,3}, {0,0}, {1,0}, {2,0},
  {0,1}, {1,1}, {2,1}, {0,2},
  {1,2}, {2,2}, {3,0}, {3,1},
  {3,2}, {3,3}, {2,3}, {0,3}
};

byte rowPins[ROWS] = {PC8, PC6, PC5, PB1}; //connect to the column pinouts of the keypad
byte colPins[COLS] = {PB15, PB14, PB13, PC4}; //connect to the row pinouts of the keypad


//initialize an instance of class NewKeypad
Keypad kp = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

MCUFRIEND_kbv tft;
File fs_root;
chip8emu *cpu;

unsigned long last_tick = 0;
unsigned long last_cycle = 0;
unsigned long now = 0;

int opcode_handler_D(chip8emu* emu) {
    /* DXYN: draw(Vx,Vy,N); draw at X,Y width 8, height N sprite from I register */
    uint8_t xo = emu->V[(emu->opcode & 0x0F00) >> 8]; /* x origin */
    uint8_t yo = emu->V[(emu->opcode & 0x00F0) >> 4];
    uint8_t height = emu->opcode & 0x000F;
    uint8_t sprite[0x10] = {0};

    memcpy(sprite, emu->memory + (emu->I * sizeof (uint8_t)), height);

    emu->V[0xF] = 0;
    for (uint8_t y = 0; y < height; y++) {
        for (uint8_t x = 0; x < 8; x++) {
            int dx = (xo + x) % 64; /* display x or dest x*/
            int dy = (yo + y) % 32;
            if ((sprite[y] & (0x80 >> x)) != 0) { /* 0x80 -> 10000000b */
                if (!emu->V[0xF] && emu->gfx[(dx + (dy * 64))]) {
                    emu->V[0xF] = 1;
                }
                emu->gfx[dx + (dy * 64)] ^= 1;
                if (emu->gfx[dy*64+dx] > 0) tft.fillRect(dx*5, 40 + dy*5, 5, 5, WHITE);
                else tft.fillRect(dx*5, 40 + dy*5, 5, 5, BLACK);
            }
        }
    }

    emu->pc += 2;
    return C8ERR_OK;
}

int opcode_handler_0(chip8emu* emu) {
    switch (emu->opcode) {
    case 0x00E0: /* clear screen */
        memset(emu->gfx, 0, 64*32);
        emu->pc += 2;
        tft.fillRect(0, 40, 320, 160, BLACK);
        break;

    case 0x00EE: /* subroutine return */
        emu->pc = emu->stack[--emu->sp & 0xF] + 2;
        break;

    default: /* 0NNN: call program at NNN address */
        break;
    }
    return C8ERR_OK;
}

void draw_callback(chip8emu* cpu) {
  
}

bool keystate_callback(chip8emu* cpu, uint8_t key){
  
  return bitRead(kp.bitMap[ kidx[key][0] ], kidx[key][1]);
}

void beep_callback(chip8emu* cpu){
  
}

void load_rom(String filename) {
  File f = SD.open(filename);
  f.read(cpu->memory+0x200, f.size());
  Serial.print(filename);
  Serial.print(" ");
  Serial.println(f.size());
  f.close();
}

void print_fs_root(File dir, int numTabs) {
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      print_fs_root(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

int random_number() {
  return random(0xFFFFFFFF);
}

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));

  kp.setHoldTime(100);
  
  Serial.print("Initializing SD card...");

  if (!SD.begin(SD_CS)) {
    Serial.println("failed!");
  } else {
    Serial.println("done!");
    fs_root = SD.open("/");
    print_fs_root(fs_root, 0);
  }
  
  uint16_t ID = tft.readID(); //
  Serial.print("LCD ID = 0x");
  Serial.println(ID, HEX);
  if (ID == 0xD3D3) ID = 0x9481; // write-only shield
  tft.begin(ID);
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  cpu = chip8emu_new();
  cpu->draw = &draw_callback;
  cpu->keystate = &keystate_callback;
  cpu->beep = &beep_callback;
  cpu->opcode_handlers[0xD] = &opcode_handler_D;
  cpu->opcode_handlers[0x0] = &opcode_handler_0;
  
  load_rom("BRIX");
}

void loop() {
  now = micros();
  if (now - last_cycle> 2500) {
    chip8emu_exec_cycle(cpu);
    last_cycle = now;
  }
  
  if (now - last_tick > 16666) {
    chip8emu_timer_tick(cpu);
    last_tick = now;
  }

  kp.getKeys();
    
  delayMicroseconds(1);
}
