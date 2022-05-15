//#include "../defs.h"
#include "../defs_mpw-two-mfix.h"
#include "../print_io.h"
//#include "spi_io.h"

// wakey_wakey defs
// Define Wishbone Addresses in CFG
#define cfg_reg_addr   (*(volatile uint32_t*)0x30000000)
#define cfg_reg_ctrl   (*(volatile uint32_t*)0x30000004)
#define cfg_reg_data_0 (*(volatile uint32_t*)0x30000008)
#define cfg_reg_data_1 (*(volatile uint32_t*)0x3000000C)
#define cfg_reg_data_2 (*(volatile uint32_t*)0x30000010)
#define cfg_reg_data_3 (*(volatile uint32_t*)0x30000014)

void cfg_store(int addr, int data_3, int data_2, int data_1, int data_0)
{
    /*
     * Store to Wakey Wakey Memory
     * addr is a 32b address in the Wakey Wakey address space
     * data_3 MSB
     * data_2
     * data_1
     * data_0 LSB
     */

    // write the store address
    cfg_reg_addr = addr;
    // write data words
    cfg_reg_data_0 = data_0;
    cfg_reg_data_1 = data_1;
    cfg_reg_data_2 = data_2;
    cfg_reg_data_3 = data_3;
    // write store command - 0x1
    cfg_reg_ctrl = 0x1;
}

void cfg_load(int addr, int *data)
{
    /*
     * Load from Wakey Wakey Memory
     * addr is a 32b address in the Wakey Wakey address space
     * returns a list of values into data
     * where data[3] is the MSB (data_3) and data[0] is the LSB (data_0)
     */

    // write address the load address
    cfg_reg_addr = addr;
    // write the load command - 0x2
    cfg_reg_ctrl = 0x2;
    // INFO: Currently need to wait one clock cycle before read starts
    __asm__("nop\n\t");

    // read the data words
    data[0] = cfg_reg_data_0;
    data[1] = cfg_reg_data_1;
    data[2] = cfg_reg_data_2;
    data[3] = cfg_reg_data_3;
}

/*
	Wakey Wakey Test:
        1. Configure PDM Data Input Pin
        2. Configure PDM Activate Input Pin
        3. Configure PDM Clock Output Pin
        4. Configure Wake Output Pin
        5. Write CFG Data via wishbone
        6. Read CFG Data via wishbone
*/

// void cfg_store(int addr, int data_3, int data_2, int data_1, int data_0)
// void cfg_load(int addr, int *data)
bool check_output(int *expected, int *observed) {
    for (int k = 0; k < 4; k++) {
        if (expected[k] != observed[k]) return false;
    }
    return true;
}

void mult_arr(int *arr) {
    for (int k = 0; k < 4; k++) {
        arr[k] *= 2;
    }
}

/* Returns true if the test passes and false if not. */
bool run_test() {
    //return false;
    int readbuf[4] = {0,0,0,0};

    // /*
    // simple test
    int writebuf[4] = {1, 3, 4, 2};
    cfg_store(0, writebuf[3], writebuf[2], writebuf[1], writebuf[0]);
    cfg_store(1, writebuf[3]*2, writebuf[2]*2, writebuf[1]*2, writebuf[0]*2);
    cfg_store(2, writebuf[3]*4, writebuf[2]*4, writebuf[1]*4, writebuf[0]*4);
    cfg_store(3, writebuf[3]*8, writebuf[2]*8, writebuf[1]*8, writebuf[0]*8);
    // cfg_store(4, writebuf[3]*5, writebuf[2]*5, writebuf[1]*5, writebuf[0]*5);
    // cfg_store(5, writebuf[3]*6, writebuf[2]*6, writebuf[1]*6, writebuf[0]*6);
    // cfg_store(6, writebuf[3]*7, writebuf[2]*7, writebuf[1]*7, writebuf[0]*7);
    // cfg_store(7, writebuf[3]*8, writebuf[2]*8, writebuf[1]*8, writebuf[0]*8);

    cfg_load(0, readbuf);
    if (!check_output(writebuf, readbuf)) return false;

    cfg_load(1, readbuf);
    mult_arr(writebuf);
    if (!check_output(writebuf, readbuf)) return false;

    cfg_load(2, readbuf);
    mult_arr(writebuf);
    if (!check_output(writebuf, readbuf)) return false;

    //return true;
    // return check_output(writebuf, readbuf);
    // */

    int expected[4];

    // WRITING

    // Sequential write to conv1 memory banks
    // for (int j = 0; j < 4; j++) {  // banks
    for (int j = 0; j < 1; j++) {  // banks
        for (int k = 0; k < 8; k++) {
            cfg_store(k + j*0x10, k+3, k+2, k+1, k+0);
        }
    }
    int k = 7;
    cfg_store(0x40, k+3, k+2, k+1, k+0);  // shift

    /*
    // Sequential write to conv2 memory banks
    for (int j = 5; j < 9; j++) {  // banks
        for (int k = 0; k < 8; k++) {
            cfg_store(k + j*0x10, k+3, k+2, k+1, k+0);
        }
    }
    k = 7;
    cfg_store(0x90, k+3, k+2, k+1, k+0);  // shift

    // Sequential write to FC memory banks
    for (int j = 0x100; j < 0x300; j += 0x100) {
        for (int k = 0; k < 208; k++) {
            cfg_store(j, k+3, k+2, k+1, k+0);
        }
    }
    k = 207;
    cfg_store(0x300, k+3, k+2, k+1, k+0);  // bias
    cfg_store(0x400, k+3, k+2, k+1, k+0);  // bias
    */

    // READING

    // for (int j = 0; j < 4; j++) {  // banks
    for (int j = 0; j < 1; j++) {  // banks
        for (int k = 0; k < 8; k++) {
            // int expected[4] = {k+3, k+2, k+1, k+0};
            int expected[4] = {k, k+1, k+2, k+3}; /// this is correct one
            cfg_load(k + j*0x10, readbuf);
            if (!check_output(expected, readbuf)) return false;
        }
    }
    // int expected[4] = {k+3, k+2, k+1, k+0};
    // k = 7;
    // cfg_load(0x40, k+3, k+2, k+1, k+0);  // shift

    return true;  // didn't fail earlier
    //return false;  // test that returning false actually fails the test bench
}

