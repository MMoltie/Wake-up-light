//display colors
int ColorOne = GC9A01A_CYAN;
int ColorTwo = GC9A01A_DARKCYAN;

//default alarm time
uint8_t wakeHour = 17;    
uint8_t wakeMinute = 45;

bool dimming = false;   //allow dimming of screen

String songs[6] = {"Bongo's", "Jungle", "Water", "Meadow", "Arabian", "Applaus"};  //names of alarm songs
byte songIndex = 1;                                                     //index of alarm song to play, default is first song
int songVolume = 15;                                                    //default song volume, MAX = 30

// 'Sunrise icon', 40x40px
const unsigned char sunIcon [] PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 
0x20, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x01, 0x00, 0x40, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0xc1, 0x80, 0x00, 0x08, 0x02, 0x00, 0x20, 0x10, 0x02, 0x04, 0x00, 0x10, 0x20, 0x01, 
0x08, 0x00, 0x08, 0x80, 0x00, 0x10, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x20, 
0x00, 0x02, 0x00, 0x00, 0x20, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x01, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x3f, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


//Display layout cursor coordinates
//reading time text
int oneCursor_x = 102;
int oneCursor_y = 42;
//time "hh" text
int twoCursor_x = 33;
int twoCursor_y = 97;
//time ":" text
int threeCursor_x = 103;
int threeCursor_y = 97;
//time "mm" text
int fourCursor_x = 138;
int fourCursor_y = 97;
//song name
int fiveCursor_x = 33;
int fiveCursor_y = 102;
//alarm icon
int sixCursor_x = 51;
int sixCursor_y = 154;
//alarm "hh" text
int sevenCursor_x = 103; 
int sevenCursor_y = 165;
//alarm ":" text
int eightCursor_x = 139;
int eightCursor_y = 165;
//alarm "mm" text
int nineCursor_x = 157;
int nineCursor_y = 165;