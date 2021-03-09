//////////////////////////////////////////////////
// Include Files
#include "mbed.h"
#include "LPC11Uxx.h"
#include "USBSerial.h"
//////////////////////////////////////////////////

//Configure the PC Serial Port for CDC USB
USBSerial serial;
Serial device(P0_19,P0_18);

char moji[32];  //入力文字列
int count = 0;  //文字数カウンタ

//DigitalOut led(LED1);
//DigitalInOut GPIOX(P0_0, PIN_OUTPUT, PullDown, 0);
DigitalOut GPIO[] = {
    DigitalOut (P0_22, 0),
    DigitalOut (P0_11, 0),
    DigitalOut (P0_12, 0),
    DigitalOut (P0_13, 0),
    DigitalOut (P0_14, 0),
    DigitalOut (P0_15, 0),
    DigitalOut (P0_16, 0),
    DigitalOut (P0_23, 0),
    DigitalOut (P1_15, 0),
    DigitalOut (P0_21, 0),
    DigitalOut (P0_5, 0),
    DigitalOut (P0_4, 0),
    DigitalOut (P0_3, 0),
    DigitalOut (P0_2, 0),
    DigitalOut (P0_20, 0),
    DigitalOut (P1_19, 0),
    DigitalOut (P0_1),
    DigitalOut (P0_19, 0),
    DigitalOut (P0_18, 0),
    DigitalOut (P0_17, 0),
};
BusOut muxaddr(P0_7,P0_8,P0_9,P0_10);     //マルチプレクサADDRバス

void helpwrite()
{
    serial.printf("\r\n------------------");
    serial.printf("\r\nrelayALLoff");
    serial.printf("\r\nrelay{1|...|8}");
    serial.printf("\r\nGPIOALL{L|H}");
    serial.printf("\r\nGPIO{1|...|20}{L|H}");
    serial.printf("\r\n------------------");
}

void Vswitch(int tmpInt)
{
    //ADDRバス切替
    if (tmpInt<7) {
        muxaddr.write(tmpInt);
    } else if (tmpInt>=7) {
        tmpInt = tmpInt + 1;
        muxaddr.write(tmpInt);
    }
    wait(0.01);
    serial.printf("\r\n OK");
}

void GPIOswitch(int tmpIO, int tmpInt)
{
    if (tmpIO==0) {
        for(int i=0; i<=19; i++) {
            GPIO[i] = tmpInt;
        }
    } else {
        tmpIO = tmpIO - 1;
        GPIO[tmpIO] = tmpInt;
    }

    serial.printf("\r\n OK");
}
/*
void GPIOXswitch(int tmpInt)
{
    GPIOX = tmpInt;
    wait(0.01);
    serial.printf("\r\n OK");
}
*/