/* Test the logic analyzer using the waveforms. Hardcode la_data_out to 'h7777777...
 * in user_project_wrapper.
 */
void la_test() {
    // reg_la2_oenb = reg_la2_iena = 0xFFFFFFFF;    // [95:64]
    reg_la0_data = 0xAAAAAAAA;  // setupt output values
    reg_la1_data = 0xAAAAAAAA;
    reg_la2_data = 0xAAAAAAAA;
    reg_la3_data = 0xAAAAAAAA;
    reg_la0_oenb = reg_la0_iena = 0x0;  // OUTPUT
    reg_la1_oenb = reg_la1_iena = 0x0;
    reg_la2_oenb = reg_la2_iena = 0x0;
    reg_la3_oenb = reg_la3_iena = 0x0;

    reg_la1_oenb = reg_la1_iena = 0xFFFFFFFF;  // INPUT
    int read = reg_la1_data;   // read data (h77777777)
    reg_la3_data = read;   // write on reg 3
    reg_la1_oenb = reg_la1_iena = 0x0;  // test switching speed
    reg_la1_oenb = reg_la1_iena = 0xFFFFFFFF;
    reg_la1_oenb = reg_la1_iena = 0x0;

}


// --------------------------------------------------------
// Firmware routines
// --------------------------------------------------------

void set_gpio(int pin)
{
    (volatile uint32_t) ((reg_mprj_datal) |= pin);
}

void clear_gpio(int pin)
{
    (volatile uint32_t) ((reg_mprj_datal) &= ~(pin));
}

