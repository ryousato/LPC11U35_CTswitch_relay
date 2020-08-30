////////////////////////////////////////////////// 
// Include Files
#include "mbed.h"
#include "LPC11Uxx.h"
#include "USBSerial.h"
//////////////////////////////////////////////////

//Configure the PC Serial Port for CDC USB
USBSerial serial;
Serial device(P0_19,P0_18);
Ticker OffsetCal; 
Ticker timerint;
Timer timecount;

//Pin Defines
DigitalOut led1(LED1);

char moji[32];  //入力文字列
int suuji[6];
int count = 0;  //文字数カウンタ
int CorF;
int adjmode;
int val;        //POTのワイパー移動数
int incr;
int wa;
    int mode_p = 0;
    int *pmode;
float Vcc = 3.292; //実測値を入力すること
float Vref;
    float ActualVref[3] = {1.65, 1.65, 1.65};
    float *pAVrefcal;
int AD_flag;
float AD_val = 0;
float AD_val_sum = 0;
float Vrms = 0;
float set_val = 0;
float diff = 0;
//float zero_cal[2];
int AD_count = 0;
float setval = 0;


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

AnalogIn AD_TRANS_IN(P0_11);    //AD_TRANS_IN
AnalogIn AD_CT_IN(P0_12);       //AD_CT_IN


SPI spi(P0_9, P0_8, P0_6);

void setup() {
    spi.format(16,0);
    wait(0.02);
    spi.frequency(1000000);  
    device.baud(115200);

    coilEN1 = 0;    //50Hz
    coilEN2 = 0;    //TRANS_OUT調整
    coilEN3 = 0;    //CT1,2_OUT調整
    coilEN4 = 0;    //CT1,2_OUT選択
    coilEN5 = 0;    //±5V,±15 OFF
    coilEN6 = 0;    //未使用
    coilEN7 = 0;    //未使用

    ENn = 1;        //CD74HC4051E CSをディセーブル
    addr0 = 0;
    addr1 = 0;
    addr2 = 0;

    WLATn = 1;      //MCP41HV51-503EST ワイパーラッチ
    SCT_AMP = 0;    //CT入力アンプ 60Aレベル切替え
    SHDNn = 0;      //電流帰還アンプ シャットダウンしない
}

void serialstart(){
    wait(0.02);
    serial.printf("\r\nReady!\r\n");
    wait(0.02);
}

void helpwrite(){
    serial.printf("\r\n------------------");
    //serial.printf("\r\ncmd");             
    serial.printf("\r\nhelp");            
    //serial.printf("\r\n?");               
    serial.printf("\r\ninitial");         
    serial.printf("\r\nV_OFF");           
    serial.printf("\r\nV_ON");            
    serial.printf("\r\n50Hz");            
    serial.printf("\r\n60Hz");            
    serial.printf("\r\nTRANS_OUT_ADINT");  
    serial.printf("\r\nCT1_OUT_ADINT");
    serial.printf("\r\nCT2_OUT_ADINT");
    serial.printf("\r\nTRANS_OUT_EXT");    
    serial.printf("\r\nCT_OUT_EXT");      
    serial.printf("\r\nTRANS_ADJ");       
    serial.printf("\r\nCT1_ADJ");         
    serial.printf("\r\nCT2_ADJ");         
    serial.printf("\r\nMANUAL_TRANS_ADJ");
    serial.printf("\r\nMANUAL_CT1_ADJ");  
    serial.printf("\r\nMANUAL_CT2_ADJ");  
    serial.printf("\r\n------------------");
}

//MCP41HV51コマンド
//デバッグ用
void TCON(){                                       //端子スイッチ接続
        int whoami = spi.write(0x40FF); 
        serial.printf("\r\nWHOAMI register = 0x%X", whoami);
}

//デバッグ用
void TCONread(){
        int whoami = spi.write(0x4C00);
        serial.printf("\r\nWHOAMI register = 0x%X", whoami);
}

//デバッグ用
void read(){
        int read_val = spi.write(0x0C00);
        serial.printf("\r\nread_val = 0x%X", read_val);
}

//デバッグ用
void write00(){
        int whoami = spi.write(0x0000);
        serial.printf("\r\nWHOAMI register = 0x%X", whoami);
}

//デバッグ用
void write7F(){
        int whoami = spi.write(0x007F);
        serial.printf("\r\nWHOAMI register = 0x%X", whoami);
}

//デバッグ用
void writeFF(){
        int whoami = spi.write(0x00FF);
        serial.printf("\r\nWHOAMI register = 0x%X", whoami);
}

//デバッグ用
void TENinc(){
        int readval = spi.write(0x0C00);
        int incval = readval & 0xFF;
        incval = incval + 10;
        int whoami = spi.write(incval);
        serial.printf("\r\nWHOAMI register = 0x%X", whoami);
}

void WLAT(){
        WLATn = 0;
        wait(0.02);
        WLATn = 1;
        //serial.printf("\r\n OK(WLAT)");
}

//CD74HC4051Eのイネーブル
void CS(int cs){
        ENn = cs;
        serial.printf("\r\n OK(CS)");
}

