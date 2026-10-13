#include "oled.h"
#include "stdlib.h"
#include "oledfont.h"
#include "delay.h"
#include "app_k230.h"
#include "key.h"        /* g_mode_select, g_selected_mode, g_task_paused */
#include "app_irtracking.h"



/* OLED I2C slave address (7-bit) */
#define OLED_I2C_ADDR 0x3C

u8 OLED_GRAM[144][4];

/* Wait until I2C controller is idle */
static void i2c_wait_idle(void)
{
    while (!(DL_I2C_getControllerStatus(OLED_I2C_INST) &
             DL_I2C_CONTROLLER_STATUS_IDLE))
        ;
}

void OLED_ColorTurn(u8 i)
{
    if (i == 0) {
        OLED_WR_Byte(0xA6, OLED_CMD);
    }
    if (i == 1) {
        OLED_WR_Byte(0xA7, OLED_CMD);
    }
}

void OLED_DisplayTurn(u8 i)
{
    if (i == 0) {
        OLED_WR_Byte(0xC8, OLED_CMD);
        OLED_WR_Byte(0xA1, OLED_CMD);
    }
    if (i == 1) {
        OLED_WR_Byte(0xC0, OLED_CMD);
        OLED_WR_Byte(0xA0, OLED_CMD);
    }
}

/*
 * Write one byte to OLED via hardware I2C.
 * mode: OLED_CMD (0) = command, OLED_DATA (1) = data
 */
void OLED_WR_Byte(u8 dat, u8 mode)
{
    uint8_t txBuf[2];
    txBuf[0] = (mode == OLED_DATA) ? 0x40 : 0x00;
    txBuf[1] = dat;

    i2c_wait_idle();
    DL_I2C_fillControllerTXFIFO(OLED_I2C_INST, txBuf, 2);
    DL_I2C_startControllerTransfer(OLED_I2C_INST, OLED_I2C_ADDR,
        DL_I2C_CONTROLLER_DIRECTION_TX, 2);
    while (DL_I2C_getControllerStatus(OLED_I2C_INST) &
           DL_I2C_CONTROLLER_STATUS_BUSY_BUS)
        ;
    i2c_wait_idle();
}

void OLED_DisPlay_On(void)
{
    OLED_WR_Byte(0x8D, OLED_CMD);
    OLED_WR_Byte(0x14, OLED_CMD);
    OLED_WR_Byte(0xAF, OLED_CMD);
}

void OLED_DisPlay_Off(void)
{
    OLED_WR_Byte(0x8D, OLED_CMD);
    OLED_WR_Byte(0x10, OLED_CMD);
    OLED_WR_Byte(0xAE, OLED_CMD);
}

void OLED_Refresh(void)
{
    u8 i, n;
    for (i = 0; i < 4; i++) {
        OLED_WR_Byte(0xB0 + i, OLED_CMD);
        OLED_WR_Byte(0x00, OLED_CMD);
        OLED_WR_Byte(0x10, OLED_CMD);

        /*
         * Transfer 128 bytes of GRAM data in chunks.
         * FIFO is 8 bytes deep: 1 control byte + up to 7 data bytes.
         */
        for (n = 0; n < 128; n += 7) {
            uint8_t chunk[8];
            uint8_t size = 1;
            uint8_t batch = (128 - n > 7) ? 7 : (128 - n);
            chunk[0] = 0x40; /* data mode */
            for (uint8_t j = 0; j < batch; j++) {
                chunk[size++] = OLED_GRAM[n + j][i];
            }

            i2c_wait_idle();
            DL_I2C_fillControllerTXFIFO(OLED_I2C_INST, chunk, size);
            DL_I2C_startControllerTransfer(OLED_I2C_INST, OLED_I2C_ADDR,
                DL_I2C_CONTROLLER_DIRECTION_TX, size);
            while (DL_I2C_getControllerStatus(OLED_I2C_INST) &
                   DL_I2C_CONTROLLER_STATUS_BUSY_BUS)
                ;
            i2c_wait_idle();
        }
    }
}

void OLED_Clear(void)
{
    u8 i, n;
    for (i = 0; i < 4; i++) {
        for (n = 0; n < 128; n++) {
            OLED_GRAM[n][i] = 0;
        }
    }
    OLED_Refresh();
}