/******************************************************************************/
//シリアル入力受付 [メイン]
/******************************************************************************/
void serial_inout()
{
    if(serial.readable()) {    // 受信確認
        moji[count] = serial.getc();        //キーボード入力文字を1文字ずつmojiに代入
        serial.printf("%c", moji[count]);   //シリアル出力表示
//    serial.printf("%c(%d)", moji[count],count);   //シリアル出力表示

        if(count == 31 ) {               // ①文字数が既定の個数になった場合
            moji[31] = '\0';                 // 末尾に終端文字を入れる
            count = 0;                       // 文字カウンタをリセット
            serial.printf("\r\n\r\n ERROR!");
        }

        else if(moji[0] == '\n') {
            count = 0;
        }

        else if((moji[count] == '\r')) {       // ②CRを受信した場合
            moji[count] = '\0';                                                     //末尾に終端文字を入れる
            count = 0;                                                              //文字カウンタをリセット
            if     (strcmp(moji, "cmd") == 0) {
                helpwrite();   //コマンド一覧表示
            } else if(strcmp(moji, "help") == 0) {
                helpwrite();   //コマンド一覧表示
            } else if(strcmp(moji, "?") == 0) {
                helpwrite();   //コマンド一覧表示
            } else if(strcmp(moji, "relayALLoff") == 0) {
                Vswitch(0);
            } else if(strcmp(moji, "relay1") == 0) {
                Vswitch(1);
            } else if(strcmp(moji, "relay2") == 0) {
                Vswitch(2);
            } else if(strcmp(moji, "relay3") == 0) {
                Vswitch(3);
            } else if(strcmp(moji, "relay4") == 0) {
                Vswitch(4);
            } else if(strcmp(moji, "relay5") == 0) {
                Vswitch(5);
            } else if(strcmp(moji, "relay6") == 0) {
                Vswitch(6);
            } else if(strcmp(moji, "relay7") == 0) {
                Vswitch(7);
            } else if(strcmp(moji, "relay8") == 0) {
                Vswitch(8);
            } else if(strcmp(moji, "relay9") == 0) {
                Vswitch(9);
            } else if(strcmp(moji, "relay10") == 0) {
                Vswitch(10);
            } else if(strcmp(moji, "GPIO1L") == 0) {
                GPIOswitch(1,0);
            } else if(strcmp(moji, "GPIO1H") == 0) {
                GPIOswitch(1,1);
            } else if(strcmp(moji, "GPIO2L") == 0) {
                GPIOswitch(2,0);
            } else if(strcmp(moji, "GPIO2H") == 0) {
                GPIOswitch(2,1);
            } else if(strcmp(moji, "GPIO3L") == 0) {
                GPIOswitch(3,0);
            } else if(strcmp(moji, "GPIO3H") == 0) {
                GPIOswitch(3,1);
            } else if(strcmp(moji, "GPIO4L") == 0) {
                GPIOswitch(4,0);
            } else if(strcmp(moji, "GPIO4H") == 0) {
                GPIOswitch(4,1);
            } else if(strcmp(moji, "GPIO5L") == 0) {
                GPIOswitch(5,0);
            } else if(strcmp(moji, "GPIO5H") == 0) {
                GPIOswitch(5,1);
            } else if(strcmp(moji, "GPIO6L") == 0) {
                GPIOswitch(6,0);
            } else if(strcmp(moji, "GPIO6H") == 0) {
                GPIOswitch(6,1);
            } else if(strcmp(moji, "GPIO7L") == 0) {
                GPIOswitch(7,0);
            } else if(strcmp(moji, "GPIO7H") == 0) {
                GPIOswitch(7,1);
            } else if(strcmp(moji, "GPIO8L") == 0) {
                GPIOswitch(8,0);
            } else if(strcmp(moji, "GPIO8H") == 0) {
                GPIOswitch(8,1);
            } else if(strcmp(moji, "GPIO9L") == 0) {
                GPIOswitch(9,0);
            } else if(strcmp(moji, "GPIO9H") == 0) {
                GPIOswitch(9,1);
            } else if(strcmp(moji, "GPIO10L") == 0) {
                GPIOswitch(10,0);
            } else if(strcmp(moji, "GPIO10H") == 0) {
                GPIOswitch(10,1);
            } else if(strcmp(moji, "GPIO11L") == 0) {
                GPIOswitch(11,0);
            } else if(strcmp(moji, "GPIO11H") == 0) {
                GPIOswitch(11,1);
            } else if(strcmp(moji, "GPIO12L") == 0) {
                GPIOswitch(12,0);
            } else if(strcmp(moji, "GPIO12H") == 0) {
                GPIOswitch(12,1);
            } else if(strcmp(moji, "GPIO13L") == 0) {
                GPIOswitch(13,0);
            } else if(strcmp(moji, "GPIO13H") == 0) {
                GPIOswitch(13,1);
            } else if(strcmp(moji, "GPIO14L") == 0) {
                GPIOswitch(14,0);
            } else if(strcmp(moji, "GPIO14H") == 0) {
                GPIOswitch(14,1);
            } else if(strcmp(moji, "GPIO15L") == 0) {
                GPIOswitch(15,0);
            } else if(strcmp(moji, "GPIO15H") == 0) {
                GPIOswitch(15,1);
            } else if(strcmp(moji, "GPIO16L") == 0) {
                GPIOswitch(16,0);
            } else if(strcmp(moji, "GPIO16H") == 0) {
                GPIOswitch(16,1);
            } else if(strcmp(moji, "GPIO17L") == 0) {
                GPIOswitch(17,0);
            } else if(strcmp(moji, "GPIO17H") == 0) {
                GPIOswitch(17,1);
            } else if(strcmp(moji, "GPIO18L") == 0) {
                GPIOswitch(18,0);
            } else if(strcmp(moji, "GPIO18H") == 0) {
                GPIOswitch(18,1);
            } else if(strcmp(moji, "GPIO19L") == 0) {
                GPIOswitch(19,0);
            } else if(strcmp(moji, "GPIO19H") == 0) {
                GPIOswitch(19,1);
            } else if(strcmp(moji, "GPIO20L") == 0) {
                GPIOswitch(20,0);
            } else if(strcmp(moji, "GPIO20H") == 0) {
                GPIOswitch(20,1);
            } else if(strcmp(moji, "GPIOALLL") == 0) {
                GPIOswitch(0,0);
            } else if(strcmp(moji, "GPIOALLH") == 0) {
                GPIOswitch(0,1);
//            } else if(strcmp(moji, "GPIOXH") == 0) {
//                GPIOXswitch(1);
//            } else if(strcmp(moji, "GPIOXL") == 0) {
//                GPIOXswitch(0);
            } else serial.printf("\r\n\r\n NG!\r\n");
        } else count++;      // 文字カウンタに1加算
    }
}

/******************************************************************************/
//メインプログラム
/******************************************************************************/
int main()
{
    wait(0.5);
    serial.printf("\r\nReady!\r\n");

    while(1) {
        serial_inout();
    }


}