/******************************************************************************/
//CD74HC4051Eのアドレスセレクト
/******************************************************************************/
void ADDR(int addr){
        ENn = 1;        //CSをディセーブルにしておく
        wait(0.02);
        if(addr == 0){
            addr0 = 0;
            addr1 = 0;
            addr2 = 0;
            }
        else if(addr == 1){
            addr0 = 1;
            addr1 = 0;
            addr2 = 0;
            }
        else if(addr == 2){
            addr0 = 0;
            addr1 = 1;
            addr2 = 0;
            }
        else if(addr == 3){
            addr0 = 1;
            addr1 = 1;
            addr2 = 0;
            }
        else if(addr == 4){
            addr0 = 0;
            addr1 = 0;
            addr2 = 1;
            }
        else if(addr == 5){
            addr0 = 1;
            addr1 = 0;
            addr2 = 1;
            }
        else if(addr == 6){
            addr0 = 0;
            addr1 = 1;
            addr2 = 1;
            }
        else if(addr == 7){
            addr0 = 1;
            addr1 = 1;
            addr2 = 1;
            }
        wait(0.02);
        ENn = 0;        //イネーブル
        wait(0.02);
        //serial.printf("\r\n OK(ADDR%d)", addr);
}

/******************************************************************************/
//POT制御
//指定したPOTの現在値から、指定した量を増減する
/******************************************************************************/
void incdec(int incr, int val, int CorF, int mode){       //incr [0:1][-:+], val [0~255], CorF [2:1][C:F], mode [1:2:3][trans:ct1:ct2]
    //現在の設定値読み取り
    ADDR(mode * 2 + CorF -2);
    int read_val = spi.write(0x0C00);
    read_val &= 0x00FF;
    
    //現在の設定値から増減値書き込み
    int write_val;
    if(incr == 1){write_val = read_val + val;}
    else if(incr == 0){write_val = read_val - val;}
    spi.write(write_val);
    
    wait(0.001);
    
    //粗、微　読み取り
    ADDR(mode * 2);             //TRANS粗調整POT
    int read_val_Coarse = spi.write(0x0C00);
    read_val_Coarse &= 0x00FF;
    
    wait(0.001);
    
    ADDR(mode * 2 - 1);         //TRANS微調整POT
    int read_val_Fine = spi.write(0x0C00);
    read_val_Fine &= 0x00FF;
    
    WLAT();
    
    wait(0.001);
    
    ENn = 1;    //ディセーブル

    serial.printf("\r\n Status  --- Coarse[%d/255], Fine[%d/255] ---", read_val_Coarse, read_val_Fine);
}

/******************************************************************************/
//AD変換
//デバッグ用
/******************************************************************************/

void AD(int ad){
        unsigned short AD_Val;
        float AD_Val_f;
        if(ad == 1){
            AD_Val = (AD_TRANS_IN.read_u16() >> 6); //右シフト「>> 6」で10bit分解能に合わせる
            AD_Val_f = AD_TRANS_IN.read();
            }
        else if(ad == 2){
            AD_Val = (AD_CT_IN.read_u16() >> 6); //右シフト「>> 6」で10bit分解能に合わせる
            AD_Val_f = AD_CT_IN.read();
            }
        wait(0.02);
        serial.printf("\r\nAD_Val : %1f [V] / (%d)d / (%x)h",(AD_Val*Vcc/1023),AD_Val,AD_Val);
        serial.printf("\r\nAD_Val_f : %1f [V] / (%f)d",(AD_Val_f*Vcc),AD_Val_f);
}


   
void AD_CT(){
        float AD_Val;
        AD_Val = AD_TRANS_IN.read();
        wait(0.02);
        serial.printf("\r\n%1.3f",AD_Val);
    }

/******************************************************************************/
//各POT初期設定
//mode 1:TRANS 2:CT1 3:CT2
/******************************************************************************/
void POTini(int mode){
    ADDR(mode * 2);             //TRANS粗調整POT
    if(mode==1){spi.write(0x0076);}
    else if(mode==2 || mode==3){spi.write(0x006B);}
    
    wait(0.001);
    
    ADDR(mode * 2 - 1);         //TRANS微調整POT
    spi.write(0x007F);
    
    WLAT();
    
    wait(0.001);
    
    ENn = 1;    //ディセーブル
}

/******************************************************************************/
//初期設定
//TRANS_OUT,CT1,2_OUTを初期値(50Hz,AC100V,60A相当)に調整
/******************************************************************************/
void initial(){
    //全てのPOTの端子スイッチをONにする(TCON)
    ADDR(1);    //TRANS微調整POT
    spi.write(0x40FF);
    ADDR(2);    //TRANS粗調整POT
    spi.write(0x40FF);
    ADDR(3);    //CT1微調整POT
    spi.write(0x40FF);
    ADDR(4);    //CT1粗調整POT
    spi.write(0x40FF);
    ADDR(5);    //CT2微調整POT
    spi.write(0x40FF);
    ADDR(6);    //CT2粗調整POT
    spi.write(0x40FF);
    wait(0.02);
    ENn = 1;        //CSをディセーブル
    
    coilEN5 = 1;    //±5V,±15VをON
    
    coilEN1 = 0;    //50Hz切替え
    
    SCT_AMP = 0;    //CT入力アンプ 60Aレベル切替え
    SHDNn = 0;      //電流帰還アンプ シャットダウンしない
    
    //TRANS_OUT初期設定
    coilEN2 = 0;    //TRANS_OUTをAD入力方向へ切替え
    
    ADDR(2);        //TRANS粗調整POT
    spi.write(0x0076);
    read();
    ADDR(1);        //TRANS微調整POT
    spi.write(0x007F);
    read();
    WLAT();         //設定したワイパーをラッチ
    wait(0.02);
    ENn = 1;        //CSをディセーブル

    //CT1_OUT初期設定
    coilEN3 = 0;    //CT1_OUT1と2をAD入力方向へ切替え
    coilEN4 = 0;    //CT1を選択
    
    ADDR(4);        //CT1粗調整POT
    spi.write(0x006B);
    read();
    ADDR(3);        //CT1微調整POT
    spi.write(0x007F);
    read();
    WLAT();         //設定したワイパーをラッチ
    wait(0.02);
    ENn = 1;        //CSをディセーブル

    //CT2_OUT初期設定
    coilEN3 = 0;    //CT1_OUT1と2をAD入力方向へ切替え
    coilEN4 = 1;    //CT2を選択
    
    POTini(3);
    ADDR(6);        //CT2粗調整POT
    spi.write(0x006B);
    read();
    ADDR(5);        //CT2微調整POT
    spi.write(0x007F);
    read();
    WLAT();         //設定したワイパーをラッチ
    wait(0.02);
    ENn = 1;        //CSをディセーブル
    
    coilEN2 = 1;    //TRANS_OUT出力
    coilEN3 = 1;    //CT1,2_OUT出力
 
    for(int i=0; i<10*2; i++){
        led1 = !led1;
        wait(0.1);
        }
    led1 = 0;

    serial.printf("\r\n OK(initial)");
}