void main()
{
	int i, j, k;

	i = 1;

    // reg_mprj_io_37 = GPIO_MODE_MGMT_STD_OUTPUT;
    // reg_mprj_io_36 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    // reg_mprj_io_35 = GPIO_MODE_MGMT_STD_OUTPUT;
    // reg_mprj_io_34 = GPIO_MODE_MGMT_STD_INPUT_PULLUP;
    reg_mprj_io_37 = GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_36 = GPIO_MODE_USER_STD_INPUT_NOPULL;
    reg_mprj_io_35 = GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_34 = GPIO_MODE_USER_STD_INPUT_PULLUP;
    reg_mprj_io_33 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_32 = GPIO_MODE_MGMT_STD_OUTPUT;

    reg_mprj_io_31 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_30 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_29 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_28 = GPIO_MODE_MGMT_STD_OUTPUT;

    reg_mprj_io_27 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_26 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_25 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_24 = GPIO_MODE_MGMT_STD_OUTPUT;

    reg_mprj_io_23 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_22 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_21 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_20 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_19 = GPIO_MODE_MGMT_STD_OUTPUT;
//
//    reg_mprj_io_18 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_17 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_16 = GPIO_MODE_MGMT_STD_OUTPUT;
//
//    reg_mprj_io_15 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_14 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_13 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_12 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_11 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_10 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_9 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_8 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_7 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_6 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_5 = GPIO_MODE_MGMT_STD_OUTPUT;
//
//    reg_mprj_io_4 = GPIO_MODE_USER_STD_INPUT_NOPULL;
//    reg_mprj_io_3 = GPIO_MODE_USER_STD_INPUT_NOPULL;
//    reg_mprj_io_2 = GPIO_MODE_USER_STD_INPUT_NOPULL;   // 0x0403
//    reg_mprj_io_1 = GPIO_MODE_USER_STD_BIDIRECTIONAL;  // 0x1803

    reg_mprj_io_6 = 0x7ff;

    reg_mprj_datal = 0;

    reg_uart_clkdiv = 1042;
    reg_uart_enable = 1;

    reg_mprj_xfer = 1;
    while (reg_mprj_xfer == 1);

	// Enable GPIO (all output, ena = 0)
	reg_gpio_ena = 0x0;
	reg_gpio_pu = 0x0;
	reg_gpio_pd = 0x0;
	reg_gpio_data = 0x1;

    // reg_mprj_io_37 = GPIO_MODE_MGMT_STD_OUTPUT;
    // reg_mprj_io_36 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    // reg_mprj_io_35 = GPIO_MODE_MGMT_STD_OUTPUT;
    // reg_mprj_io_34 = GPIO_MODE_MGMT_STD_INPUT_PULLUP;
    reg_mprj_io_37 = GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_36 = GPIO_MODE_USER_STD_INPUT_NOPULL;
    reg_mprj_io_35 = GPIO_MODE_USER_STD_OUTPUT;
    reg_mprj_io_34 = GPIO_MODE_USER_STD_INPUT_PULLUP;
    reg_mprj_io_33 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_32 = GPIO_MODE_MGMT_STD_OUTPUT;

    reg_mprj_io_31 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_30 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_29 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_28 = GPIO_MODE_MGMT_STD_OUTPUT;

    reg_mprj_io_27 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_26 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_25 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_24 = GPIO_MODE_MGMT_STD_OUTPUT;

    reg_mprj_io_23 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_22 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_21 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_20 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_19 = GPIO_MODE_MGMT_STD_OUTPUT;

//    reg_mprj_io_18 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_17 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_16 = GPIO_MODE_MGMT_STD_OUTPUT;

//    reg_mprj_io_15 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_14 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_13 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_12 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_11 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_10 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_9 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_8 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_7 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_6 = GPIO_MODE_MGMT_STD_OUTPUT;
//    reg_mprj_io_5 = GPIO_MODE_MGMT_STD_OUTPUT;

    reg_mprj_io_4 = GPIO_MODE_USER_STD_INPUT_NOPULL;
    reg_mprj_io_3 = GPIO_MODE_USER_STD_INPUT_NOPULL;
    reg_mprj_io_2 = GPIO_MODE_USER_STD_INPUT_NOPULL;   // 0x0403
    reg_mprj_io_1 = GPIO_MODE_USER_STD_BIDIRECTIONAL;  // 0x1803

//	reg_mprj_datal = 0x00000000;
//	reg_mprj_datah = 0x00000000;

//    spi_init();

//    print("Hello!\n");

    char *msg[] = {
//  "******************* ",
    " Woowzaaa!!         ",
    "                    ",
    " I'm Caravel !!     ",
    "      I'm Alive !!! ",
//  "******************* ",
    };

    int n = 4;

//    putchar("|"); putchar(0x80);
//    putchar("|"); putchar(0x9e);
//    putchar("|"); putchar(0xbc);

    //test wakey_wakey wishbone interface
    bool success = run_test();

	while(1) {
//	    print("|"); putchar(0x2d); // clear screen

        for (i=0; i < 2; i++) {
            reg_gpio_data = 0x0;
            for (j = 0; j < 3000; j++);
            reg_gpio_data = 0x1;
            for (j = 0; j < 5000; j++);
        }

        if (run_test()) {
            print("Pass");
        } else {
            print("fail");
        }


        for (j = 0; j < 10000; j++);

//        for (i=0; i < 2; i++) {
//            reg_gpio_data = 0x0;
//            for (j = 0; j < 3000; j++);
//            reg_gpio_data = 0x1;
//            for (j = 0; j < 5000; j++);
//        }

//        i = 0;
//        while (i < 38) {
//            for (j = i; j < i+4; j++) {
////                print("|"); putchar(0x2d); // clear screen
//                print_dec(j); print(" : 0x"); print_hex(reg_mprj_io_0 + j*4, 4); print("\n");
////                print_dec(j); print(" : 0x"); print_hex(reg_mprj_io_0 + j*4, 4); print("\n");
////                print_dec(j); print(" : 0x"); print_hex(reg_mprj_io_0 + j*4, 4); print("\n");
//                for (k = 0; k < 10000; k++);
//            }
//            i += 4;
//        }
//
//        for (j = 0; j < 30000; j++);

    }

//        reg_mprj_datal = 0x00080000;
//        reg_mprj_datal = 0xffffffff;
//        reg_mprj_datah = 0xffffffff;
//        set_gpio(19);
//        reg_gpio_data = 0x0;
//
//        for (j = 0; j < 3000; j++);
//
////       	reg_mprj_datal = 0x00000000;
////       	reg_mprj_datah = 0x00000000;
////        clear_gpio(19);
//        reg_gpio_data = 0x1;
//
//        for (j = 0; j < 3000; j++);

//        putchar('x');

//	}
}