void OLED_DrawPoint(u8 x, u8 y, u8 t)
{
    u8 i, m, n;
    i = y / 8;
    m = y % 8;
    n = 1 << m;
    if (t) {
        OLED_GRAM[x][i] |= n;
    } else {
        OLED_GRAM[x][i] = ~OLED_GRAM[x][i];
        OLED_GRAM[x][i] |= n;
        OLED_GRAM[x][i] = ~OLED_GRAM[x][i];
    }
}

void OLED_DrawLine(u8 x1, u8 y1, u8 x2, u8 y2, u8 mode)
{
    u16 t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1;
    delta_y = y2 - y1;
    uRow    = x1;
    uCol    = y1;
    if (delta_x > 0)
        incx = 1;
    else if (delta_x == 0)
        incx = 0;
    else {
        incx    = -1;
        delta_x = -delta_x;
    }
    if (delta_y > 0)
        incy = 1;
    else if (delta_y == 0)
        incy = 0;
    else {
        incy    = -1;
        delta_y = -delta_y;
    }
    if (delta_x > delta_y)
        distance = delta_x;
    else
        distance = delta_y;
    for (t = 0; t < distance + 1; t++) {
        OLED_DrawPoint(uRow, uCol, mode);
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance) {
            xerr -= distance;
            uRow += incx;
        }
        if (yerr > distance) {
            yerr -= distance;
            uCol += incy;
        }
    }
}

void OLED_DrawCircle(u8 x, u8 y, u8 r)
{
    int a, b, num;
    a = 0;
    b = r;
    while (2 * b * b >= r * r) {
        OLED_DrawPoint(x + a, y - b, 1);
        OLED_DrawPoint(x - a, y - b, 1);
        OLED_DrawPoint(x - a, y + b, 1);
        OLED_DrawPoint(x + a, y + b, 1);
        OLED_DrawPoint(x + b, y + a, 1);
        OLED_DrawPoint(x + b, y - a, 1);
        OLED_DrawPoint(x - b, y - a, 1);
        OLED_DrawPoint(x - b, y + a, 1);

        a++;
        num = (a * a + b * b) - r * r;
        if (num > 0) {
            b--;
            a--;
        }
    }
}

void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 size1, u8 mode)
{
    u8 i, m, temp, size2, chr1;
    u8 x0 = x, y0 = y;
    if (size1 == 8)
        size2 = 6;
    else
        size2 =
            (size1 / 8 + ((size1 % 8) ? 1 : 0)) * (size1 / 2);
    chr1 = chr - ' ';
    for (i = 0; i < size2; i++) {
        if (size1 == 8) {
            temp = asc2_0806[chr1][i];
        } else if (size1 == 12) {
            temp = asc2_1206[chr1][i];
        } else if (size1 == 16) {
            temp = asc2_1608[chr1][i];
        } else if (size1 == 24) {
            temp = asc2_2412[chr1][i];
        } else
            return;
        for (m = 0; m < 8; m++) {
            if (temp & 0x01)
                OLED_DrawPoint(x, y, mode);
            else
                OLED_DrawPoint(x, y, !mode);
            temp >>= 1;
            y++;
        }
        x++;
        if ((size1 != 8) && ((x - x0) == size1 / 2)) {
            x  = x0;
            y0 = y0 + 8;
        }
        y = y0;
    }
}

void OLED_ShowString(u8 x, u8 y, u8 *chr, u8 size1, u8 mode)
{
    while ((*chr >= ' ') && (*chr <= '~')) {
        OLED_ShowChar(x, y, *chr, size1, mode);
        if (size1 == 8)
            x += 6;
        else
            x += size1 / 2;
        chr++;
    }
}

u32 OLED_Pow(u8 m, u8 n)
{
    u32 result = 1;
    while (n--) {
        result *= m;
    }
    return result;
}

void OLED_ShowNum(u8 x, u8 y, u32 num, u8 len, u8 size1, u8 mode)
{
    u8 t, temp, m = 0;
    if (size1 == 8) m = 2;
    for (t = 0; t < len; t++) {
        temp = (num / OLED_Pow(10, len - t - 1)) % 10;
        if (temp == 0) {
            OLED_ShowChar(x + (size1 / 2 + m) * t, y, '0', size1, mode);
        } else {
            OLED_ShowChar(x + (size1 / 2 + m) * t, y, temp + '0',
                size1, mode);
        }
    }
}