/******************************************************************************/
//手動調整
/******************************************************************************/
void manual_adj(int mode){
    if(mode == 2 || mode == 3){
        serial.printf("\r\n Select range mode. Type '60A' or '120A'.");
        while(1){
            if(serial.readable()) {    // 受信確認
                moji[count] = serial.getc();        //キーボード入力文字を1文字ずつmojiに代入
                serial.printf("%c", moji[count]);   //シリアル出力表示
    
                if(count == 5 ){                // ①文字数が既定の個数になった場合
                    moji[5] = '\0';                 // 末尾に終端文字を入れる
                    serial.printf("\r\n\r\n ERROR!\r\n");
                    count = 0;                       // 文字カウンタをリセット
                    }

                else if(moji[0] == '\n'){count = 0;}
    
                else if((moji[count] == '\r')) {       // ②CRを受信した場合
                    moji[count] = '\0';                                         //末尾に終端文字を入れる
                    count = 0;                                                  //文字カウンタをリセット
                    if     (strcmp(moji, "60A") == 0){
                        SCT_AMP = 0;        //CT入力アンプ 60Aレベル切替え
                        break;}
                    else if(strcmp(moji, "120A") == 0){
                        SCT_AMP = 1;        //CT入力アンプ 120Aレベル切替え
                        break;}
                    else serial.printf("\r\n\r\n NG!\r\n");
                    }
                else count++;        // 文字カウンタに1加算
            }
        }
        serial.printf("\r\n OK");
    }

/*
    if(mode == 1){coilEN2 = 0;}                     //TRANS_OUTをAD入力方向へ切替え
    else if(mode == 2){coilEN3 = 0; coilEN4 = 0;}   //CT1_OUT1と2をAD入力方向へ切替え
    else if(mode == 3){coilEN3 = 0; coilEN4 = 1;}   //CT1_OUT1と2をAD入力方向へ切替え
*/

        //POTの状態を確認し、ステータス情報に表示する
        ADDR(mode * 2);             //TRANS粗調整POT
        wait(0.2);
        int read_val_Coarse = spi.write(0x0C00);
        read_val_Coarse &= 0x00FF;
        
        ADDR(mode * 2 - 1);         //TRANS微調整POT
        wait(0.2);
        int read_val_Fine = spi.write(0x0C00);
        read_val_Fine &= 0x00FF;
        ENn = 1;                    //CSをディセーブル
        serial.printf("\r\n\r\n Status  --- Coarse[%d/255], Fine[%d/255] ---\r\n", read_val_Coarse, read_val_Fine);

        
        //ループスタート
        while(1){   //ループ1
                serial.printf("\r\n Select mode '+-' or 'num'. ('q' quit)\r\n");
                while(1){ //adjmode
                    if(serial.readable()) {    // 受信確認
                        moji[count] = serial.getc();        //キーボード入力文字を1文字ずつmojiに代入
                        serial.printf("%c", moji[count]);   //シリアル出力表示
    
                        if(count == 4 ){                // ①文字数が既定の個数になった場合
                            moji[4] = '\0';                 // 末尾に終端文字を入れる
                            serial.printf("\r\n\r\n ERROR!\r\n");
                            count = 0;                       // 文字カウンタをリセット
                        }

                        else if(moji[0] == '\n'){count = 0;}
    
                        else if((moji[count] == '\r')) {       // ②CRを受信した場合
                            moji[count] = '\0';                                         //末尾に終端文字を入れる
                            count = 0;                                                  //文字カウンタをリセット
                            if     (strcmp(moji,"+-") == 0){
                                adjmode = 1;
                                break;}
                            else if(strcmp(moji,"num") == 0){
                                adjmode = 2;
                                break;}
                            else if(strcmp(moji,"q") == 0){break;}
                            else serial.printf("\r\n\r\n NG!\r\n");
                        }
                        else count++;        // 文字カウンタに1加算
                    }
                } //adjmode
            
                if(strcmp(moji,"q")==0){serial.printf("\r\n quit\r\n"); break;} //ループ1ブレイク

            while(1){ //ループ2
                serial.printf("\r\n Type 'C' for Coarse or 'F' for Fine. ('q' quit, 'b' back)\r\n");
                    while(1){ //CorF setting
                        if(serial.readable()) {    // 受信確認
                            moji[count] = serial.getc();        //キーボード入力文字を1文字ずつmojiに代入
                            serial.printf("%c", moji[count]);   //シリアル出力表示
    
                            if(count == 2 ){                // ①文字数が既定の個数になった場合
                                moji[2] = '\0';                 // 末尾に終端文字を入れる
                                serial.printf("\r\n\r\n ERROR!\r\n");
                               count = 0;                       // 文字カウンタをリセット
                            }

                            else if(moji[0] == '\n'){count = 0;}
    
                            else if((moji[count] == '\r')) {       // ②CRを受信した場合
                                moji[count] = '\0';                                         //末尾に終端文字を入れる
                                count = 0;                                                  //文字カウンタをリセット
                                if     (strcmp(moji, "C") == 0){
                                    CorF = 2;                                               //粗調整
                                    serial.printf("\r\n---Coarse Setting---");    
                                    break;}
                                else if(strcmp(moji, "F") == 0){
                                    CorF = 1;                                               //微調整        
                                    serial.printf("\r\n---Fine Setting---");    
                                    break;}
                                else if(strcmp(moji,"b")==0 || strcmp(moji,"q")==0){break;}
                                else serial.printf("\r\n\r\n NG!\r\n");
                            }
                            else count++;        // 文字カウンタに1加算
                        } 
                    }//CorF setting
            
                    if(strcmp(moji,"b")==0){
                        serial.printf("\r\n return\r\n");
                        break;     //ループ2ブレイク
                    }
                    else if(strcmp(moji,"q")==0){break;}   //ループ2ブレイク
                    
                while(1){ //ループ3
                    if(adjmode == 1){
                        serial.printf("\r\n Type '+' for increment, '-' for decrement. ('q' quit, 'b' back)");
                        while(1){ //+-モード
                                if(serial.readable()) {    // 受信確認
                                    moji[count] = serial.getc();        //キーボード入力文字を1文字ずつmojiに代入
                                    serial.printf("\r\n%c", moji[count]);   //シリアル出力表示
                                    moji[1] = '\0';                                         //末尾に終端文字を入れる
                                    count = 0;                                                  //文字カウンタをリセット

                                    if(strcmp(moji,"b")==0 || strcmp(moji,"q")==0){break;}
                                    else if(strcmp(moji,"\n") == 0){}
                                    else if(strcmp(moji,"+") == 0){incdec(1,1,CorF,mode);}
                                    else if(strcmp(moji,"-") == 0){incdec(0,1,CorF,mode);}
                                    else serial.printf("\r\n\r\n NG!\r\n");
                                }
                        } //+-モード
                        if(strcmp(moji,"b")==0){
                            serial.printf("\r\n return\r\n");
                            break;     //ループ3ブレイク
                        }
                        else if(strcmp(moji,"q")==0){break;}   //ループ3ブレイク
                        }                                            
                    else if(adjmode == 2){
                      while(1){ //ループ4
                        serial.printf("\r\n Type '+' for increase, '-' for decrease. ('q' quit, 'b' back)");
                        while(1){ //numモード
                                if(serial.readable()) {    // 受信確認
                                    moji[count] = serial.getc();        //キーボード入力文字を1文字ずつmojiに代入
                                    serial.printf("%c", moji[count]);   //シリアル出力表示
    
                                    if(count == 2 ){                // ①文字数が既定の個数になった場合
                                        moji[2] = '\0';                 // 末尾に終端文字を入れる
                                        serial.printf("\r\n\r\n ERROR!\r\n");
                                       count = 0;                       // 文字カウンタをリセット
                                    }

                                    else if(moji[0] == '\n'){count = 0;}
    
                                    else if((moji[count] == '\r')) {       // ②CRを受信した場合
                                        moji[count] = '\0';                                         //末尾に終端文字を入れる
                                        count = 0;                                                  //文字カウンタをリセット
                                        if     (strcmp(moji, "+") == 0){
                                            incr = 1;                                               //増加
                                            serial.printf("\r\n---increase---");    
                                            break;}
                                        else if(strcmp(moji, "-") == 0){
                                            incr = 0;                                               //減少
                                            serial.printf("\r\n---decrease---");    
                                            break;}
                                        else if(strcmp(moji,"b")==0 || strcmp(moji,"q")==0){break;}
                                        else serial.printf("\r\n\r\n NG!\r\n");
                                    }
                                    else count++;        // 文字カウンタに1加算
                                }
                        } //numモード
                        if(strcmp(moji,"b")==0 || strcmp(moji,"q")==0){break;}  //ループ4ブレイク
                            while(1){ //ループ5
                            serial.printf("\r\n Type number [0-255]. ('q' quit, 'b' back)");
                            for(int i=0; i<3; i++){moji[i] = '0';}
                            
                                while(1){ //numモード2
                                    if(serial.readable()) {    // 受信確認
                                        moji[count] = serial.getc();        //キーボード入力文字を1文字ずつmojiに代入
                                        serial.printf("%c", moji[count]);   //シリアル出力表示
    
                                        if(count == 4 ){                // ①文字数が既定の個数になった場合
                                            moji[4] = '\0';                 // 末尾に終端文字を入れる
                                            serial.printf("\r\n\r\n ERROR!\r\n");
                                           count = 0;                       // 文字カウンタをリセット
                                        }
                                        else if(moji[0] == '\n'){count = 0;}
                                        else if((moji[count] == '\r')) {       // ②CRを受信した場合
                                            moji[count] = '\0';                                         //末尾に終端文字を入れる
                                            count = 0;                                                  //文字カウンタをリセット
                                            if(strcmp(moji,"b")==0 || strcmp(moji,"q")==0){break;}
                                            for(int i=0; i<3; i++){suuji[i] = 0;}
                                            for(int i=0; i<3; i++){
                                                suuji[i] = moji[i];
                                                suuji[i] -= 48;
                                                }
                                            for(int i=0; i<3; i++){moji[i] = '0';}
                                            if(suuji[0]>=0&&suuji[0]<=9){
                                                if(suuji[1]>=0&&suuji[1]<=9){
                                                    if(suuji[2]>=0&&suuji[2]<=9){
                                                        suuji[0] *= 100;
                                                        suuji[1] *= 10;
                                                        }
                                                    else{
                                                        suuji[0] *= 10;
                                                        suuji[2] = 0;
                                                        }
                                                }
                                                else{
                                                    suuji[1] = 0;
                                                    suuji[2] = 0;
                                                    }
                                            }
                                            else{serial.printf("\r\n\r\n NG!\r\n"); break;}

                                            wa = suuji[0]+suuji[1]+suuji[2];
                                            
                                            incdec(incr,wa,CorF,mode);
                                            
                                        }
                                        else count++;        // 文字カウンタに1加算
                                    }
                                } //numモード2
                                if(strcmp(moji,"b")==0){
                                    serial.printf("\r\n return\r\n");
                                    break;  //ループ5ブレイク
                                }
                                else if(strcmp(moji,"q")==0){break;}
                            } //ループ5
                            if(strcmp(moji,"q")==0){break;}    //ループ5→4ブレイク
                        } //ループ4
                        if(strcmp(moji,"b")==0){
                        serial.printf("\r\n return\r\n");
                        break;  //ループ4→3ブレイク
                        }
                        else if(strcmp(moji,"q")==0){break;}    //ループ4→3ブレイク
                    } //if文
                } //ループ3
                if(strcmp(moji,"q")==0){break;}    //ループ3→2ブレイク
            } //ループ2
            if(strcmp(moji,"q")==0){
                serial.printf("\r\n quit\r\n");
                break;    //ループ2→1ブレイク
            }
        } //ループ１
        
        serial.printf("\r\n MANUAL_ADJ END");
}

