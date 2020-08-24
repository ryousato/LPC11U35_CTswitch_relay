////////////////////////////////////////////////// 
// Include Files
#include "mbed.h"
#include "LPC11Uxx.h"
#include "USBSerial.h"
#include "stdio.h"
#include "math.h"
//////////////////////////////////////////////////

//Configure the PC Serial Port for CDC USB
USBSerial serial;
Serial device(P0_19,P0_18);

//Pin Defines
DigitalOut myled(LED1);

char moji[32]; //入力文字列
int count = 0; //文字数カウンタ

DigitalOut coilEN1(P0_1);
DigitalOut coilEN2(P0_2);
DigitalOut coilEN3(P0_3);
DigitalOut coilEN4(P0_4);
DigitalOut coilEN5(P0_5);
DigitalOut coilEN6(P0_20);
DigitalOut coilEN7(P1_19);

DigitalOut ENn(P0_16);
DigitalOut addr0(P0_17);
DigitalOut addr1(P0_18);
DigitalOut addr2(P0_19);

DigitalOut SCT_AMP(P0_10);
DigitalOut WLATn(P0_23);
DigitalOut SHDNn(P1_15);

AnalogIn AD_TRANS_IN(P0_11);
AnalogIn AD_CT_IN(P0_12);

SPI spi(P0_9, P0_8, P0_6);

void setup() {
    spi.format(16,0);
    wait(1);
    spi.frequency(1000000);  
    device.baud(115200);

    coilEN1 = 0;
    coilEN2 = 0;
    coilEN3 = 0;
    coilEN4 = 0;
    coilEN5 = 0;
    coilEN6 = 0;
    coilEN7 = 0;

    ENn = 1;
    addr0 = 0;
    addr1 = 0;
    addr2 = 0;

    SCT_AMP = 0;
    WLATn = 1;
    SHDNn = 0;
}

void serialstart(){
    wait(0.5);
    serial.printf("Hello World!\r\n");
    wait(0.5);
}

void helpwrite(){
    serial.printf("\r\ncmd");
    serial.printf("\r\nhelp");
    serial.printf("\r\nOK");
}

void TCON(){
        int whoami = spi.write(0x40FF); 
        serial.printf("\r\nWHOAMI register = 0x%X\r\n", whoami);
}

void TCONread(){
        int whoami = spi.write(0x4C00);
        serial.printf("\r\nWHOAMI register = 0x%X\r\n", whoami);
}

void read(){
        int whoami = spi.write(0x0C00);
        serial.printf("\r\nWHOAMI register = 0x%X\r\n", whoami);
}

void write(){
        int whoami = spi.write(0x00FF);
        serial.printf("\r\nWHOAMI register = 0x%X\r\n", whoami);
}

void WLAT(){
        WLATn = 0;
        wait(0.1);
        WLATn = 1;
}

void CSn(){
        ENn = 0;
}

void CSp(){
        ENn = 1;
}

void serial_inout(){
    if(serial.readable()) {    // 受信確認
    moji[count] = serial.getc();        //キーボード入力文字を1文字ずつmojiに代入
    serial.printf("%c(%d)", moji[count],count);   //シリアル出力表示
        
        if(count == 31 ){                // ①文字数が既定の個数になった場合
        moji[31] = '\0';                 // 末尾に終端文字を入れる
        count = 0;                       // 文字カウンタをリセット
        serial.printf("\r\nERR!\r\n");
        }

        else if(moji[0] == '\n'){count = 0;}
    
        else if((moji[count] == '\r')) {       // ②CRを受信した場合
            moji[count] = '\0';                                         // 末尾に終端文字を入れる
            count = 0;                                                  // 文字カウンタをリセット
            if(strcmp(moji, "cmd") == 0){helpwrite();}
            else if(strcmp(moji, "help") == 0){helpwrite();}
            else if(strcmp(moji, "?") == 0){helpwrite();}
            else if(strcmp(moji, "initial") == 0){helpwrite();}
            else if(strcmp(moji, "VON") == 0){helpwrite();}
            else if(strcmp(moji, "VOFF") == 0){helpwrite();}
            else if(strcmp(moji, "50Hz") == 0){helpwrite();}
            else if(strcmp(moji, "60Hz") == 0){helpwrite();}
            else if(strcmp(moji, "trans_adj") == 0){helpwrite();}
            else if(strcmp(moji, "ct1_adj") == 0){helpwrite();}
            else if(strcmp(moji, "ct2_adj") == 0){helpwrite();}
            else if(strcmp(moji, "TCON") == 0){TCON();}
            else if(strcmp(moji, "TCONread") == 0){TCONread();}
            else if(strcmp(moji, "read") == 0){read();}
            else if(strcmp(moji, "write") == 0){write();}
            else if(strcmp(moji, "WLAT") == 0){WLAT();}
            else serial.printf("\r\nNG");
        }
    
        else count++;                                                   // 文字カウンタに1加算
    }
}    

//////////////////////////////////////////////////
// main Program
int main() {
    setup();
    serialstart();
    
    myled = 1;

    while(1){
        serial_inout();
    }
}
//////////////////////////////////////////////////