void OLED_ShowChinese(u8 x, u8 y, u8 num, u8 size1, u8 mode)
{
    u8 m, temp;
    u8 x0 = x, y0 = y;
    u16 i, size3 = (size1 / 8 + ((size1 % 8) ? 1 : 0)) * size1;
    for (i = 0; i < size3; i++) {
        if (size1 == 16) {
            temp = Hzk1[num][i];
        } else if (size1 == 24) {
            temp = Hzk2[num][i];
        } else if (size1 == 32) {
            temp = Hzk3[num][i];
        } else if (size1 == 64) {
            temp = Hzk4[num][i];
        } else
            return;
        for (m = 0; m < 8; m++) {
            if (temp & 0x01)
                OLED_DrawPoint(x, y, mode);
            else
                OLED_DrawPoint(x, y, !mode);
            temp >>= 1;
            y++;
        }
        x++;
        if ((x - x0) == size1) {
            x  = x0;
            y0 = y0 + 8;
        }
        y = y0;
    }
}

void OLED_ScrollDisplay(u8 num, u8 space, u8 mode)
{
    u8 i, n, t = 0, m = 0, r;
    while (1) {
        if (m == 0) {
            OLED_ShowChinese(128, 8, t, 16, mode);
            t++;
        }
        if (t == num) {
            for (r = 0; r < 16 * space; r++) {
                for (i = 1; i < 144; i++) {
                    for (n = 0; n < 4; n++) {
                        OLED_GRAM[i - 1][n] = OLED_GRAM[i][n];
                    }
                }
                OLED_Refresh();
            }
            t = 0;
        }
        m++;
        if (m == 16) { m = 0; }
        for (i = 1; i < 144; i++) {
            for (n = 0; n < 4; n++) {
                OLED_GRAM[i - 1][n] = OLED_GRAM[i][n];
            }
        }
        OLED_Refresh();
    }
}

void OLED_ShowPicture(
    u8 x, u8 y, u8 sizex, u8 sizey, u8 BMP[], u8 mode)
{
    u16 j = 0;
    u8 i, n, temp, m;
    u8 x0 = x, y0 = y;
    sizey = sizey / 8 + ((sizey % 8) ? 1 : 0);
    for (n = 0; n < sizey; n++) {
        for (i = 0; i < sizex; i++) {
            temp = BMP[j];
            j++;
            for (m = 0; m < 8; m++) {
                if (temp & 0x01)
                    OLED_DrawPoint(x, y, mode);
                else
                    OLED_DrawPoint(x, y, !mode);
                temp >>= 1;
                y++;
            }
            x++;
            if ((x - x0) == sizex) {
                x  = x0;
                y0 = y0 + 8;
            }
            y = y0;
        }
    }
}

void OLED_Init(void)
{
    delay_ms(200);

    OLED_WR_Byte(0xAE, OLED_CMD); /*display off*/
    OLED_WR_Byte(0x00, OLED_CMD); /*set lower column address*/
    OLED_WR_Byte(0x10, OLED_CMD); /*set higher column address*/
    OLED_WR_Byte(0x00, OLED_CMD); /*set display start line*/
    OLED_WR_Byte(0xB0, OLED_CMD); /*set page address*/
    OLED_WR_Byte(0x81, OLED_CMD); /*contract control*/
    OLED_WR_Byte(0xff, OLED_CMD); /*128*/
    OLED_WR_Byte(0xA1, OLED_CMD); /*set segment remap*/
    OLED_WR_Byte(0xA6, OLED_CMD); /*normal / reverse*/
    OLED_WR_Byte(0xA8, OLED_CMD); /*multiplex ratio*/
    OLED_WR_Byte(0x1F, OLED_CMD); /*duty = 1/32*/
    OLED_WR_Byte(0xC8, OLED_CMD); /*Com scan direction*/
    OLED_WR_Byte(0xD3, OLED_CMD); /*set display offset*/
    OLED_WR_Byte(0x00, OLED_CMD);
    OLED_WR_Byte(0xD5, OLED_CMD); /*set osc division*/
    OLED_WR_Byte(0x80, OLED_CMD);
    OLED_WR_Byte(0xD9, OLED_CMD); /*set pre-charge period*/
    OLED_WR_Byte(0x1f, OLED_CMD);
    OLED_WR_Byte(0xDA, OLED_CMD); /*set COM pins*/
    OLED_WR_Byte(0x00, OLED_CMD);
    OLED_WR_Byte(0xdb, OLED_CMD); /*set vcomh*/
    OLED_WR_Byte(0x40, OLED_CMD);
    OLED_WR_Byte(0x8d, OLED_CMD); /*set charge pump enable*/
    OLED_WR_Byte(0x14, OLED_CMD);
    OLED_Clear();
    OLED_WR_Byte(0xAF, OLED_CMD); /*display ON*/
}