/******************************************************************************/
//タイマー割込み https://monoist.atmarkit.co.jp/mn/articles/1506/19/news001_2.html
/******************************************************************************/
/*
void attime(){
    led1 = !led1;
}
*/
/******************************************************************************/
//タイマー割込み
//オフセット補正したActualVrefを求める (平均値を出しているだけ)
/******************************************************************************/
void AVrefcal(){
    led1 = !led1;
    
    if(mode_p == 1){AD_val = AD_TRANS_IN.read();}
    else if(mode_p==2 || mode_p==3){AD_val = AD_CT_IN.read();}
    
    AD_val = AD_val * Vcc;  //(val)
    AD_flag = 1;

    //serial.printf("\r\n AD_val = %f",AD_val);
}

////////////////////////////
//サンプリング ActualVref
//mode 1:TRANS 2:CT1 3:CT2
////////////////////////////
void offsetcal(int mode){
    AD_count = 0;
    AD_flag = 0;
    AD_val = 0;
    AD_val_sum = 0;

    mode_p = mode;
    if(mode == 1){coilEN2 = 0;}                     //TRANS_OUTをAD入力方向へ切替え
    else if(mode == 2){coilEN3 = 0; coilEN4 = 0;}   //CT1_OUT1と2をAD入力方向へ切替え
    else if(mode == 3){coilEN3 = 0; coilEN4 = 1;}   //CT1_OUT1と2をAD入力方向へ切替え

    wait(0.05);
    
    timerint.attach(&AVrefcal,0.020408);
    
    timecount.start();
    
    while(1){
        if(AD_count < 98 && AD_flag == 1){    //サンプリング数0~Nで、サンプル取得されたとき
            AD_count ++;
            AD_val_sum += AD_val;
            AD_flag = 0;
            if(AD_count == 0){timecount.start();} //0~Nのサンプリング時間計測用スタート時
            else if(AD_count == 98){        //0~Nのサンプリング時間計測用ストップ時
                timecount.stop();
                //serial.printf("\r\n t = %f",timecount.read());
                timecount.reset();
            }
        }
        else if(AD_count < 98 && AD_flag == 0){}    //サンプリング数0~Nで、サンプル取得されてないとき
        else{                                       //サンプリング完了時
            Vref = AD_val_sum / AD_count;
            serial.printf("\r\n Vref = %f",Vref);
            ActualVref[mode_p] = Vref;              //格納
            break;
        }
    }
    timerint.detach();
    led1 = 0;
}