/* ============================================================
 * OLED 模式选择/任务状态显示 (每200ms由调度器调用)
 *
 * 显示内容:
 *   模式选择界面: 第1行 "Mode:2" 第2行 "K1:OK  Long:Chg"
 *   任务运行中:   第1行 "T2:Line" 或 "T3:Ball"
 *   暂停状态:     第1行 "PAUSED"
 * ============================================================ */

void OLED_time(void)
{
    static uint8_t last_mode = 255, last_pause = 255;
    static int last_task = -1;
    static uint32_t last_sec = 0xFFFFFFFF;
    char buf[12];

    int task_done = (g_task_end_ms > 0); /* 任务已完成, 显示最终时间 */
    int t_running = (State_Machine.Main_State != STOP_STATE) && !g_mode_select && !g_task_paused;

    uint32_t now_sec = 0;
    if (task_done)
        now_sec = (g_task_end_ms - g_task_start_ms) / 1000;
    else if (t_running)
        now_sec = (Get_Time() - g_task_start_ms) / 1000;

    uint8_t changed = 0;
    if (g_mode_select) {
        if (g_selected_mode != last_mode) { last_mode = g_selected_mode; last_task = -1; changed = 1; }
    } else if (g_task_paused) {
        if (last_pause != 1) { last_pause = 1; last_task = -1; changed = 1; }
    } else if (task_done) {
        if (last_task != 99) { last_task = 99; changed = 1; }
    } else {
        if (last_task != State_Machine.Main_State) { last_task = State_Machine.Main_State; last_pause = 0; changed = 1; }
    }
    if ((!g_mode_select && !task_done && t_running) && now_sec != last_sec) { last_sec = now_sec; changed = 1; }
    if (!changed) return;

    OLED_Clear();

    if (g_mode_select) {
        sprintf(buf, "Mode:%d", g_selected_mode + 1);
        OLED_ShowString(42, 2, (uint8_t*)buf, 12, 1);
    } else if (g_task_paused) {
        OLED_ShowString(36, 2, (uint8_t*)"PAUSED", 12, 1);
    } else if (task_done) {
        OLED_ShowString(24, 0, (uint8_t*)"FINISH", 12, 1);
    } else {
        switch (State_Machine.Main_State) {
        case TASK_LINE_LAP:  OLED_ShowString(36, 0, (uint8_t*)"T1:Line", 12, 1); break;
        case TASK_BALL_CTRL: OLED_ShowString(36, 0, (uint8_t*)"T2:Ball", 12, 1); break;
        case TASK_AB_STAB:   OLED_ShowString(38, 0, (uint8_t*)"T3:A->B", 12, 1); break;
        case TASK_LAP_STAB:  OLED_ShowString(38, 0, (uint8_t*)"T4:Lap", 12, 1);  break;
        case TASK_LAP_POS:   OLED_ShowString(38, 0, (uint8_t*)"T5:Pos", 12, 1);  break;
        default: break;
        }
    }

    /* 下半屏: 计时 */
    if (task_done || t_running) {
        sprintf(buf, "%02d:%02d", (int)(now_sec/60), (int)(now_sec%60));
    } else {
        static uint8_t last_key = 0, running = 0;
        static uint32_t start_ms = 0, elapsed_ms = 0;
        uint8_t key = (DL_GPIO_readPins(KEY_PORT, DL_GPIO_PIN_18) > 0);
        if (key && !last_key) {
            delay_ms(30);
            if (DL_GPIO_readPins(KEY_PORT, DL_GPIO_PIN_18) > 0) {
                if (running) { elapsed_ms += (Get_Time() - start_ms); running = 0; }
                else         { start_ms = Get_Time(); running = 1; }
            }
        }
        last_key = key;
        uint32_t ms = running ? elapsed_ms + (Get_Time() - start_ms) : elapsed_ms;
        sprintf(buf, "%02d:%02d", (int)(ms/60000), (int)((ms/1000)%60));
    }
    OLED_ShowString(44, 22, (uint8_t*)buf, 8, 1);
    OLED_Refresh();
}