void ActVref1(){
    serial.printf("\r\n ActualVref[0] = %f", *pAVrefcal);
}
/******************************************************************************/
//タイマー割込み
//AD変換によりVrmsを求める
/******************************************************************************/
//タイマー割込み実行関数
void timerAD(){
    led1 = !led1;
    serial.printf("\r\n mode_p = %d",mode_p);
    serial.printf("\r\n ActualVref = %f",ActualVref[mode_p]);
    
    if(mode_p == 1){AD_val = AD_TRANS_IN.read();}
    else if(mode_p==2 || mode_p==3){AD_val = AD_CT_IN.read();}         
    
    //serial.printf("\r\n AD_val = %f",AD_val);
    
    AD_val *= Vcc;
    AD_val -= ActualVref[mode_p];
    //serial.printf("\r\n  AD_val*Vcc-Vref = %f",AD_val);
    AD_val *= AD_val;
    AD_flag = 1;
      
    //serial.printf("\r\n AD_val = %f",AD_val);
}
////////////////////////////
//サンプリング Vrms
//mode 1:TRANS 2:CT1 3:CT2
////////////////////////////
void ADVrms(int mode){
    AD_count = 0;
    AD_flag = 0;
    AD_val = 0;
    AD_val_sum = 0;

    mode_p = mode;
    if(mode == 1){coilEN2 = 0;}                     //TRANS_OUTをAD入力方向へ切替え
    else if(mode == 2){coilEN3 = 0; coilEN4 = 0;}   //CT1_OUT1をAD入力方向へ切替え
    else if(mode == 3){coilEN3 = 0; coilEN4 = 1;}   //CT1_OUT2をAD入力方向へ切替え

    wait(0.05);
    
    timerint.attach(&timerAD,0.020408);
    
    timecount.start();
            
    while(1){
        if(AD_count < 49 && AD_flag == 1){      //サンプリング数0~Nで、サンプル取得されたとき
            AD_count ++;
            AD_val_sum += AD_val;
            AD_flag = 0;
            if(AD_count == 0){timecount.start();}   //0~Nのサンプリング時間計測用スタート時
            else if(AD_count == 49){          //0~Nのサンプリング時間計測用ストップ時
                timecount.stop();
                //serial.printf("\r\n t = %f",timecount.read());
                timecount.reset();
            }
        }
        else if(AD_count < 49 && AD_flag == 0){}//サンプリング数0~Nで、サンプル取得されてないとき
        else{                                       //サンプリング完了時
            AD_val_sum /= AD_count;
            Vrms = sqrtf(AD_val_sum);
            //serial.printf("\r\n AD_val_sum = %f",AD_val_sum);
            serial.printf("\r\n Vrms = %f",Vrms);
            break;
        }
    }
    timerint.detach();
    led1 = 0;
}

//デバッグ用
void att(int mode){
    mode_p = mode;
    timerint.attach(&timerAD,0.020408);    //サンプリング数=49  20.4ms周期
    } 
void det(){timerint.detach();}

/******************************************************************************/
//自動調整
//TRANS_OUT設定範囲 : AC 50~138V
//CT1,2_OUT設定範囲 : AC 10~120A
/******************************************************************************/
void auto_adj(int mode){

///////////////////////////////////////////////////////////////////////////////
    POTini(mode);   //POTを初期値に設定
///////////////////////////////////////////////////////////////////////////////

    //'60A' or '120A'
    if(mode == 2 || mode == 3){
        serial.printf("\r\n Select range mode. Type '60A' or '120A'.");
        while(1){
            if(serial.readable()) {    // 受信確認
                moji[count] = serial.getc();        //キーボード入力文字を1文字ずつmojiに代入
                serial.printf("%c", moji[count]);   //シリアル出力表示
    
                if(count == 5 ){                // ①文字数が既定の個数になった場合
                    moji[5] = '\0';                 // 末尾に終端文字を入れる
                    serial.printf("\r\n\r\n ERROR!\r\n");
                    count = 0;                       // 文字カウンタをリセット
                    }

                else if(moji[0] == '\n'){count = 0;}
    
                else if((moji[count] == '\r')) {       // ②CRを受信した場合
                    moji[count] = '\0';                                         //末尾に終端文字を入れる
                    count = 0;                                                  //文字カウンタをリセット
                    if     (strcmp(moji, "60A") == 0){
                        SCT_AMP = 0;        //CT入力アンプ 60Aレベル切替え
                        break;}
                    else if(strcmp(moji, "120A") == 0){
                        SCT_AMP = 1;        //CT入力アンプ 120Aレベル切替え
                        break;}
                    else serial.printf("\r\n\r\n NG!\r\n");
                    }
                else count++;        // 文字カウンタに1加算
            }
        }
        serial.printf("\r\n OK");
    } //'60A' or '120A'

    while(1) { //ループ1
        int adjnum = 0;
    
        //シリアル表示
        if(mode == 1) {
            serial.printf("\r\n Type number AC [50-138] V. ('q' quit)");
        } else if(mode==2 || mode==3) {
            serial.printf("\r\n Type number AC [10-120] A. ('q' quit)\r\n (If 61~120A, range mode change 120A automatically.)");
        }

        for(int i=0; i<3; i++) {
            moji[i] = '0';   //mojiを初期化
        }

        while(1) { //adjnumループ
            if(serial.readable()) {    // 受信確認
                moji[count] = serial.getc();        //キーボード入力文字を1文字ずつmojiに代入
                serial.printf("%c", moji[count]);   //シリアル出力表示

                if(count == 4 ) {               // ①文字数が既定の個数になった場合
                    moji[4] = '\0';                 // 末尾に終端文字を入れる
                    serial.printf("\r\n\r\n ERROR!\r\n");
                    count = 0;                       // 文字カウンタをリセット
                } else if(moji[0] == '\n') {
                    count = 0;
                } else if((moji[count] == '\r')) {     // ②CRを受信した場合
                    moji[count] = '\0';                                         //末尾に終端文字を入れる
                    count = 0;                                                  //文字カウンタをリセット
                    if(strcmp(moji,"q")==0) {
                        break;
                    }
                    for(int i=0; i<3; i++) {
                        suuji[i] = 0;   //suujiを初期化
                    }
                    for(int i=0; i<3; i++) {
                        suuji[i] = moji[i];
                        suuji[i] -= 48;  //ASCII → 10進
                    }
                    for(int i=0; i<3; i++) {
                        moji[i] = '0';   //mojiを初期化
                    }

                    //数字であるか判定
                    if(suuji[0]>=0 && suuji[0]<=9) {
                        if(suuji[1]>=0&&suuji[1]<=9) {
                            if(suuji[2]>=0&&suuji[2]<=9) {
                                suuji[0] *= 100;
                                suuji[1] *= 10;
                            } else {
                                suuji[0] *= 10;
                                suuji[2] = 0;
                            }
                        } else {
                            suuji[1] = 0;
                            suuji[2] = 0;
                        }
                    } else {
                        serial.printf("\r\n\r\n NG!\r\n");
                        adjnum = 1;
                        break;  //adjnumループ ブレイク
                    }

                    setval = suuji[0]+suuji[1]+suuji[2];
                    
                    //設定範囲判定 と Vrms換算
                    if(mode == 1){
                        if(setval >= 50 && setval <= 138){
                            setval = setval * 0.12 * 0.196078;  //0.12:トランス比 0.196078:分圧抵抗比
                            //serial.printf("\r\n %f",setval);
                            setval /= 2;
                            setval /= sqrtf(2);
                            serial.printf("\r\n setval = %f",setval);
                            break;  //adjnumループ ブレイク
                        }
                        else{
                            serial.printf("\r\n\r\n NG!\r\n");
                            adjnum = 1;
                            break;  //adjnumループ ブレイク
                        }
                    }
                    else if(mode==2 || mode==3){
                        if(setval >= 10 && setval <= 120){
                            setval = setval / 3000 * 10 * 5.666667;  //3000:CT比 10:終端抵抗 5.666667:分圧抵抗比
                            serial.printf("\r\n setval = %f",setval);
                            if(setval >= 61 && setval <= 120){SCT_AMP = 1;}
                            break;  //adjnumループ ブレイク
                        }
                        else{
                            serial.printf("\r\n\r\n NG!\r\n");
                            adjnum = 1;
                            break;  //adjnumループ ブレイク
                        }
                    }
                } else count++;      // 文字カウンタに1加算
            } //if
        } //adjnumループ
        if(adjnum==0 || strcmp(moji,"q")==0){break;} //ループ1ブレイク
    } //ループ1

    int adjcount = 0; //ループ2上限変数
    
    while(1){ //ループ2
        if(strcmp(moji,"q")==0){
            serial.printf("\r\n quit");
            break; //ループ2ブレイク
        }
        else if(adjcount >= 20){    //ループ2上限値
            serial.printf("\r\n\r\n ERROR!\r\n");
            break; //ループ2ブレイク
        }
        
        ADVrms(mode);   //現在のVrms値
        
        diff = setval - Vrms;   //差分
        //serial.printf("\r\n diff = %f",diff);
        
        //増減方向判定
        if(diff > 0){incr = 1;} //増
        else if(diff < 0){
            incr = 0;           //減
            diff *= -1;         //絶対値
        }
        else if(diff == 0){break;}  //差分無し ループ2ブレイク
        serial.printf("\r\n diff = %f",diff);
        serial.printf("\r\n incr = %d",incr);
        
        //粗・微 調整判定
        if(diff >= 0.03){CorF = 2;} //粗
        else if(diff < 0.03){
            CorF = 1;               //微
            if(diff < 0.002){break;}//差分微小 ループ2ブレイク(通常最終ゴール)
        }
        serial.printf("\r\n CorF = %d",CorF);
        
        //調整tap値算出
        if(CorF == 2){val = diff / 0.02;}
        else if(CorF == 1){val = diff / 0.002;}
        serial.printf("\r\n tap(val) = %d",val);
        
        //ワイパー位置移動
        incdec(incr, val, CorF, mode);
        
        adjcount ++;
         
    } //ループ2
    
    if(mode==1){coilEN2 = 0;}                                //TRANS_OUTを外部出力へ切替え
    else if(mode==2 || mode==3){coilEN3 = 0; coilEN4 = 0;}   //CT1_OUT1と2を外部出力へ切替え
    
    serial.printf("\r\n ADJ END");
}

/******************************************************************************/
//シリアル入力受付 [メイン]
/******************************************************************************/
void serial_inout(){
    if(serial.readable()) {    // 受信確認
    moji[count] = serial.getc();        //キーボード入力文字を1文字ずつmojiに代入
    serial.printf("%c", moji[count]);   //シリアル出力表示
//    serial.printf("%c(%d)", moji[count],count);   //シリアル出力表示
        
        if(count == 31 ){                // ①文字数が既定の個数になった場合
        moji[31] = '\0';                 // 末尾に終端文字を入れる
        count = 0;                       // 文字カウンタをリセット
        serial.printf("\r\n\r\n ERROR!");
        }

        else if(moji[0] == '\n'){count = 0;}
    
        else if((moji[count] == '\r')) {       // ②CRを受信した場合
            moji[count] = '\0';                                                     //末尾に終端文字を入れる
            count = 0;                                                              //文字カウンタをリセット
            if     (strcmp(moji, "cmd") == 0){helpwrite();}                         //コマンド一覧表示
            else if(strcmp(moji, "help") == 0){helpwrite();}                        //コマンド一覧表示
            else if(strcmp(moji, "?") == 0){helpwrite();}                           //コマンド一覧表示
            else if(strcmp(moji, "initial") == 0){initial();}                       //初期化
            else if(strcmp(moji, "V_OFF") == 0){coilEN5 = 0;}                       //±5V,±15Vオフ
            else if(strcmp(moji, "V_ON") == 0){coilEN5 = 1;}                        //±5V,±15Vオン
            else if(strcmp(moji, "50Hz") == 0){coilEN1 = 0;}                        //周波数切替
            else if(strcmp(moji, "60Hz") == 0){coilEN1 = 1;}                        //周波数切替
            else if(strcmp(moji, "TRANS_OUT_ADINT") == 0){coilEN2 = 0;}             //TRANS_OUT内部AD入力
            else if(strcmp(moji, "TRANS_OUT_EXT") == 0){coilEN2 = 1;}               //TRANS_OUT外部出力
            else if(strcmp(moji, "CT1_OUT_ADINT") == 0){coilEN3 = 0; coilEN4 = 0;}  //CT1_OUT内部AD入力
            else if(strcmp(moji, "CT1_OUT_ADINT") == 0){coilEN3 = 0; coilEN4 = 1;}  //CT2_OUT内部AD入力
            else if(strcmp(moji, "CT_OUT_EXT") == 0){coilEN3 = 1;}                  //CT1,2_OUT外部出力
            else if(strcmp(moji, "offset1") == 0){offsetcal(1);}                    //TRANS_OUT ADオフセット補正
            else if(strcmp(moji, "offset2") == 0){offsetcal(2);}                    //CT1_OUT ADオフセット補正
            else if(strcmp(moji, "offset3") == 0){offsetcal(3);}                    //CT2_OUT ADオフセット補正
            else if(strcmp(moji, "TRANS_ADJ") == 0){auto_adj(1);}                   //TRANS_OUT調整モードに移行
            else if(strcmp(moji, "CT1_ADJ") == 0){auto_adj(2);}                     //CT1_OUT調整モードに移行
            else if(strcmp(moji, "CT2_ADJ") == 0){auto_adj(3);}                     //CT2_OUT調整モードに移行
            else if(strcmp(moji, "MANUAL_TRANS_ADJ") == 0){manual_adj(1);}          //TRANS_OUT手動調整モードに移行
            else if(strcmp(moji, "MANUAL_CT1_ADJ") == 0){manual_adj(2);}            //CT1_OUT手動調整モードに移行
            else if(strcmp(moji, "MANUAL_CT2_ADJ") == 0){manual_adj(3);}            //CT1_OU2手動調整モードに移行
            
            //デバッグ用
            else if(strcmp(moji, "TCON") == 0){TCON();}
            else if(strcmp(moji, "TCONread") == 0){TCONread();}
            else if(strcmp(moji, "read") == 0){read();}
            else if(strcmp(moji, "write00") == 0){write00();}
            else if(strcmp(moji, "write7F") == 0){write7F();}
            else if(strcmp(moji, "writeFF") == 0){writeFF();}
            else if(strcmp(moji, "WLAT") == 0){WLAT();}
            else if(strcmp(moji, "CS0") == 0){CS(0);}
            else if(strcmp(moji, "CS1") == 0){CS(1);}
            else if(strcmp(moji, "AD1") == 0){AD(1);}
            else if(strcmp(moji, "AD2") == 0){AD(2);}
            else if(strcmp(moji, "TENinc") == 0){TENinc();}
            else if(strcmp(moji, "att1") == 0){att(1);}
            else if(strcmp(moji, "att2") == 0){att(2);}
            else if(strcmp(moji, "att3") == 0){att(3);}
            else if(strcmp(moji, "det") == 0){det();}
            else if(strcmp(moji, "AD1_Vrms") == 0){ADVrms(1);}
            else if(strcmp(moji, "AD2_Vrms") == 0){ADVrms(2);}
            else if(strcmp(moji, "AD3_Vrms") == 0){ADVrms(3);}
            else if(strcmp(moji, "ActVref1") == 0){ActVref1();}
            

            
            else serial.printf("\r\n\r\n NG!\r\n");
        }
        else count++;        // 文字カウンタに1加算
    }
}    

/******************************************************************************/
//メインプログラム
/******************************************************************************/
int main() {
    pAVrefcal = &ActualVref[0];
    pmode = &mode_p;
    
    led1 = 1;
    wait(0.2);
    setup();
    initial();
    serialstart();
    
//*(volatile int *)0x4004402C = 0x2;  //  (レジスタ値を直接入力) アドレス0x4004402Cに値を代入
//*(volatile int *)0x4004402C;
//serial.printf( "data in 0x4004402C is 0x%08X\n", *(volatile int *)0x4004402C );  //  アドレス0x4004402Cの値を表示

//timerint.attach(&attime,0.1);
  
    while(1){
        serial_inout();
    }
  

}